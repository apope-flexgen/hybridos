package ftd

import (
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
