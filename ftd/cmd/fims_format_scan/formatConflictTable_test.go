package main

import (
	"fims"
	"testing"
)

// Test forming a format conflict table
func TestCreateUriFormatConflictTable(t *testing.T) {
	// create test messages and expected table
	testMsgs := []fims.FimsMsg{
		{Uri: "/a", Body: map[string]interface{}{"f1": 2.1, "f2": "hello world", "f3": true}},
		{Uri: "/b", Body: map[string]interface{}{"f1": 1.7, "f2": "good bye"}},
		{Uri: "/c", Body: map[string]interface{}{"f1": 121.5}},
		{Uri: "/d", Body: map[string]interface{}{"f1": 17.9}},
		{Uri: "/e", Body: map[string]interface{}{"f4": -16.5, "f6": "apple"}},
		{Uri: "/f", Body: map[string]interface{}{"f5": 143.12, "f6": "orange"}},
		{Uri: "/g", Body: map[string]interface{}{"f1": 2.1, "f2": "hello world", "f3": 0, "f6": "mango"}},
		{Uri: "/h", Body: map[string]interface{}{"f6": "raspberry"}},
		{Uri: "/i", Body: map[string]interface{}{"f6": 78.614}},
	}
	expectedTable := [][]bool{
		{false, false, false, false, false, true, false, false},  //a
		{false, false, false, false, false, false, false, false}, //b
		{false, false, false, false, false, false, false, false}, //c, d
		{false, false, false, false, false, false, false, true},  //e
		{false, false, false, false, false, false, false, true},  //f
		{true, false, false, false, false, false, false, true},   //g
		{false, false, false, false, false, false, false, true},  //h
		{false, false, false, true, true, true, true, false},     //i
	}
	expectedLegend := []([]string){
		{"/a"},
		{"/b"},
		{"/c", "/d"},
		{"/e"},
		{"/f"},
		{"/g"},
		{"/h"},
		{"/i"},
	}

	// collect them into necessary data structures
	formatStringToUris, formatStringToFormat := collectTestMessages(testMsgs)

	// create table
	fctable := createUriFormatConflictTable(formatStringToUris, formatStringToFormat)
	table := fctable.table
	legend := fctable.legend

	// compare legend with expected
	if len(legend) != len(expectedLegend) {
		t.Errorf("Unexpected legend length: %d, expected: %d", len(legend), len(expectedLegend))
	}
	for i := range legend {
		if len(legend[i]) != len(expectedLegend[i]) {
			t.Errorf("Unexpected legend uri list length: %d, expected: %d", len(legend[i]), len(expectedLegend[i]))
		}
		for j := range legend[i] {
			if legend[i][j] != expectedLegend[i][j] {
				t.Errorf("Unexpected legend uri: %s, expected: %s", legend[i][j], expectedLegend[i][j])
			}
		}
	}
	// compare table with expected
	if len(table) != len(expectedTable) {
		t.Errorf("Unexpected table length: %d, expected: %d", len(table), len(expectedTable))
	}
	for i := range table {
		if len(table[i]) != len(expectedTable[i]) {
			t.Errorf("Unexpected table row length: %d, expected: %d", len(table[i]), len(expectedTable[i]))
		}
		for j := range table[i] {
			if table[i][j] != expectedTable[i][j] {
				t.Errorf("Unexpected table value: %t, expected: %t", table[i][j], expectedTable[i][j])
			}
		}
	}
}

// Collect test messages into data structures for analysis
func collectTestMessages(testMsgs []fims.FimsMsg) (formatStringToUris map[string]map[string]struct{}, formatStringToFormat map[string]uriMsgFormat) {
	// collect them into necessary data structures
	formatStringToUris = make(map[string]map[string]struct{})
	formatStringToFormat = make(map[string]uriMsgFormat)
	for _, testMsg := range testMsgs {
		format := formatOf(testMsg)
		formatString := format.string()
		if _, ok := formatStringToUris[formatString]; !ok {
			formatStringToUris[formatString] = make(map[string]struct{})
			formatStringToFormat[formatString] = format.clone()
		}
		formatStringToUris[formatString][testMsg.Uri] = struct{}{}
	}
	return formatStringToUris, formatStringToFormat
}
