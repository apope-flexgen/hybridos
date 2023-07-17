package main

import (
	"fims"
	"fmt"
	"reflect"
	"sort"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
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
		log.Errorf("Message on uri %s is invalid because it is not a JSON object", msg.Uri)
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

// Creates a new uriMsgFormat that is a copy of this uriMsgFormat and returns it
func (format uriMsgFormat) clone() uriMsgFormat {
	newFormat := uriMsgFormat{
		fieldTypes: make(map[string]reflect.Type),
	}
	for field, fieldType := range format.fieldTypes {
		newFormat.fieldTypes[field] = fieldType
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

// Compares the two formats and returns whether or not all of the fields of this format
// are present in the other format and all have the same datatypes
func (format uriMsgFormat) isSubsetOf(otherFormat uriMsgFormat) bool {
	for field, fieldType := range format.fieldTypes {
		otherFieldType, ok := otherFormat.fieldTypes[field]
		// check if field is present in other format
		if !ok {
			return false
		}
		// check if field in other format has same datatype
		if otherFieldType != fieldType {
			return false
		}
	}
	return true
}

// Compares two formats and returns whether or not there exists a field which is present
// in both formats and has a different type in each format
func (format uriMsgFormat) hasConflictWith(otherFormat uriMsgFormat) bool {
	// sufficient to check if all of the first format's fields have the same type (or don't exist) in the second
	for field, fieldType := range format.fieldTypes {
		if otherFieldType, ok := otherFormat.fieldTypes[field]; ok && otherFieldType != fieldType {
			return true
		}
	}
	return false
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
