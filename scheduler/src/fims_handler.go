/**
 *
 * fims_handler.go
 *
 * Infrastructure for using the go_fims package to handle and send FIMS messages.
 *
 */

package main

import (
	"errors"
	"fims"
	"fmt"
	"os"
	"runtime/debug"

	build "github.com/flexgen-power/go_flexgen/buildinfo"
	log "github.com/flexgen-power/go_flexgen/logger"
)

var f *fims.Fims
var copsHeartbeat uint

var ErrInvalidUri = errors.New("invalid URI")

// configureFims configures a FIMS connection
func configureFims() (fimsReceive chan fims.FimsMsg, err error) {
	// Connect to FIMS
	fimsObj, err := fims.Connect("scheduler")
	if err != nil {
		return nil, fmt.Errorf("failed to connect to the FIMS server: %w", err)
	}
	f = &fimsObj

	// Subscribe to messages targeted at Scheduler
	baseUri := "/scheduler"
	if err = f.Subscribe(baseUri); err != nil {
		return nil, fmt.Errorf("failed to subscribe to FIMS server with URI %s: %w", baseUri, err)
	}

	// Start a FIMS Receive channel that will be used to hold incoming FIMS messages
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
	return fimsReceive, nil
}

// Used to streamline building an error response and sending it.
func sendErrorResponse(replyToUri, errMsg string) {
	sendReply(replyToUri, map[string]string{
		"error": errMsg,
	})
}

// Used to streamline responding to FIMS requests via the reply-to URI.
// Does nothing when replyToUri is empty string so caller does not have
// to check if reply-to URI was actually included in FIMS request.
func sendReply(replyToUri string, obj interface{}) {
	if replyToUri == "" {
		return
	}
	if err := f.SendSet(replyToUri, "", obj); err != nil {
		log.Errorf("Error replying to %s: %v.", replyToUri, err)
	}
}

func sendBackup(dbiUri string, data interface{}) {
	if err := f.SendSet(dbiUri, "", data); err != nil {
		log.Errorf("Error backing up to DBI URI %s: %v.", dbiUri, err)
	}
}

// processFims is the starting point for handling any and all incoming FIMS messages.
func processFims(msg fims.FimsMsg) {
	// put a recover here as insurance in case a FIMS message comes through that unexpectedly causes seg fault.
	// will keep Scheduler from totally crashing
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Recovered from panic during processing of %s FIMS message to URI %s: %v. Stack trace: %s\n", msg.Method, msg.Uri, r, string(debug.Stack()))
		}
	}()

	switch msg.Method {
	case "set":
		handleSet(msg)
	case "get":
		handleGet(msg)
	case "post":
		handlePost(msg)
	case "del":
		handleDel(msg)
	case "pub":
		// Publishes are ignored
	default:
		log.Errorf("FIMS message to URI %s has invalid method %s.", msg.Uri, msg.Method)
		sendErrorResponse(msg.Replyto, "Invalid Method")
	}
}

// Routes all received FIMS SETs to their appropriate handler function.
func handleSet(msg fims.FimsMsg) {
	if msg.Nfrags < 2 {
		log.Errorf("FIMS SET has URI %s but at least two fragments are required.", msg.Uri)
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return
	}
	var err error
	switch msg.Frags[1] {
	case "configuration":
		log.Infof("Received configuration SET to %s with payload %+v.", msg.Uri, msg.Body)
		err = handleConfigurationSet(msg)
	case "scada":
		err = handleScadaSet(msg)
	case "active_event_status":
		err = handleActiveEventStatusUpdate(msg)
	case "operation":
		err = handleOperationSet(msg)
	case "cops":
		err = handleCopsHeartbeatSet(msg)
	case "events":
		log.Infof("Received events SET to %s with payload %+v.", msg.Uri, msg.Body)
		err = masterSchedule.handleFimsSet(msg)
	case "modes":
		log.Infof("Received modes SET to %s with payload %+v.", msg.Uri, msg.Body)
		err = modes.handleSet(msg)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		err = ErrInvalidUri
	}
	if err != nil {
		log.Errorf("Error handling SET to %s: %v.", msg.Uri, err)
	}
}

