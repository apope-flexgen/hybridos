// This package provides functions for fetching configuration from different sources.
package cfgfetch

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"io/ioutil"
	"strings"
	"time"

	Logging "github.com/flexgen-power/go_flexgen/logger"
	"github.com/mitchellh/mapstructure"
)

type decodeFunc func(interface{}) (interface{}, error)

// RetrieveWithDecode uses Retrieve() to fetch configuration data. A mapstructure decoder
// is run with the given struct and, if the decode fails, the process is retried
// after a delay.
// If the config source is DBI, then a new FIMS connection will be made to retrieve it.
// As such, this function and its variants should not be called when the caller already has
// a FIMS connection open.
func RetrieveWithDecode(processName, cfgSource string, cfgStruct interface{}) (interface{}, error) {
	for {
		// retrieve configuration data from source
		data, err := Retrieve(processName, cfgSource)
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve configuration data: %w", err)
		}
		// decode retrieved data
		cfg, err := decodeMapToConfig(data, cfgStruct)
		if err != nil {
			delaySeconds := 2
			Logging.FullError("Failed to decode retrieved configuration data", err)
			Logging.Infof("Waiting %d seconds then will re-retrieve data", delaySeconds)
			delay := time.NewTimer(time.Duration(delaySeconds) * time.Second)
			<-delay.C
			continue
		}
		return cfg, nil
	}
}

// RetrieveWithCustomDecode uses Retrieve() to fetch configuration data. A custom decode
// func is run with the retrieved data and, if the decode fails, the process is retried
// after a delay.
// If the config source is DBI, then a new FIMS connection will be made to retrieve it.
// As such, this function and its variants should not be called when the caller already has
// a FIMS connection open.
func RetrieveWithCustomDecode(processName, cfgSource string, decode decodeFunc) (interface{}, error) {
	for {
		// retrieve configuration data from source
		data, err := Retrieve(processName, cfgSource)
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve configuration data: %w", err)
		}
		// decode retrieved data
		cfg, err := decode(data)
		if err != nil {
			delaySeconds := 2
			Logging.FullError("Failed to decode retrieved configuration data", err)
			Logging.Infof("Waiting %d seconds then will re-retrieve data", delaySeconds)
			delay := time.NewTimer(time.Duration(delaySeconds) * time.Second)
			<-delay.C
			continue
		}
		return cfg, nil
	}
}

// Retrieve will, depending on the command-line flag parsed, either retrieve configuration
// data from DBI or read it in from a given file path.
// If the config source is DBI, then a new FIMS connection will be made to retrieve it.
// As such, this function and its variants should not be called when the caller already has
// a FIMS connection open.
func Retrieve(processName, cfgSource string) (cfg map[string]interface{}, err error) {
	if strings.EqualFold(cfgSource, "dbi") {
		cfg, err = retrieveFromDbi(processName)
		if err != nil {
			return nil, fmt.Errorf("failed to retrieve configuration data from dbi: %w", err)
		}
	} else {
		cfg, err = readFromFile(cfgSource)
		if err != nil {
			return nil, fmt.Errorf("failed to read configuration data from file: %w", err)
		}
	}
	return cfg, nil
}

