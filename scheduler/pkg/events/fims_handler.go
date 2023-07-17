package events

import (
	"encoding/json"
	"errors"
	"fmt"
	"strconv"
	"time"

	"github.com/flexgen-power/scheduler/internal/flextime"
	"github.com/flexgen-power/scheduler/internal/setpoint"
)

var ErrInvalidUri = errors.New("invalid URI")

// Handles a SET request to overwrite event data.
// URI begins with /scheduler/events/<schedule ID>/<event ID>.
func (e *Event) HandleSet(uriFrags []string, body []byte, timezone *time.Location, mapOfVariables map[string]setpoint.List, isActiveEvent bool) (replyObject interface{}, err error) {
	if len(uriFrags) < 5 {
		newSettings, err := ParseEvent(body, timezone, mapOfVariables)
		if err != nil {
			return nil, fmt.Errorf("failed to parse event settings: %w", err)
		}
		newSettings.Id = e.Id

		if isActiveEvent {
			if areEqualExceptMaybeDuration, reasonNotEqual := newSettings.onlyDurationCanBeDifferent(e); !areEqualExceptMaybeDuration {
				return nil, fmt.Errorf("SET is to active event, where only duration can be changed, but non-duration change found: %s", reasonNotEqual)
			}
			proposedNewEndTime := newSettings.StartTime.Add(newSettings.TimeDuration())
			if flextime.TimeIsBeforeCurrentMinute(proposedNewEndTime) {
				return nil, fmt.Errorf("new duration value %d of active event would cause event end time to become %v which is before current time", newSettings.Duration, proposedNewEndTime)
			}
		} else {
			// cannot edit a scheduled event to be in the past
			if flextime.TimeIsBeforeCurrentMinute(newSettings.StartTime) {
				return nil, fmt.Errorf("start time field %v is before current time", newSettings.StartTime)
			}
		}

		*e = newSettings
		return e, nil
	}

	// cannot edit non-duration fields of active event because it would make new schedule inaccurate.
	// example: changing an hour-long charge event to discharge halfway through makes it look like there was an hour-long discharge, not 30min charge followed by 30min discharge
	if isActiveEvent && uriFrags[4] != "duration" {
		return nil, errors.New("cannot edit any field except for duration for active event")
	}

	switch uriFrags[4] {
	case "start_time":
		var newStartTime time.Time
		if err = json.Unmarshal(body, &newStartTime); err != nil {
			return nil, fmt.Errorf("failed to unmarshal start_time: %w", err)
		}
		if flextime.TimeIsBeforeCurrentMinute(newStartTime) {
			return nil, fmt.Errorf("start time %v is before current time", newStartTime)
		}
		e.StartTime = newStartTime
		replyObject = e.StartTime
	case "duration":
		var newDuration uint
		if err = json.Unmarshal(body, &newDuration); err != nil {
			return nil, fmt.Errorf("failed to unmarshal duration: %w", err)
		}
		proposedNewEndTime := e.StartTime.Add(time.Duration(newDuration) * time.Minute)
		if flextime.TimeIsBeforeCurrentMinute(proposedNewEndTime) {
			return nil, fmt.Errorf("new duration value %d would cause event end time to become %v which is before current time", newDuration, proposedNewEndTime)
		}
		e.Duration = newDuration
		replyObject = e.Duration
	case "variables":
		if replyObject, err = e.handleVariablesSet(uriFrags, body); err != nil {
			return nil, fmt.Errorf("failed to handle variables SET: %w", err)
		}
	case "repeat":
		if replyObject, err = e.Repeat.handleSet(uriFrags, body); err != nil {
			return nil, fmt.Errorf("failed to handle repeat SET: %w", err)
		}
	default:
		return nil, ErrInvalidUri
	}

	if err := e.Validate(timezone, mapOfVariables); err != nil {
		return nil, fmt.Errorf("failed to validate event settings: %w", err)
	}
	return replyObject, nil
}

