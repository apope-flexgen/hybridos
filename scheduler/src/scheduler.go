/**
 * Scheduler
 * scheduler.go
 *
 * Created May 2021
 *
 * Scheduler is a back-end module that allows HybridOS users to schedule, update, or
 * cancel energy export and import events. It communicates with the front-end UI and
 * the controller backbone module (either Site Controller or Fleet Manager) via FIMS.
 *
 * Interfaces include: UI, SCADA (Modbus/DNP3), Fleet-Site, DBI.
 *
 */

package main

import (
	"log"
	"time"
)

// isPrimaryScheduler indicates if the box Scheduler is running on is operating in primary mode.
// At startup, Scheduler assumes secondary mode until it is told otherwise by COPS.
var isPrimaryScheduler bool = false

// enableSchedulerEvents is a flag that controls whether Scheduler can send out setpoints or not.
var enableSchedulerEvents bool = true

// setpointEnforcing indicates if events' setpoints should have their status values tracked and updated if they deviate from their control values.
var setpointEnforcing bool = false

// setpointEnforcementFreqSeconds decides how often active event setpoints will be checked, assuming setpointEnforcing is true.
// Initialize with 100 hours. Arbitrarily large since it will not be a useful select case in main() until it is configured by user
var setpointEnforcementFreqSeconds int = 360000

// setpointEnforcementTicker keeps track of how often the active event should have its status values verified, if there is an active event.
// Must be a global variable because it is designed to be reconfigurable by the user.
var setpointEnforcementTicker *time.Ticker

var lastCheck time.Time

func main() {
	// Configuration & Initialization
	eventCheckTicker := configureScheduler()
	fimsReceive := configureFims()
	configureFleetSiteInterface()
	configureScadaInterface()
	getDbiData()
	configureVersion()

	// Operating loop
	for {
		select {
		case msg := <-fimsReceive:
			processFims(msg)
		case <-eventCheckTicker.C:
			// check all site schedules every second
			checkMasterSchedule()
		case <-setpointEnforcementTicker.C:
			// check status values of active event and send SETs if deviation from control values.
			// does not execute if Fleet Manager, there is no active event, or this feature is turned off
			if setpointEnforcing {
				err := verifyActiveEventStatus()
				if err != nil {
					log.Println("Error verifying active event status: %w", err)
				}
			}
		case msg := <-webSocketReceive:
			processWebSocketMsg(msg)
		}
	}
}

// configureScheduler configures general and global variables.
func configureScheduler() *time.Ticker {
	// set log prefix so all logs will identify themselves as Scheduler logs
	log.SetPrefix("Scheduler: ")

	// allocate memory for master schedule and mode map
	masterSchedule.data = make(schedule)
	modes.data = make(modeMap)

	// initialize lastCheck
	lastCheck = getCurrentTime()

	// start with empty default mode since it is the only required mode
	modes.data["default"] = &mode{
		variables: []setpoint{},
		constants: []setpoint{},
		colorCode: "#D1D1D1",
	}

	// configure time zone variable
	loc, err := getLocalTimezone()
	if err != nil {
		// getting the proper time zone from the system is required,
		// and we are not too far into the program to kill it
		log.Fatal("Error trying to get time zone from system:", err)
	}
	thisTimezone = loc

	// frequency of the active event status check. note, active event status check will be disabled (setpointEnforcing == false) until
	// the user finishes configuring Scheduler with a config form, and that config form will contain a new status check frequency.
	// So this initialization is only done so that a non-nil ticker can be instantiated, but the 1 second frequency will be replaced
	// with the configured value
	setpointEnforcementFreq := time.Second * time.Duration(setpointEnforcementFreqSeconds)
	setpointEnforcementTicker = time.NewTicker(setpointEnforcementFreq)

	// configure frequency of schedule checking
	eventCheckFreq := time.Duration(time.Second)
	return time.NewTicker(eventCheckFreq)
}

// updateEnableSchedulerEvents sets enableSchedulerEvents to the given value and sends an update to DBI.
func updateEnableSchedulerEvents(val bool) {
	enableSchedulerEvents = val
	sendEnableSchedulerEvents("/dbi/scheduler/enableSchedulerEvents")
}
