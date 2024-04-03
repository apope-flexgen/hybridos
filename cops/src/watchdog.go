// Implements watchdog logic on process defined in the provided list
package main

import (
	"fims"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Initialize to true. If systemd does not exist on initial try, it will be set to false.
var isDbusValid bool = true

// Define severity level for sending events
const (
	base   = iota // 0
	Info          // 1
	Status        // 2
	Alarm         // 3
	Fault         // 4
)

// Print a message if there is a fatal error and panic
func fatalErrorCheck(err error, message string) {
	if err != nil {
		log.Panicf("Panicking. Error: %v, Message: %s", err, message)
	}
}

// Checks health status of each process and calls failure actions if necessary
func patrolProcesses() error {
	for _, process := range processJurisdiction {
		if process.isStillHungOrDead() {
			log.Infof("Failure: Process %s still dead.", process.name)
		} else if process.isResurrected() {
			log.Infof("Success: Process %s resurrected.", process.name)
			process.alive = true
			process.sendPrimaryFlag(controllerMode == Primary)
		} else if process.isHungOrDead() && process.requiredForHealthyStatus {
			// Only handle failure actions for processes with heartbeats. Current expectation is that
			// failure actions are only performed on COPS processes that have heartbeats.
			takeFailureAction(process)
		}

		// Update the service status for a given process
		if isDbusValid {
			if err := process.updateStatus(); err != nil {
				return fmt.Errorf("updating service %v status: %w", process.name, err)
			}
		}

		// If connection reporting is enabled, retrieve connection information and report.
		if process.enableConnectionStatus {
			if err := process.pollConnectionStatus(); err != nil {
				return fmt.Errorf("polling connection status for process %v: %w", process.name, err)
			}
		}
	}

	// Patrol /site/configuration/configured_primary for any change in status
	if err := getCfgPrimaryUpdate(); err != nil {
		return fmt.Errorf("getting controller's configured primary flag: %w", err)
	}

	return nil
}

// Takes necessary failure actions on a process that has been newly declared hung or dead
func takeFailureAction(process *processInfo) {
	log.Infof("Failure: Process %s declared hung or dead. Last confirmed alive at %s. Sending alarm notification to events.", process.name, process.healthStats.lastConfirmedAlive.String())
	processHungMsg := fmt.Sprintf("Process %s is hung or dead (last confirmed alive at %v).", process.name, process.healthStats.lastConfirmedAlive.Round(time.Millisecond))
	if process.killOnHang {
		sendEvent(Alarm, processHungMsg+" Sending kill signal.")
		err := kill(process)
		if err != nil {
			log.Errorf("Failed to send kill signal to %s: %v", process.name, err)
			sendEvent(Alarm, fmt.Sprintf("Failed to send kill signal to process %s.", process.name))
		} else {
			sendEvent(Alarm, fmt.Sprintf("Successfully sent kill signal to process %s.", process.name))
			process.healthStats.copsRestarts++
		}
	} else {
		sendEvent(Alarm, processHungMsg)
	}
	process.alive = false
}

// Sends an alarm notification to the events module via FIMS
func sendEvent(severity int, message string) {
	// Build notification JSON
	body := make(map[string]interface{})
	body["source"] = "COPS"
	body["message"] = message
	body["severity"] = severity

	// Send notification
	_, err := f.Send(fims.FimsMsg{
		Method: "post",
		Uri:    "/events",
		Body:   body,
	})
	if err != nil {
		log.Errorf("Error sending event notificiation: %v", err)
	}
}
