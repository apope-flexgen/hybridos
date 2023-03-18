/**
 *
 * scada.go
 *
 * Contains data objects and functions that will handle the Modbus/DNP3 SCADA interface.
 *
 */

package main

import (
	"fims"
	"fmt"
	"log"
	"sort"
	"strconv"
	"strings"
	"time"

	fg "github.com/flexgen-power/go_flexgen"
	events "github.com/flexgen-power/scheduler/pkg/events"
)

// scadaEvent is a variation on a standard events.Event, structured in a way to support enumerations for SCADA
type scadaEvent struct {
	site       *siteController
	modeName   string
	year       int
	month      int
	day        int
	hour       int
	minute     int
	duration   int
	floatVars  []float64
	intVars    []int
	boolVars   []bool
	stringVars []string
}

// scadaSlot holds a scadaEvent along with a status register indicating if the event is scheduled/active or if the slot is empty (unscheduled).
type scadaSlot struct {
	event  *scadaEvent
	status slotStatus
}

type scadaConfiguration struct {
	stageSize    int // total number of events that can be written at once
	maxNumEvents int // total number of event slots that will be maintained and broadcasted
	numFloats    int // total number of float variable slots a single SCADA event slot will contain
	numInts      int // total number of int variable slots a single SCADA event slot will contain
	numBools     int // total number of bool variable slots a single SCADA event slot will contain
	numStrings   int // total number of string variable slots a single SCADA event slot will contain
	// if, for any given type of slot, there are more slots than data items, the extra slots will be zero-valued.
	// if, for any given type of slot, there are more data items than slots, the extra data items will be invisible to the SCADA interface (not broadcasted)
}

// scadaSlotList contains all of the scadaSlots accessible by the SCADA interface.
type scadaSlotList []*scadaSlot

// slotStatus can be UNSCHEDULED (event in slot is invalid / slot is empty), SCHEDULED (event in slot is a scheduled event), or ACTIVE (event in slot is an active event)
type slotStatus int

// UNSCHEDULED: slot is empty; event in slot is invalid
//
// SCHEDULED: event in slot is scheduled for the future
//
// ACTIVE: event in slot is currently active
const (
	UNSCHEDULED = 0 // slots with this status are considered empty, as their data is expired
	SCHEDULED   = 1
	ACTIVE      = 2
)

// scadaCommand is an enumerated type of SCADA command control words
type scadaCommand int

// NONE: no action
//
// POST_STAGE: insert staged event(s) into the schedule via append (not overwrite)
//
// SET_STAGE: overwrite existing schedule with staged event(s)
//
// LOAD: load stage with details of identified event
//
// DEL_STAGE: delete identified event from schedule
//
// CLEAR_STAGE: zero/nil out all fields in the stage
//
// CLEAR_SCHEDULE: end all active events and delete all scheduled events
const (
	NONE = iota
	POST_STAGE
	SET_STAGE
	LOAD
	DEL_STAGE
	CLEAR_STAGE
	CLEAR_SCHEDULE
)

// siteEnums connects enumerated integers to site IDs via slice indices
var siteEnums sort.StringSlice

// siteEnumsInverted connects site IDs to enumerated integers
var siteEnumsInverted map[string]int

// modeEnums connects enumerated integers to mode names via slice indices
var modeEnums sort.StringSlice

// modeEnumsInverted connects mode names to enumerated integers
var modeEnumsInverted map[string]int

// configuration of the SCADA interface. start with these hard-coded defaults but they can be changed via SET
var scadaConfig scadaConfiguration = scadaConfiguration{
	stageSize:    1,
	maxNumEvents: 100,
	numFloats:    5,
	numInts:      5,
	numBools:     5,
	numStrings:   5,
}

// scadaSchedule is all of the slots containing SCADA events
var scadaSchedule scadaSlotList

// scadaStagedEvents is the control/editing area for the SCADA interface
var scadaStage []*scadaEvent

// allocates memory for scadaSchedule and scadaStage
func configureScadaInterface() {
	// allocate memory for existing event list
	scadaSchedule = make(scadaSlotList, scadaConfig.maxNumEvents)
	for i := range scadaSchedule {
		newSlot := &scadaSlot{&scadaEvent{}, UNSCHEDULED}
		newSlot.event.reset()
		scadaSchedule[i] = newSlot
	}
	// allocate memory for stage events
	scadaStage = make([]*scadaEvent, scadaConfig.stageSize)
	for i := range scadaStage {
		newEvent := &scadaEvent{}
		newEvent.reset()
		scadaStage[i] = newEvent
	}
}

