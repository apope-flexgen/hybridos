// Implements watchdog logic on process defined in the provided list
package main

import (
	"fims"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
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
		} else if process.isHungOrDead() {
			takeFailureAction(process)
		}

		// Update the service status for a given process
		if err := process.updateStatus(); err != nil {
			return fmt.Errorf("updating service %v status: %w", process.name, err)
		}
	}

	return nil
}

// Takes necessary failure actions on a process that has been newly declared hung or dead
func takeFailureAction(process *processInfo) {
	log.Infof("Failure: Process %s declared hung or dead. Last confirmed alive at %s. Sending alarm notification to events.", process.name, process.healthStats.lastConfirmedAlive.String())
	processHungMsg := fmt.Sprintf("Process %s is hung or dead (last confirmed alive at %v).", process.name, process.healthStats.lastConfirmedAlive.Round(time.Millisecond))
	if process.killOnHang {
		sendAlarm(processHungMsg + " Sending kill signal.")
		err := kill(process)
		if err != nil {
			log.Errorf("Failed to send kill signal to %s: %v", process.name, err)
			sendAlarm(fmt.Sprintf("Failed to send kill signal to process %s.", process.name))
		} else {
			sendAlarm(fmt.Sprintf("Successfully sent kill signal to process %s.", process.name))
			process.healthStats.copsRestarts++
		}
	} else {
		sendAlarm(processHungMsg)
	}
	process.alive = false
}

// Sends an alarm notification to the events module via FIMS
func sendAlarm(message string) {
	// Build notification JSON
	body := make(map[string]interface{})
	body["source"] = "COPS"
	body["message"] = message
	body["severity"] = 3 // severity of 3 is for alarms

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