// Handles a POST request to add data to the event.
// URI begins with /scheduler/events/<schedule ID>/<event ID>.
func (e *Event) HandlePost(uriFrags []string, body []byte, timezone *time.Location, mapOfVariables map[string]setpoint.List) (replyObject interface{}, err error) {
	if len(uriFrags) < 5 {
		return nil, ErrInvalidUri
	}

	switch uriFrags[4] {
	case "repeat":
		if replyObject, err = e.Repeat.handlePost(uriFrags, body); err != nil {
			return nil, fmt.Errorf("failed to handle repeat POST: %w", err)
		}
	default:
		return nil, ErrInvalidUri
	}

	if err := e.Validate(timezone, mapOfVariables); err != nil {
		return nil, fmt.Errorf("failed to validate event settings: %w", err)
	}
	return replyObject, nil
}

// Handles a DEL request to remove data from the event.
// URI begins with /scheduler/events/<schedule ID>/<event ID>.
func (e *Event) HandleDel(uriFrags []string, body []byte, timezone *time.Location, mapOfVariables map[string]setpoint.List) (replyObject interface{}, err error) {
	if len(uriFrags) < 5 {
		return nil, ErrInvalidUri
	}

	switch uriFrags[4] {
	case "repeat":
		if replyObject, err = e.Repeat.handleDel(uriFrags, body); err != nil {
			return nil, fmt.Errorf("failed to handle repeat DEL: %w", err)
		}
	default:
		return nil, ErrInvalidUri
	}

	if err := e.Validate(timezone, mapOfVariables); err != nil {
		return nil, fmt.Errorf("failed to validate event settings: %w", err)
	}
	return replyObject, nil
}

