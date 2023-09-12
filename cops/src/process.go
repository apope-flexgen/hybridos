// Implement logic/methods pertaining to process tracking
package main

import (
	"fmt"
	"os"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Contains release version info about a process
type processVersion struct {
	tag    string
	commit string
	build  string
}

// Information about each process used and updated by COPS
type processInfo struct {
	name                     string             // process name
	uri                      string             // URI of the data structure holding the COPS info at the process
	replyToURI               string             // URI that heartbeat replies from this process to COPS will use
	writeOutC2C              []string           // URIs that should be written out to C2C by primary to maintain data
	heartbeat                uint               // Current COPS heartbeat value
	pid                      int                // Process ID used to update processPtr
	processPtr               *os.Process        // Pointer to system process which can be used to kill process
	alive                    bool               // True if process is passing heartbeat checks. False if process is dead/hung
	killOnHang               bool               // if true, send a kill command to the process pointer if process fails check
	requiredForHealthyStatus bool               // if true, process is included in health checkup to determine if controller is healthy or unhealthy
	hangTimeAllowance        time.Duration      // duration of time process can be missing the heartbeat before declared hung/dead
	configRestart            bool               // Whether new configuration data requires the process to restart
	healthStats              processHealthStats // statistics to be published about the process's health
	version                  processVersion     // contains release version info about the process
}

// Returns if process is hung or dead
func (process processInfo) isHungOrDead() bool {
	return time.Since(process.healthStats.lastConfirmedAlive) > process.hangTimeAllowance
}

// Returns if process is marked as dead but is neither hung nor dead
func (process processInfo) isResurrected() bool {
	return !process.isHungOrDead() && !process.alive
}

// Returns if process that is marked as alive is still neither hung nor dead
func (process processInfo) isStillAlive() bool {
	return !process.isHungOrDead() && process.alive
}

// Returns if process that is marked as dead is still either hung or dead
func (process processInfo) isStillHungOrDead() bool {
	return process.isHungOrDead() && !process.alive
}

// `getProcess` searches the processJurisdiction for a process with the given
// name and returns a pointer to it. If not found, returns nil.
func getProcess(name string) *processInfo {
	for _, process := range processJurisdiction {
		if process.name == name {
			return process
		}
	}
	return nil
}

// Update the process pointer if a new PID is received. Record restart data
func (process *processInfo) updatePID(readPID int) {
	if process.pid != readPID {
		var err error
		process.pid = readPID
		process.processPtr, err = os.FindProcess(readPID)
		fatalErrorCheck(err, "Error updating process pointer for "+process.name)
		process.recordRestart()
		if controllerMode == Primary {
			process.sendPrimaryFlag(true)
		}
	}
}

// Sends a system kill command targeting the passed-in process
func kill(process *processInfo) error {
	if process.processPtr == nil {
		return fmt.Errorf("process pointer is nil, not possible to send kill command to %s", process.name)
	}
	err := process.processPtr.Kill()
	if err != nil {
		return fmt.Errorf("error when trying to kill %s: %w", process.name, err)
	}
	return nil
}

// Kill the process after `delay` milliseconds have expired
func delayedkill(process *processInfo, delay int) {
	time.Sleep(time.Duration(delay) * time.Millisecond)
	err := kill(process)
	if err != nil {
		log.Errorf("Failed to send delayed kill signal to %s: %v", process.name, err)
	}
}