// fill fills in the SCADA schedule with the events found in the master schedule. If there are more events in the master schedule
// than there are slots in the SCADA schedule (a situation we try to avoid), the extra events do not get shown in the SCADA schedule.
func (ss *scadaSlotList) fill() {
	// add up to maxNumEvents to the SCADA schedule, ordered by site then start time
	eventIndex := 0
	for siteIndex := 0; eventIndex < scadaConfig.maxNumEvents; siteIndex++ {
		if siteIndex >= len(siteEnums) {
			break // stop searching for events if we have gone through all existing sites
		}

		// get the next site in the enumerated list
		// if the site has not been added to the schedule yet, ignore it
		site := masterSchedule.getSite(siteEnums[siteIndex])
		if site == nil {
			continue
		}

		// add the site's active event, if there is one, to the SCADA schedule
		if e := site.activeEvent; e != nil {
			(*ss)[eventIndex].insert(e, site, ACTIVE)
			eventIndex++
		}

		// add the site's scheduled events to the SCADA schedule
		if eventIndex != scadaConfig.maxNumEvents {
			// iterate over each of the scheduled events
			for _, e := range *site.scheduledEvents {
				(*ss)[eventIndex].insert(e, site, SCHEDULED)
				eventIndex++
				if eventIndex == scadaConfig.maxNumEvents {
					break
				}
			}
		}
	}
	// if we did not fill up all the SCADA event slots when filling SCADA schedule, mark the rest of the slots as UNSCHEDULED aka expired aka invalid
	for ; eventIndex < len(*ss); eventIndex++ {
		(*ss)[eventIndex].reset()
	}
}

// buildObj builds an object representing the SCADA schedule.
func (ss *scadaSlotList) buildObj() []interface{} {
	body := make([]interface{}, scadaConfig.maxNumEvents)
	for i := 0; i < scadaConfig.maxNumEvents; i++ {
		slot := (*ss)[i]
		body[i] = slot.buildObj()
	}
	return body
}

// sendScadaTo sends the SCADA schedule to the specified URI.
func sendScadaTo(uri string) error {
	body := make(map[string]interface{})
	body["schedule"] = scadaSchedule.buildObj()
	body["stage"] = buildScadaStage()
	return f.SendSet(uri, "", body)
}

// publish sends a FIMS PUB for each event slot in the SCADA schedule.
func (ss *scadaSlotList) publish() {
	for i, slot := range scadaSchedule {
		f.SendPub(fmt.Sprintf("/scheduler/scada/read/event_%d", i), slot.buildObj())
	}
}

// buildObj builds an object to represent the scadaSlot.
func (slot *scadaSlot) buildObj() map[string]interface{} {
	// most data lives in event object
	eventReg := slot.event.buildObj()
	// SCADA slot also includes event status
	eventReg["status"] = int(slot.status)
	return eventReg
}

// reset sets a slot to UNSCHEDULED and resets the event data.
func (slot *scadaSlot) reset() {
	slot.status = UNSCHEDULED
	slot.event.reset()
}

// insert adds an event with given site and slot status to the SCADA slot.
func (slot *scadaSlot) insert(e *events.Event, site *siteController, s slotStatus) {
	slot.event.fillWithData(e, site)
	slot.status = s
}

// fillWithData fills in a SCADA event with data coming from a standard event object.
func (eScada *scadaEvent) fillWithData(e *events.Event, site *siteController) {
	eScada.reset()
	eScada.site = site
	eScada.modeName = e.Mode
	startTimeUtc := e.StartTime.UTC()
	eScada.year = startTimeUtc.Year()
	eScada.month = int(startTimeUtc.Month())
	eScada.day = startTimeUtc.Day()
	eScada.hour = startTimeUtc.Hour()
	eScada.minute = startTimeUtc.Minute()
	eScada.duration = int(e.Duration.Minutes())
	floatIndex, intIndex, boolIndex, stringIndex := 0, 0, 0, 0
	mode := modes.getMode(e.Mode)
	if mode == nil {
		log.Println("Error filling SCADA event: mode", e.Mode, "not found in mode map")
		return
	}
	for _, sp := range mode.variables {
		val, ok := e.Variables[sp.id]
		if !ok {
			log.Println("Error filling SCADA event: event variables do not match mode variables!")
			eScada.reset()
			return
		}
		switch sp.varType {
		case "Float":
			valFloat, ok := val.(float64)
			if !ok {
				log.Printf("Error filling SCADA event: float variable %s has type %T, not float64\n", sp.id, val)
				eScada.reset()
				return
			}
			if floatIndex < scadaConfig.numFloats {
				eScada.floatVars[floatIndex] = valFloat
				floatIndex++
			}
		case "Int":
			valInt, ok := val.(int)
			if !ok {
				log.Printf("Error filling SCADA event: int variable %s has type %T, not int\n", sp.id, val)
				eScada.reset()
				return
			}
			if intIndex < scadaConfig.numInts {
				eScada.intVars[intIndex] = valInt
				intIndex++
			}
		case "Bool":
			valBool, ok := val.(bool)
			if !ok {
				log.Printf("Error filling SCADA event: float variable %s has type %T, not bool\n", sp.id, val)
				eScada.reset()
				return
			}
			if boolIndex < scadaConfig.numBools {
				eScada.boolVars[boolIndex] = valBool
				boolIndex++
			}
		case "String":
			valString, ok := val.(string)
			if !ok {
				log.Printf("Error filling SCADA event: string variable %s has type %T, not string\n", sp.id, val)
				eScada.reset()
				return
			}
			if stringIndex < scadaConfig.numStrings {
				eScada.stringVars[stringIndex] = valString
				stringIndex++
			}
		}
	}
}

