package parsemap

import (
	"encoding/json"
	"fmt"
)

type varType int

const (
	FLOAT64 = iota
	INT
	INT64
	BOOL
	STRING
	MAP_STRING_INTERFACE
	INTERFACE_SLICE
)

// ExtractValueWithType extracts a key's (k) value from a map[string]interface{} (m) and does a type assertion
// using the passed-in type enumeration (t).
//
// Returns an error if either the map search or the type assertion fails. If there is no error returned, the
// caller can be guaranteed that a type assertion on the value (v) with the passed-in type will not trigger
// a panic.
func ExtractValueWithType(m map[string]interface{}, k string, t varType) (v interface{}, e error) {
	v, exists := m[k]
	if !exists {
		return nil, fmt.Errorf("key %s not found in map", k)
	}
	if v == nil {
		return nil, fmt.Errorf("value associated with key %s is nil", k)
	}
	switch t {
	case FLOAT64:
		typedV, ok := v.(float64)
		if !ok {
			return nil, fmt.Errorf("key %s does not map to float64. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case INT:
		typedV, ok := v.(int)
		if !ok {
			return nil, fmt.Errorf("key %s does not map to int. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case INT64:
		typedV, ok := v.(int64)
		if !ok {
			return nil, fmt.Errorf("key %s does not map to int. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case BOOL:
		typedV, ok := v.(bool)
		if !ok {
			return nil, fmt.Errorf("key %s does not map to bool. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case STRING:
		typedV, ok := v.(string)
		if !ok {
			return nil, fmt.Errorf("key %s does not map to string. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case MAP_STRING_INTERFACE:
		typedV, ok := v.(map[string]interface{})
		if !ok {
			return nil, fmt.Errorf("key %s does not map to map[string]interface{}. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	case INTERFACE_SLICE:
		typedV, ok := v.([]interface{})
		if !ok {
			return nil, fmt.Errorf("key %s does not map to []interface{}. Instead, maps to type %T", k, v)
		}
		return typedV, nil
	}
	return nil, fmt.Errorf("type %d not supported by function extractValueWithType()", t)
}

// ExtractAsInt extracts a key's value from a map[string]interface{}
// and verifies it is an int or float64/32, then type casts it as an int.
// Returns error if either fails
func ExtractAsInt(m map[string]interface{}, k string) (int, error) {
	// check that key is in map
	v, exists := m[k]
	if !exists {
		return 0, fmt.Errorf("key %s not found in map", k)
	}

	// return int if value exists as int
	valAsInt, err := CastToInt(v)
	if err == nil {
		return valAsInt, nil
	}
	return 0, fmt.Errorf("failed to cast %s to it: %w", k, err)
}

// CastToInt tries type assertions of float64, int float32, then uint on a given interface{}.
// If any succeed, a type-casted int is returned. If none succeed, an error is returned.
func CastToInt(val interface{}) (int, error) {
	valFloat64, ok := val.(float64)
	if ok {
		return int(valFloat64), nil
	}

	valInt, ok := val.(int)
	if ok {
		return valInt, nil
	}

	valFloat32, ok := val.(float32)
	if ok {
		return int(valFloat32), nil
	}

	valUint, ok := val.(uint)
	if ok {
		return int(valUint), nil
	}

	return 0, fmt.Errorf("value is not int, and is not a cast-acceptable type (float64/32). actual type is %T", val)
}

// UnwrapVariable asserts the passed-in interface as a map[string]interface{} then returns the value of the key 'value'.
// If the type assertion fails or the key 'value' is not found, the original interface{} is returned.
// Clothed or naked values can be passed to this func and the appropriate value will be returned.
func UnwrapVariable(obj interface{}) interface{} {
	// check if value is clothed or not. either way, load payload into valueInterface for further processing
	m, ok := obj.(map[string]interface{})
	if ok {
		v, ok := m["value"]
		if ok {
			return v
		}
		return obj
	}
	return obj
}

// Unmarshals the input into a map, searches the map for a 'value' key, marshals the value of the 'value' key, and returns the marshalled bytes.
// If any of these steps failed, assumes input is not a clothed value and return the input.
func UnwrapBytes(input []byte) []byte {
	var wrappedValue map[string]interface{}
	if err := json.Unmarshal(input, &wrappedValue); err != nil {
		return input
	}
	unwrappedValue, ok := wrappedValue["value"]
	if !ok {
		return input
	}
	unwrappedBytes, err := json.Marshal(unwrappedValue)
	if err != nil {
		// we should not ever get an error here since we just came from a successful Unmarshal.
		// hypothetically if we did, just return the input which will likely cause the caller's
		// following unmarshal to fail and execute the calling module's error checking logic
		return input
	}
	return unwrappedBytes
}
