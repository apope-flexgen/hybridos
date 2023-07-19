package parsemap

import (
	"bytes"
	"encoding/json"
	"testing"
)

func TestUnwrapBytes(t *testing.T) {
	type testCase struct {
		input    []byte
		expected []byte
	}

	forceMarshal := func(x interface{}) []byte {
		b, err := json.Marshal(x)
		if err != nil {
			panic(err)
		}
		return b
	}

	tests := []testCase{
		{ // naked null
			input:    forceMarshal(nil),
			expected: forceMarshal(nil),
		},
		{ // naked integer
			input:    forceMarshal(5),
			expected: forceMarshal(5),
		},
		{ // naked floating-point
			input:    forceMarshal(-10.5),
			expected: forceMarshal(-10.5),
		},
		{ // naked string
			input:    forceMarshal("easter egg"),
			expected: forceMarshal("easter egg"),
		},
		{ // naked bool
			input:    forceMarshal(true),
			expected: forceMarshal(true),
		},
		{ // naked map
			input:    forceMarshal(map[string]string{"not value": "wumbo"}),
			expected: forceMarshal(map[string]string{"not value": "wumbo"}),
		},
		{ // clothed null
			input:    forceMarshal(map[string]interface{}{"value": nil}),
			expected: forceMarshal(nil),
		},
		{ // clothed integer
			input:    forceMarshal(map[string]int{"value": 100}),
			expected: forceMarshal(100),
		},
		{ // clothed floating-point
			input:    forceMarshal(map[string]float64{"value": 555.0123}),
			expected: forceMarshal(555.0123),
		},
		{ // clothed string
			input:    forceMarshal(map[string]string{"value": "lalala"}),
			expected: forceMarshal("lalala"),
		},
		{ // clothed bool
			input:    forceMarshal(map[string]interface{}{"value": false}),
			expected: forceMarshal(false),
		},
		{ // clothed map
			input:    forceMarshal(map[string]interface{}{"value": map[string]string{"mumbo": "jumbo"}}),
			expected: forceMarshal(map[string]string{"mumbo": "jumbo"}),
		},
	}

	for i, test := range tests {
		result := UnwrapBytes(test.input)
		if !bytes.Equal(result, test.expected) {
			t.Errorf("For test index %d, expected %v but got %v.", i, test.expected, result)
		}
	}
}
