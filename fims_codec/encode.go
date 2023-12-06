package fims_codec

import (
	"archive/tar"
	"bytes"
	"compress/gzip"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"time"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

type Encoder struct {
	// The URI of FIMS messages that will be encoded by this encoder.
	Uri string

	// An ordered list of metadata (key string, data type, enumerations if applicable) on each key found in the FIMS messages.
	// Is ordered so that the data is encoded in the same order every time.
	KeyMetadatas []*keyMetadata

	// Additional data that needs to be embedded such as destination, measurement, database, etc.
	AdditionalData map[string]string

	// Holds the encoded data sourced from each FIMS message added to the encoder
	data *bytes.Buffer

	// Number of FIMS messages that have been added to the decoder
	numMessages uint16
}

// Holds data type and (if applicable) enumerations for a given key.
type keyMetadata struct {
	key         string
	dataType    string
	stringEnums stringEncoder
	mapEnums    mapEncoder
}

// As an encoder is built from FIMS messages, any keys that hold strings get a set of string enumerations.
// Each unique string will generate a new enumeration byte, starting from 1 and counting up. A byte of 0x00
// signals a missing value.
//
// Consider a string value that, over the course of many FIMS messages, is recorded as: "Alarm On", "Alarm On", "Alarm Off",
// "Alarm On", "Alarm On", "Alarm Off". The encoder that was built by these messages will have string enumerations: 0x00 for missing
// value, 0x01 for "Alarm On", and 0x02 for "Alarm Off". The enumeration order follows the order in which the values first
// appear. The encoded list of values to represent what the value of this string over time would then be: 0x01, 0x01, 0x02, 0x01,
// 0x01, 0x02.
type stringEncoder map[string]byte

// As an encoder is built from FIMS messages, any keys that hold slices of maps get a set of enumerations for the maps.
// Each unique map will generate a new enumeration byte, starting from 1 and counting up. A byte of 0x00 signals a
// missing value. Ideally this would be a map[map[string]interface{}]byte but that is not a possible data type.
type mapEncoder map[byte]map[string]interface{}

// Characters which are not allowed to be in strings because they would interfere with control characters in the encoding
const illegalChars = ";\t\n"

// Used to mark an invalid string
const invalidStringMarker = "[CODEC MARKED INVALID STRING]"

// Instantiates a new encoder with allocated memory and the given URI.
func NewEncoder(uri string) *Encoder {
	return &Encoder{
		Uri:            uri,
		KeyMetadatas:   make([]*keyMetadata, 0),
		AdditionalData: make(map[string]string),
		data:           new(bytes.Buffer),
	}
}

// Creates a new encoder with the same URI and 'additional data'
// as the given existing encoder.
func CopyEncoder(sourceEncoder *Encoder) (newEncoder *Encoder) {
	newEncoder = NewEncoder(sourceEncoder.Uri)
	for k, v := range sourceEncoder.AdditionalData {
		newEncoder.AdditionalData[k] = v
	}
	return newEncoder
}

// Encodes the given FIMS message body into the encoder buffer.
// May log errors and continue processing if the error doesn't indicate a fully invalid msg.
func (encoder *Encoder) Encode(bodyMap map[string]interface{}) error {
	if len(bodyMap) == 0 {
		return fmt.Errorf("body map is empty")
	}
	// start the encoding with a timestamp of when this message was received
	encoder.addDataIngestionTimestamp()
	if encoder.numMessages == 0 { // the first message encoded will tell the encoder which keys to look for in following messages
		for key, val := range bodyMap {
			// timestamp key is expected from modbus_client publishes and we do not care about this key. skip for efficiency
			if strings.ToLower(key) == "timestamp" {
				continue
			}
			err := encoder.addDataToNewEncoder(key, val)
			if err != nil {
				log.Errorf("Failed to add data under key %s to encoder of URI %s: %v", key, encoder.Uri, err)
				continue
			}
		}
	} else {
		// metadata already exists so iterate through metadata and parse expected values from FIMS msg body
		for _, meta := range encoder.KeyMetadatas {
			// parse the expected key from the FIMS msg body
			data, exist := bodyMap[meta.key]
			if !exist {
				log.Warnf("Key %s not found in FIMS message with URI %s. Filling encoder with missing data marker.", meta.key, encoder.Uri)
				encoder.appendMissingDataMarker(meta.dataType)
				continue
			}

			// add the parsed data to the data record
			err := encoder.addDataToExistingEncoder(meta, data)
			if err != nil {
				log.Errorf("Failed to add data under key %s to existing encoder of URI %s: %v", meta.key, encoder.Uri, err)
				continue
			}
		}
	}
	encoder.numMessages++
	return nil
}

// Returns the number of messages encoded so far
func (encoder *Encoder) GetNumMessages() uint16 {
	return encoder.numMessages
}

// Creates a single .tar.gz archive file from the data held by the encoder.
// Includes one metadata file and one encoded data file.
// Returns file path of created .tar.gz archive along with how many bytes
// were compressed for performance benchmarking purposes.
func (encoder *Encoder) CreateArchive(destinationDirPath string, archivePrefix string) (archiveFilePath string, numUncompressedBytes int, err error) {
	// construct the full path of the archive to be produced.
	// archive name will be <prefix (i.e. database_measurement)>-<URI with slashes replaced with dashes>-<timestamp in epoch>
	creationTime := time.Now()
	creationEpoch := uint64(creationTime.UnixMicro())
	archiveName := fmt.Sprintf("%s-%s-%d.tar.gz", archivePrefix, DashifyUri(encoder.Uri), creationEpoch)
	archiveFilePath = filepath.Join(destinationDirPath, archiveName)

	// instantiate the archive .tar.gz file
	archiveFile, err := os.Create(archiveFilePath)
	if err != nil {
		return "", 0, fmt.Errorf("failed to create tar file: %w", err)
	}
	defer archiveFile.Close()

	// create the .tar.gz file writer
	gzipWriter := gzip.NewWriter(archiveFile)
	defer gzipWriter.Close()
	tarWriter := tar.NewWriter(gzipWriter)
	defer tarWriter.Close()

	// create metadata from encoder
	metadataBuffer, err := encoder.createMetadataBuffer()
	if err != nil {
		return "", 0, fmt.Errorf("failed to create metadata: %w", err)
	}
	sizeOfMetadata := metadataBuffer.Len()

	// write header for metadata
	err = tarWriter.WriteHeader(&tar.Header{
		Name:    "metadata.txt",
		Size:    int64(sizeOfMetadata),
		Mode:    int64(os.ModePerm), // full permissions
		ModTime: creationTime,
	})
	if err != nil {
		return "", 0, fmt.Errorf("failed to write header for metadata: %w", err)
	}

	// write metadata to tarfile
	_, err = tarWriter.Write(metadataBuffer.Bytes())
	if err != nil {
		return "", 0, fmt.Errorf("failed to write metadata to file: %w", err)
	}

	// write header for encoded data
	sizeOfEncodedData := encoder.data.Len() + 2 // +2 for the uint16 representing the number of messages
	err = tarWriter.WriteHeader(&tar.Header{
		Name:    fmt.Sprintf("%d", creationEpoch),
		Size:    int64(sizeOfEncodedData),
		Mode:    int64(os.ModePerm), // full permissions
		ModTime: creationTime,
	})
	if err != nil {
		return "", 0, fmt.Errorf("failed to write header for encoded data: %w", err)
	}

	// write first two bytes, representing number of messages encoded
	err = binary.Write(tarWriter, binary.BigEndian, encoder.numMessages)
	if err != nil {
		return "", 0, fmt.Errorf("failed to write number of messages to file: %w", err)
	}

	// write encoded messages to tarfile
	_, err = tarWriter.Write(encoder.data.Bytes())
	if err != nil {
		return "", 0, fmt.Errorf("failed to write encoded messages to file: %w", err)
	}
	tarWriter.Flush()
	return archiveFilePath, sizeOfMetadata + sizeOfEncodedData, nil
}

// Builds a string from the Additional Data map with the format:
// <key1>:<value1>\n<key2>:<value2>\n...
// Returns an error if the Additional Data map is empty.
func (encoder *Encoder) StringifyAdditionalData() (stringifiedData string, err error) {
	if len(encoder.AdditionalData) == 0 {
		return "", errors.New("additional data map is empty")
	}
	for key, data := range encoder.AdditionalData {
		stringifiedData += fmt.Sprintf("%s:%s\n", key, data)
	}
	return stringifiedData, err
}

// Takes the current UNIX timestamp in microseconds and adds the byte-representation
// to the data buffer. Used to keep track of when each FIMS message body was received
// and added to the encoder.
func (encoder *Encoder) addDataIngestionTimestamp() {
	timestampBuffer := make([]byte, 8)
	binary.BigEndian.PutUint64(timestampBuffer, uint64(time.Now().UnixMicro()))
	encoder.data.Write(timestampBuffer)
}

// Reads in a single key-value pair and adds it to the encoder. Encodes the data based on its data type
// and returns an error if the data type is not recognized or in an invalid format.
func (encoder *Encoder) addDataToNewEncoder(key string, valueInterface interface{}) error {
	// value must be non-null
	if valueInterface == nil {
		return fmt.Errorf("value is null")
	}

	// when a map is found, it is expected to be a wrapper around an internal value.
	// internal value is expected to be found under a "value" key in the map then processed as the actual value
	if valueWrapping, ok := valueInterface.(map[string]interface{}); ok {
		// if value is a UI control, ignore it
		if isUiControl, err := mapHoldsUiControl(valueWrapping); err != nil {
			return fmt.Errorf("data is map[string]interface{} but failed to determine if it holds UI control or not: %w", err)
		} else if isUiControl {
			return nil
		}
		valueInterface, ok = valueWrapping["value"]
		if !ok {
			return fmt.Errorf("data is map[string]interface{} but it does not contain 'value'")
		}
	}

	// any numeric data gets processed as a float
	valueFloat, successfulCastToFloat := castToFloat(valueInterface)
	if successfulCastToFloat {
		valueInterface = valueFloat
	}

	// create a new metadata tracker for this key-value pair
	meta := &keyMetadata{
		key:      key,
		dataType: reflect.TypeOf(valueInterface).String(),
	}

	// each data type is encoded in its own unique way
	switch typedValue := valueInterface.(type) {
	case float64: // numerical data does not get any special encoding other than the earlier cast to float64
		binary.Write(encoder.data, binary.BigEndian, typedValue) // returned error will always be nil
	case string: // each unique string value found gets its own enumeration which is only a single byte. enumeration translations are held in the metadata file and the enumeration bytes are added to the data files
		// allocate memory for the string encoder for this key and add the first enumeration
		meta.stringEnums = make(stringEncoder)
		enumByte, err := meta.stringEnums.getOrUpdateStringEnum(typedValue)
		if err != nil {
			enumByte = meta.stringEnums.getOrUpdateInvalidStringEnum()
			log.Errorf("Failed to get or update string enum with %s at key %s for encoder of URI %s. Adding invalid data marker: %v.", typedValue, meta.key, encoder.Uri, err)
		}
		encoder.data.WriteByte(enumByte)
	case bool: // boolean can be represented as byte: 0 for false and 1 for true
		if typedValue {
			encoder.data.WriteByte(1)
		} else {
			encoder.data.WriteByte(0)
		}
	case []interface{}: // when a slice is found, it is expected to be a slice of maps. the encoding for a slice of maps is [N E1 E2 E3...E#] where N is the number of maps and E# is the enumeration for each map in the slice
		mapSlice := castToMapSlice(typedValue)
		if mapSlice == nil {
			return fmt.Errorf("data is array, but not array of maps, and arrays of maps are the only kind of arrays allowed. data read is: %v", typedValue)
		}
		// []map[string] interface{} will have meta initially set to []interface {} so remedy that after the type cast
		meta.dataType = reflect.TypeOf(mapSlice).String()
		// instantiate a slice to hold map enumerations prior to adding them to the encoding. necessary because any maps that contain illegal characters cannot be added, but encoding must start with the number of maps in the slice
		mapEnumerations := make([]byte, 0, len(mapSlice))
		// allocate memory for the map encoder for this key and add the first enumerations
		meta.mapEnums = make(mapEncoder)
		for _, mapInSlice := range mapSlice {
			enumByte, err := meta.mapEnums.getOrUpdateMapEnum(mapInSlice)
			if err != nil {
				enumByte = meta.mapEnums.getOrUpdateInvalidMapEnum(mapInSlice)
				log.Errorf("Failed to get or update map enum for %+v in slice of maps at key %s for encoder of URI %s. Adding invalid data marker: %v.", mapInSlice, meta.key, encoder.Uri, err)
			}
			mapEnumerations = append(mapEnumerations, enumByte)
		}
		// begin encoded map slice with # of maps in slice
		encoder.data.WriteByte(byte(len(mapEnumerations)))
		// enter all the validated enumerations
		encoder.data.Write(mapEnumerations)
	default:
		return fmt.Errorf("data type is %T which is unaccounted for", valueInterface)
	}

	// add the validated new key and metadata to the encoder
	encoder.KeyMetadatas = append(encoder.KeyMetadatas, meta)
	return nil
}

// Adds the data to the encoder with the expectation that it matches the data type set forth
// by the very first message added to this encoder. When there are problems such as data type
// mismatch, a "missing data marker" (implementation of this varies by data type) is added
// so that the structure of the encoded bytes is maintained but the decoder will know to ignore
// this key for the message that it was invalid/missing.
func (encoder *Encoder) addDataToExistingEncoder(meta *keyMetadata, data interface{}) error {
	// if type is map[string]interface{}, then expect value to be found in 'value' field
	if valueWrapping, ok := data.(map[string]interface{}); ok {
		data, ok = valueWrapping["value"]
		if !ok {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s appeared as map[string]interface{}, but field 'value' not found. Adding missing data marker", meta.key)
		}
	}

	// encode the data based on data type
	switch meta.dataType {
	case "float64":
		valueFloat, successfulCast := castToFloat(data)
		if !successfulCast {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s is expecting float64-castable data types but got %T instead. Adding missing data marker", meta.key, data)
		}
		binary.Write(encoder.data, binary.BigEndian, valueFloat) // returned error is always nil for binary.Write when bytes.Buffer is passed to it
	case "string":
		valueString, ok := data.(string)
		if !ok {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s is expecting string data type but got %T instead. Adding missing data marker", meta.key, data)
		}
		enumByte, err := meta.stringEnums.getOrUpdateStringEnum(valueString)
		if err != nil {
			enumByte = meta.stringEnums.getOrUpdateInvalidStringEnum()
			log.Errorf("Failed to get or update string enum with %s at key %s for encoder of URI %s. Adding invalid data marker: %v.", valueString, meta.key, encoder.Uri, err)
		}
		encoder.data.WriteByte(enumByte)
	case "bool":
		valueBool, ok := data.(bool)
		if !ok {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s is expecting bool data type but got %T instead. Adding missing data marker", meta.key, data)
		}
		binary.Write(encoder.data, binary.BigEndian, valueBool) // returned error is always nil for binary.Write when bytes.Buffer is passed to it
	case sliceOfMapsDataType:
		interfaceSlice, ok := data.([]interface{})
		if !ok {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s is expecting []interface{} data type but got %T instead. Adding missing data marker", meta.key, data)
		}
		// if no maps in slice, add missing data marker
		if len(interfaceSlice) == 0 {
			encoder.appendMissingDataMarker(meta.dataType)
			return nil
		}
		// verify all elements of slice are maps
		mapSlice := castToMapSlice(interfaceSlice)
		if mapSlice == nil {
			encoder.appendMissingDataMarker(meta.dataType)
			return fmt.Errorf("key %s is expecting []map[string]interface{} but at least one element in array is not a map[string]interface{}. Adding missing data marker. Received array: %v", meta.key, interfaceSlice)
		}
		// instantiate a slice to hold map enumerations prior to adding them to the encoding. necessary because any maps that contain illegal characters cannot be added, but encoding must start with the number of maps in the slice
		mapEnumerations := make([]byte, 0, len(mapSlice))
		// add one enumeration per map
		for _, valueMap := range mapSlice {
			enumByte, err := meta.mapEnums.getOrUpdateMapEnum(valueMap)
			if err != nil {
				enumByte = meta.mapEnums.getOrUpdateInvalidMapEnum(valueMap)
				log.Errorf("Failed to get or update map enum for %+v in slice of maps at key %s for encoder of URI %s. Adding invalid data marker: %v.", valueMap, meta.key, encoder.Uri, err)
			}
			mapEnumerations = append(mapEnumerations, enumByte)
		}
		// begin encoded map slice with # of maps in slice
		encoder.data.WriteByte(byte(len(mapEnumerations)))
		// enter all the validated enumerations
		encoder.data.Write(mapEnumerations)
	default:
		return fmt.Errorf("key %s has invalid data type %s", meta.key, meta.dataType)
	}

	return nil
}

// Appends the bytes that represent a missing data marker for the key's data type to the given buffer.
func (encoder *Encoder) appendMissingDataMarker(dataType string) {
	switch dataType {
	case "string":
		encoder.data.WriteByte(0)
	case "bool":
		encoder.data.WriteByte(missingBoolMarker)
	case sliceOfMapsDataType:
		encoder.data.WriteByte(0)
	default: // float64
		binary.Write(encoder.data, binary.BigEndian, missingFloatMarker) // returned error is always nil for binary.Write when bytes.Buffer is passed to it
	}
}

// Stringifies all metadata and loads it into a bytes buffer to be ready to be loaded into the metadata.txt file.
func (encoder *Encoder) createMetadataBuffer() (metadataBuffer *bytes.Buffer, err error) {
	metadataBuffer = new(bytes.Buffer)
	// first line of metadata file is fims uri
	metadataBuffer.WriteString(encoder.Uri + "\n")

	// add any embedded additional data in this section
	stringifiedAdditionalData, err := encoder.StringifyAdditionalData()
	if err != nil {
		return nil, fmt.Errorf("failed to stringify additional data map: %w", err)
	}
	// stringified additional data already includes new-line character at the end of it,
	// but include another to split additional data from key data with empty line for the
	// benefit of the decoder.
	metadataBuffer.WriteString(fmt.Sprintf("%s\n", stringifiedAdditionalData))

	// add the metadata for each key: key string, data type, and enumerations for strings and map slices
	for _, meta := range encoder.KeyMetadatas {
		metadataBuffer.WriteString(fmt.Sprintf("%s\t%s\n", meta.key, meta.dataType))
		switch meta.dataType {
		case "string":
			metadataBuffer.WriteString(meta.stringEnums.stringifyStringEnums())
		case sliceOfMapsDataType:
			stringifiedOrderedMapEnums, err := meta.mapEnums.stringifyMapEnums()
			if err != nil {
				return nil, fmt.Errorf("failed to stringify maps enumerations for key %s: %w", meta.key, err)
			}
			metadataBuffer.WriteString(stringifiedOrderedMapEnums)
		}
	}

	// pop off the last new-line character before returning to prevent an empty line from sitting at end of metadata.
	// would be negligible but removing it also makes it simpler to decode
	metadataBuffer.Truncate(len(metadataBuffer.Bytes()) - 1)
	return metadataBuffer, nil
}

// Searches the string enumerations map for an enumeration for the given string.
// If one does not exist, creates it. If an error occurs, then the enums won't be updated.
func (stringEnums stringEncoder) getOrUpdateStringEnum(valueString string) (byte, error) {
	enumeration, alreadyExists := stringEnums[valueString]
	if alreadyExists {
		return enumeration, nil
	}
	if strings.ContainsAny(valueString, illegalChars) {
		return 0x00, fmt.Errorf("data has a string %s which contains illegal characters", valueString)
	}
	enumeration = byte(len(stringEnums) + 1)
	stringEnums[valueString] = enumeration
	return enumeration, nil
}

// Adds an enum for an invalid string marker or gets the enum if it already exists.
// Returns the enumerated byte.
func (stringEnums stringEncoder) getOrUpdateInvalidStringEnum() byte {
	// Updating with the invalid marker should never give an error
	enumeration, _ := stringEnums.getOrUpdateStringEnum(invalidStringMarker)
	return enumeration
}

// Orders all enumerated strings in order of their enumerations with new-line characters
// separating them and terminating the concatenated string. Used for stringifying metadata.
func (stringEnums stringEncoder) stringifyStringEnums() (stringifiedStringEnums string) {
	stringsInOrderOfEnumeration := make([]string, len(stringEnums))
	for valueString, stringEnumeration := range stringEnums {
		// enumerations start at 1 so subtract 1 to get slice index
		stringsInOrderOfEnumeration[stringEnumeration-1] = valueString
	}
	return strings.Join(stringsInOrderOfEnumeration, "\n") + "\n"
}

// Searches the map enumerations map for an enumeration for the given map.
// If one does not exist, creates it. If an error occurs, then the enums won't be updated.
func (mapEnums mapEncoder) getOrUpdateMapEnum(valueMap map[string]interface{}) (byte, error) {
	for enumeration, existingMap := range mapEnums {
		if reflect.DeepEqual(valueMap, existingMap) {
			return enumeration, nil
		}
	}
	for key, val := range valueMap {
		if strings.ContainsAny(key, illegalChars) {
			return 0x00, fmt.Errorf("data has a map slice with a key %s which contains illegal characters", key)
		}
		if stringVal, ok := val.(string); ok && strings.ContainsAny(stringVal, illegalChars) {
			return 0x00, fmt.Errorf("data has a map slice element with a string %s which contains illegal characters", key)
		}
	}
	newEnumeration := byte(len(mapEnums) + 1)
	mapEnums[newEnumeration] = valueMap
	return newEnumeration, nil
}

// Adds an enum for an invalid map marker corresponding to the given map or gets the enum if it already exists.
// Returns the enumerated byte.
func (mapEnums mapEncoder) getOrUpdateInvalidMapEnum(invalidMap map[string]interface{}) byte {
	invalidMapMarker := make(map[string]interface{})
	for key, val := range invalidMap {
		validVal := val
		if stringVal, ok := val.(string); ok && strings.ContainsAny(stringVal, illegalChars) {
			validVal = invalidStringMarker
		}
		invalidMapMarker[key] = validVal
	}
	// Updating with the invalid marker should never give an error
	enumeration, _ := mapEnums.getOrUpdateMapEnum(invalidMapMarker)
	return enumeration
}

// Stringifies all maps with a format the decoder will be able to parse.
// Orders all stringified maps in order of their enumerations with new-line
// characters separating them and terminating the concatentated string.
// Used for stringifying metadata.
func (mapEnums mapEncoder) stringifyMapEnums() (stringifiedMapEnums string, err error) {
	for mapInd := 1; mapInd <= len(mapEnums); mapInd++ {
		valueMap, exists := mapEnums[byte(mapInd)]
		if !exists {
			return "", fmt.Errorf("there are %d map enumerations but %d is not one of them", len(mapEnums), mapInd)
		}
		for k, v := range valueMap {
			stringifiedValue, err := stringifyPrimitive(v)
			if err != nil {
				return "", fmt.Errorf("error stringifying value for key %s in map with enumeration %d. map: %+v. : %w", k, mapInd, valueMap, err)
			}
			stringifiedMapEnums += fmt.Sprintf("%s:%s;", k, stringifiedValue)
		}
		// do not include last semi-colon since semi-colons are only meant to be delimiters
		if len(valueMap) != 0 {
			stringifiedMapEnums = stringifiedMapEnums[:len(stringifiedMapEnums)-1]
		}
		stringifiedMapEnums += "\n"
	}
	return stringifiedMapEnums, nil
}

// Creates a string representation of a string, bool, or float64 for use in metadata enumerations.
// The string representation has format (<type>)value.
func stringifyPrimitive(primitiveValue interface{}) (string, error) {
	switch typedValue := primitiveValue.(type) {
	case string:
		return fmt.Sprintf("(string)%s", typedValue), nil
	case bool:
		return fmt.Sprintf("(bool)%t", typedValue), nil
	case float64:
		return fmt.Sprintf("(float64)%f", typedValue), nil
	default:
		return "", fmt.Errorf("found value of type %T when only string, bool, or float64 was expected", primitiveValue)
	}
}
