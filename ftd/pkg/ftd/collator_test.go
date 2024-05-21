package ftd

import (
	"encoding/json"
	"fims"
	"reflect"
	"testing"
)

// Test finding the correct config associated with the given uri.
func TestFindUriConfig(t *testing.T) {
	configUris := []UriConfig{
		0: {BaseUri: "/events", Method: []string{"post"}},
		1: {BaseUri: "/assets/ess", Sources: []string{"ess_1", "ess_2"}, Method: []string{"pub"}},
		2: {BaseUri: "/components", Method: []string{"pub"}},
		3: {BaseUri: "/ftd_test", Sources: []string{""}, Method: []string{"pub"}},
		4: {BaseUri: "/assets/ess", Sources: []string{"ess_1", "ess_2"}, Method: []string{"set"}},
	}

	// test messages that should not be found
	shouldNotExistTestCases := []fims.FimsMsg{
		{Uri: "/site", Method: "pub"},
		{Uri: "/assets/ess", Method: "pub"},
		{Uri: "/assets/ess/summary", Method: "pub"},
		{Uri: "/assets/ess/ess_10", Method: "pub"},
		{Uri: "/assets/ess/ess_11", Method: "pub"},
		{Uri: "/assets/ess/ess_", Method: "pub"},
		{Uri: "/assets/ess/ess_1/", Method: "pub"},
		{Uri: "/assets/ess/ess_1/soc", Method: "pub"},
		{Uri: "/ftd_test/data", Method: "pub"},
		{Uri: "/assets/ess/ess_1", Method: "post"},
	}
	testShouldNotExist := func(testMsg fims.FimsMsg) {
		cfg, exists := findUriConfig(testMsg.Uri, testMsg.Method, configUris)
		if exists {
			t.Errorf("Uri %s and method %s config was erroneously found with config %v", testMsg.Uri, testMsg.Method, cfg)
		}
	}
	for _, testCase := range shouldNotExistTestCases {
		testShouldNotExist(testCase)
	}

	// test messages that should be found
	type shouldExistTestCase struct {
		testMsg     fims.FimsMsg
		expectedCfg *UriConfig
	}
	shouldExistTestCases := []shouldExistTestCase{
		{fims.FimsMsg{Uri: "/events", Method: "post"}, &configUris[0]},
		{fims.FimsMsg{Uri: "/assets/ess/ess_1", Method: "pub"}, &configUris[1]},
		{fims.FimsMsg{Uri: "/assets/ess/ess_2", Method: "pub"}, &configUris[1]},
		{fims.FimsMsg{Uri: "/components", Method: "pub"}, &configUris[2]},
		{fims.FimsMsg{Uri: "/components/feeders", Method: "pub"}, &configUris[2]},
		{fims.FimsMsg{Uri: "/components/feeders/feed_1", Method: "pub"}, &configUris[2]},
		{fims.FimsMsg{Uri: "/ftd_test", Method: "pub"}, &configUris[3]},
		{fims.FimsMsg{Uri: "/assets/ess/ess_1", Method: "set"}, &configUris[4]},
		{fims.FimsMsg{Uri: "/assets/ess/ess_2", Method: "set"}, &configUris[4]},
	}
	testShouldExist := func(testMsg fims.FimsMsg, expectedCfg *UriConfig) {
		cfg, exists := findUriConfig(testMsg.Uri, testMsg.Method, configUris)
		if !exists {
			t.Errorf("Uri %s and method %s config was erroneously not found", testMsg.Uri, testMsg.Method)
			if cfg != expectedCfg {
				t.Errorf("Uri %s and method %s found the wrong config: %v", testMsg.Uri, testMsg.Method, cfg)
			}
		}
	}
	for _, testCase := range shouldExistTestCases {
		testShouldExist(testCase.testMsg, testCase.expectedCfg)
	}
}

