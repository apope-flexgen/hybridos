package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"path/filepath"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/fims_codec"
)

const testInputsParentDirectoryPath string = "./test/test-data-path/input-jsons"
const testOutputsDirectoryPath string = "./test/test-data-path/compressed-archives"

func main() {
	// initialize logger
	err := (&log.Config{
		Moniker:           "fims codec encode test",
		ToConsole:         true,
		ToFile:            false,
		SeverityThreshold: log.INFO,
	}).Init("fims codec encode test")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	// first program arg will be name of the directory of input jsons
	inputJsonsDirName := os.Args[1]

	// read in test input JSONs
	sourceDirectory, err := os.Open(path.Join(testInputsParentDirectoryPath, inputJsonsDirName))
	if err != nil {
		log.Fatalf("Error opening source directory: %s\n", err.Error())
	}
	defer sourceDirectory.Close()

	sourceFileInfos, err := sourceDirectory.Readdir(0)
	if err != nil {
		log.Fatalf("Error reading source directory: %s\n", err.Error())
	}

	fimsMessages := make(map[string]map[string]interface{})
	for _, fileInfo := range sourceFileInfos {
		jsonFile, err := os.Open(filepath.Join(sourceDirectory.Name(), fileInfo.Name()))
		if err != nil {
			log.Fatalf("Error opening file %s: %s\n", fileInfo.Name(), err.Error())
		}
		defer jsonFile.Close()

		byteValue, err := ioutil.ReadAll(jsonFile)
		if err != nil {
			log.Fatalf("Error reading JSON file: %s\n", err.Error())
		}
		defer jsonFile.Close()

		var data map[string]interface{}
		err = json.Unmarshal(byteValue, &data)
		if err != nil {
			log.Fatalf("Error unmarshaling JSON data: %s\n", err.Error())
		}
		fimsMessages[fileInfo.Name()] = data
	}

	// remove directory for output paths if it already exists
	err = os.RemoveAll(testOutputsDirectoryPath)
	if err != nil {
		log.Errorf("Failed to remove output directory: %v", err)
	}
	// make directory for output archives
	err = os.Mkdir(testOutputsDirectoryPath, os.ModePerm)
	if err != nil {
		log.Fatalf("Failed to make output directory %s: %s.\n", testOutputsDirectoryPath, err.Error())
	}

	// call the msg encode API on all input test JSONs
	startTime := time.Now()
	for fileName, msgBody := range fimsMessages {
		// instantiate encoder for test
		encoder := fims_codec.NewEncoder("/fims_codec/test/uri/" + fileName + "/!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")
		encoder.AdditionalData["destination"] = "influx"
		encoder.AdditionalData["database"] = "test-database"
		encoder.AdditionalData["measurement"] = "test-measurement-" + fileName

		// encode the same message repeated once to test adding data to a new encoder and an existing encoder
		numMsgRepeats := 2
		for i := 0; i < numMsgRepeats; i++ {
			err = encoder.Encode(msgBody)
			if err != nil {
				log.Fatalf("Failed to add data from file %s to encoder: %s\n", fileName, err.Error())
			}
		}

		// call the archive creation API
		outputArchivePath, numUncompressedBytes, err := encoder.CreateArchive(testOutputsDirectoryPath,
			encoder.AdditionalData["database"]+"_"+encoder.AdditionalData["measurement"])
		if err != nil {
			fmt.Printf("Error creating archive: %s\n", err.Error())
			return
		}
		endTime := time.Now()

		// calculate and print stats
		encodeAndCompressDuration := endTime.Sub(startTime)
		fmt.Printf("Created archive %s in %d microseconds\n", outputArchivePath, encodeAndCompressDuration.Microseconds())
		archiveInfo, err := os.Stat(outputArchivePath)
		if err != nil {
			log.Fatalf("Failed to get file stat for archive: %s.\n", err.Error())
		}
		numCompressedBytes := archiveInfo.Size()
		fmt.Printf("Number of messages: %d.\n", numMsgRepeats)
		fmt.Printf("Uncompressed size: %d bytes.\n", numUncompressedBytes)
		fmt.Printf("Compressed size: %d bytes.\n", numCompressedBytes)
		fmt.Printf("Compression ratio: %f%%.\n", 100*(1-float64(numCompressedBytes)/float64(numUncompressedBytes)))
	}
}