// buildObj builds an object to represent a scadaEvent for the SCADA interface.
func (e *scadaEvent) buildObj() map[string]interface{} {
	// add site
	eventReg := make(map[string]interface{})
	if e.site == nil {
		eventReg["site"] = -1
	} else if siteIndex, ok := siteEnumsInverted[e.site.id]; ok {
		eventReg["site"] = siteIndex
	} else {
		eventReg["site"] = -1
	}
	// add mode
	if modeIndex, ok := modeEnumsInverted[e.modeName]; ok {
		eventReg["mode"] = modeIndex
	} else {
		eventReg["mode"] = -1
	}
	// add date/time
	eventReg["year"] = e.year
	eventReg["month"] = e.month
	eventReg["day"] = e.day
	eventReg["hour"] = e.hour
	eventReg["minute"] = e.minute
	// add duration and repeatsDaily flag
	eventReg["duration"] = e.duration
	// add float vars
	for j := 0; j < scadaConfig.numFloats; j++ {
		eventReg["float_"+fmt.Sprint(j+1)] = e.floatVars[j]
	}
	// add int vars
	for j := 0; j < scadaConfig.numInts; j++ {
		eventReg["int_"+fmt.Sprint(j+1)] = e.intVars[j]
	}
	// add bool vars
	for j := 0; j < scadaConfig.numBools; j++ {
		eventReg["bool_"+fmt.Sprint(j+1)] = e.boolVars[j]
	}
	// add string vars
	for j := 0; j < scadaConfig.numStrings; j++ {
		eventReg["string_"+fmt.Sprint(j+1)] = e.stringVars[j]
	}
	return eventReg
}

// reset allocates memory for a SCADA event's variables.
func (e *scadaEvent) reset() {
	e.site = nil
	e.modeName = ""
	e.year = 0
	e.month = 0
	e.day = 0
	e.hour = 0
	e.minute = 0
	e.duration = 0
	e.floatVars = make([]float64, scadaConfig.numFloats)
	for i := range e.floatVars {
		e.floatVars[i] = 0.0
	}
	e.intVars = make([]int, scadaConfig.numInts)
	for i := range e.intVars {
		e.intVars[i] = 0
	}
	e.boolVars = make([]bool, scadaConfig.numBools)
	for i := range e.boolVars {
		e.boolVars[i] = false
	}
	e.stringVars = make([]string, scadaConfig.numStrings)
	for i := range e.stringVars {
		e.stringVars[i] = ""
	}
}

// getEventTime returns the event's start time using all the date/time fields consolidated into a time.Time struct.
func (eScada *scadaEvent) getEventTime() time.Time {
	return time.Date(eScada.year, time.Month(eScada.month), eScada.day, eScada.hour, eScada.minute, 0, 0, time.UTC).In(eScada.site.timezone)
}

// convertToEvent builds a standard events.Event object based on a scadaEvent object.
func (eScada *scadaEvent) convertToEvent() (*events.Event, error) {
	// extract mode
	m := modes.getMode(eScada.modeName)
	if m == nil {
		return nil, fmt.Errorf("scada.go::scadaEvent::convertToEvent() ~ Mode %s does not exist in mode map", eScada.modeName)
	}

	// build event
	eNew := events.NewEvent()
	eNew.StartTime = eScada.getEventTime()
	eNew.Duration = time.Minute * time.Duration(eScada.duration)
	eNew.Mode = eScada.modeName

	// the memory for variable slots in the arrays has already been allocated (see reset func),
	// so must index into existing register rather than append new one.
	// each of the below index variables correspond to one of the variable arrays
	iFloat, iInt, iBool, iString := 0, 0, 0, 0
	for _, sp := range m.variables {
		switch sp.varType {
		case "Float":
			if iFloat >= scadaConfig.numFloats {
				return nil, fmt.Errorf("scada.go::scadaEvent::convertToEvent() ~ Number of float variables in mode %v exceeds maximum number of floats allowed in SCADA event", eScada.modeName)
			}
			eNew.Variables[sp.id] = eScada.floatVars[iFloat]
			iFloat++
		case "Int":
			if iInt >= scadaConfig.numInts {
				return nil, fmt.Errorf("scada.go::scadaEvent::convertToEvent() ~ Number of int variables in mode %v exceeds maximum number of ints allowed in SCADA event", eScada.modeName)
			}
			eNew.Variables[sp.id] = eScada.intVars[iInt]
			iInt++
		case "Bool":
			if iBool >= scadaConfig.numBools {
				return nil, fmt.Errorf("scada.go::scadaEvent::convertToEvent() ~ Number of bool variables in mode %v exceeds maximum number of bools allowed in SCADA event", eScada.modeName)
			}
			eNew.Variables[sp.id] = eScada.boolVars[iBool]
			iBool++
		case "String":
			if iString >= scadaConfig.numStrings {
				return nil, fmt.Errorf("scada.go::scadaEvent::convertToEvent() ~ Number of string variables in mode %v exceeds maximum number of strings allowed in SCADA event", eScada.modeName)
			}
			eNew.Variables[sp.id] = eScada.stringVars[iString]
			iString++
		}
	}
	return eNew, nil
}

