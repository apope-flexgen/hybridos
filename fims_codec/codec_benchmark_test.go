package fims_codec

import (
	"log"
	"testing"
)

// Benchmark encoding and decoding messages
func BenchmarkEncodeDecode(b *testing.B) {
	testCases := map[string]codecTestCase{
		"ess_msg": {"good-jsons/sample_ess_msg.json", "good-jsons/sample_ess_msg_expected_decode.json", true, true},
	}
	numMessages := 1000

	// encode subbenchmark
	var archiveFilePath string
	encode := func(subB *testing.B) {
		for i := 0; i < subB.N; i++ {
			for testCaseName, testCase := range testCases {
				messageSubPath := testCase.messageSubPath
				expectedSubPath := testCase.expectedDecodeSubPath

				// load a bunch of references to the same message to avoid copying
				testMessage := loadTestMessageFromJson(messageSubPath)
				testMessages := make([]map[string]interface{}, numMessages)
				for i := 0; i < numMessages; i++ {
					testMessages[i] = testMessage
				}

				expectedMessage := loadTestMessageFromJson(expectedSubPath)
				expectedMessages := make([]map[string]interface{}, numMessages)
				for i := 0; i < numMessages; i++ {
					expectedMessages[i] = expectedMessage
				}

				var err error
				archiveFilePath, err = encodeToTestArchive(testCaseName, testMessages)
				if err != nil {
					log.Panicf("Failed to encode with error %v.", err)
				}
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

	// decode bytes subbenchmark
	decodeBytes := func(subB *testing.B) {
		for i := 0; i < subB.N; i++ {
			_, err := decodeBytesFromTestArchive(archiveFilePath)
			if err != nil {
				log.Panicf("Failed to encode with error %v.", err)
			}
		}
	}

	// run subbenchmarks
	b.Run("encode", encode)
	b.Run("decode", decode)
	b.Run("decodeBytes", decodeBytes)
}
