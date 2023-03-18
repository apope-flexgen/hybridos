package main

import (
	"encoding/xml"
	"errors"
	"fmt"
	"os"
	"testing"
	"time"
)

var fiveMinsFromNow time.Time

func init() {
	fiveMinsFromNow = time.Now().Add(5 * time.Minute)
}

func TestIsSupportedBatchType(t *testing.T) {
	testStrings := []string{
		// Batch type 0, parsing rule 0
		"<DispatchBatch batchUID=\"DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892\" xmlns=\"http://ads.caiso.com\">\n<marketID>120072117</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-07-21T23:16:04Z</batchReceived>\n<batchSent>2020-07-21T23:16:04Z</batchSent>\n<batchExpires>2020-07-21T23:16:04Z</batchExpires>\n<batchType>0</batchType>\n<startTime>" + fiveMinsFromNow.Format(time.RFC3339) + "</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>2</revisionNo>\n</DispatchBatch>\n",
		// Batch type 0, parsing rule 1, invalid
		"<DispatchBatch batchUID=\"DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892\" xmlns=\"http://ads.caiso.com\">\n<marketID>120072117</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-07-21T23:16:04Z</batchReceived>\n<batchSent>2020-07-21T23:16:04Z</batchSent>\n<batchExpires>2020-07-21T23:16:04Z</batchExpires>\n<batchType>0</batchType>\n<startTime>2020-07-21T23:20:00Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>2</revisionNo>\n</DispatchBatch>\n",
		// Batch type 1, parsing rule 0, invalid
		"<DispatchBatch batchUID=\"DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892\" xmlns=\"http://ads.caiso.com\">\n<marketID>120072117</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-07-21T23:16:04Z</batchReceived>\n<batchSent>2020-07-21T23:16:04Z</batchSent>\n<batchExpires>2020-07-21T23:16:04Z</batchExpires>\n<batchType>1</batchType>\n<startTime>2020-07-21T23:20:00Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>2</revisionNo>\n</DispatchBatch>\n",
		// Can't parse batch type, parsing rule 0, invalid
		"<DispatchBatch batchUID=\"DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892\" xmlns=\"http://ads.caiso.com\">\n<marketID>120072117</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-07-21T23:16:04Z</batchReceived>\n<batchSent>2020-07-21T23:16:04Z</batchSent>\n<batchExpires>2020-07-21T23:16:04Z</batchExpires>\n<batchType>Queso</batchType>\n<startTime>2020-07-21T23:20:00Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>2</revisionNo>\n</DispatchBatch>\n",
	}

	testParsingRules := []ParsingRule{
		// Valid batch
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 0},
			},
			[]ParsingVariable{},
		},
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 1},
			},
			[]ParsingVariable{},
		},
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 0},
			},
			[]ParsingVariable{},
		},
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 0},
			},
			[]ParsingVariable{},
		},
	}

	expectedErrs := []error{nil, nil, nil, fmt.Errorf("error")}
	expectedResults := []bool{true, false, false, false}

	for i, testString := range testStrings {
		var data RecursiveXML
		err := xml.Unmarshal([]byte(testString), &data)
		if err != nil {
			t.Errorf("failed to parse testString %s as xml", testString)
		}
		supportedBatchType, err := isSupportedBatchType(testParsingRules[i], &data)
		if err != nil && expectedErrs[i] == nil {
			t.Errorf("failed to parse batch: %s", err.Error())
		}
		if supportedBatchType != expectedResults[i] {
			t.Error("fail")
		}
	}
}

