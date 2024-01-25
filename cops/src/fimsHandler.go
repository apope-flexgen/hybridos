// fimsHandler implements logic pertaining to fims messages
package main

import (
	"fims"
	"fmt"
	"path"
	"reflect"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Parse the PID out of a SET message
func parsePID(body interface{}) (receivedPID int, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, "pid", reflect.TypeOf(receivedPID))
	if errMsg != "" {
		return 0, errMsg
	}
	return int(extractedVal.Int()), ""
}

// Starting point for handling any and all incoming FIMS messages
func processFIMS(msg fims.FimsMsg) error {
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Error processing FIMS message: %v", r)
		}
	}()
	switch msg.Method {
	case "set":
		err := handleSet(msg)
		if err != nil {
			if configErr, ok := err.(*configurationError); ok {
				log.Fatalf("Fatal error: %v", configErr)
			} else {
				return fmt.Errorf("handling set: %w", err)
			}
		}
	case "get":
		if err := handleGet(msg); err != nil {
			return fmt.Errorf("handling FIMs get: %w", err)
		}
	default:
		log.Warnf("Received FIMS message that was not a SET. No action taken.")
	}

	return nil
}

// Check if the controller was previously put into update mode
// If no response is received, simply continue
func getDBIUpdateModeStatus() {
	err := f.SendGet("/dbi/cops/update_mode", "/cops/update_mode")
	if err != nil {
		log.Errorf("error getting update_mode status from dbi: %v", err)
	}
}

// Call function to actively poll FIMS data for connection status of a service.
func getClientConnectionStatus(uri string, replyTo string) error {
	err := f.SendGet(uri, replyTo)
	if err != nil {
		return fmt.Errorf("retrieving connection status at uri: %s: %w", uri, err)
	}
	return nil
}

// As the primary, the other controller (secondary) has requested our latest setpoints on its startup
func getDBIUpdate() {
	for _, process := range processJurisdiction {
		// Get all the latest config from DBI
		for _, writeOutUri := range process.writeOutC2C {
			// Get whatever is stored in DBI for each of the process's write out uris
			_, data := path.Split(writeOutUri)
			// The response will be returned back to this COPS with the final fragment dbi_update from our replyto
			// Then written through to the other controller via C2C
			f.SendGet(path.Join("/dbi/", process.name, data), path.Join(writeOutUri, "/dbi_update"))
		}
		// The above will produce an undetermined number of responses back to cops with the keyword dbi_update
		// To resolve the issue of uncertainty as to when the last response has been received, send one additional response
		// that will be handled last, and mark the end of the documents response for this process
		// Will produce the response: SET /cops/<process name>/dbi_update_complete {}
		f.SendGet("/dbi/update/complete", path.Join("/cops/", process.name, "/dbi_update_complete"))
	}
}

// Put all FIMS GET hooks here
func handleGet(msg fims.FimsMsg) error {
	switch {
	case msg.Uri == "/cops/processStats":
		f.SendSet(msg.Replyto, "", buildHealthStatsMap())
	case msg.Uri == "/cops/stats":
		f.SendSet(msg.Replyto, "", buildHealthStatsMap())
	case msg.Uri == "/cops/healthScore":
		f.SendSet(msg.Replyto, "", dr.healthCheckup())
	case msg.Uri == "/cops/status":
		f.SendSet(msg.Replyto, "", statusNames[controllerMode])
	case msg.Uri == "/cops/controller_name":
		f.SendSet(msg.Replyto, "", controllerName)
	case msg.Uri == "/cops":
		f.SendSet(msg.Replyto, "", buildBriefingReport())
	case strings.HasPrefix(msg.Uri, "/cops/stats/"):
		if err := handleGetStats(msg); err != nil {
			return fmt.Errorf("getting cops stats: %w", err)
		}
	default:
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}
	return nil
}

