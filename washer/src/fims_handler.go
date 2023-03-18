/**
 * fims_handler
 * fims_handler.go
 *
 * Created September 2021
 *
 * The fims_handler handles all fims messages passing to and from the washer.
 * Currently, this only includes issuing the parsed power commands from ISO to scheduler
 *
 */

package main

import (
	"fims"
	"fmt"
	"log"
	"strings"
)

// Status of this controller, Primary or Secondary
const (
	Primary = iota
	Secondary
)

var controllerStatus = Primary
var f *fims.Fims

// Channel to notify that configuration has been received, used to block operation until then
var configurationReceived chan bool

// How often to resend unverified set (in seconds)
var fimsVerificationRateSecs int

// How long to wait (in minutes) before considering the set to have failed and resending it
var fimsVerificationFailureTimeoutMins int

// Configure FIMS connection
func configureFIMS() (chan fims.FimsMsg, error) {
	// Connect to FIMS
	// Start a FIMS Receive channel that will be used to hold incoming FIMS messages
	fimsReceive := make(chan fims.FimsMsg)

	fimsObj, err := fims.Connect("WASHER")
	if err != nil {
		log.Println("unable to connect to FIMS: ", err)
		return nil, err
	}
	f = &fimsObj

	// Subscribe to messages targeted at Scheduler
	err = f.Subscribe("/washer")
	if err != nil {
		log.Println("unable to subscribe to FIMS: ", err)
		return nil, err
	}

	// Start a configuration channel that will be used to determine when config has been read successfully from DBI
	configurationReceived = make(chan bool, 1)
	go f.ReceiveChannel(fimsReceive)
	return fimsReceive, nil
}

// Main fims handling routine
func handleFims(msg fims.FimsMsg) {
	// put a recover here as insurance in case a FIMS message comes through that unexpectedly causes seg fault.
	// will keep Washer from totally crashing
	defer func() {
		if r := recover(); r != nil {
			log.Fatal("error processing FIMS message:", r)
		}
	}()

	var err error
	switch msg.Method {
	case "get":
		err = handleGet(msg)
	case "set":
		err = handleSet(msg)
	default:
		log.Println("received FIMS message with invalid method. No action taken.")
	}

	if err != nil {
		log.Println("error handling fims message: ", err)
	}
}

// Handle fims gets
func handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		return fmt.Errorf("fims_handler::handleGet ~ %v URI fragments received, expected at least 2", len(msg.Frags))
	}
	endpoint := msg.Frags[msg.Nfrags-1]
	switch endpoint {
	default:
		return fmt.Errorf("%v is an invalid Scheduler GET endpoint", msg.Uri)
	}
}

// Request the latest batchUID received by Fleet Manager
// The response is received by washer in handleSet(), which automatically triggers the query to the ISO
func getLatestBatchToQuery() {
	err := f.SendGet("/fleet/"+iso+"/latestBatch", "/washer/latestBatch")
	if err != nil {
		log.Println("Failed request to Fleet Manager for its latest id: ", err)
	}
}

func handleSet(msg fims.FimsMsg) error {
	switch {
	case strings.HasPrefix(msg.Uri, "/washer/latestBatch"):
		// Response from Fleet Manager with the last batchUID triggers query to ISO with the ID provided
		id, ok := msg.Body.(string)
		if !ok {
			return fmt.Errorf("received invalid batch id from fleet_manager")
		}
		// Starts the process for interacting with the ISO
		go sendISORequest(done, requests[0], id)
	case strings.HasPrefix(msg.Uri, "/washer/configuration"):
		// Parse configuration and configure wsdl_interface
		log.Println("Loading configuration")
		return loadWasher(msg)
	case strings.HasPrefix(msg.Uri, "/washer/queryDocument"):
		return configureISOQuery(msg)
	case strings.HasPrefix(msg.Uri, "/washer/dispatchBatch/verification"):
		// Replyto response, handled by go_fims library but not an invalid set endpoint
	default:
		return fmt.Errorf("URI is not a valid SET endpoint")
	}
	return nil
}

// Set the parsed fimsObjs to fleet manager
func sendFimsObjs(fimsObjs map[string]FimsObj) {
	for uri, fimsObj := range fimsObjs {
		// Send set with verification tracking every set to ensure it was received
		_, err := f.SendAndVerify("all", fims.FimsMsg{
			Method:  "set",
			Uri:     uri,
			Replyto: "/washer/dispatchBatch/verification",
			Body:    fimsObj,
		})
		if err != nil {
			log.Println("Failed to send with verification: ", err)
		}
	}
}

// Resend sets with error handling
func retrySets() {
	err := f.ResendUnverifiedMessages()
	if err != nil {
		log.Println("Failed to reissue sets: ", err)
	}
}

// Send GETs to DBI to get any backed up data that might be there from a previous run
func getDbiData() {
	err := f.SendGet("/dbi/washer/configuration", "/washer/configuration")
	if err != nil {
		log.Println(err)
	}
	err = f.SendGet("/dbi/washer/queryDocument", "/washer/queryDocument")
	if err != nil {
		log.Println(err)
	}
}