func (e *scadaEvent) handleGet(endpoint, replyTo string) error {
	var replyBody interface{}
	switch endpoint {
	case "site":
		if e.site == nil {
			replyBody = -1
		} else {
			var ok bool
			replyBody, ok = siteEnumsInverted[e.site.id]
			if !ok {
				return fmt.Errorf("site ID %s not found in enum map", e.site.id)
			}
		}
	case "mode":
		if e.modeName == "" {
			replyBody = -1
		} else {
			var ok bool
			replyBody, ok = modeEnumsInverted[e.modeName]
			if !ok {
				return fmt.Errorf("mode ID %s not found in enum map", e.modeName)
			}
		}
	case "year":
		replyBody = e.year
	case "month":
		replyBody = e.month
	case "day":
		replyBody = e.day
	case "hour":
		replyBody = e.hour
	case "minute":
		replyBody = e.minute
	case "duration":
		replyBody = e.duration
	default:
		replyBody = nil
	}
	if replyBody != nil {
		err := f.SendSet(replyTo, "", replyBody)
		if err != nil {
			return fmt.Errorf("failed to send variable %s reply SET: %w", endpoint, err)
		}
		return nil
	}
	// at this point, must be float/int/bool/string var
	typeAndId := strings.Split(endpoint, "_")
	if len(typeAndId) != 2 {
		return fmt.Errorf("split on '_' but found %d fragments instead of 2", len(typeAndId))
	}
	id, err := strconv.Atoi(typeAndId[1])
	if err != nil {
		return fmt.Errorf("failed to read variable index: %w", err)
	}
	if id < 1 {
		return fmt.Errorf("received variable ID of %d but variables are indexed starting from 1", id)
	}
	index := id - 1
	switch typeAndId[0] {
	case "float":
		if index > len(e.floatVars) {
			return fmt.Errorf("variable index %d exceeds floats array which has length %d", index, len(e.floatVars))
		}
		replyBody = e.floatVars[index]
	case "int":
		if index > len(e.intVars) {
			return fmt.Errorf("variable index %d exceeds ints array which has length %d", index, len(e.intVars))
		}
		replyBody = e.intVars[index]
	case "bool":
		if index > len(e.boolVars) {
			return fmt.Errorf("variable index %d exceeds bools array which has length %d", index, len(e.boolVars))
		}
		replyBody = e.boolVars[index]
	case "string":
		if index > len(e.stringVars) {
			return fmt.Errorf("variable index %d exceeds strings array which has length %d", index, len(e.stringVars))
		}
		replyBody = e.stringVars[index]
	default:
		return fmt.Errorf("invalid variable type %s", typeAndId[0])
	}
	err = f.SendSet(replyTo, "", replyBody)
	if err != nil {
		return fmt.Errorf("failed to send variable %s reply SET: %w", endpoint, err)
	}
	return nil
}

// parses the event index from a URI fragment containing a SCADA event
// identifier of the form "event_<event index>"
func parseEventIndex(eventFrag string) (int, error) {
	if !strings.HasPrefix(eventFrag, "event_") || len(eventFrag) < 7 {
		return 0, fmt.Errorf("event frag does not have form 'event_<event index>'")
	}
	index, err := strconv.Atoi(eventFrag[6:])
	if err != nil {
		return 0, fmt.Errorf("failed to read event index: %w", err)
	}
	return index, nil
}

// handles all GETs to URIs starting with /scheduler/scada/read
func handleScadaReadGet(msg fims.FimsMsg) error {
	// URI is /scheduler/scada/read
	if msg.Nfrags < 4 {
		err := f.SendSet(msg.Replyto, "", scadaSchedule.buildObj())
		if err != nil {
			return fmt.Errorf("failed to send SCADA schedule reply SET: %w", err)
		}
		return nil
	}

	// parse out which event the GET is to
	eventIndex, err := parseEventIndex(msg.Frags[3])
	if err != nil {
		return fmt.Errorf("failed to parse event index: %w", err)
	}
	if eventIndex < 0 {
		return fmt.Errorf("event index is %d but indexing for events starts at 0", eventIndex)
	}
	if eventIndex >= len(scadaSchedule) {
		return fmt.Errorf("requested event with index %d but highest index is %d", eventIndex, len(scadaSchedule)-1)
	}
	eScada := scadaSchedule[eventIndex]

	// if get is to entire event
	if msg.Nfrags == 4 {
		if err = f.SendSet(msg.Replyto, "", eScada.buildObj()); err != nil {
			return fmt.Errorf("failed to send SCADA schedule event reply SET: %w", err)
		}
		return nil
	}

	// if get is to an individual field of event
	if err = eScada.event.handleGet(msg.Frags[4], msg.Replyto); err != nil {
		return fmt.Errorf("failed to handle GET for individual field: %w", err)
	}
	return nil
}

