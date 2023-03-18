/**
 *
 * events.go
 *
 * Methods for the `event` data type. This file is separate from the external events package as it contains
 * methods which are reliant upon -or tightly coupled to- internal scheduler logic
 *
 */
package main

import (
	"fmt"
	"log"
	"time"

	fg "github.com/flexgen-power/go_flexgen"
	events "github.com/flexgen-power/scheduler/pkg/events"
)

// eventStatus identifies how an event fits into the schedule of a specific site
type eventStatus int

// Types of events with respect to specific sites
//
// EXPIRED_EVENT: both start time and end time have passed
//
// RETRO_ACTIVE_EVENT: start time has passed and end time has not come, but event has not been triggered (it is being seen for the first time)
//
// ACTIVE_EVENT: start time has passed and end time has not come, and event has already been triggered
//
// SCHEDULED_EVENT: start time has not yet come
const (
	EXPIRED_EVENT = iota
	RETRO_ACTIVE_EVENT
	ACTIVE_EVENT
	SCHEDULED_EVENT
)

// eventIsDone returns if an event's start time + duration is at or before the current time
func eventIsDone(e *events.Event) bool {
	return getCurrentTime().After(e.StartTime.Add(e.Duration))
}

// eventStartTimePassed returns if an event's start minute has passed
func eventStartTimePassed(e *events.Event) bool {
	// do not want 1 second past start minute to be considered "passed". Needs to be in the next minute
	oneMinuteFromStartTime := e.StartTime.Add(time.Minute)
	return !getCurrentTime().Before(oneMinuteFromStartTime)
}

// eventReadyToStart returns if an event's start time is right now
func eventReadyToStart(e *events.Event) bool {
	return !getCurrentTime().Before(e.StartTime) && !getCurrentTime().After(e.StartTime.Add(time.Minute))
}

// executeEvent sends an event's setpoints to their respective URIs
func executeEvent(e *events.Event) {
	// get mode for this event from mode hash table and send all constants
	m := modes.getMode(e.Mode)
	if m == nil {
		log.Printf("events.go::execute() ~ Mode %v not found in modes map", e.Mode)
		return
	}
	m.sendConstants()
	// send all variable setpoints
	for _, modeVar := range m.variables {
		variableValue, ok := e.Variables[modeVar.id]
		// if event does not have a mode variable because mode was modified after event was made, skip the variable
		if !ok {
			log.Println("events.go::Execute() ~ Did not find mode variable", modeVar.id, "in event variable map")
			continue
		}
		// verify that variable type is as expected. if the mode changed the var type since instantiation, skip the variable
		switch modeVar.varType {
		case "Float":
			variableValue, ok = variableValue.(float64)
		case "Int":
			variableValue, ok = variableValue.(int)
		case "Bool":
			variableValue, ok = variableValue.(bool)
		case "String":
			variableValue, ok = variableValue.(string)
		}
		if !ok {
			log.Println("events.go::Execute() ~ Mode varType does not match actual event var type for variable", modeVar.id)
			continue
		}
		// set and send the mode variable with the decided value
		ok = modeVar.set(variableValue)
		if !ok {
			log.Println("events.go::Execute() ~ Error trying to set event variable", modeVar.id)
			continue
		}
		modeVar.sendSet()
	}
}

// updateActiveEvent is a special case of update for active events
// start time and mode are assumed to be the same heading into this function
func updateActiveEvent(eOld *events.Event, eNew *events.Event) {
	// verify total start time and mode are the same
	if !eOld.StartTime.Equal(eNew.StartTime) {
		log.Printf("Active event's start time %v does not match start time of event that is meant to be editing it %v", eOld.StartTime, eNew.StartTime)
		return
	}
	if eOld.Mode != eNew.Mode {
		log.Printf("Active event's mode %v does not match mode of event that is meant to be editing it %v", eOld.Mode, eNew.Mode)
		return
	}
	// update start time and duration
	eOld.Duration = eNew.Duration
	eOld.StartTime = eNew.StartTime
	// get mode for this event from mode hash table
	m := modes.getMode(eNew.Mode)
	if m == nil {
		log.Printf("Mode %s not found in modes map while trying to update active event\n", eOld.Mode)
		return
	}
	// if any new variable values, send them
	for _, modeVar := range m.variables {
		// get old value of variable that is stored in the event's variable map
		oldVarVal, ok := eOld.Variables[modeVar.id]
		// if old event's variable map does not include one of the mode's
		// variables, there has been an error because adding a new variable to
		// the active event's mode should have ended the active event. if this
		// case somehow happens, just skip this setpoint
		if !ok {
			log.Println("Mode variable", modeVar.id, "does not match any variables in old event variable map while trying to update active event")
			continue
		}
		// get new value of variable this is stored in the received event's variable map
		newVarVal, ok := eNew.Variables[modeVar.id]
		// if received event's variable map does not include one of the mode's
		// variables, the input data was not properly validated. if this
		// case somehow happens, just skip this setpoint
		if !ok {
			log.Println("Mode variable", modeVar.id, "does not match any variables in new event variable map while trying to update active event")
			continue
		}
		// if there are any changes to variable types, ignore it since those changes should have caused this event to end
		switch modeVar.varType {
		case "Float":
			oldValFloat, ok := oldVarVal.(float64)
			if !ok {
				continue
			}
			newValFloat, ok := newVarVal.(float64)
			// if old event variable value is same as new event variable value, do not issue new SET
			if !ok || oldValFloat == newValFloat {
				continue
			}
		case "Int":
			oldValInt, ok := oldVarVal.(int)
			if !ok {
				continue
			}
			newValInt, ok := newVarVal.(int)
			// if old event variable value is same as new event variable value, do not issue new SET
			if !ok || oldValInt == newValInt {
				continue
			}
		case "Bool":
			oldValBool, ok := oldVarVal.(bool)
			if !ok {
				continue
			}
			newValBool, ok := newVarVal.(bool)
			// if old event variable value is same as new event variable value, do not issue new SET
			if !ok || oldValBool == newValBool {
				continue
			}
		case "String":
			oldValString, ok := oldVarVal.(string)
			if !ok {
				continue
			}
			newValString, ok := newVarVal.(string)
			// if old event variable value is same as new event variable value, do not issue new SET
			if !ok || oldValString == newValString {
				continue
			}
		}
		// if we have made it to this point, then a variable value has been changed. send an update
		ok = modeVar.set(newVarVal)
		if !ok {
			log.Printf("Error trying to set event variable %v while trying to update active event\n", modeVar.id)
			continue
		}
		modeVar.sendSet()
		// also update event object's stored value
		eOld.Variables[modeVar.id] = newVarVal
	}
}

