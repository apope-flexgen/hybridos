package ftd

import (
	"fims"
	"testing"

	"github.com/flexgen-power/hybridos/fims_codec"
)

// Test finding the correct config associated with the given uri.
func TestFindUriConfig(t *testing.T) {
	configUris := []UriConfig{
		0: {BaseUri: "/events"},
		1: {BaseUri: "/assets/ess", Sources: []string{"ess_1", "ess_2"}},
		2: {BaseUri: "/components"},
		3: {BaseUri: "/ftd_test", Sources: []string{""}},
	}

	// Test uris that should not be found
	shouldNotExistTestCases := []string{
		"/site",
		"/assets/ess",
		"/assets/ess/summary",
		"/assets/ess/ess_10",
		"/assets/ess/ess_11",
		"/assets/ess/ess_",
		"/assets/ess/ess_1/",
		"/assets/ess/ess_1/soc",
		"/ftd_test/data",
	}
	testShouldNotExist := func(testUri string) {
		cfg, exists := findUriConfig(testUri, configUris)
		if exists {
			t.Errorf("%s config was erroneously found with config %v", testUri, cfg)
		}
	}
	for _, testCase := range shouldNotExistTestCases {
		testShouldNotExist(testCase)
	}

	// Test uris that should be found
	type shouldExistTestCase struct {
		testUri     string
		expectedCfg *UriConfig
	}
	shouldExistTestCases := []shouldExistTestCase{
		{"/events", &configUris[0]},
		{"/assets/ess/ess_1", &configUris[1]},
		{"/assets/ess/ess_2", &configUris[1]},
		{"/components", &configUris[2]},
		{"/components/feeders", &configUris[2]},
		{"/components/feeders/feed_1", &configUris[2]},
		{"/ftd_test", &configUris[3]},
	}
	testShouldExist := func(testUri string, expectedCfg *UriConfig) {
		cfg, exists := findUriConfig(testUri, configUris)
		if !exists {
			t.Errorf("%s config was erroneously not found", testUri)
			if cfg != expectedCfg {
				t.Errorf("%s found the wrong config: %v", testUri, cfg)
			}
		}
	}
	for _, testCase := range shouldExistTestCases {
		testShouldExist(testCase.testUri, testCase.expectedCfg)
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
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*fims_codec.Encoder),
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

	data, ok := collator.FimsMsgs["/test"]
	if !ok {
		t.Fatalf("Did not find any encoder messages associated with expected uri.")
	}
	encodedKeys := data.encoder.GetKeys()
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
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*fims_codec.Encoder),
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

	data, ok := collator.FimsMsgs["/test"]
	if !ok {
		t.Fatalf("Did not find any encoder associated with expected uri.")
	}
	encodedKeys := data.encoder.GetKeys()
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
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*fims_codec.Encoder),
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

	encoder, ok := collator.groups["test_group"]
	if !ok {
		t.Fatalf("Did not find any encoder associated with expected uri.")
	}
	encodedKeys := encoder.GetKeys()
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
					DestinationDb: "influx",
					Measurement:   "test",
				},
			},
		},
		laneName: "1",
		FimsMsgs: make(map[string]ftdData),
		groups:   make(map[string]*fims_codec.Encoder),
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

	data, ok := collator.FimsMsgs["/test"]
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
