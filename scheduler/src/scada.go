/**
 *
 * scada.go
 *
 * Contains data objects and functions that will handle the Modbus/DNP3 SCADA interface.
 *
 */

package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"sort"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	events "github.com/flexgen-power/scheduler/pkg/events"
)

// scadaEvent is a variation on a standard events.Event, structured in a way to support enumerations for SCADA
type scadaEvent struct {
	site        *schedule
	modeId      string
	year        int
	month       int
	day         int
	hour        int
	minute      int
	duration    int
	repeatDaily scadaBool
	floatVars   []float64
	intVars     []int
	boolVars    []scadaBool
	stringVars  []string
}

// scadaSlot holds a scadaEvent along with a status register indicating if the event is scheduled/active or if the slot is empty (unscheduled).
type scadaSlot struct {
	event  *scadaEvent
	status slotStatus
}

type scadaConfiguration struct {
	// When true, the SCADA command Append can edit existing events if they match the start time and site selection of the staged event.
	// When false, Append can only add new events.
	AppendCanEdit bool `json:"append_can_edit"`

	StageSize    int `json:"stage_size"`     // Total number of events that can be written at once.
	MaxNumEvents int `json:"max_num_events"` // Total number of event slots that will be maintained and broadcasted.
	NumFloats    int `json:"num_floats"`     // Total number of float variable slots a single SCADA event slot will contain.
	NumInts      int `json:"num_ints"`       // Total number of int variable slots a single SCADA event slot will contain.
	NumBools     int `json:"num_bools"`      // Total number of bool variable slots a single SCADA event slot will contain.
	NumStrings   int `json:"num_strings"`    // Total number of string variable slots a single SCADA event slot will contain.
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
// APPEND: insert staged event(s) into the schedule. Only allowed to overwrite if append_can_edit configuration flag is turned on.
//
// OVERWRITE: overwrite existing schedule with staged event(s).
//
// LOAD_STAGE: load stage with details of identified event.
//
// DELETE: delete identified event(s) from schedule.
//
// CLEAR_STAGE: zero/nil out all fields in the stage.
//
// CLEAR_SCHEDULE: end all active events and delete all scheduled events.
const (
	NONE = iota
	APPEND
	OVERWRITE
	LOAD_STAGE
	DELETE
	CLEAR_STAGE
	CLEAR_SCHEDULE
)

// SCADA system users prefer data to be represented as integers, so in the SCADA interface 1 represents true and 0 represents false.
type scadaBool int

const (
	scadaFalse scadaBool = 0
	scadaTrue  scadaBool = 1
)

// siteEnums connects enumerated integers to site IDs via slice indices
var siteEnums sort.StringSlice

// siteEnumsInverted connects site IDs to enumerated integers
var siteEnumsInverted map[string]int

// modeEnums connects enumerated integers to mode names via slice indices
var modeEnums sort.StringSlice

// modeEnumsInverted connects mode names to enumerated integers
var modeEnumsInverted map[string]int

// scadaSchedule is all of the slots containing SCADA events
var scadaSchedule scadaSlotList

// scadaStagedEvents is the control/editing area for the SCADA interface
var scadaStage []*scadaEvent

// fill fills in the SCADA schedule with the events found in the master schedule. If there are more events in the master schedule
// than there are slots in the SCADA schedule (a situation we try to avoid), the extra events do not get shown in the SCADA schedule.
func (ss *scadaSlotList) fill() {
	// add up to maxNumEvents to the SCADA schedule, ordered by site then start time
	eventIndex := 0
	for siteIndex := 0; eventIndex < schedCfg.Scada.MaxNumEvents; siteIndex++ {
		if siteIndex >= len(siteEnums) {
			break // stop searching for events if we have gone through all existing sites
		}

		// get the next site in the enumerated list
		// if the site has not been added to the schedule yet, ignore it
		site, ok := masterSchedule[siteEnums[siteIndex]]
		if !ok {
			continue
		}

		// add the site's active event, if there is one, to the SCADA schedule
		if e := site.activeEvent; e != nil {
			(*ss)[eventIndex].insert(e, site, ACTIVE)
			eventIndex++
		}

		// add the site's scheduled events to the SCADA schedule
		if eventIndex != schedCfg.Scada.MaxNumEvents {
			// iterate over each of the scheduled events
			for _, e := range *site.scheduledEvents {
				(*ss)[eventIndex].insert(e, site, SCHEDULED)
				eventIndex++
				if eventIndex == schedCfg.Scada.MaxNumEvents {
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
	body := make([]interface{}, schedCfg.Scada.MaxNumEvents)
	for i := 0; i < schedCfg.Scada.MaxNumEvents; i++ {
		slot := (*ss)[i]
		body[i] = slot.buildObj()
	}
	return body
}

func buildScadaObject() map[string]interface{} {
	return map[string]interface{}{
		"schedule": scadaSchedule.buildObj(),
		"stage":    buildScadaStage(),
	}
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
func (slot *scadaSlot) insert(e *events.Event, site *schedule, s slotStatus) {
	if err := slot.event.fillWithData(e, site); err != nil {
		log.Errorf("Error filling data: %v.", err)
	}
	slot.status = s
}

func (slot *scadaSlot) handleGet(endpoint, replyTo string) error {
	switch endpoint {
	case "status":
		err := f.SendSet(replyTo, "", slot.status)
		if err != nil {
			return fmt.Errorf("failed to send slot status in reply SET: %w", err)
		}
		return nil
	default:
		err := slot.event.handleGet(endpoint, replyTo)
		if err != nil {
			return fmt.Errorf("failed to handle GET to slot event: %w", err)
		}
		return nil
	}
}

// fillWithData fills in a SCADA event with data coming from a standard event object.
func (eScada *scadaEvent) fillWithData(e *events.Event, site *schedule) error {
	eScada.reset()
	eScada.site = site
	eScada.modeId = e.Mode
	startTimeUtc := e.StartTime.UTC()
	eScada.year = startTimeUtc.Year()
	eScada.month = int(startTimeUtc.Month())
	eScada.day = startTimeUtc.Day()
	eScada.hour = startTimeUtc.Hour()
	eScada.minute = startTimeUtc.Minute()
	eScada.duration = int(e.Duration)
	// for now, assume that when SCADA interface is being used then all events are either daily repeating forever or one-offs (based on repeatDaily flag)
	if e.Repeat.EndCount == -1 {
		eScada.repeatDaily = scadaTrue
	} else {
		eScada.repeatDaily = scadaFalse
	}
	floatIndex, intIndex, boolIndex, stringIndex := 0, 0, 0, 0
	mode, ok := modes[e.Mode]
	if !ok {
		return fmt.Errorf("mode %s not found in mode map", e.Mode)
	}
	for _, sp := range mode.Variables {
		val, ok := e.Variables[sp.Id]
		if !ok {
			return fmt.Errorf("could not find setpoint id \"%s\" in variable", sp.Id)
		}

                _, ok = val.(map[string]interface{})
                if ok {
			eScada.reset()
			return fmt.Errorf("value of type %T. Currently not supported. You are likely trying to use batching for SCADA which is not supported", val)
                }

		switch sp.VarType {
		case "Float":
			valFloat, ok := val.(float64)
			if !ok {
				eScada.reset()
				return fmt.Errorf("float variable %s has type %T, not float64", sp.Id, val)
			}
			if floatIndex < schedCfg.Scada.NumFloats {
				eScada.floatVars[floatIndex] = valFloat
				floatIndex++
			}
		case "Int":
			valInt, ok := val.(int)
			if !ok {
				eScada.reset()
				return fmt.Errorf("int variable %s has type %T, not int", sp.Id, val)
			}
			if intIndex < schedCfg.Scada.NumInts {
				eScada.intVars[intIndex] = valInt
				intIndex++
			}
		case "Bool":
			valBool, ok := val.(bool)
			if !ok {
				eScada.reset()
				return fmt.Errorf("boolean variable %s has type %T, not bool", sp.Id, val)
			}
			if boolIndex < schedCfg.Scada.NumBools {
				if valBool {
					eScada.boolVars[boolIndex] = scadaTrue
				} else {
					eScada.boolVars[boolIndex] = scadaFalse
				}
				boolIndex++
			}
		case "String":
			valString, ok := val.(string)
			if !ok {
				eScada.reset()
				return fmt.Errorf("string variable %s has type %T, not string", sp.Id, val)
			}
			if stringIndex < schedCfg.Scada.NumStrings {
				eScada.stringVars[stringIndex] = valString
				stringIndex++
			}
		}
	}
	return nil
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
				sendErrorResponse(replyTo, "Resource Not Found")
				return fmt.Errorf("site ID %s not found in enum map", e.site.id)
			}
		}
	case "mode":
		if e.modeId == "" {
			replyBody = -1
		} else {
			var ok bool
			replyBody, ok = modeEnumsInverted[e.modeId]
			if !ok {
				sendErrorResponse(replyTo, "Resource Not Found")
				return fmt.Errorf("mode ID %s not found in enum map", e.modeId)
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
	case "repeat_daily":
		replyBody = e.repeatDaily
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
		sendErrorResponse(replyTo, "Invalid URI")
		return fmt.Errorf("split on '_' but found %d fragments instead of 2", len(typeAndId))
	}
	id, err := strconv.Atoi(typeAndId[1])
	if err != nil {
		sendErrorResponse(replyTo, "Invalid URI")
		return fmt.Errorf("failed to read variable index: %w", err)
	}
	if id < 1 {
		sendErrorResponse(replyTo, "Invalid URI")
		return fmt.Errorf("received variable ID of %d but variables are indexed starting from 1", id)
	}
	index := id - 1
	switch typeAndId[0] {
	case "float":
		if index > len(e.floatVars) {
			sendErrorResponse(replyTo, "Invalid URI")
			return fmt.Errorf("variable index %d exceeds floats array which has length %d", index, len(e.floatVars))
		}
		replyBody = e.floatVars[index]
	case "int":
		if index > len(e.intVars) {
			sendErrorResponse(replyTo, "Invalid URI")
			return fmt.Errorf("variable index %d exceeds ints array which has length %d", index, len(e.intVars))
		}
		replyBody = e.intVars[index]
	case "bool":
		if index > len(e.boolVars) {
			sendErrorResponse(replyTo, "Invalid URI")
			return fmt.Errorf("variable index %d exceeds bools array which has length %d", index, len(e.boolVars))
		}
		replyBody = e.boolVars[index]
	case "string":
		if index > len(e.stringVars) {
			sendErrorResponse(replyTo, "Invalid URI")
			return fmt.Errorf("variable index %d exceeds strings array which has length %d", index, len(e.stringVars))
		}
		replyBody = e.stringVars[index]
	default:
		sendErrorResponse(replyTo, "Invalid URI")
		return fmt.Errorf("invalid variable type %s", typeAndId[0])
	}
	err = f.SendSet(replyTo, "", replyBody)
	if err != nil {
		return fmt.Errorf("failed to send variable %s reply SET: %w", endpoint, err)
	}
	return nil
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
	if modeIndex, ok := modeEnumsInverted[e.modeId]; ok {
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
	// add duration and repeatDaily flag
	eventReg["duration"] = e.duration
	eventReg["repeat_daily"] = e.repeatDaily
	// add float vars
	for j := 0; j < schedCfg.Scada.NumFloats; j++ {
		eventReg["float_"+fmt.Sprint(j+1)] = e.floatVars[j]
	}
	// add int vars
	for j := 0; j < schedCfg.Scada.NumInts; j++ {
		eventReg["int_"+fmt.Sprint(j+1)] = e.intVars[j]
	}
	// add bool vars
	for j := 0; j < schedCfg.Scada.NumBools; j++ {
		eventReg["bool_"+fmt.Sprint(j+1)] = e.boolVars[j]
	}
	// add string vars
	for j := 0; j < schedCfg.Scada.NumStrings; j++ {
		eventReg["string_"+fmt.Sprint(j+1)] = e.stringVars[j]
	}
	return eventReg
}

// reset allocates memory for a SCADA event's variables.
func (e *scadaEvent) reset() {
	e.site = nil
	e.modeId = ""
	e.year = 0
	e.month = 0
	e.day = 0
	e.hour = 0
	e.minute = 0
	e.duration = 0
	e.repeatDaily = scadaFalse
	e.floatVars = make([]float64, schedCfg.Scada.NumFloats)
	for i := range e.floatVars {
		e.floatVars[i] = 0.0
	}
	e.intVars = make([]int, schedCfg.Scada.NumInts)
	for i := range e.intVars {
		e.intVars[i] = 0
	}
	e.boolVars = make([]scadaBool, schedCfg.Scada.NumBools)
	for i := range e.boolVars {
		e.boolVars[i] = scadaFalse
	}
	e.stringVars = make([]string, schedCfg.Scada.NumStrings)
	for i := range e.stringVars {
		e.stringVars[i] = ""
	}
}

// getEventTime returns the event's start time using all the date/time fields consolidated into a time.Time struct.
func (eScada *scadaEvent) getEventTime() time.Time {
	return time.Date(eScada.year, time.Month(eScada.month), eScada.day, eScada.hour, eScada.minute, 0, 0, time.UTC).In(eScada.site.timezone)
}

// Builds a standard events.Event object based on a scadaEvent object.
// If the site pointer of the scadaEvent is nil, it can be overridden
// by the defaultSite parameter unless the given defaultSite is nil,
// in which case an error will be returned.
func (eScada *scadaEvent) convertToEvent(defaultSchedule *schedule) (*events.Event, error) {
	// error checking to make sure the user has selected a site
	if eScada.site == nil {
		if defaultSchedule == nil {
			return nil, errors.New("schedule has not been selected and there is not only 1 schedule in the schedule map")
		}
		eScada.site = defaultSchedule
	}

	// extract mode
	m, ok := modes[eScada.modeId]
	if !ok {
		return nil, fmt.Errorf("failed to convert SCADA event to normal event: mode %s does not exist in the mode map", eScada.modeId)
	}

	// build event
	eNew := events.CreateEvent(eScada.getEventTime(), time.Minute*time.Duration(eScada.duration), eScada.modeId)
	// default series is a one-off event, but if repeatDaily flag is true then set EndCount to -1 so that event repeats daily forever
	if eScada.repeatDaily == scadaTrue {
		eNew.Repeat.EndCount = -1
	}

	// the memory for variable slots in the arrays has already been allocated (see reset func),
	// so must index into existing register rather than append new one.
	// each of the below index variables correspond to one of the variable arrays
	iFloat, iInt, iBool, iString := 0, 0, 0, 0
	for _, sp := range m.Variables {
		switch sp.VarType {
		case "Float":
			if iFloat >= schedCfg.Scada.NumFloats {
				return nil, fmt.Errorf("number of float variables in mode %s exceeds maximum number of floats allowed in SCADA event", eScada.modeId)
			}
			eNew.Variables[sp.Id] = eScada.floatVars[iFloat]
			iFloat++
		case "Int":
			if iInt >= schedCfg.Scada.NumInts {
				return nil, fmt.Errorf("number of int variables in mode %s exceeds maximum number of ints allowed in SCADA event", eScada.modeId)
			}
			eNew.Variables[sp.Id] = eScada.intVars[iInt]
			iInt++
		case "Bool":
			if iBool >= schedCfg.Scada.NumBools {
				return nil, fmt.Errorf("number of bool variables in mode %s exceeds maximum number of bools allowed in SCADA event", eScada.modeId)
			}
			eNew.Variables[sp.Id] = eScada.boolVars[iBool] != scadaFalse
			iBool++
		case "String":
			if iString >= schedCfg.Scada.NumStrings {
				return nil, fmt.Errorf("number of string variables in mode %s exceeds maximum number of strings allowed in SCADA event", eScada.modeId)
			}
			eNew.Variables[sp.Id] = eScada.stringVars[iString]
			iString++
		}
	}
	return eNew, nil
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
		sendReply(msg.Replyto, scadaSchedule.buildObj())
		return nil
	}

	// parse out which event the GET is to
	eventIndex, err := parseEventIndex(msg.Frags[3])
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("failed to parse event index: %w", err)
	}
	if eventIndex < 0 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("event index is %d but indexing for events starts at 0", eventIndex)
	}
	if eventIndex >= len(scadaSchedule) {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("requested event with index %d but highest index is %d", eventIndex, len(scadaSchedule)-1)
	}
	eScada := scadaSchedule[eventIndex]

	// if get is to entire event
	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, eScada.buildObj())
		return nil
	}

	// if get is to an individual field of event
	if err = eScada.handleGet(msg.Frags[4], msg.Replyto); err != nil {
		return fmt.Errorf("failed to handle GET for individual field: %w", err)
	}
	return nil
}

// handles all GETs to URIs starting with /scheduler/scada/write
func handleScadaWriteGet(msg fims.FimsMsg) error {
	// /scheduler/scada/write
	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, buildScadaStage())
		return nil
	}
	// since command is a pulsed value, it should always be 0 in steady-state
	if msg.Frags[3] == "command" {
		sendReply(msg.Replyto, 0)
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
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("failed to parse event index from %s: %w", msg.Frags[3], err)
	}
	if eventIndex < 0 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("event number is %d but indexing for events starts at 0", eventIndex)
	}
	if eventIndex >= len(scadaStage) {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("requested event with index %d but length of SCADA stage is only %d", eventIndex, len(scadaStage))
	}
	eScada := scadaStage[eventIndex]
	// URI is /scheduler/scada/write/event_<event index>
	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, eScada.buildObj())
		return nil
	}
	// URI is /scheduler/scada/write/event_<event index>/<field name>
	if err = eScada.handleGet(msg.Frags[4], msg.Replyto); err != nil {
		return fmt.Errorf("failed to handle GET for individual event field: %w", err)
	}
	return nil
}