// Put all FIMS SET hooks here
func handleSet(msg fims.FimsMsg) error {
	switch {
	// Set received from UI or command line enabling or disabling Update mode
	case msg.Uri == "/cops/update_mode":
		return handleUpdateMode(msg)
	case msg.Nfrags == 3 && strings.HasPrefix(msg.Uri, "/cops/connection"):
		// Handle monitoring connect/disconnect.
		return handleConnectionStatus(msg)
	case msg.Uri == "/cops/configuration":
		// verify type of FIMS msg body
		body, ok := msg.Body.(map[string]interface{})
		if !ok {
			return &configurationError{fmt.Errorf("expected DBI configuration of the form map[string]interface{}, but received %T", body)}
		}
		// Ignore empty response {} from DBI
		if len(body) == 0 {
			return &configurationError{fmt.Errorf("expected DBI configuration document, but it was empty")}
		}
		return &configurationError{handleConfigBody(body)}

	// Check URI is one of /cops/stats/<processName> to enact an action.
	case msg.Nfrags == 5 && strings.Contains(msg.Uri, "controls"):
		if config.AllowActions {
			// TODO: Handle fims reply to for errors from handlecontorlMsg
			if err := handleControlMsg(msg); err != nil {
				f.SendSet(msg.Replyto, "", fmt.Sprintf("error handling control set: %v", err))
				return fmt.Errorf("handling control message: %v", err)
			}
		} else {
			// TODO: convert to reply to message. tech debt ticket.
			return fmt.Errorf("Received a FIMS set to take an action on process %s but actions are not allowed. %s",
				msg.Frags[2], "Please configure the COPS JSON file to allow actions.")
		}

	// Check for DBI update response first
	// Check URI is one of /cops/<process_name>/dbi_update_complete or <writeouturi>/dbi_update
	case msg.Nfrags == 3 && strings.HasSuffix(msg.Uri, "dbi_update_complete"):
		_, processKnown := processJurisdiction[msg.Frags[1]]
		if processKnown {
			handleDBIUpdate(msg)
			return nil
		}

	case msg.Nfrags == 3 && msg.Frags[1] == "heartbeat":
		process := processJurisdiction[msg.Frags[2]]
		if err := handleHeartbeatReply(msg, process); err != nil {
			return fmt.Errorf("failed to handle heartbeat reply from process %s: %w", msg.Frags[2], err)
		}
		return nil

	// This check of the uri against <writeoutui>/dbi_update needs to happen before the check used
	// for Setpoints messages against just the writeOut uri, otherwise this check won't be reached
	case strings.HasSuffix(msg.Uri, "dbi_update"):
		for _, process := range processJurisdiction {
			// If more efficiency is needed, the writeout uris could be put into a hashset at configuration to speed up this check
			for _, writeOutUri := range process.writeOutC2C {
				if strings.HasPrefix(msg.Uri, writeOutUri) {
					handleDBIUpdate(msg)
					return nil
				}
			}
		}
	// Cops heartbeat response and setpoint
	// Check uri is of form /cops/<process_name>
	// TODO: give heartbeats a more specific endpoint, but this will require changes in other services

	case msg.Nfrags > 1:
		process, processKnown := processJurisdiction[msg.Frags[1]]
		if processKnown {
			// Check uri contains one of <writeouturi>
			for _, writeOutUri := range process.writeOutC2C {
				if strings.Contains(msg.Uri, writeOutUri) {
					handleSetpointWriteout(msg, process)
					return nil
				}
			}
		}

	default:
		// If not caught by any of the above checks, then the set is invalid
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}
	return nil
}

// Handle reporting connection status for a given process.
func handleConnectionStatus(msg fims.FimsMsg) error {
	// Process name in uri is built off existing process. Guranteed to always exist.
	process := processJurisdiction[msg.Frags[2]]

	// Check for connection status.
	if m, ok := msg.Body.(map[string]interface{}); ok {
		// Update process connection information
		isConnected := m["component_connected"].(float64) != 0.0
		if isConnected {
			process.connected = true
			process.lastUpdate = time.Now()
			return nil
		} else {
			process.connected = false
		}
	} else {
		return fmt.Errorf("fims body is not a map: %v", m)
	}

	return nil
}

