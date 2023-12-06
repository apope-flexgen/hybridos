package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path"
	"path/filepath"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/fims_codec"
)

const testInputsParentDirectoryPath string = "./test/test-data-path/"
const testOutputsDirectoryPath string = "./test/test-data-path/output-jsons/"

func main() {
	// initialize logger
	err := (&log.Config{
		Moniker:           "fims codec decode test",
		ToConsole:         true,
		ToFile:            false,
		SeverityThreshold: log.INFO,
	}).Init("fims codec decode test")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	// first program arg will be name of the directory of input archives
	if len(os.Args) < 2 {
		log.Fatalf("Not enough arguments, please provide a path for input archives relative to the test data path.")
	}
	inputArchivesDirName := os.Args[1]

	testInputsDirectoryPath := filepath.Join(testInputsParentDirectoryPath, inputArchivesDirName)

	// read-in test input archives
	archiveDirectory, err := os.Open(testInputsDirectoryPath)
	if err != nil {
		log.Fatalf("Error opening archive directory: %s", err.Error())
	}
	defer archiveDirectory.Close()

	archiveFileInfos, err := archiveDirectory.Readdir(0)
	if err != nil {
		log.Fatalf("Error reading archive directory: %s", err.Error())
	}

	// remove directory for output paths if it already exists
	fmt.Println("Deleting preexisting output directory")
	err = os.RemoveAll(testOutputsDirectoryPath)
	if err != nil {
		log.Errorf("Failed to remove output directory: %v", err)
	}
	// make directory for output JSONs
	err = os.Mkdir(testOutputsDirectoryPath, os.ModePerm)
	if err != nil {
		log.Fatalf("Failed to make outermost output directory %s: %s.", testOutputsDirectoryPath, err.Error())
	}

	// call the decode API on each test input archive
	for _, archiveInfo := range archiveFileInfos {
		startTime := time.Now()
		uriData, err := fims_codec.Decode(path.Join(testInputsDirectoryPath, archiveInfo.Name()))
		if err != nil {
			log.Errorf("Error decoding FIMS messages for archive %s: %s", archiveInfo.Name(), err.Error())
			continue
		}
		endTime := time.Now()
		decodeDuration := endTime.Sub(startTime)
		fmt.Printf("Archive %s took %d microseconds to decode\n", archiveInfo.Name(), decodeDuration.Microseconds())

		// dump the decoded data to JSON files separated into directories for each test input archive
		archiveOutputDirPath := path.Join(testOutputsDirectoryPath, fims_codec.DashifyUri(uriData.Uri))
		if err := os.Mkdir(archiveOutputDirPath, os.ModePerm); err != nil && !os.IsExist(err) {
			log.Fatalf("Failed to make archive output directory %s: %s.\n", archiveOutputDirPath, err.Error())
		}

		for i := 0; i < len(uriData.MsgBodies); i++ {
			outputJsonFilePath := path.Join(archiveOutputDirPath, fmt.Sprintf("%d.json", uriData.MsgTimestamps[i]))
			err := createOutputJson(outputJsonFilePath, uriData.MsgBodies[i])
			if err != nil {
				log.Fatalf("Failed to output message on uri %s at index %d to a JSON file.", uriData.Uri, i)
			}
		}
		fmt.Printf("Archive %s contents written\n", archiveInfo.Name())
	}
}

// Create json file from message data
func createOutputJson(outputJsonFilePath string, msg map[string]interface{}) error {
	outputJsonFile, err := os.Create(outputJsonFilePath)
	if err != nil {
		return fmt.Errorf("failed to create output JSON file %s: %w", outputJsonFilePath, err)
	}
	defer outputJsonFile.Close()

	jsonData, err := json.Marshal(msg)
	if err != nil {
		return fmt.Errorf("failed to marshal FIMS msg body into JSON: %w", err)
	}

	_, err = outputJsonFile.Write(jsonData)
	if err != nil {
		return fmt.Errorf("failed to write JSON to file %s: %w", outputJsonFilePath, err)
	}
	return nil
}