// Creates a FIMS connection and sends GETs to DBI until configuration data is received via FIMS.
// Verifies data has type map[string]interface{} then returns it.
func retrieveFromDbi(processName string) (map[string]interface{}, error) {
	// Setup fims connection
	fimsConn, err := connectToFims(processName)
	if err != nil {
		return nil, fmt.Errorf("failed to connect to FIMS")
	}
	defer fimsConn.Close()

	// subscribe to a uri specifically for config reply from dbi
	configUri := fmt.Sprintf("/cfg_%s", processName)
	err = fimsConn.Subscribe(configUri)
	if err != nil {
		return nil, fmt.Errorf("failed to subscribe to config reply uri %s: %w", configUri, err)
	}

	// set up channel to receive data from receive thread
	dataReceive := make(chan map[string]interface{})
	defer close(dataReceive)
	// start thread to receive configuration data over FIMS
	go receiveFimsMsg(fimsConn, configUri, dataReceive)
	// start thread to signal when DBI has taken too long to reply
	waitSeconds := 10
	timeout := time.NewTicker(time.Duration(waitSeconds) * time.Second)
	defer timeout.Stop()
	// start request-receive cycle
	for {
		if !fimsConn.Connected() {
			return nil, errors.New("FIMS lost connection")
		}
		// request DBI on every attempt, expecting data to be loaded to DBI
		err := fimsConn.SendGet(fmt.Sprintf("/dbi/%s/configuration", processName), configUri)
		if err != nil {
			delaySeconds := 2
			Logging.Errorf("Error sending GET for configuration data to DBI: %s. Waiting %d seconds then trying again.", err.Error(), delaySeconds)
			delay := time.NewTimer(time.Duration(delaySeconds) * time.Second)
			<-delay.C
			timeout.Reset(time.Duration(waitSeconds) * time.Second)
		}
		// wait for DBI to reply, and re-request if it takes too long
		select {
		case data := <-dataReceive:
			return data, nil
		case <-timeout.C:
			Logging.Errorf("Timed out after waiting %d seconds for configuration from DBI. Resending GET to DBI.", waitSeconds)
		}
	}
}

// Connects to the FIMS server
func connectToFims(processName string) (*fims.Fims, error) {
	// connect to FIMS server
	fimsObj, err := fims.Connect(processName)
	if err != nil {
		return nil, fmt.Errorf("failed to connect to FIMS server: %w", err)
	}

	return &fimsObj, nil
}

// Recursively handle incoming FIMS messages until we get a valid set to the config uri
func receiveFimsMsg(fimsConn *fims.Fims, configUri string, dataReceive chan map[string]interface{}) {
	msg, err := fimsConn.Receive()
	if err != nil {
		Logging.FullError("failure while receiving FIMS message", err)
		go receiveFimsMsg(fimsConn, configUri, dataReceive)
		return
	}
	if msg.Uri != configUri {
		Logging.Errorf("Received FIMS msg to URI %s, but only expecting %s during configuration", msg.Uri, configUri)
		go receiveFimsMsg(fimsConn, configUri, dataReceive)
		return
	}
	if msg.Method == "get" && msg.Replyto != "" {
		fimsConn.SendSet(msg.Replyto, "", map[string]interface{}{})
		go receiveFimsMsg(fimsConn, configUri, dataReceive)
		return
	}
	if msg.Method != "set" {
		Logging.Errorf("Received FIMS msg with method %s but expecting set", msg.Method)
		go receiveFimsMsg(fimsConn, configUri, dataReceive)
		return
	}
	data, ok := msg.Body.(map[string]interface{})
	if !ok {
		Logging.Errorf("Received %T but expected map[string]interface{}", msg.Body)
		go receiveFimsMsg(fimsConn, configUri, dataReceive)
		return
	}
	dataReceive <- data
}

func readFromFile(cfgSource string) (data map[string]interface{}, err error) {
	bytes, err := ioutil.ReadFile(cfgSource)
	if err != nil {
		return nil, fmt.Errorf("failed to read from file: %w", err)
	}
	err = json.Unmarshal(bytes, &data)
	if err != nil {
		return nil, fmt.Errorf("failed to unmarshal configuration file %s to map[string]interface{}: %w", cfgSource, err)
	}
	return data, nil
}

func decodeMapToConfig(inputMap map[string]interface{}, inputConfig interface{}) (interface{}, error) {
	decoderConfig := &mapstructure.DecoderConfig{
		WeaklyTypedInput: true,
		Result:           &inputConfig,
		MatchName:        simplePluralComparison, // Use our custom comparator that will match singular and plural nouns
	}
	decoder, err := mapstructure.NewDecoder(decoderConfig)
	if err != nil {
		return inputConfig, fmt.Errorf("failed to build new decoder: %w", err)
	}
	// load payload data into config struct, using weak decoding for more flexible input
	err = decoder.Decode(inputMap)
	if err != nil {
		return inputConfig, fmt.Errorf("failed to decode input map: %w", err)
	}
	return inputConfig, nil
}

func simplePluralComparison(mapKey string, fieldName string) bool {
	return strings.EqualFold(mapKey, fieldName) || strings.EqualFold(mapKey+"s", fieldName) || strings.EqualFold(mapKey+"es", fieldName)
}
