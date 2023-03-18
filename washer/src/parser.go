/**
 * Parser
 * parser.go
 *
 * Created September 2021
 *
 * The parser parses the message received via the wsdl_interface as a WSDL message and extracts a power command from it to be issued to scheduler
 *
 */

package main

import (
	"bytes"
	"compress/gzip"
	"encoding/base64"
	"encoding/xml"
	"fmt"
	"html"
	"io/ioutil"
	"log"
	"strconv"
	"strings"
	"time"
)

// SOAP envelope for xml parsing
type SOAPEnvelope struct {
	XMLName xml.Name
	Header  []byte       `xml:"http://schemas.xmlsoap.org/soap/envelope/ Header"`
	Body    RecursiveXML `xml:"http://schemas.xmlsoap.org/soap/envelope/ Body"`
	// Fault interface{}
}

// Generic XML element
type RecursiveXML struct {
	XMLName    xml.Name
	Attributes []xml.Attr     `xml:",any,attr"` // XML attributes, i.e. <elem attr=10></elem>
	Elements   []RecursiveXML `xml:",any"`      // XML subelements, i.e. <elem><subelem>...</subelem></elem>
	Contents   string         `xml:",chardata"` // non-element contents, i.e. <elem>contents</elem>
}

type BatchType struct {
	Id         string // ID/name of the batch type e.g. 5_min_dispatchable
	BatchValue int    // Value associated with the ID
}

type ParsingVariable struct {
	Id       string      // ID of this variable, which goes out over fims as part of the larger object
	ParseUri string      // URI indicating the path to the variable within the XML structure
	DataType string      // Datatype type of data stored in the XML structure. Currently int, float, string, and time supported
	Value    interface{} // Value published over fims. If configured, this value replaces the value that would be parsed from the URI
}

type ParsingRule struct {
	FimsUri    string            // URI on which to set the object parsed
	BatchTypes []BatchType       // List of batch types for which this parsing rule is valid
	Variables  []ParsingVariable // List of variables to parse in the received XML
}

// Object extracted by the parser (dispatch batch) to be sent over fims
type FimsObj struct {
	Id        string                 // id (batchUID for CAISO) of the object
	StartTime time.Time              // Start time of the object
	Data      map[string]interface{} //	Any additional parsed variables, typically including the power command
}

// Rules for parsing the XML
var parsingRules []ParsingRule

// Get the first subelement whose name matches the given local name
// If the optional attributes are provided by the parseURI, ensure each attribute is matched in the element as well
func (elem *RecursiveXML) getFirstElement(localName string, parsedAttributes []string) (*RecursiveXML, error) {
	for _, subelem := range elem.Elements {
		// Match name provided
		if subelem.XMLName.Local == localName {
			// Match attributes if present
			if len(parsedAttributes) > 0 && len(subelem.Attributes) > 0 {
				matchedAllAttributes, err := matchXMLAttributes(parsedAttributes, subelem.Attributes)
				if err != nil {
					return nil, err
				}
				if !matchedAllAttributes {
					// Failed to match, try the next subelement
					continue
				}
			}
			// Either no attributes or all attributes matched successfully
			return &subelem, nil
		}
	}
	return nil, fmt.Errorf("no such XML element found: %s", localName)
}

// Recursively getFirstElement for the given URI fragments
// Returns the value pointed to by the URI in string form, to be parsed appropriately elsewhere based on type
func parseRecursively(uri string, elem *RecursiveXML) (returnElem *RecursiveXML, err error) {
	frags := strings.Split(uri, "/")
	// Iterate through the fragments of the uri, skipping the first "/"
	// This function is always called on the first element in the URI, so check it's name first
	if elem.XMLName.Local != frags[1] {
		return nil, fmt.Errorf("failed to match first element %s of %s", frags[1], uri)
	}
	for _, frag := range frags[2:] {
		attrs := strings.Split(frag, " ")
		// First attribute of split will always be the string itself if no match
		elem, err = elem.getFirstElement(attrs[0], attrs[1:])
		if err != nil {
			return nil, fmt.Errorf("failed to parse element %s of %s: %w", frag, uri, err)
		}
	}
	return elem, nil
}