// Handles a GET request to retrieve event data.
// URI begins with /scheduler/events/<schedule ID>/<event ID>.
func (e *Event) HandleGet(uriFrags []string) (replyObject interface{}, err error) {
	if len(uriFrags) < 5 {
		return e, nil
	}

	switch uriFrags[4] {
	case "start_time":
		return e.StartTime, nil
	case "duration":
		return e.Duration, nil
	case "mode":
		return e.Mode, nil
	case "variables":
		obj, err := e.handleVariablesGet(uriFrags)
		if err != nil {
			return nil, fmt.Errorf("failed to handle variables GET: %w", err)
		}
		return obj, nil
	case "repeat":
		obj, err := e.Repeat.handleGet(uriFrags)
		if err != nil {
			return nil, fmt.Errorf("failed to handle repeat GET: %w", err)
		}
		return obj, nil
	}

	return nil, ErrInvalidUri
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/variables.
func (e *Event) handleVariablesSet(uriFrags []string, body []byte) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		if err = json.Unmarshal(body, &e.Variables); err != nil {
			return nil, fmt.Errorf("failed to unmarshal variables map: %w", err)
		}
		return e.Variables, nil
	}

	varId := uriFrags[5]
	_, ok := e.Variables[varId]
	if !ok {
		return nil, fmt.Errorf("variable %s does not belong to mode %s", varId, e.Mode)
	}

	var newVarVal interface{}
	if err = json.Unmarshal(body, &newVarVal); err != nil {
		return nil, fmt.Errorf("failed to unmarshal variable %s value: %w", varId, err)
	}
	// validate function will verify data type is correct
	e.Variables[varId] = newVarVal
	return newVarVal, nil
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/variables.
func (e *Event) handleVariablesGet(uriFrags []string) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		return e.Variables, nil
	}

	varId := uriFrags[5]
	varValue, ok := e.Variables[varId]
	if ok {
		return varValue, nil
	}
	return nil, fmt.Errorf("could not find variable %s in variables map %v", varId, e.Variables)
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/repeat.
func (ser *Series) handleSet(uriFrags []string, body []byte) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		// non-repeating series object has most of the default values we want, except that EndCount should be initialized to 0.
		// the difference is that a write to an event object that does not contain a repeat field is allowed and should
		// default to a non-repeating series, but a write that is specifically to the repeat field of an existing event
		// object MUST include either end_count or end_time, and leaving out both should result in error - not default
		// to non-repeating
		newSettings := NewNonRepeatingSeries()
		newSettings.EndCount = 0
		if err = json.Unmarshal(body, newSettings); err != nil {
			return nil, fmt.Errorf("failed to unmarshal repeat object: %w", err)
		}
		newSettings.Id = ser.Id
		// Event::handleSet will deal with validation
		*ser = *newSettings
		return ser, nil
	}

	switch uriFrags[5] {
	case "cycle":
		if err = json.Unmarshal(body, &ser.Cycle); err != nil {
			return nil, fmt.Errorf("failed to unmarshal cycle: %w", err)
		}
		return ser.Cycle, nil
	case "day_mask":
		if err = json.Unmarshal(body, &ser.DayMask); err != nil {
			return nil, fmt.Errorf("failed to unmarshal day_mask: %w", err)
		}
		return ser.DayMask, nil
	case "end_count":
		if err = json.Unmarshal(body, &ser.EndCount); err != nil {
			return nil, fmt.Errorf("failed to unmarshal end_count: %w", err)
		}
		ser.EndTime = DefaultSeriesEndTime
		return ser.EndCount, nil
	case "end_time":
		if err = json.Unmarshal(body, &ser.EndTime); err != nil {
			return nil, fmt.Errorf("failed to unmarshal end_time: %w", err)
		}
		ser.EndCount = 0
		return ser.EndTime, nil
	case "exceptions":
		if err = json.Unmarshal(body, &ser.Exceptions); err != nil {
			return nil, fmt.Errorf("failed to unmarshal exceptions: %w", err)
		}
		return ser.Exceptions, nil
	case "frequency":
		if err = json.Unmarshal(body, &ser.Frequency); err != nil {
			return nil, fmt.Errorf("failed to unmarshal frequency: %w", err)
		}
		return ser.Frequency, nil
	}

	return nil, ErrInvalidUri
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/repeat.
func (ser *Series) handleGet(uriFrags []string) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		return ser, nil
	}

	switch uriFrags[5] {
	case "cycle":
		return ser.Cycle, nil
	case "day_mask":
		return ser.DayMask, nil
	case "end_count":
		return ser.EndCount, nil
	case "end_time":
		return ser.EndTime, nil
	case "exceptions":
		return ser.Exceptions, nil
	case "frequency":
		return ser.Frequency, nil
	}

	return nil, ErrInvalidUri
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/repeat.
func (ser *Series) handlePost(uriFrags []string, body []byte) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		return nil, ErrInvalidUri
	}

	switch uriFrags[5] {
	case "exceptions":
		var newException time.Time
		if err = json.Unmarshal(body, &newException); err != nil {
			return nil, fmt.Errorf("failed to unmarshal exception: %w", err)
		}
		ser.addException(newException)
		return ser.Exceptions, nil
	}

	return nil, ErrInvalidUri
}

// URI begins with /scheduler/events/<schedule ID>/<event ID>/repeat.
func (ser *Series) handleDel(uriFrags []string, body []byte) (replyObject interface{}, err error) {
	if len(uriFrags) < 6 {
		return nil, ErrInvalidUri
	}

	switch uriFrags[5] {
	case "exceptions":
		if len(uriFrags) == 6 {
			// DEL is to /scheduler/events/<schedule ID>/<event ID>/repeat/exceptions.
			// delete all exceptions
			ser.Exceptions = make([]time.Time, 0)
			return ser.Exceptions, nil
		}

		// DEL is to /scheduler/events/<schedule ID>/<event ID>/repeat/exceptions/<exception index>
		iExceptionString := uriFrags[6]
		iException, err := strconv.ParseUint(iExceptionString, 10, 32)
		if err != nil {
			return nil, fmt.Errorf("failed to parse exception index from %s: %w", iExceptionString, err)
		}
		if int(iException) >= len(ser.Exceptions) {
			return nil, fmt.Errorf("parsed exception index of %d but there are only %d exceptions", iException, len(ser.Exceptions))
		}
		ser.deleteException(ser.Exceptions[iException])
		return ser.Exceptions, nil
	}

	return nil, ErrInvalidUri
}
