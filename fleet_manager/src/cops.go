/*
 * cops.go
 *
 * Functions and variables related to the COPS process watchdog and redundant failover.
 *
 */

package main

import (
	"fims"
	"fmt"
	"os"

	build "github.com/flexgen-power/go_flexgen/buildinfo"
)

// inPrimaryMode indicates if the box Fleet Manager is running on is operating in primary mode.
// At startup, Fleet Manager assumes secondary mode until it is told otherwise by COPS.
var inPrimaryMode bool = false

// heartbeat counter to keep track of for COPS
var cops_heartbeat uint

// sendHeartbeat packages Fleet Manager's COPS heartbeat and PID into an object and sends it to COPS.
func sendHeartBeat(uri string) error {
	body := make(map[string]interface{})
	body["cops_heartbeat"] = cops_heartbeat
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

	return f.SendSet(uri, "", body)
}

// handleCopsHeartbeatSet is the handler function for a FIMS SET to the URI /fleet/cops/cops_heartbeat.
// COPS has sent an updated heartbeat value that Fleet Manager should store so that when COPS sends a GET for it, Fleet Manager can verify it is alive.
func handleCopsHeartbeatSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(float64) // ints are converted to floats during FIMS unmarshalling
	if !ok {
		return fmt.Errorf("type assertion of uint on /fleet/cops/cops_heartbeat message body failed")
	}

	// update the internal COPS heartbeat value
	cops_heartbeat = uint(body)
	return nil
}

// handlePrimaryFlagSet is the handler function for a FIMS SET to the URI /fleet/operation/primary_controller.
// COPS set the Fleet Manager to be primary if the received flag is true.
func handlePrimaryFlagSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(bool)
	if !ok {
		return fmt.Errorf("type assertion of bool on /fleet/operation/primary_controller message body failed")
	}

	// update inPrimaryMode if it is being changed
	if inPrimaryMode != body {
		if body {
			getDbiData() // if taking over as primary, send GET to DBI to get latest data
		}
		inPrimaryMode = body
	}
	return nil
}
