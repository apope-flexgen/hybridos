package main

import (
	"fims"
	"fmt"
	"reflect"
	"sort"
	"strings"
)

// A uri's message format is the datatypes associated with each of its fields
type uriMsgFormat struct {
	fieldTypes map[string]reflect.Type // map from field name to value type
}

// Gets the format of a fims message
func formatOf(msg fims.FimsMsg) uriMsgFormat {
	format := uriMsgFormat{
		fieldTypes: make(map[string]reflect.Type),
	}
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		// if not castable to a map[string]interface{}, treat the message like a single field with an empty-string key
		format.fieldTypes[""] = reflect.TypeOf(msg.Body)
		return format
	}
	for k, v := range bodyMap {
		// if the field is a map with a value key, use the type of that value instead
		if vAsMap, vIsMap := v.(map[string]interface{}); vIsMap {
			if innerValue, vHasValueKey := vAsMap["value"]; vHasValueKey {
				// if the field is a map with a value key and a "ui_type": "control" pair, then ignore it
				if uiType, vHasUiTypeKey := vAsMap["ui_type"]; vHasUiTypeKey && uiType == "control" {
					continue
				}
				format.fieldTypes[k] = reflect.TypeOf(innerValue)
				continue
			}
		}
		format.fieldTypes[k] = reflect.TypeOf(v)
	}
	return format
}

// Converts a uri message format to an equivalent (invertible) string representation
func (format uriMsgFormat) string() string {
	var stringRepBuilder strings.Builder
	keys := format.getSortedFieldNames()
	for _, k := range keys {
		stringRepBuilder.WriteString(
			fmt.Sprintf("<%d\"%s\":%s>", len(k), k, format.fieldTypes[k]))
	}
	return stringRepBuilder.String()
}

// Returns a slice of the field names sorted in alphanumeric order
func (format uriMsgFormat) getSortedFieldNames() []string {
	fieldNames := []string{}
	for field := range format.fieldTypes {
		fieldNames = append(fieldNames, field)
	}
	sort.Strings(fieldNames)
	return fieldNames
}