// handles all GETs to URIs starting with /scheduler/scada/write
func handleScadaWriteGet(msg fims.FimsMsg) error {
	// /scheduler/scada/write
	if msg.Nfrags == 3 {
		if err := f.SendSet(msg.Replyto, "", buildScadaStage()); err != nil {
			return fmt.Errorf("failed to send SCADA stage in reply SET: %w", err)
		}
		return nil
	}
	// since command is a pulsed value, it should always be 0 in steady-state
	if msg.Frags[3] == "command" {
		if err := f.SendSet(msg.Replyto, "", 0); err != nil {
			return fmt.Errorf("failed to send command value in reply SET: %w", err)
		}
		return nil
	}
	// if the stage is configured for only one event, "event_#" frag will not be in URI
	if len(scadaStage) == 1 {
		if err := scadaStage[0].handleGet(msg.Frags[3], msg.Replyto); err != nil {
			return fmt.Errorf("failed to handle GET for individual stage field with single-event stage: %w", err)
		}
		return nil
	}
	// identify which event GET is for
	eventIndex, err := parseEventIndex(msg.Frags[3])
	if err != nil {
		return fmt.Errorf("failed to parse event index from %s: %w", msg.Frags[3], err)
	}
	if eventIndex < 0 {
		return fmt.Errorf("event number is %d but indexing for events starts at 0", eventIndex)
	}
	if eventIndex >= len(scadaStage) {
		return fmt.Errorf("requested event with index %d but length of SCADA stage is only %d", eventIndex, len(scadaStage))
	}
	eScada := scadaStage[eventIndex]
	// URI is /scheduler/scada/write/event_<event index>
	if msg.Nfrags == 4 {
		if err = f.SendSet(msg.Replyto, "", eScada.buildObj()); err != nil {
			return fmt.Errorf("failed to send reply SET for SCADA schedule event GET: %w", err)
		}
		return nil
	}
	// URI is /scheduler/scada/write/event_<event index>/<field name>
	if err = eScada.handleGet(msg.Frags[4], msg.Replyto); err != nil {
		return fmt.Errorf("failed to handle GET for individual event field: %w", err)
	}
	return nil
}

// handles all GETs to URIs starting with /scheduler/scada
func handleScadaGet(msg fims.FimsMsg) {
	if msg.Nfrags < 3 { // /scheduler/scada
		if err := sendScadaTo(msg.Replyto); err != nil {
			log.Printf("Error replying to GET request for SCADA schedule: %s\n", err.Error())
		}
		return
	}

	// GETs to URIs that begin with /scheduler/scada/read
	if msg.Frags[2] == "read" {
		if err := handleScadaReadGet(msg); err != nil {
			log.Printf("Error handling SCADA read GET to %s: %s\n", msg.Uri, err.Error())
		}
		return
	}

	// a GET that begins with /scheduler/scada/write is a GET to the SCADA staging area
	if msg.Frags[2] == "write" {
		if err := handleScadaWriteGet(msg); err != nil {
			log.Printf("Error handling SCADA write GET to %s: %s\n", msg.Uri, err.Error())
		}
		return
	}
	log.Printf("%s is invalid SCADA GET URI.\n", msg.Uri)
}

// Matches the given varId to a SCADA event field and sets it to the value found in the given body.
// Returns an error if the varId cannot be matched to a field or if the given body is incompatible with the matched field.
func (eScada *scadaEvent) setVariable(varId string, body interface{}) error {
	valueInterface := unwrapVariable(body)

	// look for SETs to event setpoint registers, indicated by <type name>_<setpoint index>
	varIdSplit := strings.Split(varId, "_")
	if len(varIdSplit) == 2 {
		// parse setpoint index
		setpointNumber, err := strconv.Atoi(varIdSplit[1])
		if err != nil {
			return fmt.Errorf("failed to parse number after '_' in variable ID %s: %w", varId, err)
		} else if setpointNumber < 1 {
			return fmt.Errorf("expected positive number after '_' in variable ID %s", varId)
		}
		setpointIndex := setpointNumber - 1 // numbers in the IDs start at 1, indices start at 0

		// use type name to assign value to appropriate variable
		switch varIdSplit[0] {
		case "float":
			if setpointNumber > scadaConfig.numFloats {
				return fmt.Errorf("attempted to set float_%d but max number of floats is %d", setpointNumber, scadaConfig.numFloats)
			}
			value, ok := valueInterface.(float64)
			if ok {
				eScada.floatVars[setpointIndex] = value
				return nil
			}
		case "int":
			if setpointNumber > scadaConfig.numInts {
				return fmt.Errorf("attempted to set int_%d but max number of ints is %d", setpointNumber, scadaConfig.numInts)
			}
			value, ok := valueInterface.(float64) // ints get turned into floats during the fims mashalling/unmarshalling process
			if ok {
				eScada.intVars[setpointIndex] = int(value)
				return nil
			}
		case "bool":
			if setpointNumber > scadaConfig.numBools {
				return fmt.Errorf("attempted to set bool_%d but max number of bools is %d", setpointNumber, scadaConfig.numBools)
			}
			value, ok := valueInterface.(bool)
			if ok {
				eScada.boolVars[setpointIndex] = value
				return nil
			}
		case "string":
			if setpointNumber > scadaConfig.numStrings {
				return fmt.Errorf("attempted to set string_%d but max number of strings is %d", setpointNumber, scadaConfig.numStrings)
			}
			value, ok := valueInterface.(string)
			if ok {
				eScada.stringVars[setpointIndex] = value
				return nil
			}
		}
		return fmt.Errorf("value did not pass type assertion. Actual type is %T", valueInterface)
	}

	// if not variable registers, try other registers, all of which should be ints
	value, err := fg.CastToInt(valueInterface)
	if err != nil {
		return fmt.Errorf("failed to received value as int: %w", err)
	}

	switch varId {
	case "site":
		if value < len(siteEnums) {
			eScada.site = masterSchedule.getSite(siteEnums[value])
		} else {
			return fmt.Errorf("received site index %d exceeded bounds of siteEnums", value)
		}
	case "year":
		eScada.year = value
	case "month":
		eScada.month = value
	case "day":
		eScada.day = value
	case "hour":
		eScada.hour = value
	case "minute":
		eScada.minute = value
	case "duration":
		eScada.duration = value
	case "mode":
		if value < len(modeEnums) {
			eScada.modeName = modeEnums[value]
		} else {
			return fmt.Errorf("received mode index %d exceeded bounds of modeEnums", value)
		}
	}
	return nil
}

