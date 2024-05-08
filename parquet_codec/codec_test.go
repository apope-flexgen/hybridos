package parquet_codec

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
	"time"
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
		"all_chars":                 {"good-jsons/all_chars.json", "good-jsons/all_chars_expected_decode.json", true, true},
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

		_, err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// Test encoding and decoding invalid data
func TestEncodeDecodeInvalidData(t *testing.T) {
	testCases := map[string]codecTestCase{
		"illegal_chars":  {"bad-jsons/illegal_chars.json", "bad-jsons/illegal_chars_expected_decode.json", true, true},
		"bad_mapslice":   {"bad-jsons/bad_mapslice.json", "bad-jsons/bad_mapslice_expected_decode.json", true, true},
		"some_good_data": {"bad-jsons/some_good_data.json", "bad-jsons/some_good_data_expected_decode.json", true, true},
		"has_null":       {"bad-jsons/has_null.json", "bad-jsons/has_null_expected_decode.json", true, true},
	}

	for testCaseName, testCase := range testCases {
		messageSubPath := testCase.messageSubPath
		expectedSubPath := testCase.expectedDecodeSubPath
		// encode the same message repeated once to test adding data to a new encoder and an existing encoder
		testMessages := []map[string]interface{}{loadTestMessageFromJson(messageSubPath), loadTestMessageFromJson(messageSubPath)}
		expectedMessages := []map[string]interface{}{loadTestMessageFromJson(expectedSubPath), loadTestMessageFromJson(expectedSubPath)}

		_, err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// Test encoding and decoding lots of messages
func TestEncodeDecodeStuffedArchive(t *testing.T) {
	testCases := map[string]codecTestCase{
		"lots_of_ess_msg": {"good-jsons/sample_ess_msg.json", "good-jsons/sample_ess_msg_expected_decode.json", true, true},
	}

	for testCaseName, testCase := range testCases {
		messageSubPath := testCase.messageSubPath
		expectedSubPath := testCase.expectedDecodeSubPath
		numMessages := math.MaxUint16 + 5
		// load a bunch of references to the same message to avoid copying
		testMessage := loadTestMessageFromJson(messageSubPath)
		testMessages := make([]map[string]interface{}, numMessages)
		for i := 0; i < numMessages; i++ {
			testMessages[i] = deepCopy(testMessage)
		}

		expectedMessage := loadTestMessageFromJson(expectedSubPath)
		expectedMessages := make([]map[string]interface{}, numMessages)
		for i := 0; i < numMessages; i++ {
			expectedMessages[i] = expectedMessage
		}

		_, err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
		if err != nil {
			t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
		}
	}
}

// Test encoding and decoding data with randomized values
func TestEncodeDecodeRandomvalues(t *testing.T) {
	testCaseName := "randomized_values"
	testCase := codecTestCase{"", "", true, true}

	numMessages := 1000
	// generate 512 random strings of length 16
	randStringPool := make([]string, 512)
	allowedChars := " !\"#$%&'()*+,-./0123456789:<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	for i := 0; i < len(randStringPool); i++ {
		randString := make([]byte, 16)
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

	// expected messages are a deep copy of test messages
	expectedMessages := make([]map[string]interface{}, numMessages)
	for i, msg := range testMessages {
		expectedMessages[i] = deepCopy(msg)
	}

	_, err := checkEncodeDecode(testCaseName, testMessages, expectedMessages, testCase.encodeShouldPass, testCase.decodeShouldPass)
	if err != nil {
		t.Errorf("Test case %s failed with error: %v.", testCaseName, err)
	}
}

// Checks that the messages encode and decode as specified and returns an error upon failure
func checkEncodeDecode(testCaseName string, testMessages, expectedMessages []map[string]interface{}, encodeShouldPass, decodeShouldPass bool) (resultMessages []map[string]interface{}, err error) {
	archiveFilePath, err := encodeToTestArchive(testCaseName, testMessages)
	if err != nil {
		if encodeShouldPass {
			return nil, fmt.Errorf("failed to encode test case with error: %w", err)
		} else {
			return nil, nil
		}
	} else if !encodeShouldPass {
		return nil, fmt.Errorf("encode passed when it should not have")
	}
	decodedData, err := decodeFromTestArchive(archiveFilePath)
	if err != nil {
		if decodeShouldPass {
			return nil, fmt.Errorf("failed to decode test case with error: %w", err)
		} else {
			return decodedData.MsgBodies, nil
		}
	} else if !decodeShouldPass {
		return decodedData.MsgBodies, fmt.Errorf("decode passed when it should not have")
	}
	// check that decoded metadata and messages are equal to expected
	err = checkMetadataIsExpected(decodedData.Metadata, testCaseName)
	if err != nil {
		return decodedData.MsgBodies, fmt.Errorf("metadata was corrupted in encode-decode test case: %w", err)
	}
	err = checkTimestampsAreRecent(decodedData.MsgTimestamps)
	if err != nil {
		return decodedData.MsgBodies, fmt.Errorf("timestamps aren't recent: %w", err)
	}
	if !messagesDeepEqual(expectedMessages, decodedData.MsgBodies) {
		return decodedData.MsgBodies, fmt.Errorf("messages do not match expected in encode-decode test case")
	}
	return decodedData.MsgBodies, nil
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
	dataSourceId := "/parquet_codec/test/uri/" + uriName + "/!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	encoder := NewEncoder(map[string]string{})
	encoder.Metadata["destination"] = "influx"
	encoder.Metadata["database"] = "test-database"
	encoder.Metadata["measurement"] = "test-measurement-" + uriName
	encoder.Metadata["data_source_id"] = dataSourceId

	// call the msg encode API on all passed in messages
	for _, msg := range messages {
		err = encoder.Encode(msg)
		if err != nil {
			return "", fmt.Errorf("failed to add data to encoder: %w", err)
		}
	}

	// call the archive creation API
	archiveFilePath, err = encoder.CreateArchive(testOutputsDirectoryPath,
		encoder.Metadata["database"]+"_"+encoder.Metadata["measurement"], dataSourceId)
	if err != nil {
		return "", fmt.Errorf("error creating archive: %w", err)
	}
	return archiveFilePath, nil
}

// Check that metadata matches expected
func checkMetadataIsExpected(metadata map[string]string, uriName string) error {
	dataSourceId := "/parquet_codec/test/uri/" + uriName + "/!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
	if dest := metadata["destination"]; dest != "influx" {
		return fmt.Errorf("destination is not influx but is %s", dest)
	}
	if database := metadata["database"]; database != "test-database" {
		return fmt.Errorf("database is not test database but is %s", database)
	}
	if measurement := metadata["measurement"]; measurement != "test-measurement-"+uriName {
		return fmt.Errorf("measurement is not expected measurement but is %s", measurement)
	}
	if id := metadata["data_source_id"]; id != dataSourceId {
		return fmt.Errorf("data source id is not expected id but is %s", id)
	}
	return nil
}

// Decodes the archive at the given path into message bodies.
// Returns the message bodies and an error if the decoding fails.
func decodeFromTestArchive(archiveFilePath string) (data *DecodedData, decodeErr error) {
	return DecodeLocalGZippedParquet(archiveFilePath)
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

// Checks that timestamps appear approximately correct
func checkTimestampsAreRecent(timestamps []uint64) error {
	now := uint64(time.Now().UnixMicro())
	for _, ts := range timestamps {
		if now < ts {
			return fmt.Errorf("saw timestamp from future")
		} else if now-ts > 60*1000000 {
			return fmt.Errorf("saw timestamp older than a minute")
		}
	}
	return nil
}

// Creates a deep copy of a map
func deepCopy(src map[string]interface{}) map[string]interface{} {
	dst := map[string]interface{}{}
	for key, val := range src {
		dst[key] = val
	}
	return dst
}