// handles all GETs to URIs starting with /scheduler/scada
func handleScadaGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 { // /scheduler/scada
		sendReply(msg.Replyto, buildScadaObject())
		return nil
	}

	switch msg.Frags[2] {
	case "read":
		if err := handleScadaReadGet(msg); err != nil {
			return fmt.Errorf("failed to handle SCADA read GET: %w", err)
		}
	case "write":
		if err := handleScadaWriteGet(msg); err != nil {
			return fmt.Errorf("failed to handle SCADA write GET: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Parses an integer that is meant to represent a boolean.
// 0 is false and non-zero is true, but standardize non-zero values as 1 AKA scadaTrue.
func parseScadaBool(valueInterface interface{}) (scadaBool, error) {
	valueInt, err := parsemap.CastToInt(valueInterface)
	if err != nil {
		return 0, fmt.Errorf("failed to cast %v to int: %w", valueInterface, err)
	}
	if valueInt == 0 {
		return scadaFalse, nil
	} else {
		return scadaTrue, nil
	}
}

// Matches the given varId to a SCADA event field and sets it to the value found in the given body.
// Returns an error if the varId cannot be matched to a field or if the given body is incompatible with the matched field.
func (eScada *scadaEvent) setVariable(varId string, body interface{}) (replyObj interface{}, err error) {
	valueInterface := parsemap.UnwrapVariable(body)

	if varId == "repeat_daily" {
		valScadaBool, err := parseScadaBool(valueInterface)
		if err != nil {
			return nil, fmt.Errorf("failed to parse SCADA boolean: %w", err)
		}
		eScada.repeatDaily = valScadaBool
		return valScadaBool, nil
	}

	// look for SETs to event setpoint registers, indicated by <type name>_<setpoint index>
	varIdSplit := strings.Split(varId, "_")
	if len(varIdSplit) == 2 {
		// parse setpoint index
		setpointNumber, err := strconv.Atoi(varIdSplit[1])
		if err != nil {
			return nil, fmt.Errorf("failed to parse number after '_' in variable ID %s: %w", varId, err)
		} else if setpointNumber < 1 {
			return nil, fmt.Errorf("expected positive number after '_' in variable ID %s", varId)
		}
		setpointIndex := setpointNumber - 1 // numbers in the IDs start at 1, indices start at 0

		// use type name to assign value to appropriate variable
		switch varIdSplit[0] {
		case "float":
			if setpointNumber > schedCfg.Scada.NumFloats {
				return nil, fmt.Errorf("attempted to set float_%d but max number of floats is %d", setpointNumber, schedCfg.Scada.NumFloats)
			}
			value, ok := valueInterface.(float64)
			if ok {
				eScada.floatVars[setpointIndex] = value
				return value, nil
			}
		case "int":
			if setpointNumber > schedCfg.Scada.NumInts {
				return nil, fmt.Errorf("attempted to set int_%d but max number of ints is %d", setpointNumber, schedCfg.Scada.NumInts)
			}
			value, ok := valueInterface.(float64) // ints get turned into floats during the fims mashalling/unmarshalling process
			if ok {
				eScada.intVars[setpointIndex] = int(value)
				return value, nil
			}
		case "bool":
			if setpointNumber > schedCfg.Scada.NumBools {
				return nil, fmt.Errorf("attempted to set bool_%d but max number of bools is %d", setpointNumber, schedCfg.Scada.NumBools)
			}
			valScadaBool, err := parseScadaBool(valueInterface)
			if err != nil {
				return nil, fmt.Errorf("failed to parse SCADA boolean: %w", err)
			}
			eScada.boolVars[setpointIndex] = valScadaBool
			return valScadaBool, nil
		case "string":
			if setpointNumber > schedCfg.Scada.NumStrings {
				return nil, fmt.Errorf("attempted to set string_%d but max number of strings is %d", setpointNumber, schedCfg.Scada.NumStrings)
			}
			value, ok := valueInterface.(string)
			if ok {
				eScada.stringVars[setpointIndex] = value
				return value, nil
			}
		}
		return nil, fmt.Errorf("value did not pass type assertion. Actual type is %T", valueInterface)
	}

	// if not variable registers or repeat daily flag, try other registers, all of which are ints
	value, err := parsemap.CastToInt(valueInterface)
	if err != nil {
		return nil, fmt.Errorf("failed to received value as int: %w", err)
	}

	switch varId {
	case "site":
		if value < -1 || value >= len(siteEnums) {
			return nil, fmt.Errorf("received site index %d exceeded bounds of siteEnums", value)
		}
		if value != -1 {
			scheduleId := siteEnums[value]
			selectedSchedule, ok := masterSchedule[scheduleId]
			if !ok {
				return nil, fmt.Errorf("%s was not found in the schedule map", scheduleId)
			}
			eScada.site = selectedSchedule
		} else {
			eScada.site = nil
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
			eScada.modeId = modeEnums[value]
		} else {
			return nil, fmt.Errorf("received mode index %d exceeded bounds of modeEnums", value)
		}
	}
	return value, nil
}

// handleScadaSet allows the SCADA interface to modify the staging area.
func handleScadaSet(msg fims.FimsMsg) error {
	// staging area is only valid SCADA SET endpoint
	if !strings.HasPrefix(msg.Uri, "/scheduler/scada/write") {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("%s is not a valid URI", msg.Uri)
	}

	// SCADA command
	if msg.Frags[3] == "command" {
		commandInterface := parsemap.UnwrapVariable(msg.Body)
		command, err := parsemap.CastToInt(commandInterface)
		if err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to extract command value as int: %w", err)
		}
		if err = commandScada(scadaCommand(command)); err != nil {
			if errors.Is(err, ErrInvalidUri) {
				sendErrorResponse(msg.Replyto, "Invalid URI")
			} else {
				// only errors commandScada might return other than ErrInvalidUri are related to the LOAD command
				// being sent when the staging area does not have events selected, so Resource Not Found is most appropriate
				sendErrorResponse(msg.Replyto, "Resource Not Found")
			}
			return fmt.Errorf("SCADA command failed: %w", err)
		}
		sendReply(msg.Replyto, "Success")
		return nil
	}

	// at this point, we know SET is trying to edit stage so defer a PUB of the stage so that we publish the stage edit
	defer pubScadaStage()

	// SET to staged event when stage is configured to hold multiple events
	if msg.Nfrags == 5 { // /scheduler/scada/write/event_#/<scada var ID>
		if len(scadaStage) == 1 {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return fmt.Errorf("SCADA stage size is 1 so its variables should be accessed directly with the endpoint /scheduler/scada/write/<scada var ID>")
		}

		// parse target event index out of URI
		eventId := msg.Frags[3]
		eventIdSplit := strings.Split(eventId, "_")
		if len(eventIdSplit) != 2 {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return fmt.Errorf("SET to URI beginning with /scheduler/scada/write and having 5 frags expects the fourth frag to be of the form 'event_#'. Instead got %s", eventId)
		}
		eventIndex, err := strconv.Atoi(eventIdSplit[1])
		if err != nil {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return fmt.Errorf("failed to parse number after '_' in event ID %s: %w", eventId, err)
		} else if eventIndex < 0 {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return fmt.Errorf("expected non-negative number after '_' in event ID %s", eventId)
		}

		// set variable of target event
		if eventIndex >= len(scadaStage) {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return fmt.Errorf("requested event index %d is outside bounds of SCADA stage which is size %d", eventIndex, len(scadaStage))
		}
		replyObj, err := scadaStage[eventIndex].setVariable(msg.Frags[4], msg.Body)
		if err != nil {
			sendErrorResponse(msg.Replyto, "Request Failed") // super generic, but error responses to SCADA SETs are only ever going to be viewed on the command line so logs are easily accessible
			return fmt.Errorf("failed to set SCADA stage (size %d) variable: %w", len(scadaStage), err)
		}
		sendReply(msg.Replyto, replyObj)
		return nil
	} else if msg.Nfrags != 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("%s has an invalid number of frags (%d)", msg.Uri, msg.Nfrags)
	}

	// SET to staged event when stage is configured to hold only one event
	if len(scadaStage) < 1 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("SCADA stage is empty. Check SCADA configuration")
	}
	replyObj, err := scadaStage[0].setVariable(msg.Frags[3], msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Request Failed") // super generic, but error responses to SCADA SETs are only ever going to be viewed on the command line so logs are easily accessible
		return fmt.Errorf("failed to set SCADA stage (size 1) variable: %w", err)
	}
	sendReply(msg.Replyto, replyObj)
	return nil
}