// handleScadaSet allows the SCADA interface to modify the staging area.
func handleScadaSet(msg fims.FimsMsg) error {
	// staging area is only valid SCADA SET endpoint
	if !strings.HasPrefix(msg.Uri, "/scheduler/scada/write") {
		return fmt.Errorf("%s is not a valid URI", msg.Uri)
	}

	// SCADA command
	if msg.Frags[3] == "command" {
		commandInterface := unwrapVariable(msg.Body)
		command, err := fg.CastToInt(commandInterface)
		if err != nil {
			return fmt.Errorf("failed to extract command value as int: %w", err)
		}
		if err = commandScada(scadaCommand(command)); err != nil {
			return fmt.Errorf("SCADA command failed: %w", err)
		}
		return nil
	}

	// SET to staged event when stage is configured to hold multiple events
	if msg.Nfrags == 5 { // /scheduler/scada/write/event_#/<scada var ID>
		if len(scadaStage) == 1 {
			return fmt.Errorf("SCADA stage size is 1 so its variables should be accessed directly with the endpoint /scheduler/scada/write/<scada var ID>")
		}

		// parse target event index out of URI
		eventId := msg.Frags[3]
		eventIdSplit := strings.Split(eventId, "_")
		if len(eventIdSplit) != 2 {
			return fmt.Errorf("SET to URI beginning with /scheduler/scada/write and having 5 frags expects the fourth frag to be of the form 'event_#'. Instead got %s", eventId)
		}
		eventIndex, err := strconv.Atoi(eventIdSplit[1])
		if err != nil {
			return fmt.Errorf("failed to parse number after '_' in event ID %s: %w", eventId, err)
		} else if eventIndex < 1 {
			return fmt.Errorf("expected positive number after '_' in event ID %s", eventId)
		}

		// set variable of target event
		if eventIndex > len(scadaStage) {
			return fmt.Errorf("requested event index %d is outside bounds of SCADA stage which is size %d", eventIndex, len(scadaStage))
		}
		scadaStage[eventIndex-1].setVariable(msg.Frags[4], msg.Body)
		return nil
	} else if msg.Nfrags != 4 {
		return fmt.Errorf("%s has an invalid number of frags (%d)", msg.Uri, msg.Nfrags)
	}

	// SET to staged event when stage is configured to hold only one event
	if len(scadaStage) < 1 {
		return fmt.Errorf("SCADA stage is empty. Check SCADA configuration")
	}
	if err := scadaStage[0].setVariable(msg.Frags[3], msg.Body); err != nil {
		return fmt.Errorf("failed to SCADA stage variable: %w", err)
	}
	return nil
}

// handle all SETs to URIs that begin with /scheduler/configuration/scada
func handleScadaConfigSet(msg fims.FimsMsg) error {
	if msg.Nfrags == 3 { // //scheduler/configuration/scada
		err := handleScadaConfigStructSet(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to configure SCADA interface: %w", err)
		}
	} else if msg.Nfrags == 4 { // /scheduler/configuration/scada/<CONFIG FIELD>
		err := setScadaConfigField(msg.Frags[3], unwrapVariable(msg.Body))
		if err != nil {
			return fmt.Errorf("failed to edit SCADA configuration field %s: %w", msg.Frags[3], err)
		}
	} else {
		return fmt.Errorf("invalid uri")
	}

	// save new configuration to DBI
	sendConfigData("/dbi/scheduler/configuration")

	// update SCADA interface with new configuration
	configureScadaInterface()
	updateScadaRegs()
	return nil
}

