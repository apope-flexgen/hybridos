/**
 * Washer
 * washer.go
 *
 * Created September 2021
 *
 * Washer is a parsing module that accepts ISO XML commands and converts them into actionable scheduler commands issued over fims
 *
 */

package main

import (
	"encoding/json"
	"fims"
	"fmt"
	"log"
	"reflect"
	"time"
)

type Cfg struct {
	Iso                                string        // Name of the ISO
	ServerIP                           string        // IP of the ISO server
	ServerPort                         string        // Server port used to service requests
	APIEndpoint                        string        // String appended to the server IP to give the full URL to which API requests are sent
	ServerQueryRateSecs                int           // How often to query the server for new data
	FimsVerificationRateSecs           int           // How often to resend unverified sets
	FimsVerificationFailureTimeoutMins int           // How long to wait before considering a set to have expired
	MaxConnectionAttempts              int           // How many times a connection is attempted before failing and restarting
	ParsingRules                       []ParsingRule // List of parsing rules
}

var fimsVerificationTicker = time.NewTicker(time.Duration(24) * time.Hour) // Ticker signaling when sets should be reissued

// Main function, calls configuration then starts message handling
func main() {
	// Complete configuration not dependent on DBI
	log.SetPrefix("Washer: ")

	// Returns a valid FIMS connection via channel
	// to service Washer and subscribes to uri "/washer".
	fimsConn, err := fims.Configure("WASHER", "/washer")
	if err != nil {
		log.Fatalf("Error configuring FIMS: %v.", err)
	}
	// Assign our connection to the global pointer
	f = &fimsConn

	// Start a FIMS channel that will recieve FIMS requests.
	fimsReceive := make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)

	// Start a configuration channel that will be used to determine
	// when a config has been read successfully from DBI.
	configurationReceived = make(chan bool, 1)

	getDbiData()

	err = configureInterface()
	if err != nil {
		return
	}

	// Routine for interfacing with ISO
	go handleConnection()

	// Operating loop
	for {
		select {
		// Receive fims messages
		case msg := <-fimsReceive:
			handleFims(msg)
		// Reissue sets for verification
		case <-fimsVerificationTicker.C:
			retrySets()
		case <-done:
			return
		}
	}
}

// Parses the configuration received from dbi
func loadWasher(msg fims.FimsMsg) error {
	var config Cfg
	// verify type of FIMS msg body
	body, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of SET to /washer/config is not map[string]interface{}. Real type is %v", reflect.TypeOf(msg.Body))
	}
	// Marshal as json so we can conveniently unmarshal into the Cfg struct
	jsonbody, err := json.Marshal(body)
	if err != nil {
		return fmt.Errorf("could not parse config as json: %w", err)
	}
	err = json.Unmarshal(jsonbody, &config)
	if err != nil {
		return fmt.Errorf("could not extract parsing rules from message: %w", err)
	}

	// configure global variables
	iso = config.Iso
	serverIP = config.ServerIP
	serverPort = config.ServerPort
	apiEndpoint = config.APIEndpoint
	serverQueryRateSecs = config.ServerQueryRateSecs
	fimsVerificationFailureTimeoutMins = config.FimsVerificationFailureTimeoutMins
	fimsVerificationRateSecs = config.FimsVerificationRateSecs
	parsingRules = config.ParsingRules
	maxConnectionAttempts = config.MaxConnectionAttempts
	// Update fims with set verification configuration
	fims.ConfigureVerification(fimsVerificationFailureTimeoutMins, nil)
	fimsVerificationTicker = time.NewTicker(time.Duration(fimsVerificationRateSecs) * time.Second)

	// notify main process loop that configuration was received
	configurationReceived <- true
	return nil
}

// Parses the wsdl document used as a template for ISO queries
func configureISOQuery(msg fims.FimsMsg) error {
	// dbi will send an empty interface if no document exists
	if msg.Body == nil {
		return fmt.Errorf("body of SET to /washer/queryDocument is empty")
	}

	// verify type of FIMS msg body
	body, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of SET to /washer/queryDocument is not map[string]interface{}. Real type is %v", reflect.TypeOf(msg.Body))
	}

	documentStr, ok := body["document"].(string)
	if !ok {
		return fmt.Errorf("failed to extract /washer/queryDocument as string")
	}

	queryDocument = documentStr
	return nil
}