// Expose gets on all stats.
func handleGetStats(msg fims.FimsMsg) error {
	switch {
	case msg.Nfrags == 3:
		// Get process info: /cops/stats/<process>
		if err := handleGetProcess(msg); err != nil {
			return fmt.Errorf("getting process: %w", err)
		}
	case msg.Nfrags > 3:
		// Get process statistic: /cops/stats/<process>/<item>
		if err := handleGetProcessItem(msg); err != nil {
			return fmt.Errorf("getting process item: %w", err)
		}
	default:
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}

	return nil
}

// Handle URIs targeted at specific process stats.
func handleGetProcessItem(msg fims.FimsMsg) error {
	switch {
	case msg.Nfrags == 4:
		// Handle retrieval of statistics: /cops/stats/<process>/<item>
		if err := handleGetProcessStatistic(msg); err != nil {
			return fmt.Errorf("getting stat: %w", err)
		}
	case msg.Nfrags == 5 && strings.Contains(msg.Uri, "controls"):
		// Get actions map: /cops/stats/<process>/controls/<action>
		if err := handleGetProcessControlsAction(msg); err != nil {
			return fmt.Errorf("getting controls: %w", err)
		}
	case msg.Nfrags == 6 && strings.Contains(msg.Uri, "controls"):
		// Get actions enabled status: /cops/stats/<process>/controls/<action>/enabled
		if err := handleGetProcessControl(msg); err != nil {
			return fmt.Errorf("getting controls: %w", err)
		}
	default:
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}

	return nil
}

// Reply with a given process enabled status.
func handleGetProcessControl(msg fims.FimsMsg) error {

	// Determine which control to get.
	switch {
	case msg.Frags[5] == "value":
		if err := handleGetProcessControlValue(msg); err != nil {
			return fmt.Errorf("getting control value: %w", err)
		}
	case msg.Frags[5] == "enabled":
		if err := handleGetProcessControlEnabled(msg); err != nil {
			return fmt.Errorf("getting enabled status: %w", err)
		}
	default:
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}

	return nil
}

// Send a reply to with process information.
func handleGetProcess(msg fims.FimsMsg) error {

	// Get process name.
	processName := msg.Frags[2]

	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[processName]; !ok {
		f.SendSet(msg.Replyto, "", "process does not exist.")
		return fmt.Errorf("process: %s does not exist", processName)
	}

	// Send reply to with stats report on proces
	process := processJurisdiction[processName]

	// Reply with designated process.
	f.SendSet(msg.Replyto, "", process.buildStatsReport())
	return nil
}

// Handle gets on controls for a process.
func handleGetProcessStatistic(msg fims.FimsMsg) error {

	// Get process name.
	processName := msg.Frags[2]

	// Get the item.
	item := msg.Frags[3]

	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[processName]; !ok {
		f.SendSet(msg.Replyto, "", "process does not exist.")
		return fmt.Errorf("process: %s does not exist", processName)
	}

	// Retrieve stats pertaining to the process.
	stats := processJurisdiction[processName].buildStatsReport()

	// Verify the item exists in the statistics report.
	if _, ok := stats[item]; !ok {
		f.SendSet(msg.Replyto, "", "statistic does not exist.")
		return fmt.Errorf("statistic: %s does not exist", item)
	}

	// Send reply with specified statistic.
	f.SendSet(msg.Replyto, "", stats[item])
	return nil
}

// Return map of a given action.
func handleGetProcessControlsAction(msg fims.FimsMsg) error {
	// /cops/stats/<process>/controls/<action>
	processName := msg.Frags[2]
	action := msg.Frags[4]

	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[processName]; !ok {
		f.SendSet(msg.Replyto, "", "process does not exist.")
		return fmt.Errorf("process: %s does not exist", processName)
	}

	// Retrieve controls pertaining to the process.
	controlsMap := processJurisdiction[processName].generateControlsMap()

	// Verify the action exists in the process controls.
	if _, ok := controlsMap[action]; !ok {
		f.SendSet(msg.Replyto, "", "action does not exist")
		return fmt.Errorf("action: %s does not exist", action)
	}

	// Send reply with specified actions map.
	f.SendSet(msg.Replyto, "", controlsMap[action])
	return nil

}

