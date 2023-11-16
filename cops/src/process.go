// Implement logic/methods pertaining to process tracking
package main

import (
	"fmt"
	"os"
	"time"

	dbus "github.com/coreos/go-systemd/dbus"
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
	unitFileState            string             // encodes the install state of the unit file retrieved from systemd
	serviceStatus            string             // Stores value of systemctl status of active and sub state on a given process
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
func (process *processInfo) updatePID(readPID int) error {
	var err error
	if process.pid != readPID {
		process.pid = readPID
		process.processPtr, err = os.FindProcess(readPID)
		if err != nil {
			return fmt.Errorf("error updating process pointer for %v: %w", process.name, err)
		}
		process.recordRestart()
		if controllerMode == Primary {
			process.sendPrimaryFlag(true)
		}
	}
	return nil
}

// Establish temporary connection to dbus and retrieve systemctl status info on a given process.
// Update the service time since it was last restarted here, as we already establish a connection
// to retrieve that value.
func (process *processInfo) updateStatus() error {
	// Open dbus connection.
	conn, err := dbus.New()
	if err != nil {
		return fmt.Errorf("failed to connect to D-Bus: %w", err)
	}
	defer conn.Close()

	// Generate service name.
	service := fmt.Sprintf("%s.service", process.name)

	// Update our process status.
	status, unitFileState, err := getUnitState(conn, service)
	if err != nil {
		return fmt.Errorf("getting service %v status: %w", service, err)
	}
	process.serviceStatus = status
	process.unitFileState = unitFileState

	// Update our process timestamp and elapsed time.
	timeUint64, err := getUnitTime(conn, service)
	if err != nil {
		return fmt.Errorf("getting timestamp for service: %s: %w", service, err)
	}

	// Verify our service is actually running to report runtime values.
	if timeUint64 == 0 {
		process.healthStats.elapsedTimeSinceRestart = ""
	} else {
		// Format our time accordingly and store the values to health statistics.
		timeUnix := convertMicrosecondsToUnixTime(timeUint64)
		process.healthStats.timestampOfLastRestart = timeUnix.Format("01-02-2006 15:04:05")
		process.healthStats.elapsedTimeSinceRestart = formatUnixDuration(timeUnix)
	}

	return nil
}

// Get the service status and return in the format of "[ActiveState] ([SubState])".
func getUnitState(conn *dbus.Conn, service string) (string, string, error) {
	var status string
	// Retrieve service properties.
	// If the .service file does not exist, it will return "inactive" for its state.
	stats, err := conn.GetUnitProperties(service)
	if err != nil {
		return "", "", fmt.Errorf("failed to get unit properties list: %v", err)
	}

	// Assign the newly retrieved value to its process information.
	if stats["ActiveState"] == nil || stats["SubState"] == nil {
		return "", "", fmt.Errorf("%s active and sub states are nil", service)
	}
	// Generate out status string.
	status = fmt.Sprintf("%s (%s)", stats["ActiveState"].(string), stats["SubState"].(string))

	// Retrieve enable/disabled state of the unit file.
	if stats["UnitFileState"] == nil {
		return status, "", fmt.Errorf("%s unit file state is nil", service)
	}
	unitFileState := stats["UnitFileState"].(string)

	return status, unitFileState, nil
}

// Get the time integer value for a given service since it was started.
func getUnitTime(conn *dbus.Conn, service string) (uint64, error) {
	var TimeUint64 uint64

	// Retrieve service properties.
	// If the .service file does not exist, it will return "inactive" for its state.
	stats, err := conn.GetUnitProperties(service)
	if err != nil {
		return 0, fmt.Errorf("Failed to get service status: %v", err)
	}

	if stats["ActiveEnterTimestamp"] != nil {
		// Retrieve Time since active.
		// Requires some calculations to convert from integer to proper format.
		TimeUint64 = stats["ActiveEnterTimestamp"].(uint64)
	} else {
		return 0, fmt.Errorf("service [time since last restart] property for %v is not available", service)
	}

	return TimeUint64, nil
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