// Decompress the ISO's gzipped response and unmarshal to RecursiveXML
func parseGzip(elem *RecursiveXML) (*RecursiveXML, error) {
	// Passed the "result" by parameter, which is the gzipped archive, decompress and read
	decoded, err := base64.StdEncoding.DecodeString(elem.Contents)
	if err != nil {
		return nil, err
	}
	gzreader, err := gzip.NewReader(bytes.NewReader(decoded))
	if err != nil {
		return nil, err
	}
	defer gzreader.Close()
	unzipped, err := ioutil.ReadAll(gzreader)
	if err != nil {
		return nil, err
	}
	// Unmarshal the decompressed gzip to our RecursiveXML
	err = xml.Unmarshal(unzipped, elem)
	return elem, err
}

// Parse data from a SOAP envelope based on the parsing rules configured
// This function handles two queries: getDispatchBatchesSinceUID, and getDispatchBatch
// getDispatchBatchesSinceUID returns a list of dispatch batches, and this list is parsed for each entry's batch type and UID
// getDispatchBatch returns a list of instructions for a UID, which is then parsed for variables using the the configured parsing rules
// For DispatchBatchesSinceUIDResponses, this function will parse and return a list of batchUIDs for which to query further instructions from the ISO
// For DistpatchBatchResponses, this function will parse and return a map of fims URIs and the extracted data associated with these URIs (FimsObjs)
// TODO: Given a sequence file, this function can be made more generic with less hard-coded logic
func parseSOAPEnvelope(rawMsg []byte) (batchList []string, fimsObjs map[string]FimsObj, responseType string, err error) {
	// Part of the message comes through as unescaped html, e.g. ascii code 60 for the character '<' comes through as html code '&lt;'
	// unescape for consistent parsing of bytes
	// TODO: we could replace bytes manually to avoid the conversion from byte to string back to byte
	unescapedMsg := html.UnescapeString(string(rawMsg))

	// Try to parse out a soap envelope
	var soap SOAPEnvelope
	err = xml.Unmarshal([]byte(unescapedMsg), &soap)
	if err != nil {
		return nil, nil, responseType, fmt.Errorf("failed to unmarshal to soap: %w", err)
	}

	// Recursive XML struct used to parse through the soap envelope
	var elem *RecursiveXML
	// Get the element name which indicates the type of response from the ISO
	responseType = soap.Body.Elements[0].XMLName.Local
	// Parse down to the <result> tag
	// TODO: should always be the same, but make this configurable as part of the handshake sequence
	elem, err = soap.Body.Elements[0].getFirstElement("result", nil)
	if err != nil {
		return nil, nil, responseType, fmt.Errorf("failed to parse <result> element from response: %w", err)
	}
	switch responseType {
	case "getDispatchBatchesSinceUIDResponse":
		// Parse out the list (potentially multiple) of dispatch batches
		parsedBatchList, err := parseRecursively("/result/APIDispatchResponse/dispatchBatchList", elem)
		if err != nil {
			return nil, nil, responseType, err
		}
		for _, parsedBatch := range parsedBatchList.Elements {
			// Ensure fresh data that matches our batch type
			batchUID, err := isSupportedDispatchBatch(parsingRules, parsedBatch)
			if err != nil {
				// Errors are not critical here, just report and move on to the next batch
				log.Println(err)
				continue
			}

			// If there were no errors but the batch type is nil then the data is stale, so continue to the next batch
			if batchUID == nil {
				continue
			}

			// Add the batchUID to the list of batches to query for more instructions
			batchList = append(batchList, batchUID.Value)
		}
	case "getDispatchBatchResponse":
		fimsObjs = make(map[string]FimsObj)
		// Parse out the compressed result and decode it
		elem, err := parseGzip(elem)
		if err != nil {
			return nil, nil, responseType, fmt.Errorf("failed to parse gzip archive: %w", err)
		}
		// Objects that will be published (one at a time) over fims
		// Try parsing with each of the parsing rules configured
		for _, parsingRule := range parsingRules {
			if parsingRule.FimsUri == "" {
				return nil, nil, responseType, fmt.Errorf("fims uri missing from parsing rule")
			}
			// Determine if the current parsing rule applies to the batch type received
			supportedBatchType, err := isSupportedBatchType(parsingRule, elem)
			if err != nil {
				return nil, nil, responseType, err
			}
			if supportedBatchType {
				// The current object being parsed as a collection of each variable defined by the parsing rules
				var parsedObj FimsObj
				parsedObj.Data = make(map[string]interface{})
				// Map of all the parsed variables that will go out over fims (e.g. startTime, duration, powerCmd)
				for _, variable := range parsingRule.Variables {
					// Assign map variable to parsed variable
					// If variable has default value provided by configuration, use it instead of parsing
					// TOOD: some way to allow configuring a default value of 0 for optional variable (issue: unmarshals to 0 if not provided)
					if variable.Value == nil {
						// Default value not set, parse for the value in the document based on the URI configured
						parsedElem, err := parseRecursively(variable.ParseUri, elem)
						if err != nil {
							return nil, nil, responseType, err
						}
						// Assign the string extracted from XML element using the appropriate type based on configuration
						err = variable.setValueFromType(parsedElem.Contents)
						if err != nil {
							return nil, nil, responseType, fmt.Errorf("failed to parse extracted variable to configured type: %w", err)
						}
					}
					switch variable.Id {
					// First check the supported hardcoded fields
					case "Id":
						parsedObj.Id = variable.Value.(string)
					case "StartTime":
						parsedObj.StartTime = variable.Value.(time.Time)
					default:
						parsedObj.Data[variable.Id] = variable.Value
					}
				}
				// Ensure the object is not empty
				if parsedObj.Id == "" || parsedObj.StartTime.IsZero() || parsedObj.Data == nil {
					return nil, nil, responseType, fmt.Errorf("expected to parse an object for uri: %s but got nil", parsingRule.FimsUri)
				}
				// Add the parsed object to the list of objects to be sent out over fims
				fimsObjs[parsingRule.FimsUri] = parsedObj
			}
		}
	default:
		return nil, nil, responseType, fmt.Errorf("invalid ISO response type received")
	}

	return batchList, fimsObjs, responseType, nil
}