// Return value of control enabled status.
func handleGetProcessControlEnabled(msg fims.FimsMsg) error {
	// /cops/stats/<process>/controls/<action>/enabled
	processName := msg.Frags[2]
	action := msg.Frags[4]

	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[processName]; !ok {
		f.SendSet(msg.Replyto, "", "process does not exist.")
		return fmt.Errorf("process: %s does not exist", processName)
	}

	// Get controls information.
	controls := processJurisdiction[processName].enableControls

	// Verify action is a valid action.
	switch action {
	case "start":
		f.SendSet(msg.Replyto, "", controls.startEnabled)
	case "stop":
		f.SendSet(msg.Replyto, "", controls.stopEnabled)
	case "restart":
		f.SendSet(msg.Replyto, "", controls.restartEnabled)
	default:
		return fmt.Errorf("message URI: \"%s\" doesn't match any known patterns", msg.Uri)
	}

	return nil
}

// Retrieve the value of a given process control action.
func handleGetProcessControlValue(msg fims.FimsMsg) error {
	// /cops/stats/<process>/controls/<action>/value
	processName := msg.Frags[2]
	action := msg.Frags[4]
	item := msg.Frags[5]

	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[processName]; !ok {
		f.SendSet(msg.Replyto, "", "process does not exist.")
		return fmt.Errorf("process: %s does not exist", processName)
	}

	// Retrieve controls pertaining to the process.
	controlsMap := processJurisdiction[processName].generateControlsMap()

	// Verify the action exists in the process controls.
	if _, ok := controlsMap[action]; !ok {
		f.SendSet(msg.Replyto, "", "action does not exist")
		return fmt.Errorf("action: %s does not exist", action)
	}

	// No need for check - will always be a map[string]interface{}
	// as it's defined as such in generateControlsMap()
	actionVals := controlsMap[action].(map[string]interface{})

	// Verify the value exists in the process controls.
	if _, ok := actionVals[item]; !ok {
		f.SendSet(msg.Replyto, "", "value does not exist")
		return fmt.Errorf("value does not exist")
	}

	f.SendSet(msg.Replyto, "", actionVals[item])
	return nil
}

// Handle sets on a control action for a defined process.
// Function verifies process is defined in the COPS file, extracts
// control value and process from the FIMS message, and checks
// to make sure actions are allowed for the process.
func handleControlMsg(msg fims.FimsMsg) error {
	// Expected URI format: /cops/stats/[process]/controls/[action]
	process := msg.Frags[2]

	// Validate process exists and actions are enabled.
	if err := validateProcessControls(process); err != nil {
		return fmt.Errorf("validating controls message: %w", err)
	}

	// Extract action command type: start, stop, restart.
	cmd, err := getSystemdCmd(msg)
	if err != nil {
		return fmt.Errorf("retrieving control type for process %s: %w", process, err)
	}

	// Handle the control type for the defined process.
	if err := handleSystemdCmd(cmd, msg); err != nil {
		return fmt.Errorf("handling command \"%s\" for %s: %w", cmd, process, err)
	}

	return nil
}