// handle SET to /scheduler/scada/configuration
func handleScadaConfigStructSet(msgBody interface{}) error {
	var newScadaConfig scadaConfiguration

	configMap, ok := msgBody.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected SCADA configuration object to be a map[string]interface{}, but received type is %T", msgBody)
	}

	// stageSize is optional and will default to 1 if not found
	stageSizeInterface, ok := configMap["stageSize"]
	if ok {
		stageSize, err := fg.CastToInt(stageSizeInterface)
		if err != nil {
			return fmt.Errorf("failed to cast stageSize value as int: %w", err)
		}
		newScadaConfig.stageSize = stageSize
	} else {
		log.Println("No stageSize found in scada config, so defaulting to 1")
		newScadaConfig.stageSize = 1
	}

	maxNumEvents, err := fg.ExtractAsInt(&configMap, "maxNumEvents")
	if err != nil {
		return fmt.Errorf("failed to extract maxNumEvents from received SCADA configuration: %w", err)
	}
	newScadaConfig.maxNumEvents = maxNumEvents

	numFloats, err := fg.ExtractAsInt(&configMap, "numFloats")
	if err != nil {
		return fmt.Errorf("failed to extract numFloats from received SCADA configuration: %w", err)
	}
	newScadaConfig.numFloats = numFloats

	numInts, err := fg.ExtractAsInt(&configMap, "numInts")
	if err != nil {
		return fmt.Errorf("failed to extract numInts from received SCADA configuration: %w", err)
	}
	newScadaConfig.numInts = numInts

	numBools, err := fg.ExtractAsInt(&configMap, "numBools")
	if err != nil {
		return fmt.Errorf("failed to extract numBools from received SCADA configuration: %w", err)
	}
	newScadaConfig.numBools = numBools

	numStrings, err := fg.ExtractAsInt(&configMap, "numStrings")
	if err != nil {
		return fmt.Errorf("failed to extract numStrings from received SCADA configuration: %w", err)
	}
	newScadaConfig.numStrings = numStrings

	// successfully parsed entire struct so safe to replace existing configuration data now
	scadaConfig = newScadaConfig
	return nil
}

// sets the given config field to the given new value
func setScadaConfigField(fieldName string, newValueInterface interface{}) error {
	// make sure new value is an int (or a type that can be casted as an int)
	newValue, err := fg.CastToInt(newValueInterface)
	if err != nil {
		return fmt.Errorf("new %s value is not usable as an int: %w", fieldName, err)
	}

	switch fieldName {
	case "stageSize":
		scadaConfig.stageSize = newValue
	case "maxNumEvents":
		scadaConfig.maxNumEvents = newValue
	case "numFloats":
		scadaConfig.numFloats = newValue
	case "numInts":
		scadaConfig.numInts = newValue
	case "numBools":
		scadaConfig.numBools = newValue
	case "numStrings":
		scadaConfig.numStrings = newValue
	}
	return nil
}

// handles all GETs to URIs starting with /scheduler/configuration/scada
func handleScadaConfigGet(msg fims.FimsMsg) error {
	if msg.Nfrags == 3 { // /scheduler/configuration/scada
		return f.SendSet(msg.Replyto, "", scadaConfig.buildObj())
	}

	if msg.Nfrags < 4 {
		return fmt.Errorf("invalid uri")
	}

	switch msg.Frags[3] { // /scheduler/configuration/scada/<CONFIG FIELD>
	case "stageSize":
		return f.SendSet(msg.Replyto, "", scadaConfig.stageSize)
	case "maxNumEvents":
		return f.SendSet(msg.Replyto, "", scadaConfig.maxNumEvents)
	case "numFloats":
		return f.SendSet(msg.Replyto, "", scadaConfig.numFloats)
	case "numInts":
		return f.SendSet(msg.Replyto, "", scadaConfig.numInts)
	case "numBools":
		return f.SendSet(msg.Replyto, "", scadaConfig.numBools)
	case "numStrings":
		return f.SendSet(msg.Replyto, "", scadaConfig.numStrings)
	default:
		return fmt.Errorf("invalid uri")
	}
}

func (sc *scadaConfiguration) buildObj() map[string]interface{} {
	return map[string]interface{}{
		"stageSize":    sc.stageSize,
		"maxNumEvents": sc.maxNumEvents,
		"numFloats":    sc.numFloats,
		"numInts":      sc.numInts,
		"numBools":     sc.numBools,
		"numStrings":   sc.numStrings,
	}
}

// commandScada carries out a SCADA command.
func commandScada(command scadaCommand) error {
	switch command {
	case NONE:
		return nil
	case POST_STAGE:
		return scadaEdit(POST)
	case SET_STAGE:
		return scadaEdit(SET)
	case LOAD:
		return loadStage()
	case DEL_STAGE:
		return scadaEdit(DEL)
	case CLEAR_STAGE:
		clearStage()
	case CLEAR_SCHEDULE:
		clearMasterSchedule(SCADA)
	default:
		return fmt.Errorf("invalid SCADA command: %v. Clearing the SCADA stage", command)
	}
	return nil
}

