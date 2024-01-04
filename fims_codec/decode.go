package fims_codec

import (
	"archive/tar"
	"bytes"
	"compress/gzip"
	"errors"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

// Every archive gets decoded into this struct which holds the URI of the FIMS messages, all decoded FIMS message bodies,
// their timestamps, and any additional data included with the FIMS messages (database destination, measurement, etc.).
type DecodedData struct {
	Uri            string
	MsgTimestamps  []uint64
	MsgBodies      []map[string]interface{}
	AdditionalData map[string]string
}

// Collection of individual key decoders that, together, will decode an entire FIMS message.
type keyDecoders []*keyDecoder

// Translates encoded data for a specific in the FIMS message's body map.
type keyDecoder struct {
	key      string
	dataType string

	// Maps are enumerated as bytes, with the enumeration values starting from 0x01 and counting up.
	// The mapDecoder will be created from the information in metadata.txt such that the enumeration
	// value for a given map will be one greater than the index of the map in mapDecoder.
	mapDecoder []map[string]interface{}

	// Strings are enumerated as bytes, with the enumeration values starting from 0x01 and counting up.
	// The stringDecoder will be created from the information in metadata.txt such that the enumeration
	// value for a given string will be one greater than the index of the string in stringDecoder.
	stringDecoder []string
}

// Decodes the data from an archive file found at the given path.
func Decode(archiveFilePath string) (data *DecodedData, err error) {
	// open archive file
	archiveFile, err := os.Open(archiveFilePath)
	if err != nil {
		return nil, fmt.Errorf("failed to open archive %s: %w", archiveFilePath, err)
	}
	defer archiveFile.Close()

	data, err = decodeFrom(archiveFile)
	if err != nil {
		return nil, fmt.Errorf("failed to decode archive %s: %w", archiveFilePath, err)
	}

	return data, nil
}

// Decodes the data from the contents of an archive file which has been loaded into an array of bytes.
func DecodeBytes(archiveBytes []byte) (data *DecodedData, err error) {
	archiveReader := bytes.NewReader(archiveBytes)

	data, err = decodeFrom(archiveReader)
	if err != nil {
		return nil, fmt.Errorf("failed to decode archive: %w", err)
	}

	return data, nil
}

// Extracts the metadata.txt file and data files held by the given .tar.gz archive, then decodes all of the data.
func decodeFrom(archiveReader io.Reader) (data *DecodedData, err error) {
	// read the contents of each file in the archive (including metadata.txt) into a separate bytes buffer
	fileNameToDataBufferMap, err := readArchiveDataIntoBuffers(archiveReader)
	if err != nil {
		return nil, fmt.Errorf("failed to extract archive: %w", err)
	}

	// get the buffer for the metadata.txt file, then remove it from the map so map only contains buffers for encoded data
	metadataBuffer, ok := fileNameToDataBufferMap["metadata.txt"]
	if !ok {
		return nil, errors.New("did not find metadata.txt in archive")
	}
	delete(fileNameToDataBufferMap, "metadata.txt")

	// use the contents of the metadata.txt file to construct key decoders.
	// also read in the URI and 'additional data' that apply to all FIMS messages
	// in the archive.
	uri, decoders, additionalData, err := readMetadata(metadataBuffer.String())
	if err != nil {
		return nil, fmt.Errorf("failed to interpret contents of metadata.txt: %w", err)
	}

	// configure struct to hold decoded data
	data = &DecodedData{
		Uri:            uri,
		MsgTimestamps:  make([]uint64, 0),
		MsgBodies:      make([]map[string]interface{}, 0),
		AdditionalData: additionalData,
	}

	// iterate through all data files extracted from archive and decode them
	for fileName, dataBuffer := range fileNameToDataBufferMap {
		msgTimestamps, msgBodies, err := decoders.decodeFimsMessages(dataBuffer)
		if err != nil {
			return nil, fmt.Errorf("failed to decode data file %s for URI %s: %w", fileName, uri, err)
		}
		data.MsgTimestamps = append(data.MsgTimestamps, msgTimestamps...)
		data.MsgBodies = append(data.MsgBodies, msgBodies...)
	}
	return data, nil
}

// Decodes a buffer containing the contents of a single data file into a list of message bodies and the timestamps associated with them.
func (decoders keyDecoders) decodeFimsMessages(dataBuffer *bytes.Buffer) (msgTimestamps []uint64, msgBodies []map[string]interface{}, err error) {
	// the first two bytes contain a uint16 representing the number of messages encoded into the rest of the data buffer
	numMessagesBuffer := make([]byte, 2)
	_, err = dataBuffer.Read(numMessagesBuffer)
	if err != nil {
		return nil, nil, fmt.Errorf("failed to read first two bytes of data file (number of messages): %w", err)
	}
	numMessages, _ := convertBytesToUint(numMessagesBuffer)

	// decode one message at a time
	msgTimestamps = make([]uint64, 0, numMessages)
	msgBodies = make([]map[string]interface{}, 0)
	for i := uint64(0); i < numMessages; i++ {
		msgTimestamp, msgBody, err := decoders.decodeSingleFimsMsg(dataBuffer)
		if err != nil {
			return nil, nil, fmt.Errorf("failed to decode FIMS message %d of %d: %w", i+1, numMessages, err)
		}
		msgTimestamps = append(msgTimestamps, msgTimestamp)
		msgBodies = append(msgBodies, msgBody)
	}
	return msgTimestamps, msgBodies, nil
}

// Decodes one FIMS message from the given buffer.
func (decoders keyDecoders) decodeSingleFimsMsg(dataBuffer *bytes.Buffer) (msgTimestamp uint64, msgBody map[string]interface{}, err error) {
	// read timestamp of FIMS message
	uint64AsBytes, err := readBytesIntoBuffer(dataBuffer, 8)
	if err != nil {
		return 0, nil, fmt.Errorf("failed to decode message timestamp from bytes to uint64: %w", err)
	}
	msgTimestamp, _ = convertBytesToUint(uint64AsBytes)

	// use each key decoder to decode the entire FIMS message body
	msgBody = make(map[string]interface{})
	for _, decoder := range decoders {
		switch decoder.dataType {
		case "float64":
			err = decoder.decodeSingleFloat64FromBufferToMap(dataBuffer, msgBody)
		case "bool":
			err = decoder.decodeSingleBoolFromBufferToMap(dataBuffer, msgBody)
		case "string":
			err = decoder.decodeSingleStringFromBufferToMap(dataBuffer, msgBody)
		case sliceOfMapsDataType:
			err = decoder.decodeSingleMapSliceFromBufferToMap(dataBuffer, msgBody)
		default:
			err = fmt.Errorf("key decoder has invalid data type %s", decoder.dataType)
		}
		if err != nil {
			return 0, nil, fmt.Errorf("failed to decode key %s of data type %s: %w", decoder.key, decoder.dataType, err)
		}
	}
	return msgTimestamp, msgBody, nil
}

// From the contents of a metadata.txt file, interprets the URI of the FIMS messages that are encoded in the archive,
// builds decoders for each key found in the FIMS messages' bodies, and reads in 'additional data' contained in the
// metadata such as database destination for the archived data, database measurement, etc.
func readMetadata(concatenatedMetadata string) (uri string, decoders keyDecoders, additionalData map[string]string, err error) {
	decoders = make([]*keyDecoder, 0)

	// split metadata file into individual lines and read in first line as URI
	metadataFileLines := strings.Split(concatenatedMetadata, "\n")
	if len(metadataFileLines) == 1 {
		return "", nil, nil, errors.New("metadata only contains a single line")
	}
	uri = metadataFileLines[0]
	metadataFileLines = metadataFileLines[1:]

	// read next lines as "additional data" which is in the format <additional data key (always string)>:<additional data value (always string)>.
	// once empty line is found, "additional data" is over
	additionalData = make(map[string]string)
	for len(metadataFileLines[0]) > 0 && len(metadataFileLines) > 0 {
		keyValuePair := strings.Split(metadataFileLines[0], ":")
		if len(keyValuePair) != 2 {
			return "", nil, nil, fmt.Errorf("found 'additional data' line with multiple colons: %s", metadataFileLines[0])
		}
		additionalData[keyValuePair[0]] = keyValuePair[1]
		metadataFileLines = metadataFileLines[1:]
	}
	if len(metadataFileLines) < 2 { // 2 not 1 since empty line is expected between uri / additional data and key data
		return "", nil, nil, errors.New("metadata does not contain any key data")
	}

	// empty line is expected between uri / additional data and key data,
	// so do another pop from front of slice before parsing key data.
	metadataFileLines = metadataFileLines[1:]

	// each of the remaining lines should give a key and its data type, separated by a tab character ('\t').
	// for enumerated types, their key-type pairs will be followed by enumerations on following lines.
	for len(metadataFileLines) > 0 {
		// get key-type pair and create a new key decoder
		keyTypePair := strings.Split(metadataFileLines[0], "\t")
		if len(keyTypePair) != 2 {
			return "", nil, nil, fmt.Errorf("expecting key-type pair separated by tab but found: %s", metadataFileLines[0])
		}
		metadataFileLines = metadataFileLines[1:]
		key, dataType := keyTypePair[0], keyTypePair[1]
		decoder := &keyDecoder{
			key:      key,
			dataType: dataType,
		}
		decoders = append(decoders, decoder)

		// if an enumerated type, instantiate their enum interpreter and read in enumerated values.
		// the order of the values determine their enumerations (first value is 0x01 followed by 0x02, 0x03, and so on).
		// use index of enumeration slice to represent enumeration (index in slice + 1 = enumeration value)
		if dataType == "string" {
			decoder.stringDecoder = make([]string, 0)
			for len(metadataFileLines) > 0 && !strings.Contains(metadataFileLines[0], "\t") {
				decoder.stringDecoder = append(decoder.stringDecoder, metadataFileLines[0])
				metadataFileLines = metadataFileLines[1:]
			}
		} else if dataType == sliceOfMapsDataType {
			decoder.mapDecoder = make([]map[string]interface{}, 0)
			for len(metadataFileLines) > 0 && !strings.Contains(metadataFileLines[0], "\t") {
				parsedMap, err := parseStringifiedMap(metadataFileLines[0])
				if err != nil {
					return "", nil, nil, fmt.Errorf("failed to read map enumerations for key %s in line %s: %w", key, metadataFileLines[0], err)
				}
				decoder.mapDecoder = append(decoder.mapDecoder, parsedMap)
				metadataFileLines = metadataFileLines[1:]
			}
		}
	}
	return uri, decoders, additionalData, nil
}

// Parses the string representation of a map, the expected format is:
// "<key 1>:(<type 1)<value 1>;<key 2>:(<type 2)<value 2>;<key 3>:(<type 3)<value 3>" etc.
func parseStringifiedMap(stringifiedMap string) (parsedMap map[string]interface{}, err error) {
	parsedMap = make(map[string]interface{})
	for i, stringifiedMapElement := range strings.Split(stringifiedMap, ";") {
		// clean whitespace
		stringifiedMapElement = strings.TrimSpace(stringifiedMapElement)
		if len(stringifiedMapElement) == 0 {
			return nil, fmt.Errorf("stringified map element at index %d is empty", i)
		}
		// split into key, type, and value
		keyMetadataPair := strings.SplitN(stringifiedMapElement, ":", 2)
		if len(keyMetadataPair) != 2 {
			return nil, fmt.Errorf("invalid map pair entry %v detected at index %d in stringified map", keyMetadataPair, i)
		}
		keyInMap, typeAndValueConcatenated := keyMetadataPair[0], keyMetadataPair[1]
		typeAndValueConcatenated = strings.TrimPrefix(typeAndValueConcatenated, "(")
		typeAndValueSplit := strings.SplitN(typeAndValueConcatenated, ")", 2)
		if len(typeAndValueSplit) != 2 {
			return nil, fmt.Errorf("invalid type-value pair entry %s detected at index %d in stringified map", typeAndValueConcatenated, i)
		}
		dataType, dataValueStringified := typeAndValueSplit[0], typeAndValueSplit[1]
		// convert value from string to identified datatype and add to map
		switch dataType {
		case "string":
			parsedMap[keyInMap] = dataValueStringified
		case "bool":
			parsedBool, err := strconv.ParseBool(dataValueStringified)
			if err != nil {
				return nil, fmt.Errorf("failed to parse bool from string at stringified map element at index %d: %w", i, err)
			}
			parsedMap[keyInMap] = parsedBool
		default:
			//parse to float64 by default.
			parsedFloat, err := strconv.ParseFloat(dataValueStringified, 64)
			if err != nil {
				return nil, fmt.Errorf("failed to parse float64 from string at stringified map element at index %d: %w", i, err)
			}
			parsedMap[keyInMap] = parsedFloat
		}
	}
	return parsedMap, nil
}

// Extracts the contents of each file in the given .tar.gz archive into a buffer and returns a map
// of file names to data buffers for each file.
func readArchiveDataIntoBuffers(archiveReader io.Reader) (fileNameToDataBufferMap map[string]*bytes.Buffer, err error) {
	// create decompressing file reader
	fileReader, err := gzip.NewReader(archiveReader)
	if err != nil {
		return nil, fmt.Errorf("failed to create archive gzip reader: %w", err)
	}
	defer fileReader.Close()
	// add tar filter to the reader
	tarBallReader := tar.NewReader(fileReader)

	// extract the contents of each file to a bytes buffer
	fileNameToDataBufferMap = make(map[string]*bytes.Buffer)
	for { // loop over all files in the archive
		fileHeader, err := tarBallReader.Next()
		if err != nil {
			if err == io.EOF {
				break // only successful loop exit
			}
			return nil, fmt.Errorf("failure reading data from archive tar reader: %w", err)
		}

		if fileHeader.Typeflag != tar.TypeReg {
			log.Errorf("Unable to untar type %c in file %s within archive", fileHeader.Typeflag, fileHeader.Name)
			continue
		}

		// read data from tar reader into buffer until EOF is found (handled by ReadFrom method)
		dataBuff := new(bytes.Buffer)
		_, err = dataBuff.ReadFrom(tarBallReader)
		if err != nil {
			log.Errorf("Failed to read contents of file %s within archive: %v", fileHeader.Name, err)
			continue
		}
		fileNameToDataBufferMap[fileHeader.Name] = dataBuff
	}
	return fileNameToDataBufferMap, nil
}
