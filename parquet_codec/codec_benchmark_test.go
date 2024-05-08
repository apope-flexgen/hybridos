package parquet_codec

import (
	"log"
	"testing"
)

// Benchmark encoding and decoding messages
func BenchmarkEncodeDecode(b *testing.B) {
	testCase := codecTestCase{
		"good-jsons/sample_ess_msg.json", "good-jsons/sample_ess_msg_expected_decode.json", true, true,
	}
	testCaseName := "ess_msg"
	numMessages := 1000

	messageSubPath := testCase.messageSubPath

	testMessage := loadTestMessageFromJson(messageSubPath)
	testMessages := make([]map[string]interface{}, numMessages)
	for i := 0; i < numMessages; i++ {
		testMessages[i] = testMessage
	}

	// encode subbenchmark
	var archiveFilePath string
	encode := func(subB *testing.B) {
		for i := 0; i < subB.N; i++ {
			var err error
			// deep copy test messages
			testMessagesCopy := make([]map[string]interface{}, numMessages)
			for i := 0; i < numMessages; i++ {
				testMessagesCopy[i] = deepCopy(testMessage)
			}

			archiveFilePath, err = encodeToTestArchive(testCaseName, testMessagesCopy)
			if err != nil {
				log.Panicf("Failed to encode with error %v.", err)
			}
		}
	}

	// decode subbenchmark
	decode := func(subB *testing.B) {
		for i := 0; i < subB.N; i++ {
			_, err := decodeFromTestArchive(archiveFilePath)
			if err != nil {
				log.Panicf("Failed to encode with error %v.", err)
			}
		}
	}

	// run subbenchmarks
	b.Run("encode", encode)
	b.Run("decode", decode)
}