func TestIsSupportedDispatchBatch(t *testing.T) {
	// Begin test data
	testStrings := []string{
		// Valid xml, 5 mins from now
		"<DispatchBatch batchUID=\"DISP-58EDF60-7821-4038-8F21-AC18002CA84C\">\n<marketID>120051409</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-05-14T14:56:31Z</batchReceived>\n<batchSent>2020-05-14T14:56:31Z</batchSent>\n<batchExpires>2020-05-14T15:44:45Z</batchExpires>\n<batchType>1</batchType>\n<startTime>" + fiveMinsFromNow.Format(time.RFC3339) + "</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>803</revisionNo>\n</DispatchBatch>\n",
		// Valid xml, invalid time
		"<DispatchBatch batchUID=\"DISP-58EDF60-7821-4038-8F21-AC18002CA84C\">\n<marketID>120051409</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-05-14T14:56:31Z</batchReceived>\n<batchSent>2020-05-14T14:56:31Z</batchSent>\n<batchExpires>2020-05-14T15:44:45Z</batchExpires>\n<batchType>1</batchType>\n<startTime>2020-05-14T14:56:31Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>803</revisionNo>\n</DispatchBatch>\n",
		// Valid xml, but invalid batch type
		"<DispatchBatch batchUID=\"DISP-58EDF60-7821-4038-8F21-AC18002CA84C\">\n<marketID>120051409</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-05-14T14:56:31Z</batchReceived>\n<batchSent>2020-05-14T14:56:31Z</batchSent>\n<batchExpires>2020-05-14T15:44:45Z</batchExpires>\n<batchType>%d</batchType>\n<startTime>2020-05-14T14:56:31Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>803</revisionNo>\n</DispatchBatch>\n",
		// Valid xml but missing batchUID
		"<DispatchBatch>\n<marketID>120051409</marketID>\n<batchStatus>3</batchStatus>\n<batchReceived>2020-05-14T14:56:31Z</batchReceived>\n<batchSent>2020-05-14T14:56:31Z</batchSent>\n<batchExpires>2020-05-14T15:44:45Z</batchExpires>\n<batchType>1</batchType>\n<startTime>2020-05-14T14:56:31Z</startTime>\n<dispatchMode>0</dispatchMode>\n<bindingFlag>Y</bindingFlag>\n<revisionNo>803</revisionNo>\n</DispatchBatch>\n",
	}

	testParsingRules := []ParsingRule{
		// Valid batch
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 1},
			},
			[]ParsingVariable{},
		},
	}

	expectedBatchUIDs := []*xml.Attr{
		{
			Name:  xml.Name{Space: "", Local: "batchUID"},
			Value: "DISP-58EDF60-7821-4038-8F21-AC18002CA84C",
		},
		nil,
		nil,
		nil,
	}
	expectedErrs := []error{nil, fmt.Errorf("error"), fmt.Errorf("error"), fmt.Errorf("error")}
	// End test data

	parsingRules = testParsingRules

	for i, testString := range testStrings {
		var cdata RecursiveXML
		err := xml.Unmarshal([]byte(testString), &cdata)
		if err != nil {
			t.Errorf("failed to parse testString %s as xml", testString)
		}
		batchUID, err := isSupportedDispatchBatch(testParsingRules, cdata)
		if (err != nil) && (expectedErrs[i] == nil) {
			t.Errorf("failed to parse batch: %s", err.Error())
		}
		if (expectedBatchUIDs[i] == nil && batchUID != nil) || (expectedBatchUIDs[i] != nil && (batchUID == nil || (expectedBatchUIDs[i].Value != batchUID.Value))) {
			t.Errorf("failed to parse out batchUID from testString %s", testString)
		}
	}
}

func TestExtractAttributeNameVal(t *testing.T) {
	testStrings := []string{
		"valid=\"Example\"",
		"valid=NoQuotes",
		"InvalidNoAssignment",
		"",
	}
	expectedResults := [][]string{
		{"valid", "Example"},
		{"valid", "NoQuotes"},
		nil, // Invalid example, expect no returned string results
		nil, // Invalid example, expect no returned string results
	}
	for i, testString := range testStrings {
		results, err := extractAttributeNameVal(testString)
		if err != nil && results != nil {
			t.Errorf("extraction got error and non-nil results: %+v", results)
		} else if (results != nil && expectedResults[i] != nil) && (results[0] != expectedResults[i][0] || results[1] != expectedResults[i][1]) {
			t.Errorf("extraction of %s expected %+v but got %+v", testStrings[i], expectedResults[i], results)
		}
	}
}