// Handles SETs to URIs that begin with /scheduler/configuration/scada.
func handleScadaConfigSet(msg fims.FimsMsg) error {
	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var replyObject interface{}
	switch msg.Nfrags {
	case 3:
		if replyObject, err = handleScadaFullConfigSet(jsonBytes); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to handle SET to SCADA configuration object: %w", err)
		}
	case 4:
		if replyObject, err = handleScadaConfigFieldSet(msg.Frags[3], jsonBytes); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to handle SET to SCADA configuration field: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	sendReply(msg.Replyto, replyObject)
	return nil
}

// Handles a FIMS SET to /scheduler/configuration/scada.
func handleScadaFullConfigSet(input []byte) (newCfg scadaConfiguration, err error) {
	if err := json.Unmarshal(input, &newCfg); err != nil {
		return newCfg, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = newCfg.validate(); err != nil {
		return newCfg, fmt.Errorf("failed to validate new SCADA settings: %w", err)
	}

	schedCfg.Scada = newCfg
	// TODO: evaluate if we really do need to do an entire reconfiguration
	reconfigureScheduler(schedCfg)

	sendBackup("/dbi/scheduler/configuration/scada", newCfg)
	return newCfg, nil
}

// Handles a FIMS SET to /scheduler/configuration/scada/<field ID>.
func handleScadaConfigFieldSet(fieldId string, input []byte) (newValue interface{}, err error) {
	input = parsemap.UnwrapBytes(input)
	newCfg := schedCfg.Scada
	switch fieldId {
	case "append_can_edit":
		if err = json.Unmarshal(input, &newCfg.AppendCanEdit); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal append_can_edit: %w", err)
		}
		newValue = newCfg.AppendCanEdit
	case "stage_size":
		if err = json.Unmarshal(input, &newCfg.StageSize); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal stage_size: %w", err)
		}
		newValue = newCfg.StageSize
	case "max_num_events":
		if err = json.Unmarshal(input, &newCfg.MaxNumEvents); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal max_num_events: %w", err)
		}
		newValue = newCfg.MaxNumEvents
	case "num_floats":
		if err = json.Unmarshal(input, &newCfg.NumFloats); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal num_floats: %w", err)
		}
		newValue = newCfg.NumFloats
	case "num_ints":
		if err = json.Unmarshal(input, &newCfg.NumInts); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal num_ints: %w", err)
		}
		newValue = newCfg.NumInts
	case "num_bools":
		if err = json.Unmarshal(input, &newCfg.NumBools); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal num_bools: %w", err)
		}
		newValue = newCfg.NumBools
	case "num_strings":
		if err = json.Unmarshal(input, &newCfg.NumStrings); err != nil {
			return newValue, fmt.Errorf("failed to unmarshal num_strings: %w", err)
		}
		newValue = newCfg.NumStrings
	default:
		return newValue, ErrInvalidUri
	}

	if err = newCfg.validate(); err != nil {
		return newValue, fmt.Errorf("failed to validate new SCADA setting: %w", err)
	}

	schedCfg.Scada = newCfg
	// TODO: evaluate if we really do need to do an entire reconfiguration
	reconfigureScheduler(schedCfg)

	sendBackup(fmt.Sprintf("/dbi/scheduler/configuration/scada/%s", fieldId), newValue)
	return newValue, nil
}

