// Provides utility functions for counting the number of bytes in various pieces of data.
package countbytes

import "reflect"

// Returns number of bytes in a parsed JSON value which is neither a JSON object nor
// an array
func SizeOfJsonLeaf(val interface{}) int {
	// Only possibilities are string or some direct type
	switch typedVal := val.(type) {
	case string:
		return SizeOfString(typedVal)
	default:
		return SizeOfDirectValue(val)
	}
}

// Returns number of bytes in a string.
// Note that go doesn't end strings with null bytes
func SizeOfString(val string) int {
	return len(val)
}

// Returns number of bytes in a value with a type which doesn't use any indirection.
// i.e. an int is just a direct value, but a string is indirect because it is a pointer
// to the first element of an array
func SizeOfDirectValue(val interface{}) int {
	return int(reflect.TypeOf(val).Size())
}