// Test that all fields are encoded when configured to encode all fields
func TestAllFieldsEncoded(t *testing.T) {
	collator := MsgCollator{
		laneCfg: LaneConfig{
			DbName: "test",
			Uris: []UriConfig{
				{
					BaseUri:       "/test",
					Sources:       []string{},
					Fields:        []string{},
					Group:         "",
					Method:        []string{"post"},
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*Encoder),
	}

	testMsgBody := map[string]interface{}{
		"p":     100,
		"value": 31,
		"name":  "test_string",
		"OPEN":  true,
	}
	testMsg := fims.FimsMsg{
		Method: "post",
		Uri:    "/test",
		Body:   testMsgBody,
	}
	collator.collate(&testMsg)

	data, ok := collator.FimsMsgs["/test_post"]
	if !ok {
		t.Fatalf("Did not find any encoder messages associated with expected uri.")
	}
	encodedKeys := data.encoder.fims.GetKeys()
	checkEncoderKeysAreExpected(t, encodedKeys, []string{"p", "value", "name", "OPEN"})
}

// Test that fields are filtered as expected when configured to encode specific fields
func TestFieldsFiltered(t *testing.T) {
	collator := MsgCollator{
		laneCfg: LaneConfig{
			DbName: "test",
			Uris: []UriConfig{
				{
					BaseUri:       "/test",
					Sources:       []string{},
					Fields:        []string{"p"},
					Group:         "",
					Method:        []string{"pub"},
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*Encoder),
	}

	testMsgBody := map[string]interface{}{
		"p":     100,
		"value": 31,
		"name":  "test_string",
		"OPEN":  true,
	}
	testMsg := fims.FimsMsg{
		Method: "pub",
		Uri:    "/test",
		Body:   testMsgBody,
	}
	collator.collate(&testMsg)

	data, ok := collator.FimsMsgs["/test_pub"]
	if !ok {
		t.Fatalf("Did not find any encoder associated with expected uri.")
	}
	encodedKeys := data.encoder.fims.GetKeys()
	checkEncoderKeysAreExpected(t, encodedKeys, []string{"p"})
}

// Test that filtering fields still leaves the ftd_group key when using grouping
func TestFieldsFilteredGrouping(t *testing.T) {
	collator := MsgCollator{
		laneCfg: LaneConfig{
			DbName: "test",
			Uris: []UriConfig{
				{
					BaseUri:       "/test",
					Sources:       []string{},
					Fields:        []string{"p"},
					Group:         "test_group",
					Method:        []string{"pub"},
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*Encoder),
	}

	testMsgBody := map[string]interface{}{
		"p":     100,
		"value": 31,
		"name":  "test_string",
		"OPEN":  true,
	}
	testMsg := fims.FimsMsg{
		Method: "pub",
		Uri:    "/test",
		Body:   testMsgBody,
	}
	collator.collate(&testMsg)

	encoder, ok := collator.groups["test_group_pub"]
	if !ok {
		t.Fatalf("Did not find any encoder associated with expected uri.")
	}
	encodedKeys := encoder.fims.GetKeys()
	checkEncoderKeysAreExpected(t, encodedKeys, []string{"p", "ftd_group"})
}

// Test that nothing is encoded in the case where all fields are filtered out
func TestAllFieldsFilteredOut(t *testing.T) {
	collator := MsgCollator{
		laneCfg: LaneConfig{
			DbName: "test",
			Uris: []UriConfig{
				{
					BaseUri:       "/test",
					Sources:       []string{},
					Fields:        []string{"unused"},
					Group:         "",
					Method:        []string{"post", "pub"},
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*Encoder),
	}

	testMsgBody := map[string]interface{}{
		"p":     100,
		"value": 31,
		"name":  "test_string",
		"OPEN":  true,
	}
	testMsg := fims.FimsMsg{
		Method: "pub",
		Uri:    "/test",
		Body:   testMsgBody,
	}
	collator.collate(&testMsg)

	data, ok := collator.FimsMsgs["/test_pub"]
	if !ok {
		// we will stil create an encoder, it just won't have any messages yet
		t.Fatalf("Did not find any encoder associated with expected uri.")
	}
	if data.encoder.GetNumMessages() != 0 {
		t.Fatalf("Encoder has a non-zero number of messages.")
	}
}

// Helper function checking if the encoded keys from an encoder match an expected list of keys
func checkEncoderKeysAreExpected(t *testing.T, keys []string, expectedKeys []string) {
	t.Helper()

	// check that lengths match
	if len(keys) != len(expectedKeys) {
		t.Errorf("Keys list does not match length of expected keys list")
	}

	// check each expected key is in the keys list
	for _, expectedKey := range expectedKeys {
		found := false
		for _, key := range keys {
			if key == expectedKey {
				found = true
				break
			}
		}
		if !found {
			t.Errorf("Did not find expected key %s in keys list", expectedKey)
		}
	}

	// check each key in the keys list is in the expected keys list
	for _, key := range keys {
		found := false
		for _, expectedKey := range expectedKeys {
			if key == expectedKey {
				found = true
				break
			}
		}
		if !found {
			t.Errorf("Did not find key %s in expected keys list", key)
		}
	}
}

func TestConformMessage(t *testing.T) {
	testMessages := []fims.FimsMsg{
		{
			Uri:  "/site/configuration/reserved_bool_11",
			Body: true,
		},
		{
			Uri: "/dbi/site_controller/setpoints/site/configuration/reserved_bool_11",
			Body: map[string]interface{}{
				"value": true,
			},
		},
		{
			Uri: "/test/multiple_fields",
			Body: map[string]interface{}{
				"value":  12,
				"second": 13,
			},
		},
	}

	expectedResultMessages := []fims.FimsMsg{
		{
			Uri: "/site/configuration",
			Body: map[string]interface{}{
				"reserved_bool_11": true,
			},
		},
		{
			Uri: "/dbi/site_controller/setpoints/site/configuration",
			Body: map[string]interface{}{
				"reserved_bool_11": true,
			},
		},
		{
			Uri: "/test/multiple_fields",
			Body: map[string]interface{}{
				"value":  12,
				"second": 13,
			},
		},
	}

	for i, testMsg := range testMessages {
		resultMsgBody, conformedUri := conformMessage(&testMsg)
		if conformedUri != expectedResultMessages[i].Uri {
			t.Errorf("Conformed uri %v did not match expected conformed uri: %v", i, conformedUri)
		}
		if !reflect.DeepEqual(resultMsgBody, expectedResultMessages[i].Body) {
			t.Errorf("Conformed message %v had unexpected body: %v", i, resultMsgBody)
		}
	}
}

func TestInsertBitStringBitFields(t *testing.T) {
	uriCfg := UriConfig{
		BitStringFields: []BitStringFieldConfig{
			{
				FieldName: "bms_faults",
				BitStrings: []string{
					"BMS_DC_Current_Max_Threshold_Exceeded",
					"BMS_DC_Current_Min_Threshold_Exceeded",
					"BMS_Max_Cell_Temperature_Threshold_Exceeded",
					"BMS_Max_Cell_Voltage_Threshold_Exceeded",
					"BMS_Min_Cell_Temperature_Threshold_Exceeded",
					"BMS_Min_Cell_Voltage_Threshold_Exceeded",
					"BMS_State_of_Health_Below_Threshold",
					"BMS_DC_Voltage_Max_Threshold_Exceeded",
					"BMS_DC_Voltage_Min_Threshold_Exceeded",
					"BMS_SOC_Max_Threshold_Exceeded",
					"BMS_SOC_Min_Threshold_Exceeded",
					"BMS_Cell_Voltage_Delta_Max_Threshold_Exceeded",
					"BMS_Cell_Temperature_Delta_Max_Threshold_Exceeded",
					"BMS_Number_of_Racks_Online_Below_Min_Threshold",
				},
			},
			{
				FieldName: "status",
				BitStrings: []string{
					"StatusChange_Modifier",
					"State_Online",
					"State_On_Battery",
					"State_Bypass",
					"State_Output_Off",
					"Fault_Alarm",
					"Input_Bad",
					"Test",
					"Pending_Output_On",
					"Pending_Output_Off",
					"Commanded",
					"Reserved_Address_0_Bit_11",
					"Reserved_Address_0_Bit_12",
					"High_Efficency",
					"Informational_Alert",
					"Fault",
					"Reserved_Address_0_Bit_16",
					"Reserved_Address_0_Bit_17",
					"Reserved_Address_0_Bit_18",
					"Mains_Bad_State",
					"Fault_Recovery_State",
					"Overload_State",
					"Maintenance_Mode",
				},
			},
			{
				FieldName: "pcs_faults",
				BitStrings: []string{
					"PCS_Active_Power_Max_Threshold_Exceeded",
					"PCS_DC_Voltage_Max_Threshold_Exceeded",
					"PCS_DC_Voltage_Min_Threshold_Exceeded",
				},
			},
			{
				FieldName: "pcs_alarms",
				BitStrings: []string{
					"PCS_Active_Power_Max_Threshold_Exceeded",
					"PCS_DC_Voltage_Max_Threshold_Exceeded",
					"PCS_DC_Voltage_Min_Threshold_Exceeded",
				},
			},
		},
	}
	testMessageBodyStrings := []string{
		// message with no bitstrings
		`{
			"penguin": 341,
			"name":    "Charon",
			"gold":    41000
		}`,
		// message with modbus_client style bitstrings
		`{
			"penguin":    341,
			"name":       "Charon",
			"gold":       41000,
			"bms_faults": [],
			"status": [
				{
					"value":  22,
					"string": "Maintenance Mode"
				}
			],
			"pcs_faults": [
				{
					"value":  0,
					"string": "PCS Active Power Max Threshold Exceeded"
				},
				{
					"value":  1,
					"string": "PCS DC Voltage Max Threshold Exceeded"
				},
				{
					"value":  2,
					"string": "PCS DC Voltage Min Threshold Exceeded"
				}
			]
		}`,
		// message with site_controller style bitstrings
		`{
			"penguin": 341,
			"name":    "Charon",
			"gold":    41000,
			"bms_faults": {
				"value":   4705,
				"options": [
					{
						"name": "BMS DC Current Max Threshold Exceeded",
						"return_value": 0
					},
					{
						"name": "BMS Min Cell Voltage Threshold Exceeded",
						"return_value": 5
					},
					{
						"name": "BMS State of Health Below Threshold",
						"return_value": 6
					},
					{
						"name": "BMS SOC Max Threshold Exceeded",
						"return_value": 9
					},
					{
						"name": "BMS Cell Temperature Delta Max Threshold Exceeded",
						"return_value": 12
					}
				]
			},
			"status": 0,
			"pcs_faults": {
				"value":   0,
				"options": []
			},
			"pcs_alarms": {
				"value":   6,
				"options": [
					{
						"name": "PCS DC Voltage Max Threshold Exceeded",
						"return_value": 1
					},
					{
						"name": "PCS DC Voltage Min Threshold Exceeded",
						"return_value": 2
					}
				]
			}
		}`,
	}

	// the new fields that are expected to have been added to the message bodies
	expectedNewFields := []map[string]interface{}{
		{
			// none
		},
		{
			"bms_faults__BMS_DC_Current_Max_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_DC_Current_Min_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_Max_Cell_Temperature_Threshold_Exceeded":       float64(0),
			"bms_faults__BMS_Max_Cell_Voltage_Threshold_Exceeded":           float64(0),
			"bms_faults__BMS_Min_Cell_Temperature_Threshold_Exceeded":       float64(0),
			"bms_faults__BMS_Min_Cell_Voltage_Threshold_Exceeded":           float64(0),
			"bms_faults__BMS_State_of_Health_Below_Threshold":               float64(0),
			"bms_faults__BMS_DC_Voltage_Max_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_DC_Voltage_Min_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_SOC_Max_Threshold_Exceeded":                    float64(0),
			"bms_faults__BMS_SOC_Min_Threshold_Exceeded":                    float64(0),
			"bms_faults__BMS_Cell_Voltage_Delta_Max_Threshold_Exceeded":     float64(0),
			"bms_faults__BMS_Cell_Temperature_Delta_Max_Threshold_Exceeded": float64(0),
			"bms_faults__BMS_Number_of_Racks_Online_Below_Min_Threshold":    float64(0),
			"status__StatusChange_Modifier":                                 float64(0),
			"status__State_Online":                                          float64(0),
			"status__State_On_Battery":                                      float64(0),
			"status__State_Bypass":                                          float64(0),
			"status__State_Output_Off":                                      float64(0),
			"status__Fault_Alarm":                                           float64(0),
			"status__Input_Bad":                                             float64(0),
			"status__Test":                                                  float64(0),
			"status__Pending_Output_On":                                     float64(0),
			"status__Pending_Output_Off":                                    float64(0),
			"status__Commanded":                                             float64(0),
			"status__Reserved_Address_0_Bit_11":                             float64(0),
			"status__Reserved_Address_0_Bit_12":                             float64(0),
			"status__High_Efficency":                                        float64(0),
			"status__Informational_Alert":                                   float64(0),
			"status__Fault":                                                 float64(0),
			"status__Reserved_Address_0_Bit_16":                             float64(0),
			"status__Reserved_Address_0_Bit_17":                             float64(0),
			"status__Reserved_Address_0_Bit_18":                             float64(0),
			"status__Mains_Bad_State":                                       float64(0),
			"status__Fault_Recovery_State":                                  float64(0),
			"status__Overload_State":                                        float64(0),
			"status__Maintenance_Mode":                                      float64(1),
			"pcs_faults__PCS_Active_Power_Max_Threshold_Exceeded":           float64(1),
			"pcs_faults__PCS_DC_Voltage_Max_Threshold_Exceeded":             float64(1),
			"pcs_faults__PCS_DC_Voltage_Min_Threshold_Exceeded":             float64(1),
		},
		{
			"bms_faults__BMS_DC_Current_Max_Threshold_Exceeded":             float64(1),
			"bms_faults__BMS_DC_Current_Min_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_Max_Cell_Temperature_Threshold_Exceeded":       float64(0),
			"bms_faults__BMS_Max_Cell_Voltage_Threshold_Exceeded":           float64(0),
			"bms_faults__BMS_Min_Cell_Temperature_Threshold_Exceeded":       float64(0),
			"bms_faults__BMS_Min_Cell_Voltage_Threshold_Exceeded":           float64(1),
			"bms_faults__BMS_State_of_Health_Below_Threshold":               float64(1),
			"bms_faults__BMS_DC_Voltage_Max_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_DC_Voltage_Min_Threshold_Exceeded":             float64(0),
			"bms_faults__BMS_SOC_Max_Threshold_Exceeded":                    float64(1),
			"bms_faults__BMS_SOC_Min_Threshold_Exceeded":                    float64(0),
			"bms_faults__BMS_Cell_Voltage_Delta_Max_Threshold_Exceeded":     float64(0),
			"bms_faults__BMS_Cell_Temperature_Delta_Max_Threshold_Exceeded": float64(1),
			"bms_faults__BMS_Number_of_Racks_Online_Below_Min_Threshold":    float64(0),
			"status__StatusChange_Modifier":                                 float64(0),
			"status__State_Online":                                          float64(0),
			"status__State_On_Battery":                                      float64(0),
			"status__State_Bypass":                                          float64(0),
			"status__State_Output_Off":                                      float64(0),
			"status__Fault_Alarm":                                           float64(0),
			"status__Input_Bad":                                             float64(0),
			"status__Test":                                                  float64(0),
			"status__Pending_Output_On":                                     float64(0),
			"status__Pending_Output_Off":                                    float64(0),
			"status__Commanded":                                             float64(0),
			"status__Reserved_Address_0_Bit_11":                             float64(0),
			"status__Reserved_Address_0_Bit_12":                             float64(0),
			"status__High_Efficency":                                        float64(0),
			"status__Informational_Alert":                                   float64(0),
			"status__Fault":                                                 float64(0),
			"status__Reserved_Address_0_Bit_16":                             float64(0),
			"status__Reserved_Address_0_Bit_17":                             float64(0),
			"status__Reserved_Address_0_Bit_18":                             float64(0),
			"status__Mains_Bad_State":                                       float64(0),
			"status__Fault_Recovery_State":                                  float64(0),
			"status__Overload_State":                                        float64(0),
			"status__Maintenance_Mode":                                      float64(0),
			"pcs_faults__PCS_Active_Power_Max_Threshold_Exceeded":           float64(0),
			"pcs_faults__PCS_DC_Voltage_Max_Threshold_Exceeded":             float64(0),
			"pcs_faults__PCS_DC_Voltage_Min_Threshold_Exceeded":             float64(0),
			"pcs_alarms__PCS_Active_Power_Max_Threshold_Exceeded":           float64(0),
			"pcs_alarms__PCS_DC_Voltage_Max_Threshold_Exceeded":             float64(1),
			"pcs_alarms__PCS_DC_Voltage_Min_Threshold_Exceeded":             float64(1),
		},
	}

	for i, testMsgBodyString := range testMessageBodyStrings {
		resultMsgBody := map[string]interface{}{}
		json.Unmarshal([]byte(testMsgBodyString), &resultMsgBody)

		expectedMsgBody := map[string]interface{}{}
		json.Unmarshal([]byte(testMsgBodyString), &expectedMsgBody)
		for key, val := range expectedNewFields[i] {
			expectedMsgBody[key] = val
		}

		insertBitStringBitFields(&resultMsgBody, &uriCfg)
		if !reflect.DeepEqual(resultMsgBody, expectedMsgBody) {
			t.Errorf("Result message %v had unexpected body: %v", i, resultMsgBody)
		}
	}
}