// scadaEdit modifies the schedule with the events found in the SCADA stage.
// The passed-in editing method determines how the staged events modify the
// schedule (SET, POST, DEL).
func scadaEdit(method editingMethod) error {
	// get all settings of existing schedule, but without its existing events
	inputSchedule := masterSchedule.createEmptyCopy()

	// add each staged event to the input schedule
	for i, eScada := range scadaStage {
		// convert SCADA representation of a Scheduler event to a true internal Scheduler event object
		eNew, err := eScada.convertToEvent()
		if err != nil {
			// possible that not all stage event slots will be used in every SET, so assume misconfigured
			// slots are unused slots
			continue
		}

		// for now, only allow events to be made for "today" or "tomorrow" (with respect to the current time on site)
		timesAreSameDay := func(t1, t2 time.Time) bool {
			return t1.Year() == t2.Year() && t1.Month() == t2.Month() && t1.Day() == t2.Day()
		}
		today := time.Now().In(eScada.site.timezone)
		tomorrow := getMidnightInNDays(1, today)
		newEventIsToday := timesAreSameDay(today, eNew.StartTime) // eNew.StartTime already converted to site time zone in convertToEvent()
		newEventIsTomorrow := timesAreSameDay(tomorrow, eNew.StartTime)
		if !(newEventIsToday || newEventIsTomorrow) {
			return fmt.Errorf("can only write an event that occurs today or tomorrow with respect to the current time on site. Current time on site: %s. Attempted start time for staged SCADA event with index %d: %s", today.String(), i, eNew.StartTime.String())
		}

		// add the event to the input schedule in the appropriate site's schedule
		inputSchedule[eScada.site.id].scheduledEvents.appendEvent(eNew)
	}

	// edit the schedule
	editMultipleSchedules(inputSchedule, SCADA, method)

	// clear the staging area
	clearStage()
	return nil
}

// loadStage loads the rest of the staging area's fields with the data of the event identified by the site/day/start.
func loadStage() error {
	// if stage has many slots, assumed we are using it to overwrite existing schedule with new
	// events so no point in supporting load operation
	if len(scadaStage) > 1 {
		return fmt.Errorf("bulk loads not supported")
	} else if len(scadaStage) == 0 {
		return fmt.Errorf("SCADA staging area is empty")
	}
	scadaStagedEvent := scadaStage[0]

	if scadaStagedEvent.site == nil {
		return fmt.Errorf("cannot load SCADA stage when no site is selected")
	}
	var e *events.Event
	if scadaStagedEvent.site.hasActiveEvent() && scadaStagedEvent.site.activeEvent.StartTime.Equal(scadaStagedEvent.getEventTime()) {
		e = scadaStagedEvent.site.activeEvent
	} else {
		e, _ = scadaStagedEvent.site.scheduledEvents.get(scadaStagedEvent.getEventTime())
		if e == nil {
			return fmt.Errorf("no event found with selected start time within the schedule of the selected site/day")
		}
	}
	scadaStagedEvent.fillWithData(e, scadaStagedEvent.site)
	pubScadaStage()
	return nil
}

// clearStage zeroes out the staging area.
func clearStage() {
	for _, eScada := range scadaStage {
		eScada.reset()
	}
	pubScadaStage()
}

// updateScadaRegs fills in the SCADA Schedule with the master schedule then sends PUBs for all SCADA server registers.
// do not need to save SCADA schedule to DBI since it is sourced from the master schedule, which is already saved to DBI.
func updateScadaRegs() {
	scadaSchedule.fill()
	scadaSchedule.publish()
}

// builds an object to represent the entire staging area
// if stage is only configured to hold one event, put it's fields on top level of stage object.
// if stage is configured to hold many events, put them each in their own sub-objects.
func buildScadaStage() map[string]interface{} {
	var obj map[string]interface{}
	if len(scadaStage) == 1 {
		obj = scadaStage[0].buildObj()
	} else {
		obj = make(map[string]interface{})
		for i, eStage := range scadaStage {
			obj[fmt.Sprintf("event_%d", i+1)] = eStage.buildObj()
		}
	}
	// scada command is a pulsed command, so always publish 0 to clear command register after a command was written to it
	obj["command"] = 0
	return obj
}

// pubScadaStage sends a FIMS PUB of the staging area to its URI.
func pubScadaStage() {
	f.SendPub("/scheduler/scada/write", buildScadaStage())
	if scadaConfig.stageSize == 1 {
		return
	}
	// some processes can only parse a single layer of a PUB, so if staging area is configured for bulk writing
	// then pub each individual staged event again in their own individual PUB so these processes can read them.
	// Modbus server is a process that falls into this category
	for i, eStage := range scadaStage {
		f.SendPub(fmt.Sprintf("/scheduler/scada/write/event_%d", i), eStage.buildObj())
	}
}

// refreshModeEnums loads the modeEnums slice with all existing modes, alphabetizes them, and adds them to the inverted modes map.
func refreshModeEnums() {
	modeEnums = make([]string, 0)
	for _, modeId := range modes.getModeNames() {
		if modeId == "default" {
			continue
		}
		modeEnums = append(modeEnums, modeId)
	}
	modeEnums.Sort()
	modeEnumsInverted = make(map[string]int)
	for i, v := range modeEnums {
		modeEnumsInverted[v] = i
	}
}

// refreshSiteEnums loads the siteEnums slice with all existing modes, alphabetizes them, and adds them to the inverted sites map.
func refreshSiteEnums() {
	siteEnums = make([]string, 0)
	for _, site := range masterSchedule.getSites() {
		siteEnums = append(siteEnums, site.id)
	}
	siteEnums.Sort()
	siteEnumsInverted = make(map[string]int)
	for i, v := range siteEnums {
		siteEnumsInverted[v] = i
	}
}