// Determine if the provided parsing rule applies to the batch type received
func isSupportedBatchType(rule ParsingRule, elem *RecursiveXML) (supportedBatchType bool, err error) {
	// Parse out the batchType
	supportedBatchType = false
	parsedBatchType, err := parseRecursively("/DispatchBatch/batchType", elem)
	if err != nil {
		return false, err
	}
	parsedBatchValue, err := strconv.Atoi(parsedBatchType.Contents)
	if err != nil {
		return false, fmt.Errorf("failed to parse batch type as int: %w", err)
	}
	// Check each of the batchTypes associated with the current parsing rule for a match
	for _, batchType := range rule.BatchTypes {
		if batchType.BatchValue == parsedBatchValue {
			supportedBatchType = true
		}
	}
	return supportedBatchType, nil
}

// Determine if the parsed dispatch batch should be supported based on its batch type and start time
// Supported batches will be those with a supported batchType based on available parsing rules and a startTime that occurs
// at least one minute from now
// All errors raised by this function will be reported by the logger, but will not cause early termination of the batch parsing
func isSupportedDispatchBatch(rules []ParsingRule, parsedBatch RecursiveXML) (batchUID *xml.Attr, err error) {
	// Check if the batch starts within the configured thresh data period
	parsedStartTimeString, err := parseRecursively("/DispatchBatch/startTime", &parsedBatch)
	if err != nil {
		return nil, err
	}
	parsedStartTime, err := time.Parse(time.RFC3339, parsedStartTimeString.Contents)
	if err != nil {
		return nil, err
	}
	// TODO: less than 0, configurable, what should threshold be?
	// Give at least 1 minute of advance or the batch is considered stale data
	if time.Until(parsedStartTime) <= time.Minute {
		return nil, nil
	}

	// Check for batchUID
	if len(parsedBatch.Attributes) == 0 {
		return nil, fmt.Errorf("failed to parse batchUID")
	}
	batchUID = &parsedBatch.Attributes[0]
	if batchUID == nil || batchUID.Value == "" {
		return nil, fmt.Errorf("could not parse batchUID from parsedBatch attributes: %+v", parsedBatch.Attributes)
	}
	// Check for valid batch type
	batchType, err := parsedBatch.getFirstElement("batchType", nil)
	if err != nil {
		return batchUID, err
	}
	receivedBatchValue, err := strconv.Atoi(batchType.Contents)
	if err != nil {
		return batchUID, fmt.Errorf("failed to parse batch type as int: %w", err)
	}
	// Ensure batch type matches
	for _, parsingRule := range parsingRules {
		for _, parsingBatchType := range parsingRule.BatchTypes {
			if parsingBatchType.BatchValue == receivedBatchValue {
				return batchUID, nil
			}
		}
	}
	return batchUID, fmt.Errorf("failed to find a supported parsing rule for batchUID: %s, batchType %d", batchUID.Value, receivedBatchValue)
}