// parseEvent expects a specific format of object (see set_events.sh script in test folder for details).
// The object will be parsed to build an event object, which will be returned.
// A dayIndex indicates to which day the event belongs in the full schedule map, used as a temporary solution to support mins since midnight
func parseEvent(eMap *map[string]interface{}, dayIndex int, timeZone *time.Location) (e *events.Event, err error) {
	// allocate memory for new event
	e = events.NewEvent()

	// parse start_time (int, mins since midnight)
	start_time, err := extractIntThatMightBeFloat64(eMap, "start_time")
	if err != nil || dayIndex == -1 {
		// parse startTimeString (timestamp) since mins since midnight failed
		startTimeString, err := fg.ExtractValueWithType(eMap, "start_timestamp", fg.STRING)
		if err != nil {
			// Both failed
			return nil, fmt.Errorf("failed to extract start_time and start_timestamp from event data: %w", err)
		}
		startTime, err := time.Parse(time.RFC3339, startTimeString.(string))
		if err != nil {
			return nil, fmt.Errorf("failed to parse received start timestamp string %s: %w", startTimeString, err)
		}
		e.StartTime = time.Date(startTime.Year(), startTime.Month(), startTime.Day(), startTime.Hour(), startTime.Minute(), 0, 0, startTime.Location()) // start times should be on the minute
	} else {
		// mins since midnight provided, use it regardless of timestamp
		e.StartTime = convertIntToTimestamp(start_time, dayIndex, timeZone)
	}

	// parse mode name
	modeName, err := fg.ExtractValueWithType(eMap, "mode", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract mode from event data: %w", err)
	}
	e.Mode = modeName.(string)

	// parse duration, which must be less than 24 hours and representing minutes
	durationMinutes, err := extractIntThatMightBeFloat64(eMap, "duration")
	if err != nil {
		return nil, fmt.Errorf("failed to extract duration from event data: %w", err)
	}
	duration := time.Duration(durationMinutes) * time.Minute
	if duration >= 24*time.Hour {
		return nil, fmt.Errorf("duration must be less than 24 hours")
	}
	e.Duration = duration

	// parse variable list
	varMapInterface, err := fg.ExtractValueWithType(eMap, "variables", fg.MAP_STRING_INTERFACE)
	if err != nil {
		return nil, fmt.Errorf("failed to extract variables map from event data: %w", err)
	}
	varMap := varMapInterface.(map[string]interface{})

	// get mode object from mode map
	mode := modes.getMode(e.Mode)
	if mode == nil {
		return nil, fmt.Errorf("mode map does not have a %s mode in it", e.Mode)
	}

	// parse variables
	for _, modeVar := range mode.variables {
		switch modeVar.varType {
		case "Float":
			varValue, err := fg.ExtractValueWithType(&varMap, modeVar.id, fg.FLOAT64)
			if err != nil {
				return nil, fmt.Errorf("failed to extract mode variable %s from event data: %w", modeVar.id, err)
			}
			e.Variables[modeVar.id] = varValue
		case "Int":
			varValue, err := extractIntThatMightBeFloat64(&varMap, modeVar.id)
			if err != nil {
				return nil, fmt.Errorf("failed to extract mode variable %s from event data: %w", modeVar.id, err)
			}
			e.Variables[modeVar.id] = varValue
		case "Bool":
			varValue, err := fg.ExtractValueWithType(&varMap, modeVar.id, fg.BOOL)
			if err != nil {
				return nil, fmt.Errorf("failed to extract mode variable %s from event data: %w", modeVar.id, err)
			}
			e.Variables[modeVar.id] = varValue
		case "String":
			varValue, err := fg.ExtractValueWithType(&varMap, modeVar.id, fg.STRING)
			if err != nil {
				return nil, fmt.Errorf("failed to extract mode variable %s from event data: %w", modeVar.id, err)
			}
			e.Variables[modeVar.id] = varValue
		}
	}

	// parse nonRolling flag
	nonRolling, err := fg.ExtractValueWithType(eMap, "nonRolling", fg.BOOL)
	if err != nil {
		// default to false if flag not found
		nonRolling = false
	}
	e.NonRolling = nonRolling.(bool)

	return e, nil
}
