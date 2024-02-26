// Implement logic/methods pertaining to process tracking
package main

import (
	"fims"
	"fmt"
	"os"
	"strings"
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

// Define enabled statuses for processes based on runtime state of process.
type processControls struct {
	startEnabled   bool
	stopEnabled    bool
	restartEnabled bool
}

// Information about each process used and updated by COPS
type processInfo struct {
	name                     string // process name
	uri                      string // URI of the data structure holding the COPS info at the process
	allowActions             bool   // Configurable value for whether or not actions can be taken on the process
	enableConnectionStatus   bool   // Configure whether to report connection status for a process. Mainly for modbus/dnp
	connected                bool
	lastUpdate               time.Time          // Store most recently updated connection status for process
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
	dependencies             []string           // Lists the dependencies required for the process to run
	healthStats              processHealthStats // statistics to be published about the process's health
	version                  processVersion     // contains release version info about the process
	enableControls           processControls    // defines bools for start,stop,restart commands depending on state of process
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

// Send poll to retrieve connection information for a given client.
func (process *processInfo) pollConnectionStatus() error {
	// Update current reporting connection status
	waitTime := time.Duration(config.ConnectionHangtimeAllowance / 1000)
	if time.Since(process.lastUpdate) > waitTime*time.Second {
		// Auto timeout connected status after a configurable wait time.
		if process.connected {
			process.connected = false
			log.Infof("Process %s is disconnected \n", process.name)
		}
	}

	// Build URI and replyto message.
	replyTo := fmt.Sprintf("/cops/connection/%s", process.name)

	// Send FIMS request to get back info on component.
	if err := getClientConnectionStatus(process.uri, replyTo); err != nil {
		return fmt.Errorf("getting client connection status: %w", err)
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

// Capture dependency list from systemd and update the list in a given process
func (process *processInfo) updateDependencies() error {
	procDeps := []string{}

	// Open dbus connection.
	conn, err := dbus.New()
	if err != nil {
		return fmt.Errorf("failed to connect to D-Bus: %w", err)
	}
	defer conn.Close()

	// Generate service name.
	service := fmt.Sprintf("%s.service", process.name)

	// Update our process status.
	stats, err := conn.GetAllProperties(service)
	if err != nil {
		return fmt.Errorf("getting service %v status: %w", service, err)
	}

	// Verify property field exists
	// Assign the newly retrieved value to its process information.
	if stats["After"] == nil {
		return fmt.Errorf("%s dependency list is nil", service)
	}

	// Initialize and set our list of dependencies back to the process.
	// Only select .service dependencies
	dependencies := stats["After"].([]string)
	for _, dep := range dependencies {
		if strings.Contains(dep, "service") {
			procDeps = append(procDeps, dep)
		}
	}

	process.dependencies = procDeps

	return nil
}

// Update our process control booleans each time its status updates.
func (process *processInfo) updateEnableControls(status string) {
	// Disable all actions for UI to read if not allowed.
	if !process.allowActions {
		process.enableControls.startEnabled = false
		process.enableControls.stopEnabled = false
		process.enableControls.restartEnabled = false
		return
	}

	// If active, set start enable to false.
	if strings.Contains(status, "running") || strings.Contains(status, "activating") {
		process.enableControls.startEnabled = false
		process.enableControls.stopEnabled = true
		process.enableControls.restartEnabled = true
	} else if strings.Contains(status, "dead") {
		process.enableControls.startEnabled = true
		process.enableControls.stopEnabled = false
		process.enableControls.restartEnabled = true
	} else if strings.Contains(status, "failed") {
		process.enableControls.startEnabled = true
		process.enableControls.stopEnabled = false
		process.enableControls.restartEnabled = true
	}
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

	// Update controls status based on process status
	process.updateEnableControls(status)

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
		return 0, fmt.Errorf("failed to get service status: %w", err)
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

// Validate a process is eligible to enact a systemctl action.
func validateProcessControls(process string) error {
	// Verify the process exists in COPS config file.
	if _, ok := processJurisdiction[process]; !ok {
		return fmt.Errorf("process %s not found in COPS configuration process list", process)
	}

	// Verify actions are permitted for the given service.
	if !processJurisdiction[process].allowActions {
		return fmt.Errorf("actions not enabled for process %s. Please enable actions on a per process level", process)
	}
	return nil
}

// Handles a given command to enact an action on a process via systemd.
// Verifies current state of process first via checking the enabled status
// on the process and action. Once verified, takes action.
func handleSystemdCmd(action string, msg fims.FimsMsg) error {
	process := msg.Frags[2]
	var enabled bool

	// Select action to determine enabled flag.
	switch action {
	case "start":
		// Check enabled status for action on given process
		enabled = processJurisdiction[process].enableControls.startEnabled
	case "stop":
		// Check enabled status for action on given process
		enabled = processJurisdiction[process].enableControls.stopEnabled
	case "restart":
		// Check enabled status for action on given process
		enabled = processJurisdiction[process].enableControls.restartEnabled
	default:
		return fmt.Errorf("action: %s for process %s is unrecognized", action, process)
	}

	// Take action.
	if enabled {
		if err := takeAction(action, msg); err != nil {
			return fmt.Errorf("performing %s on %s: %w", action, process, err)
		}
	} else {
		return fmt.Errorf("action: %s not enabled for process %s: action conflicts with process state", action, process)
	}

	return nil
}

// Establish dbus connection and take defined action
func takeAction(action string, msg fims.FimsMsg) error {
	process := msg.Frags[2]

	// Generate service string for sending to dbus.
	service := fmt.Sprintf("%s.service", process)

	// Establish dbus connection.
	conn, err := dbus.New()
	if err != nil {
		return fmt.Errorf("failed to connect to systemd D-Bus: %w", err)
	}
	defer conn.Close()

	// Define results channel for dbus connection to report on.
	resultChan := make(chan string)

	switch action {
	case "start":
		// Start our unit
		log.Infof("Starting unit %s\n", service)
		_, err := conn.StartUnit(service, "replace", resultChan)
		if err != nil {
			return fmt.Errorf("error starting unit: %w", err)
		}
	case "stop":
		// Stop our unit
		log.Infof("Stopping unit %s\n", service)
		_, err := conn.StopUnit(service, "replace", resultChan)
		if err != nil {
			return fmt.Errorf("error stopping unit: %w", err)
		}
	case "restart":
		// Restart unit
		log.Infof("Restarting unit %s\n", service)
		_, err := conn.RestartUnit(service, "replace", resultChan)
		if err != nil {
			return fmt.Errorf("error restarting unit: %w", err)
		}
	default:
		return fmt.Errorf("action: %s for process %s is unrecognized", action, process)
	}

	// Report job result to FIMS and logs.
	result := <-resultChan
	log.Infof("Performed %s on service %s. Result: %s\n", action, service, result)

	// Validate FIMS connection to reply on.
	if f.Connected() {
		f.SendSet(msg.Replyto, "", result)
	}
	return nil
}