// Sets the parsingVariable's value based on the data type configured
// Currently supported types are string, int, float, time
// If the data type has not been properly configured or is not supported, defaults to string
func (variable *ParsingVariable) setValueFromType(data string) (err error) {
	if variable.DataType == "int" {
		// ParseInt returns int64 which can cause issues, use generic Atoi
		variable.Value, err = strconv.Atoi(data)
	} else if variable.DataType == "float" {
		// Assumes float32
		variable.Value, err = strconv.ParseFloat(data, 32)
	} else if variable.DataType == "time" {
		// Formats timestamp based on reference 'layout' time (ISO 8601/RFC3339 standard)
		variable.Value, err = time.Parse(time.RFC3339, data)
	} else {
		// Default string
		variable.Value = data
	}
	return err
}

// Match the provided parseURI attributes to the RecursiveXML Element's attributes
func matchXMLAttributes(parseAttributes []string, xmlAttributes []xml.Attr) (matchedAttributes bool, err error) {
	for _, parsingAttribute := range parseAttributes {
		// Determine whether the individual attribute was found
		matchedAttributes := false
		// Additional parsing to match the unmarshalled XML format
		parsingAttrNameVal, err := extractAttributeNameVal(parsingAttribute)
		if err != nil {
			return false, fmt.Errorf("getFirstElement failed to extract name/value pair from attribute: %+v, %w", parseAttributes, err)
		}
		// Check the current attribute against all of the attributes in the XML element currently being parsed
		for _, xmlAttr := range xmlAttributes {
			// Found a single common attribute
			if parsingAttrNameVal[0] == xmlAttr.Name.Local && parsingAttrNameVal[1] == xmlAttr.Value {
				matchedAttributes = true
				// Check for next attribute in our parseAttributes list
				break
			}
		}
		// Failed to find one of our attributes, fail
		if !matchedAttributes {
			return matchedAttributes, nil
		}
	}
	// All attributes matched
	return true, nil
}

// Parse out the xml attribute's name/value pair from a single string
func extractAttributeNameVal(attribute string) (attributeNameVal []string, err error) {
	// Parse the name/value pair from the provided attribute
	attributeNameVal = strings.Split(attribute, "=")
	if len(attributeNameVal) != 2 {
		return nil, fmt.Errorf("invalid number of arguments")
	}
	// Quotes are parsed out when unmarshalling, remove them
	attributeNameVal[1] = strings.Replace(attributeNameVal[1], "\"", "", -1)
	return
}
