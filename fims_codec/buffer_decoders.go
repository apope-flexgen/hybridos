package fims_codec

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
)

// Decodes a single float64 from a buffer and adds it to the destination map with the key decoder's key, unless
// the decoded value is a missing data marker.
func (decoder *keyDecoder) decodeSingleFloat64FromBufferToMap(dataBuffer *bytes.Buffer, destinationMap map[string]interface{}) error {
	float64AsBytes, err := readBytesIntoBuffer(dataBuffer, 8)
	if err != nil {
		return fmt.Errorf("failed to read byte-representation of float64: %w", err)
	}
	floatBits := binary.BigEndian.Uint64(float64AsBytes)
	valueFloat := math.Float64frombits(floatBits)
	if valueFloat != missingFloatMarker {
		destinationMap[decoder.key] = valueFloat
	}
	return nil
}

// Decodes a single boolean from a buffer and adds it to the destination map with the key decoder's key, unless
// the decoded value is a missing data marker.
func (decoder *keyDecoder) decodeSingleBoolFromBufferToMap(dataBuffer *bytes.Buffer, destinationMap map[string]interface{}) error {
	boolByte, err := dataBuffer.ReadByte()
	if err != nil {
		return fmt.Errorf("failed to read byte-representation of bool: %w", err)
	}
	switch boolByte {
	case byte(0):
		destinationMap[decoder.key] = false
	case byte(1):
		destinationMap[decoder.key] = true
	default: // if neither 0x00 nor 0x01, then is a missing data marker
	}
	return nil
}

// Decodes a single string enumeration from a buffer and adds the associated string value to the destination
// map with the key decoder's key, unless the decoded enumeration is a missing data marker.
func (decoder *keyDecoder) decodeSingleStringFromBufferToMap(dataBuffer *bytes.Buffer, destinationMap map[string]interface{}) error {
	enumeration, err := dataBuffer.ReadByte()
	if err != nil {
		return fmt.Errorf("failed to read string enumeration byte from buffer: %w", err)
	}
	if int(enumeration) > len(decoder.stringDecoder) {
		return fmt.Errorf("read string enumeration of %d but number of possible string enumerations is %d", int(enumeration), len(decoder.stringDecoder))
	}
	// enumeration of 0 is missing data marker
	if enumeration != 0 {
		destinationMap[decoder.key] = decoder.stringDecoder[enumeration-1]
	}
	return nil
}

// Slices of maps are written to data files by starting with a single "number of enumerations" byte
// followed by one byte per enumeration. For example, the following group of bytes...
//
// 0x04 0x02 0x02 0x03 0x03
//
// ... translates to: "There are 4 maps in this slice. The enumerations for the first two maps are 2
// and the enumerations for the second two maps are 3". In this example, there is an existing map value
// that the enumeration 0x01 would translate to, but that map value was not found in this slice of maps.
//
// Decodes a slice of maps from a buffer using enumerations and adds the slice to the destination map
// with the key decoder's key, unless a missing data marker is found.
func (decoder *keyDecoder) decodeSingleMapSliceFromBufferToMap(dataBuffer *bytes.Buffer, destinationMap map[string]interface{}) error {
	// read in first byte, which represents how many enumerations there are
	numEnumerationsBuffer, err := readBytesIntoBuffer(dataBuffer, 1)
	if err != nil {
		return fmt.Errorf("failed to read number of map slices from buffer: %w", err)
	}
	numEnumerations := uint64(numEnumerationsBuffer[0])

	// if there are no enumerations, that means missing value so do not add slice to map
	if numEnumerations == 0 {
		return nil
	}

	// read in all map enumerations
	mapEnums, err := readBytesIntoBuffer(dataBuffer, uint64(numEnumerationsBuffer[0]))
	if err != nil {
		return fmt.Errorf("failed to read map enumeration bytes: %w", err)
	}

	// instantiate the slice of maps that the map enumerations will be decoded into
	sliceOfMaps := make([]map[string]interface{}, 0)
	for _, mapEnum := range mapEnums {
		// if it is an invalid enumeration, skip it
		if mapEnum == 0 || int(mapEnum) > len(decoder.mapDecoder) {
			return fmt.Errorf("found invalid map enumeration %d where max enumeration is %d", mapEnum, len(decoder.mapDecoder))
		}
		sliceOfMaps = append(sliceOfMaps, decoder.mapDecoder[mapEnum-1])
	}
	destinationMap[decoder.key] = sliceOfMaps
	return nil
}

// Attempts to read the given number of bytes from the given buffer. Returns an error if
// there is a read error or if the number of bytes read did not match the number of
// bytes requested.
func readBytesIntoBuffer(sourceBuff *bytes.Buffer, numBytesToRead uint64) (destinationBuff []byte, err error) {
	destinationBuff = make([]byte, numBytesToRead)
	numBytesActuallyRead, err := sourceBuff.Read(destinationBuff)
	if err != nil {
		return nil, fmt.Errorf("failed to read bytes from data file: %w", err)
	}
	if uint64(numBytesActuallyRead) != numBytesToRead {
		return nil, fmt.Errorf("needed to read %d bytes from data file but was only able to read %d", numBytesActuallyRead, numBytesToRead)
	}
	return destinationBuff, nil
}