// handleGet checks the endpoint of the GET message and replies with the appropriate data.
func handleGet(msg fims.FimsMsg) {
	if msg.Nfrags < 1 {
		log.Errorf("FIMS GET has URI %s but at least one fragment is required.", msg.Uri)
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return
	}
	if msg.Nfrags == 1 {
		sendReply(msg.Replyto, buildState())
		return
	}

	var err error
	switch msg.Frags[1] {
	case "events":
		err = masterSchedule.handleFimsGet(msg)
	case "modes":
		err = modes.handleGet(msg)
	case "scada":
		err = handleScadaGet(msg)
	case "cops":
		sendHeartBeat(msg.Replyto)
	case "configuration":
		err = schedCfg.handleGet(msg)
	case "connected":
		sendReply(msg.Replyto, buildConnectionMap())
	case "timezones":
		sendReply(msg.Replyto, buildTimezoneMap())
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		err = ErrInvalidUri
	}
	if err != nil {
		log.Errorf("Error handling GET to %s: %v.", msg.Uri, err)
	}
}

// handlePost routes all received FIMS messages that have the method post to their appropriate handler functions.
func handlePost(msg fims.FimsMsg) {
	log.Infof("Received POST to %s with payload %+v.", msg.Uri, msg.Body)
	if msg.Nfrags < 2 {
		log.Errorf("FIMS POST has URI %s but at least two fragments are required.", msg.Uri)
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return
	}
	var err error
	switch msg.Frags[1] {
	case "events":
		err = masterSchedule.handleFimsPost(msg)
	case "modes":
		err = modes.handlePost(msg)
	case "configuration":
		err = schedCfg.handlePost(msg)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		err = ErrInvalidUri
	}
	if err != nil {
		log.Errorf("Error handling POST to %s: %v.", msg.Uri, err)
	}
}

// handleDel routes all received FIMS messages that have the method del to their appropriate handler functions.
func handleDel(msg fims.FimsMsg) {
	log.Infof("Received DEL to %s.", msg.Uri)
	if msg.Nfrags < 2 {
		log.Errorf("FIMS DEL has URI %s but at least two fragments are required.", msg.Uri)
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return
	}
	var err error
	switch msg.Frags[1] {
	case "modes":
		err = modes.handleDel(msg)
	case "events":
		err = masterSchedule.handleFimsDel(msg)
	case "configuration":
		err = schedCfg.handleDel(msg)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		err = ErrInvalidUri
	}
	if err != nil {
		log.Errorf("Error handling DEL to %s: %v.", msg.Uri, err)
	}
}

