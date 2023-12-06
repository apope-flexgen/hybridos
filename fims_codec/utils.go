package fims_codec

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"strings"
)

// String generated by reflect when used on slices of maps.
const sliceOfMapsDataType string = "[]map[string]interface {}"

// Used to indicate an invalid or missing float64 data point.
const missingFloatMarker float64 = 0xFFDEADBEEFFF

// Used to indicate an invalid or missing boolean data point.
const missingBoolMarker byte = 0xAB

// Trims the leading slash of a URI then replaces all other slashes with dashes.
// Used when the URI is going to be a directory or file name, which cannot have
// slashes.
func DashifyUri(uri string) string {
	uri = strings.TrimPrefix(uri, "/")
	return strings.Replace(uri, "/", "-", -1)
}

// Decode byte stream to unsigned integer.
// Returns uint64 for any type of buffer provided.
// Upscales uint8/uint16/uint32 to uint64.
func convertBytesToUint(buff []byte) (uint64, error) {
	bb := bytes.NewBuffer(buff)
	switch len(buff) {
	case 1:
		return (uint64)(buff[0]), nil
	case 2:
		var i16 uint16
		binary.Read(bb, binary.BigEndian, &i16)
		return (uint64)(i16), nil
	case 4:
		var i32 uint32
		binary.Read(bb, binary.BigEndian, &i32)
		return (uint64)(i32), nil
	case 8:
		var i64 uint64
		binary.Read(bb, binary.BigEndian, &i64)
		return (uint64)(i64), nil
	default:
		return 0xFFDEADBEEFFF, fmt.Errorf("given buffer of size %d but only 1, 2, 4, and 8 allowed", len(buff))
	}
}

// Casts the given value to a float64 if the given value is of a datatype that
// can be casted to a float64. If the datatype cannot be casted to a float64,
// returns a false 'castSuccessful' flag.
func castToFloat(valueInterface interface{}) (valueFloat float64, castSuccessful bool) {
	switch v := valueInterface.(type) {
	case float64:
		return v, true
	case float32:
		return float64(v), true
	case int:
		return float64(v), true
	case uint:
		return float64(v), true
	case int8:
		return float64(v), true
	case uint8:
		return float64(v), true
	case int16:
		return float64(v), true
	case uint16:
		return float64(v), true
	case int32:
		return float64(v), true
	case uint32:
		return float64(v), true
	case int64:
		return float64(v), true
	case uint64:
		return float64(v), true
	default:
		return missingFloatMarker, false
	}
}

// Returns true if the map does not have a "value" key, which implies that it is a UI control,
// OR if the map has a key "ui_type" and the associated value is "control". Some UI controls
// have "ui_type" AND "value", while some have neither, depending on if they are considered
// "UI configureable" variables (which do not need "ui_type" or "value" in the data).
// Returns false otherwise. Only returns error if value of "ui_type" key is not a string.
func mapHoldsUiControl(data map[string]interface{}) (bool, error) {
	if _, hasValueKey := data["value"]; !hasValueKey {
		return true, nil
	}
	uiTypeInterface, ok := data["ui_type"]
	if !ok {
		return false, nil
	}
	uiType, ok := uiTypeInterface.(string)
	if !ok {
		return false, fmt.Errorf("ui_type field found but is a %T, not a string", uiTypeInterface)
	}
	return uiType == "control", nil
}

// Iterates over a slice of interfaces and type-asserts each interface as a map[string]interface{}.
// Returns the slice of maps if successful, or nil if any of the type assertions fail.
func castToMapSlice(sourceSlice []interface{}) []map[string]interface{} {
	mapSlice := make([]map[string]interface{}, len(sourceSlice))
	var ok bool
	for i, v := range sourceSlice {
		if mapSlice[i], ok = v.(map[string]interface{}); !ok {
			return nil
		}
	}
	return mapSlice
}