// Overwrites the existing SCADA interface config with a new one and resets the registers.
func reconfigureScadaInterface(newCfg scadaConfiguration) {
	// overwrite existing config
	schedCfg.Scada = newCfg
	// allocate memory for existing event list
	scadaSchedule = make(scadaSlotList, schedCfg.Scada.MaxNumEvents)
	for i := range scadaSchedule {
		newSlot := &scadaSlot{&scadaEvent{}, UNSCHEDULED}
		newSlot.event.reset()
		scadaSchedule[i] = newSlot
	}
	// allocate memory for stage events
	scadaStage = make([]*scadaEvent, schedCfg.Scada.StageSize)
	for i := range scadaStage {
		newEvent := &scadaEvent{}
		newEvent.reset()
		scadaStage[i] = newEvent
	}
	// update registers with newly configured format
	updateScadaRegs()
}

// Handles all GETs to URIs starting with /scheduler/configuration/scada.
func (cfg *scadaConfiguration) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	switch msg.Frags[3] {
	case "append_can_edit":
		sendReply(msg.Replyto, cfg.AppendCanEdit)
	case "stage_size":
		sendReply(msg.Replyto, cfg.StageSize)
	case "max_num_events":
		sendReply(msg.Replyto, cfg.MaxNumEvents)
	case "num_floats":
		sendReply(msg.Replyto, cfg.NumFloats)
	case "num_ints":
		sendReply(msg.Replyto, cfg.NumInts)
	case "num_bools":
		sendReply(msg.Replyto, cfg.NumBools)
	case "num_strings":
		sendReply(msg.Replyto, cfg.NumStrings)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// commandScada carries out a SCADA command.
func commandScada(command scadaCommand) error {
	switch command {
	case NONE:
		return nil
	case APPEND:
		scadaEdit(command)
	case OVERWRITE:
		scadaEdit(command)
	case LOAD_STAGE:
		return loadStage()
	case DELETE:
		scadaEdit(command)
	case CLEAR_STAGE:
		clearStage()
	case CLEAR_SCHEDULE:
		clearMasterSchedule()
	default:
		return ErrInvalidUri
	}
	return nil
}

// scadaEdit modifies the schedule with the events found in the SCADA stage.
// The passed-in editing method determines how the staged events modify the
// schedule (Append, Overwrite, or Delete).
func scadaEdit(editingMethod scadaCommand) {
	eLists := make(map[string]*eventList)

	// if there is only one schedule in the schedule map, allow it to be used as the default schedule
	// so the schedule does not have to be selected by the SCADA client. if there is not only
	// one schedule, leave the pointer as nil so any events that do not have their schedules
	// selected will be skipped
	var onlySchedule *schedule
	if len(masterSchedule) == 1 {
		for _, sched := range masterSchedule {
			onlySchedule = sched
		}
	}

	// add each staged event to the input schedule
	for _, eScada := range scadaStage {
		// convert SCADA representation of a Scheduler event to a true internal Scheduler event object
		eNew, err := eScada.convertToEvent(onlySchedule)
		if err != nil {
			// possible that not all stage event slots will be used in every SET, so assume misconfigured
			// slots are unused slots
			continue
		}

		// add the event to the input schedule in the appropriate site's schedule
		if _, ok := eLists[eScada.site.id]; !ok {
			eLists[eScada.site.id] = &eventList{}
		}
		eLists[eScada.site.id].add(eNew)
	}

	for scheduleId, inputEventList := range eLists {
		sched, ok := masterSchedule[scheduleId]
		if !ok {
			log.Errorf("Error handling SCADA edit for schedule %s: no schedule with ID %s found in master schedule.", scheduleId, scheduleId)
			continue
		}

		switch editingMethod {
		case OVERWRITE:
			if err := sched.handleFullScheduleSet(*inputEventList); err != nil {
				log.Errorf("Error handling SCADA Overwrite for schedule %s: %v.", scheduleId, err)
			}
		case APPEND:
			if schedCfg.Scada.AppendCanEdit {
				if err := sched.handleFullSchedulePatch(*inputEventList); err != nil {
					log.Errorf("Error handling SCADA edit-enabled Append for schedule %s: %v.", scheduleId, err)
				}
			} else {
				if err := sched.handleFullSchedulePost(*inputEventList); err != nil {
					log.Errorf("Error handling SCADA Append for schedule %s: %v.", scheduleId, err)
				}
			}
		case DELETE:
			startTimes := inputEventList.getStartTimes()
			idsToDelete := sched.getIdsOfEventsWithStartTimes(startTimes)
			if err := sched.deleteMultipleEvents(idsToDelete); err != nil {
				log.Errorf("Error handling SCADA Delete for schedule %s: %v.", scheduleId, err)
			}
		}
	}

	// clear the staging area
	clearStage()
}

// loadStage loads the rest of the staging area's fields with the data of the event identified by the site/day/start.
func loadStage() error {
	// if stage has many slots, assumed we are using it to overwrite existing schedule with new
	// events so no point in supporting load operation
	if len(scadaStage) > 1 {
		return ErrInvalidUri
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

	if err := scadaStagedEvent.fillWithData(e, scadaStagedEvent.site); err != nil {
		return fmt.Errorf("error filling data: %v.", err)
	}
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
	if schedCfg.Scada.StageSize == 1 {
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
	for modeId := range modes {
		if modeId == defaultModeId {
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
	for _, site := range masterSchedule {
		siteEnums = append(siteEnums, site.id)
	}
	siteEnums.Sort()
	siteEnumsInverted = make(map[string]int)
	for i, v := range siteEnums {
		siteEnumsInverted[v] = i
	}
}

func (cfg *scadaConfiguration) validate() error {
	if cfg.StageSize < 0 {
		return errors.New("stage_size cannot be negative")
	}
	if cfg.MaxNumEvents < 0 {
		return errors.New("max_num_events cannot be negative")
	}
	if cfg.NumFloats < 0 {
		return errors.New("num_floats cannot be negative")
	}
	if cfg.NumInts < 0 {
		return errors.New("num_ints cannot be negative")
	}
	if cfg.NumBools < 0 {
		return errors.New("num_bools cannot be negative")
	}
	if cfg.NumStrings < 0 {
		return errors.New("num_strings cannot be negative")
	}
	return nil
}