func TestMatchXMLAttributes(t *testing.T) {
	parseAttributes := [][]string{
		{"segNo=\"0\""},
		{"segNo=\"0\""},
		{"segNo=\"0\"", "batchUID=\"DISP-8FC0FF70-7825-4038-8F21-AC18002CA84C\""},
		{"segNo=\"0\"", "batchUID=\"DISP-8FC0FF70-7825-4038-8F21-AC18002CA84C\""},
	}
	xmlAttributes := [][]xml.Attr{
		{xml.Attr{Name: xml.Name{Space: " ", Local: "segNo"}, Value: "0"}},
		{xml.Attr{Name: xml.Name{Space: " ", Local: "segNo"}, Value: "0"}, xml.Attr{Name: xml.Name{Space: " ", Local: "example"}, Value: "2"}},
		{xml.Attr{Name: xml.Name{Space: " ", Local: "segNo"}, Value: "0"}, xml.Attr{Name: xml.Name{Space: " ", Local: "batchUID"}, Value: "DISP-8FC0FF70-7825-4038-8F21-AC18002CA84C"}},
		{xml.Attr{Name: xml.Name{Space: " ", Local: "segNo"}, Value: "0"}},
	}
	expectedResult := []bool{true, true, true, false}

	for i, parseAttribute := range parseAttributes {
		result, err := matchXMLAttributes(parseAttribute, xmlAttributes[i])
		if err != nil {
			t.Error("Test should not produce errors")
		}
		if result != expectedResult[i] {
			t.Errorf("Expected %t, got %t", expectedResult[i], result)
		}
	}
}

