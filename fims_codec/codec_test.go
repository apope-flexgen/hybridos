package fims_codec

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"math"
	"math/rand"
	"os"
	"path/filepath"
	"testing"
)

const testInputsParentDirectoryPath string = "./test/test-data-path/input-jsons"
const testOutputsDirectoryPath string = "./test/test-data-path/compressed-archives"

type codecTestCase struct {
	messageSubPath        string // sub path to json file for message to encode
	expectedDecodeSubPath string // sub path to json file containing expected message after encode and decode
	encodeShouldPass      bool   // whether or not the encode should pass without an error
	decodeShouldPass      bool   // whether or not the decode should pass without an error
}

// Test encoding and decoding valid data
func TestEncodeDecodeValidData(t *testing.T) {
	testCases := map[string]codecTestCase{
		"all_chars":                 {"good-jsons/all_chars.json", "good-jsons/all_chars.json", true, true},
		"all_types_but_empty_slice": {"good-jsons/all_types.json", "good-jsons/all_types_expected_decode.json", true, true},
		"sample_ess_msg":            {"good-jsons/sample_ess_msg.json", "good-jsons/sample_ess_msg_expected_decode.json", true, true},
		"sample_component_msg":      {"good-jsons/sample_component_msg.json", "good-jsons/sample_component_msg_expected_decode.json", true, true},
	}

	for testCaseName, testCase := range testCases {
		messageSubPath := testCase.messageSubPath
		expectedSubPath := testCase.expectedDecodeSubPath
		// encode the same message repeated once to test adding data to a new encoder and an existing encoder
		testMessages := []map[string]interface{}{loadTestMessageFromJson(messageSubPath), loadTestMessageFromJson(messageSubPath)}
		expectedMessages := []map[string]interface{}{loadTestMessageFromJson(expectedSubPath), loadTestMessageFromJson(expectedSubPath)}

		err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// Test encoding and decoding invalid data
func TestEncodeDecodeInvalidData(t *testing.T) {
	testCases := map[string]codecTestCase{
		"illegal_chars":  {"bad-jsons/illegal_chars.json", "bad-jsons/illegal_chars_expected_decode.json", true, true},
		"bad_mapslice":   {"bad-jsons/bad_mapslice.json", "bad-jsons/bad_mapslice.json", true, true}, // may want to expect encode to fail for this case
		"some_good_data": {"bad-jsons/some_good_data.json", "bad-jsons/some_good_data_expected_decode.json", true, true},
		"has_null":       {"bad-jsons/has_null.json", "bad-jsons/has_null_expected_decode.json", true, true},
	}

	for testCaseName, testCase := range testCases {
		messageSubPath := testCase.messageSubPath
		expectedSubPath := testCase.expectedDecodeSubPath
		// encode the same message repeated once to test adding data to a new encoder and an existing encoder
		testMessages := []map[string]interface{}{loadTestMessageFromJson(messageSubPath), loadTestMessageFromJson(messageSubPath)}
		expectedMessages := []map[string]interface{}{loadTestMessageFromJson(expectedSubPath), loadTestMessageFromJson(expectedSubPath)}

		err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// Test encoding and decoding max number of messages
func TestEncodeDecodeFullArchive(t *testing.T) {
	testCases := map[string]codecTestCase{
		"max_ess_msg": {"good-jsons/sample_ess_msg.json", "good-jsons/sample_ess_msg_expected_decode.json", true, true},
	}

	for testCaseName, testCase := range testCases {
		messageSubPath := testCase.messageSubPath
		expectedSubPath := testCase.expectedDecodeSubPath
		// max messages is uint16 max
		// load a bunch of references to the same message to avoid copying
		testMessage := loadTestMessageFromJson(messageSubPath)
		testMessages := make([]map[string]interface{}, math.MaxUint16)
		for i := 0; i < math.MaxUint16; i++ {
			testMessages[i] = testMessage
		}

		expectedMessage := loadTestMessageFromJson(expectedSubPath)
		expectedMessages := make([]map[string]interface{}, math.MaxUint16)
		for i := 0; i < math.MaxUint16; i++ {
			expectedMessages[i] = expectedMessage
		}

		err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// // Test encoding and decoding more than max number of messages, will not pass until we have some sort of handling for this case
// func TestEncodeDecodeOverfilledArchive(t *testing.T) {
// 	testCases := map[string]codecTestCase{
// 		"overmax_ess_msg": {"good-jsons/sample_ess_msg.json", "", false, false},
// 	}

// 	for testCaseName, testCase := range testCases {
// 		messageSubPath := testCase.messageSubPath
// 		// max messages is uint16 max
// 		overmax := math.MaxUint16 + 1
// 		// load a bunch of references to the same message to avoid copying
// 		testMessage := loadTestMessageFromJson(messageSubPath)
// 		testMessages := make([]map[string]interface{}, overmax)
// 		for i := 0; i < overmax; i++ {
// 			testMessages[i] = testMessage
// 		}

// 		expectedMessages := ([]map[string]interface{})(nil)

// 		err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
// 		if err != nil {
// 			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
// 		}
// 	}
// }

// Test encoding and decoding data with randomized values
func TestEncodeDecodeRandomvalues(t *testing.T) {
	testCaseName := "randomized_values"
	testCase := codecTestCase{"", "", true, true}

	numMessages := 1000
	// generate 255 (max allowed for encoding) random strings of length 8
	randStringPool := make([]string, 255)
	allowedChars := " !\"#$%&'()*+,-./0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	for i := 0; i < len(randStringPool); i++ {
		randString := make([]byte, 8)
		for j := 0; j < 8; j++ {
			randString[j] = allowedChars[rand.Intn(len(allowedChars))]
		}
		randStringPool[i] = string(randString)
	}

	testMessages := make([]map[string]interface{}, numMessages)
	for i := 0; i < numMessages; i++ {
		randString := randStringPool[rand.Intn(len(randStringPool))]

		testMessages[i] = map[string]interface{}{
			"numeric": rand.Float64(),
			"bool":    rand.Intn(2) == 0,
			"string":  randString,
		}
	}

	expectedMessages := testMessages

	err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
	if err != nil {
		t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
	}
}

// Checks that the messages encode and decode as specified and returns an error upon failure
func checkEncodeDecode(testCaseName string, testMessages, expectedMessages []map[string]interface{}, encodeShouldPass, decodeShouldPass bool) error {
	archiveFilePath, err := encodeToTestArchive(testCaseName, testMessages)
	if err != nil {
		if encodeShouldPass {
			return fmt.Errorf("failed to encode test case with error: %w", err)
		} else {
			return nil
		}
	} else if !encodeShouldPass {
		return fmt.Errorf("encode passed when it should not have")
	}
	decodedMessages, err := decodeFromTestArchive(archiveFilePath)
	if err != nil {
		if decodeShouldPass {
			return fmt.Errorf("failed to decode test case with error: %w", err)
		} else {
			return nil
		}
	} else if !decodeShouldPass {
		return fmt.Errorf("decode passed when it should not have")
	}
	// check that decoded messages are equal to expected messages
	if !messagesDeepEqual(expectedMessages, decodedMessages) {
		return fmt.Errorf("messages were corrupted in encode-decode test case")
	}
	decodedMessages, err = decodeBytesFromTestArchive(archiveFilePath)
	if err != nil {
		if decodeShouldPass {
			return fmt.Errorf("failed to decode as bytes test case with error: %w", err)
		} else {
			return nil
		}
	} else if !decodeShouldPass {
		return fmt.Errorf("decode as bytes passed when it should not have")
	}
	// check that decoded messages are equal to expected messages
	if !messagesDeepEqual(expectedMessages, decodedMessages) {
		return fmt.Errorf("messages were corrupted in encode-decodeBytes test case")
	}
	return nil
}

// Encodes the given message bodies to an archive in the test output dir.
// Returns the archive path and an error if the encoding fails, panics if the test directory can't be created.
func encodeToTestArchive(uriName string, messages []map[string]interface{}) (archiveFilePath string, encodeErr error) {
	// remove directory for output archives if it already exists
	err := os.RemoveAll(testOutputsDirectoryPath)
	if err != nil {
		log.Panicf("Failed to remove output directory: %v.", err)
	}
	// make directory for output archives
	err = os.Mkdir(testOutputsDirectoryPath, os.ModePerm)
	if err != nil {
		log.Panicf("Failed to make output directory %s: %s.", testOutputsDirectoryPath, err.Error())
	}

	// instantiate encoder
	encoder := NewEncoder("/fims_codec/test/uri/" + uriName + "/!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")
	encoder.AdditionalData["destination"] = "influx"
	encoder.AdditionalData["database"] = "test-database"
	encoder.AdditionalData["measurement"] = "test-measurement-" + uriName

	// call the msg encode API on all passed in messages
	for _, msg := range messages {
		err = encoder.Encode(msg)
		if err != nil {
			return "", fmt.Errorf("failed to add data to encoder: %w", err)
		}
	}

	// call the archive creation API
	archiveFilePath, _, err = encoder.CreateArchive(testOutputsDirectoryPath,
		encoder.AdditionalData["database"]+"_"+encoder.AdditionalData["measurement"])
	if err != nil {
		return "", fmt.Errorf("error creating archive: %w", err)
	}
	return archiveFilePath, nil
}

// Decodes the archive at the given path into message bodies.
// Returns the message bodies and an error if the decoding fails.
func decodeFromTestArchive(archiveFilePath string) (messages []map[string]interface{}, decodeErr error) {
	data, decodeErr := Decode(archiveFilePath)
	return data.MsgBodies, decodeErr
}

// Loads the archive at the given path as bytes and decodes the bytes into message bodies.
// Returns the message bodies and an error if the decoding fails.
func decodeBytesFromTestArchive(archiveFilePath string) (messages []map[string]interface{}, decodeErr error) {
	fileBytes, err := os.ReadFile(archiveFilePath)
	if err != nil {
		log.Panicf("Failed to read file at %s: %v.", archiveFilePath, err)
	}
	data, decodeErr := DecodeBytes(fileBytes)
	return data.MsgBodies, decodeErr
}

// Loads a message from the json file with the given subpath in the test inputs directory.
// Panics if reading or parsing the file fails. Returns nil if passed an empty string
func loadTestMessageFromJson(fileSubPath string) map[string]interface{} {
	if fileSubPath == "" {
		return nil
	}
	filePath := filepath.Join(testInputsParentDirectoryPath, fileSubPath)
	fileBytes, err := os.ReadFile(filepath.Join(testInputsParentDirectoryPath, fileSubPath))
	if err != nil {
		log.Panicf("Failed to read file at %s: %v.", filePath, err)
	}
	message := make(map[string]interface{})
	err = json.Unmarshal(fileBytes, &message)
	if err != nil {
		log.Panicf("Failed to unmarshal file at %s: %v.", filePath, err)
	}
	return message
}

// Checks if two slices of messages are deeply equal, by json marshalling both and comparing the results.
// reflect.DeepEqual doesn't work on its own due to type differences between []interface{} and []map[string]interface{}.
// Because []interface{} and []map[string]interface{} are considered distinct, reflect.DeepEqual is too strict of a comparison.
func messagesDeepEqual(ms1 []map[string]interface{}, ms2 []map[string]interface{}) bool {
	// note: marshal sorts map keys
	ms1Marshal, err := json.Marshal(ms1)
	if err != nil {
		return false
	}
	ms2Marshal, err := json.Marshal(ms2)
	if err != nil {
		return false
	}
	return bytes.Equal(ms1Marshal, ms2Marshal)
}
