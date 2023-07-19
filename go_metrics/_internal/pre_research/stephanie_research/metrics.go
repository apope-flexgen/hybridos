package main

import (
	"encoding/json"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func getAndParseFile(args []string) {

	// verify the correct number of command line arguments
	if len(os.Args) < 2 {
		log.Fatal("Please supply a path to .json configuration file. Usage: ./metrics path/to/config/")
	}

	// resolve the command line args into absolute filepath
	configPath, err := filepath.Abs(os.Args[1])
	if err != nil {
		log.Fatal("Could not resolve filepath")
	}

	// check for file extension. If not there, try a couple of things before giving up
	var configPathAndFile string
	if !strings.Contains(configPath, ".json") {
		configPathAndFile = configPath + "metrics.json"
	} else {
		configPathAndFile = configPath
	}
	if _, err := os.Stat(configPathAndFile); err != nil {
		configPathAndFile = configPath + "_metrics.json"
		if _, err = os.Stat(configPathAndFile); err != nil {
			log.Fatalf(".json configuration not found in %s. Please supply a path to .json configuration file. Usage: node metrics.js path/to/config/", configPath)
		}
	}

	// extract filename and path individually
	configPathTemp := strings.Split(configPathAndFile, "/")
	i := len(configPathTemp)
	configFileName := configPathTemp[i-1]
	configPath = strings.Join(configPathTemp[0:i-1], "/")

	// make sure path got extracted correctly
	if len(os.Args) > 2 && os.Args[2] == "pathTest" {
		log.Printf("\npath argument received: %s\n", os.Args[1])
		log.Printf("path derivedfrom that: %s\n", configPath)
		log.Printf("config file name: %s\n", configFileName)
		log.Printf("path to mdo file: %s\n", configPath+"/mdo_"+configFileName)
		log.Printf("path to config file: %s\n\n\n", configPathAndFile)
		log.Fatal()
	}

	var meta map[string]interface{}
	data, _ := os.ReadFile(configPathAndFile)
	json.Unmarshal(data, &meta)
}

func main() {

	//meta_byte, _ := meta.(interface{})
	// hashbrown := sha256.Sum256(meta_byte)

	// // connect to fims!!
	// f, _ := fims.Connect("metrics")

	// Test sending a message
	// msg := fims.FimsMsg{
	// 	Method: "pub",
	// 	Uri:    "/metrics",
	// 	Body:   string(hashbrown[:]),
	// }
	// f.Send(msg)

	// TODO: Consol log and Configure MDO

	consoleLogAndConfigureMdo(meta)
}

func consoleLogAndConfigureMdo(meta map[string]interface{}) {
	msg_byte, _ := json.MarshalIndent(meta, "", "    ")
	log.Printf(string(msg_byte))
	publishUris, _ := meta["PublishURIs"].([]string)
	configureMdo(publishUris)
}

func configureMdo(publishUris []string) (mdo metrics_data_object) {

	for _, pubUri := range publishUris {
		mdo.PubUri = pubUri
	}
	return mdo

}
