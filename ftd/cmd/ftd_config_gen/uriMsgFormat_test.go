package main

import (
	"fims"
	"testing"
)

// Test comparing uri msg formats with their format strings
func TestCompareUriMsgFormatStrings(t *testing.T) {
	expectedEqualFormatMsgPairs := [][]fims.FimsMsg{
		0: {
			{Body: map[string]interface{}{"f1": 2.1, "f2": "hello world", "f3": true}},
			{Body: map[string]interface{}{"f1": 0.9, "f2": "good bye", "f3": true}},
		},
	}

	for i, msgPair := range expectedEqualFormatMsgPairs {
		if formatOf(msgPair[0]).string() != formatOf(msgPair[1]).string() {
			t.Errorf("Pair of messages expected to be equal were not equal for pair %d", i)
		}
	}

	expectedUnequalFormatMsgPairs := [][]fims.FimsMsg{
		0: {
			{Body: map[string]interface{}{"f1": 2.1, "f2": "hello world", "f3": true}},
			{Body: map[string]interface{}{"a": 0.9}},
		},
	}

	for i, msgPair := range expectedUnequalFormatMsgPairs {
		if formatOf(msgPair[0]).string() == formatOf(msgPair[1]).string() {
			t.Errorf("Pair of messages expected to be unequal were equal for pair %d", i)
		}
	}
}