// May be more of an integration test, testing all functions involved in parsing a SOAP message recceived from the ISO
func TestParseSoap(t *testing.T) {
	// Separate hardcoded tests
	// Begin test 1 - DispatchBatchesSinceUID
	testData1, err := os.ReadFile("../test_data/batchesSince.wsdl")
	if err != nil {
		t.Error("Could not read test data for batchesSince.wsdl")
	}
	// Returning a slice of map[string]interface{} so order should always be preserved
	// Need to ensure that the order dictated by the ISO is preserved
	expectedBatches := []string{
		"DISP-58EDF60-7821-4038-8F21-AC18002CA84C",
		"DISP-19116120-7821-4038-8F21-AC18002CA84C",
		"DISP-BEF78E00-7823-4038-8F21-AC18002CA84C",
		"DISP-BFCC6A30-7823-4038-8F21-AC18002CA84C",
		"DISP-8FC0FF70-7825-4038-8F21-AC18002CA84C",
	}

	batchParsingRules := []ParsingRule{
		// Tests 1, Valid Parsing
		{
			"/fleet_manager/caiso/dispatchBatch",
			[]BatchType{
				{Id: "example", BatchValue: 0},
				{Id: "example", BatchValue: 1},
				{Id: "example", BatchValue: 2},
				{Id: "example", BatchValue: 3},
				{Id: "example", BatchValue: 4},
				{Id: "example", BatchValue: 5},
			},
			[]ParsingVariable{
				{Id: "start_time", ParseUri: "/DispatchBatch/instructions/instruction/startTime", DataType: "time"},
				{Id: "power_cmd", ParseUri: "/DispatchBatch/instructions/instruction/detail/instructionDetail segNo=\"3\"/mw", DataType: "float"},
				{Id: "duration", DataType: "float", Value: 300},
			},
		},
	}

	parsingRules = batchParsingRules

	batchesList, _, responseType, err := parseSOAPEnvelope(testData1)
	if err != nil {
		t.Error("parsing error", err.Error())
	}
	if responseType != "getDispatchBatchesSinceUIDResponse" {
		t.Error("received incorrect response type")
	}
	for i, batchUID := range batchesList {
		if batchUID != expectedBatches[i] {
			t.Errorf("received batchUID: %s did not match expected batchUID: %s", batchUID, expectedBatches[i])
		}
	}

	// Begin test 2 - GetDispatchBatch (gzipped)
	// This is definitely an integration test
	testData2, err := os.ReadFile("../test_data/batchInstructions.wsdl")
	if err != nil {
		t.Error("could not read test data for batchInstructions.wsdl")
	}
	testParsingRulesList := [][]ParsingRule{
		{
			// Tests 1, Valid Parsing
			{
				"/fleet_manager/caiso/dispatchBatch",
				[]BatchType{
					{Id: "example", BatchValue: 0},
				},
				[]ParsingVariable{
					{Id: "Id", ParseUri: "/DispatchBatch/instructions/instruction/batchUID", DataType: "string"},
					{Id: "StartTime", ParseUri: "/DispatchBatch/instructions/instruction/startTime", DataType: "time"},
					{Id: "Command", ParseUri: "/DispatchBatch/instructions/instruction/detail/instructionDetail segNo=\"3\"/mw", DataType: "float"},
					{Id: "duration", DataType: "float", Value: 300},
				},
			},
		},
		{
			// Test 2, Invalid URI
			{
				"/fleet_manager/caiso/dispatchBatch",
				[]BatchType{
					{Id: "example", BatchValue: 0},
				},
				[]ParsingVariable{
					{Id: "Id", ParseUri: "/DispatchBatch/instructions/instruction/batchUID", DataType: "string"},
					{Id: "StartTime", ParseUri: "/DispatchBatch/invalid/uri", DataType: "time"},
					{Id: "Command", ParseUri: "/DispatchBatch/instructions/instruction/detail/instructionDetail segNo=\"3\"/mw", DataType: "float"},
					{Id: "duration", DataType: "float", Value: 300},
				},
			},
		},
		{
			// Test 3, Invalid datatype, should interpret data as string and pass test
			{
				"/fleet_manager/caiso/dispatchBatch",
				[]BatchType{
					{Id: "example", BatchValue: 0},
				},
				[]ParsingVariable{
					{Id: "Id", ParseUri: "/DispatchBatch/instructions/instruction/batchUID", DataType: "string"},
					{Id: "StartTime", ParseUri: "/DispatchBatch/instructions/instruction/startTime", DataType: "time"},
					{Id: "Command", ParseUri: "/DispatchBatch/instructions/instruction/detail/instructionDetail segNo=\"3\"/mw", DataType: "foo"},
					{Id: "duration", DataType: "float", Value: 300},
				},
			},
		},
	}
	expectedErrs := []error{nil, errors.New("error"), nil}
	expectedTime, _ := time.Parse(time.RFC3339, "2020-07-21T23:22:30Z")
	expectedVariables := []FimsObj{
		// Test 1
		{Id: "DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892", StartTime: expectedTime, Data: map[string]interface{}{"Command": 47, "duration": 300}},
		// Test 2
		{},
		// Test 3
		{Id: "DISP-12A1D8C0-ADD6-4038-FFFD-AC15E2EC8892", StartTime: expectedTime, Data: map[string]interface{}{"Command": "47.0", "duration": 300}},
	}

	for i, parsingRulesEntry := range testParsingRulesList {
		// Will need to make ParsingRules public in parser.go to enable this endpoint
		parsingRules = parsingRulesEntry
		_, parsedFimsObjs, responseType, err := parseSOAPEnvelope(testData2)
		if err != nil && expectedErrs[i] == nil {
			t.Errorf("parsing error for index %d, %s", i, err.Error())
		}
		if responseType != "getDispatchBatchResponse" {
			t.Error("received incorrect response type")
		}
		if len(parsedFimsObjs) > 0 {
			for _, fimsObj := range parsedFimsObjs {
				if expectedVariables[i].Id != fimsObj.Id {
					t.Errorf("expected id %s but got %s", expectedVariables[i].Id, fimsObj.Id)
				}
				if expectedVariables[i].StartTime != fimsObj.StartTime {
					t.Errorf("expected startTime %s but got %s", expectedVariables[i].StartTime.String(), fimsObj.StartTime.String())
				}
				for key, val := range fimsObj.Data {
					if expectedVariables[i].Data[key] == nil {
						t.Errorf("entry for key %s not expected", key)
					}
					switch val.(type) {
					case int:
						if val != expectedVariables[i].Data[key].(int) {
							t.Errorf("expected %v but got %v\n", expectedVariables[i], val)
						}
					case float32:
						if val != expectedVariables[i].Data[key].(float32) {
							t.Errorf("expected %v but got %v\n", expectedVariables[i], val)
						}
					case time.Time:
						expectedTime, err := time.Parse(time.RFC3339, expectedVariables[i].Data[key].(string))
						if err != nil {
							t.Errorf("failed to parse %s as time", expectedVariables[i].Data[key].(string))
						}
						if val != expectedTime {
							t.Errorf("expected %v but got %v\n", expectedVariables[i], val)
						}
					case string:
						if val != expectedVariables[i].Data[key].(string) {
							t.Errorf("expected %v but got %v\n", expectedVariables[i], val)
						}
					}
				}
			}
		}
	}
}