// Extract control command from a FIMS body.
// TODO: streamline getting the value from a naked set with how scheduler does it.
func getSystemdCmd(msg fims.FimsMsg) (string, error) {
	var value bool
	var ok bool

	// Expected command stored as URI fragment in 5th position:
	// Example: /cops/stats/[process]/controls/[action]
	action := msg.Frags[4]

	// Handle the FIMS body whether it's a naked set or a map.
	// This if statement is quite ugly. I'd like to find a better way to do this.
	if _, ok = msg.Body.(bool); ok {
		// Handle if FIMS set sends a single boolean value.
		value = msg.Body.(bool)
	} else if _, ok = msg.Body.(map[string]interface{}); ok {
		// Handle FIMS set using a key value pair.
		msgBody := msg.Body.(map[string]interface{})

		// Return error on an empty body.
		if len(msgBody) == 0 {
			return "", fmt.Errorf("body of FIMS message is empty")
		}

		// Verify we received a value of true on the action.
		value, ok = msgBody["value"].(bool)
		if !ok {
			return "", fmt.Errorf("command \"%s\" received non boolean value in FIMS set", action)
		}
	} else {
		// Handle exception of incorrect set format.
		return "", fmt.Errorf("invalid FIMS body. Expected body is key-value pair or boolean value")
	}

	// Verify our value is actually true for enacting the command.
	if !value {
		return "", fmt.Errorf("FIMS set command \"%s\" to false - no action taken", action)
	}

	return action, nil
}

// Handle a set to update mode coming from either ansible or dbi
func handleUpdateMode(msg fims.FimsMsg) error {
	var interfaceBody interface{}
	// Parse response from dbi that will be parsed as map[string]interface{}
	if mapBody, ok := msg.Body.(map[string]interface{}); ok {
		// Ignore empty response when no document exists
		if len(mapBody) == 0 {
			return nil
		}
		if interfaceBody, ok = mapBody["update_mode"]; !ok {
			return fmt.Errorf("failed to extract update_mode entry from map %v", mapBody)
		}
	} else {
		interfaceBody = msg.Body
	}
	// Parse single bool sent from ansible or by hand
	body, ok := interfaceBody.(bool)
	if !ok {
		return fmt.Errorf("expected Update Mode value to be a bool, but received %T instead", msg.Body)
	}

	// Save the update mode bool received
	f.SendSet("/dbi/cops/update_mode", "", map[string]interface{}{"update_mode": body})

	// Update mode enabled
	if body {
		log.Infof("Locking this controller into update mode")
		controllerMode = Update
	} else if controllerMode == Update {
		log.Infof("Exiting this controller from update mode")
		// Received exit update mode signal
		for _, process := range updatedProcesses {
			// If the process requires a restart in order to properly configure such as site controller, restart it
			if process.configRestart {
				// The process may have no fully started up with a PID yet, delay it's restart then kill to ensure
				// all of the latest configuration has been read
				err := kill(process)
				if err != nil {
					log.Errorf("Failed to send kill signal to %s in attempt to ensure latest configuration: %v", process.name, err)
				}
			} else {
				// Notify the process's (optional) dbi_update endpoint
				// This endpoint is used for processes that support reading configuration data on the fly such as scheduler
				f.SendSet(path.Join(process.uri, "/operation/dbi_update"), "", true)
			}
		}
		// Clear the list of updated processes, processes still maintained in processJurisdiction
		updatedProcesses = nil
		// Now that this controller has fully configured with new data, take over as primary
		// This will also sync the db of other controller (now secondary) with this controller's updated db
		takeOverAsPrimary()
	}
	return nil
}

// Pub all process health statistics over FIMS
func publishBriefing() {
	// prevent a failure here from crashing all of COPS
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Error with briefing publish: %v", r)
		}
	}()
	// publish summary data
	f.SendPub("/cops/summary", buildBriefingReport())
	// publish process health statistics
	for _, process := range processJurisdiction {
		f.SendPub("/cops/stats/"+process.name, process.buildStatsReport())
	}
}

// Builds a map containing general information desired by external processes
func buildBriefingReport() map[string]interface{} {
	briefingReport := make(map[string]interface{})
	// Add the controller info
	if len(controllerName) != 0 {
		briefingReport["controller_name"] = controllerName
	}
	briefingReport["status"] = statusNames[controllerMode]
	// Add the temperature only if a source is provided
	if len(tempSource) != 0 {
		systemTemp, err := readSystemTemp()
		if err != nil {
			log.Errorf("Error: %v", err)
		} else {
			briefingReport["system_temp"] = systemTemp
		}
	}
	return briefingReport
}