// Handles FIMS SETs to URIs beginning with /scheduler/configuration.
func handleConfigurationSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// SET is to /scheduler/configuration
	if msg.Nfrags == 2 {
		if err := handleFullConfigurationSet(msg); err != nil {
			return fmt.Errorf("failed to handle full configuration SET: %w", err)
		}
		return nil
	}

	switch msg.Frags[2] {
	case "scada":
		if err := handleScadaConfigSet(msg); err != nil {
			return fmt.Errorf("failed to handle SCADA config SET: %w", err)
		}
	case "web_sockets":
		if err := handleWebSocketsConfigSet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSockets config SET: %w", err)
		}
	case "local_schedule":
		if err := handleLocalScheduleSet(msg); err != nil {
			return fmt.Errorf("failed to handle Local Schedule config SET: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles SETs to URIs beginning with /scheduler/operation.
func handleOperationSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("at least 3 frags required but only got %d", msg.Nfrags)
	}

	switch msg.Frags[2] {
	case "primary_controller":
		if err := handlePrimaryFlagSet(msg); err != nil {
			return fmt.Errorf("failed to handle primary flag SET: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// handlePrimaryFlagSet is the handler function for a FIMS SET to the URI /scheduler/operation/primary_controller.
// COPS set the Scheduler to be primary if the received flag is true.
func handlePrimaryFlagSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(bool)
	if !ok {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("type assertion of bool on /scheduler/operation/primary_controller message body failed")
	}

	// update isPrimaryScheduler if it is being changed
	if isPrimaryScheduler != body {
		log.Infof("Changing primary flag to %v.", body)
		isPrimaryScheduler = body
		if body {
			// did not use WebSockets as Secondary, so reconfigure to turn them on now that controller is Primary
			reconfigureWebSockets(schedCfg.WebSockets)
		}
	}
	sendReply(msg.Replyto, isPrimaryScheduler)
	return nil
}

// handleCopsHeartbeatSet is the handler function for a FIMS SET to the URI /scheduler/cops/cops_heartbeat.
// COPS has sent an updated heartbeat value that Scheduler should store so that when COPS sends a GET for it, Scheduler can verify it is alive.
func handleCopsHeartbeatSet(msg fims.FimsMsg) error {
	correctUri := "/scheduler/cops/cops_heartbeat"
	if msg.Uri != correctUri {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("%s is invalid COPS heartbeat URI. Expected %s", msg.Uri, correctUri)
	}

	// verify type of FIMS msg body
	body, ok := msg.Body.(float64) // ints seem to be converted to floats somewhere in the FIMS pipeline
	if !ok {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("type assertion of uint on %s message body failed", msg.Uri)
	}

	// update the internal COPS heartbeat value
	copsHeartbeat = uint(body)
	sendReply(msg.Replyto, copsHeartbeat)
	return nil
}

// handleActiveEventStatusUpdate handles FIMS SETs to URIs with the format /scheduler/active_event_status/<variable ID>.
// Compares the value to the setpoint it should be at, and sends an update SET if the value does not match the setpoint.
func handleActiveEventStatusUpdate(msg fims.FimsMsg) error {
	if msg.Nfrags != 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("3 frags required but got %d", msg.Nfrags)
	}
	varId := msg.Frags[2]
	if schedCfg.LocalSchedule == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return errors.New("no configured local schedule found")
	}
	// get pointer to schedule object
	localSchedule := masterSchedule[schedCfg.LocalSchedule.Id]
	if localSchedule == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("schedule with ID '%s' not found", schedCfg.LocalSchedule.Id)
	}

	if err := localSchedule.handleActiveEventStatusUpdate(varId, msg.Body); err != nil {
		sendErrorResponse(msg.Replyto, "Internal Server Error")
		return fmt.Errorf("failed to handle active event status update: %w", err)
	}
	sendReply(msg.Replyto, "Success")
	return nil
}

// Sends a SET to DBI and broadcasts a PUB with a map of each schedule's time zone.
func broadcastNewTimezones() {
	timeZoneMap := buildTimezoneMap()
	f.SendPub("/scheduler/timezones", timeZoneMap)
	f.SendSet("/dbi/scheduler/timezones", "", timeZoneMap)
}

// Sends COPS heartbeat info
func sendHeartBeat(uri string) {
	body := make(map[string]interface{})
	body["cops_heartbeat"] = copsHeartbeat
	body["pid"] = os.Getpid()
	if build.BuildVersion.Tag != "" {
		body["version_tag"] = build.BuildVersion.Tag
	} else {
		body["version_tag"] = "Invalid"
	}
	if build.BuildVersion.Commit != "" {
		body["version_commit"] = build.BuildVersion.Commit
	} else {
		body["version_commit"] = "Invalid"
	}
	if build.BuildVersion.Build != "" {
		body["version_build"] = build.BuildVersion.Build
	} else {
		body["version_build"] = "Invalid"
	}

	sendReply(uri, body)
}
