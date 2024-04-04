// Package events contains logic for creating and tracking Scheduler events.
//
// While this package is primarily used by Scheduler, non-Scheduler applications can use this library to create events
// with the proper JSON shape to send to the Scheduler application over a network connection.
package events

import (
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	"github.com/flexgen-power/scheduler/internal/flextime"
	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/internal/setpoint"
)

// Event is the lowest-level object containing fundamental data about a Scheduler event
type Event struct {
	Id        idcache.Id             `json:"id"`         // unique ID allowing the event to be identified independently from the start time
	StartTime time.Time              `json:"start_time"` // timestamp of when the event should start.
	Duration  uint                   `json:"duration"`   // how long before the event should end, expressed as minutes
	Mode      string                 `json:"mode"`       // name of the Scheduler mode the event is using.
	Variables map[string]interface{} `json:"variables"`  // map of event variable IDs and values.
	Repeat    *Series                `json:"repeat"`     // information about if / when / how much the event should repeat
}

// CreateEvent allocates memory for a new event object and initializes it
// with the given start time, duration, and mode.
func CreateEvent(startTime time.Time, duration time.Duration, mode string) *Event {
	return &Event{
		StartTime: startTime,
		Duration:  uint(duration.Minutes()),
		Mode:      mode,
		Variables: make(map[string]interface{}),
		Repeat:    NewNonRepeatingSeries(),
	}
}

// Parses the JSON representation of an event. In order to validate selected mode against parsed variables,
// must be given a map of mode IDs to mode variable lists.
func ParseEvent(input []byte, timeZone *time.Location, mapOfVariables map[string]setpoint.List) (parsedEvent Event, err error) {
	if err = json.Unmarshal(input, &parsedEvent); err != nil {
		return parsedEvent, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = parsedEvent.Validate(timeZone, mapOfVariables); err != nil {
		return parsedEvent, fmt.Errorf("failed to validate event settings: %w", err)
	}
	return parsedEvent, nil
}

func (e *Event) TimeDuration() time.Duration {
	return time.Duration(e.Duration) * time.Minute
}

func (e *Event) Validate(timeZone *time.Location, mapOfVariables map[string]setpoint.List) error {
	if e == nil {
		return errors.New("nil pointer")
	}

	// want to support receiving timestamps of any time zone, but normalize to the given time zone.
	// do not allow edits prior to time zone being established
	if timeZone == nil {
		return fmt.Errorf("time zone is nil")
	}
	e.StartTime = e.StartTime.In(timeZone)

	// unmarshal duration into int field that represents minutes, then normalize it into time.Duration field
	if err := validateDuration(e.TimeDuration()); err != nil {
		return fmt.Errorf("invalid duration: %w", err)
	}

	// modes are allowed to have no variables and all constants
	if e.Variables == nil {
		e.Variables = make(map[string]interface{})
	}

	// validate received variables against identified mode
	modeVariables, ok := mapOfVariables[e.Mode]
	if !ok {
		return fmt.Errorf("mode %s not found", e.Mode)
	}
	if len(modeVariables) != len(e.Variables) {
		return fmt.Errorf("variables map has %d entries but corresponding mode %s has %d", len(e.Variables), e.Mode, len(modeVariables))
	}
	for _, modeVar := range modeVariables {
		var err error
		switch eventVar := e.Variables[modeVar.Id].(type) {
		case map[string]interface{}:
			// get the inner value
			_, err := parsemap.ExtractValueWithType(eventVar, "value", parsemap.BOOL)
			if err != nil {
				return fmt.Errorf("error while extracting %v value: %w", eventVar, err)
			}
			_, err = parsemap.ExtractValueWithType(eventVar, "batch_value", parsemap.INTERFACE_SLICE)
			if err != nil {
				return fmt.Errorf("error while extracting %v batch_value: %w", eventVar, err)
			}
		default:
			switch modeVar.VarType {
			case "Float":
				_, err = parsemap.ExtractValueWithType(e.Variables, modeVar.Id, parsemap.FLOAT64)
				if err != nil {
					return fmt.Errorf("error while extracting float value: %w", eventVar, err)
				}
			case "Int":
				var valueInt int
				valueInt, err = parsemap.ExtractAsInt(e.Variables, modeVar.Id)
				if err != nil {
					return fmt.Errorf("error while extracting int value: %w", eventVar, err)
				}
				e.Variables[modeVar.Id] = valueInt
			case "Bool":
				_, err = parsemap.ExtractValueWithType(e.Variables, modeVar.Id, parsemap.BOOL)
				if err != nil {
					return fmt.Errorf("error while extracting bool value: %w", eventVar, err)
				}
			case "String":
				_, err = parsemap.ExtractValueWithType(e.Variables, modeVar.Id, parsemap.STRING)
				if err != nil {
					return fmt.Errorf("error while extracting string value: %w", eventVar, err)
				}
			}
		}
	}

	// when repeat settings not included in larger event and/or schedule object, default to non-repeating series
	if e.Repeat == nil {
		e.Repeat = NewNonRepeatingSeries()
	}

	// validate repeat settings
	if err := e.Repeat.validate(e.StartTime); err != nil {
		return fmt.Errorf("invalid repeat settings: %w", err)
	}
	return nil
}

func validateDuration(dur time.Duration) error {
	if dur >= 24*time.Hour {
		return errors.New("duration must be less than 24 hours")
	}
	return nil
}

// Shifts event to next iteration of series. If Spring Forward DST conflict is found, skip that event in the series, isolate it,
// and return it as an independent event to be handled by the caller.
func (e *Event) ShiftSeries() (springForwardEvent *Event, seriesOver bool) {
	if e == nil {
		return nil, true
	}

	// calculate the next time the event will occur based on series' repeat settings
	daysUntilNextOccurrence := e.Repeat.calculateDaysUntilNextOccurrence(e.StartTime.Weekday())
	midnightBeforeNext := flextime.GetMidnightInNDays(int(daysUntilNextOccurrence), e.StartTime)
	nextStartTime := flextime.ApplyNewDateToTime(midnightBeforeNext, e.StartTime)

	// if the next start time is on a Spring Forward Daylight Saving Time day, handle this special case
	if nextStartTime.Hour() != e.StartTime.Hour() {
		// split the Spring Forward event from the series as a one-off event
		springForwardEvent = e.Copy()
		springForwardEvent.Repeat = NewNonRepeatingSeries()
		springForwardEvent.StartTime = nextStartTime

		// if the series had an exception for the Spring Forward event, transfer that exception to the new one-off event
		if e.Repeat.hasException(nextStartTime) {
			e.Repeat.deleteException(nextStartTime)
			springForwardEvent.Repeat.addException(nextStartTime)
		}
		log.Infof("Splitting Spring Forward event from series into one-off event: %+v with repeat settings %+v.", springForwardEvent, springForwardEvent.Repeat)

		// update the series end field.
		// if it is over, then no need to shift the series past the Spring Forward event so return early
		seriesOver = e.Repeat.updateSeriesEnd(nextStartTime)
		if seriesOver {
			return springForwardEvent, true
		}

		// if the series is not over, calculate the next iteration's start time after the Spring Forward event since the Spring Forward event has been split off
		daysUntilNextOccurrence += e.Repeat.calculateDaysUntilNextOccurrence(nextStartTime.Weekday())
		midnightBeforeNext = flextime.GetMidnightInNDays(int(daysUntilNextOccurrence), e.StartTime)
		nextStartTime = time.Date(midnightBeforeNext.Year(), midnightBeforeNext.Month(), midnightBeforeNext.Day(), e.StartTime.Hour(), e.StartTime.Minute(), e.StartTime.Second(), e.StartTime.Nanosecond(), e.StartTime.Location())
	}

	// assign the next iteration's start time to the series event
	e.StartTime = nextStartTime

	// update the series end field, noting if it has actually ended
	seriesOver = e.Repeat.updateSeriesEnd(e.StartTime)

	// have the function caller handle the special cases of a Spring Foward event being split off from the series, and the series ending
	return springForwardEvent, seriesOver
}

func (e *Event) HasException(timestamp time.Time) bool {
	if e == nil {
		return false
	}

	return e.Repeat.hasException(timestamp)
}

// AddVariable takes a variable ID and value and adds them to the event's variables map.
// If the variable already exists in the variables map, its value is overwritten.
func (e *Event) AddVariable(id string, val interface{}) {
	e.Variables[id] = val
}

// Update gives an event new settings
func (e *Event) Update(x *Event) {
	*e = *x.Copy()
}

func (e *Event) Print() {
	fmt.Println("start time:", e.StartTime)
	fmt.Println("duration:", e.Duration)
	fmt.Println("mode:", e.Mode)
	fmt.Println("variables:")
	i := 1
	for k, v := range e.Variables {
		fmt.Println("variable", i)
		fmt.Println("id:", k)
		fmt.Println("uri:", v)
		i++
	}
	e.Repeat.print()
	fmt.Printf("\n")
}

// Deep copy a given event
func (e *Event) Copy() *Event {
	if e == nil {
		return nil
	}
	newEvent := *e
	newEvent.Variables = make(map[string]interface{})
	for key, val := range e.Variables {
		newEvent.Variables[key] = val
	}
	newEvent.Repeat = e.Repeat.copy()
	return &newEvent
}

// Checks if any data in e2 is not equal to the data in e1, besides duration.
// Used to protect edits to active events, where only duration edits are allowed.
func (e1 *Event) onlyDurationCanBeDifferent(e2 *Event) (areEqual bool, reasonNotEqual string) {
	// temporarily store e1's duration and set e1's duration to e2's duration so Equals function call does not check that diff
	tempE1Duration := e1.Duration
	e1.Duration = e2.Duration
	equalBesidesDuration, reasonNotEqual := e1.Equals(e2)
	e1.Duration = tempE1Duration
	return equalBesidesDuration, reasonNotEqual
}

// Compares an event to another event and returns if they match or not.
// If they do not match, also returns a string explaining the reason.
func (e1 *Event) Equals(e2 *Event) (areEqual bool, reasonNotEqual string) {
	// nil checks
	if e1 == nil {
		if e2 != nil {
			return false, "e1 is nil but e2 is not"
		}
		return true, ""
	}
	if e2 == nil {
		return false, "e1 is not nil but e2 is"
	}

	if !e1.StartTime.Equal(e2.StartTime) {
		return false, fmt.Sprintf("start time %s does not equal start time %s", e1.StartTime.String(), e2.StartTime.String())
	}

	if e1.Duration != e2.Duration {
		return false, fmt.Sprintf("duration %d does not equal duration %d", e1.Duration, e2.Duration)
	}

	if e1.Mode != e2.Mode {
		return false, fmt.Sprintf("mode %s does not equal mode %s", e1.Mode, e2.Mode)
	}

	if len(e1.Variables) != len(e2.Variables) {
		return false, fmt.Sprintf("number of variables %d does not equal number of variables %d", len(e1.Variables), len(e2.Variables))
	}

	// check each variable value, also checking if the same variables are in each map
	for k, v1 := range e1.Variables {
		v2, ok := e2.Variables[k]
		if !ok {
			// since variable array lengths were already verified to be equal, do not need to check that all e2 vars are in e1
			return false, fmt.Sprintf("variable %s is in e1 but missing from e2", k)
		}
		if reflect.TypeOf(v1) == reflect.TypeOf(v2) {
			if T1, ok := v1.(map[string]interface{}); ok {
				mapsEqual := true
				// Check if the maps have the same number of key-value pairs
				T2, ok := v2.(map[string]interface{})
				if !ok {
					return false, fmt.Sprintf("types for v1 and v2 are not the same.")
				}
				if len(T1) != len(T2) {
					mapsEqual = false
				}

				// Iterate over each key-value pair in the first map
				for key, val1 := range T1 {
					// Check if the key exists in the second map
					val2, ok := T2[key]
					if !ok {
						mapsEqual = false
					}

					// Check if the values are equal
					if !reflect.DeepEqual(val1, val2) {
						mapsEqual = false
					}
				}
				if !mapsEqual {
					return false, fmt.Sprintf("variable %s's value %v does not equal %v", k, v1, v2)
				}
			} else {
				if v1 != v2 {
					return false, fmt.Sprintf("variable %s's value %v does not equal %v", k, v1, v2)
				}
			}
		}
	}

	if areEqual, reasonNotEqual := e1.Repeat.equals(e2.Repeat); !areEqual {
		return false, fmt.Sprintf("repeat settings are not equal: %s", reasonNotEqual)
	}
	return true, ""
}

// Determines if there is continuity between events. Continuity is defined as the first event ending at the same time
// as the following event starts, and both events having the same mode. This would mean that default sepoints would
// cause flicker so they should not be sent out.
func (firstEvent *Event) HasContinuousTransition(followingEvent *Event) bool {
	thisEventEndTime := firstEvent.StartTime.Add(firstEvent.TimeDuration())
	return thisEventEndTime.Equal(followingEvent.StartTime) && firstEvent.Mode == followingEvent.Mode
}

// Returns if the current time is after the event's start time + duration.
func (e *Event) IsDone(currentTime time.Time) bool {
	return currentTime.After(e.StartTime.Add(e.TimeDuration()))
}

// Returns if an event's start minute has passed.
// Do not want 1 second past start minute to be considered "passed".
// Needs to be in the next minute.
func (e *Event) PassedStartTime(currentTime time.Time) bool {
	oneMinuteFromStartTime := e.StartTime.Add(time.Minute)
	return !currentTime.Before(oneMinuteFromStartTime)
}

// Returns if an event's start time is right now or passed.
func (e *Event) ReadyToStart(currentTime time.Time) bool {
	return !currentTime.Before(e.StartTime)
}

func (e *Event) HasOneOfStartTimes(startTimes []time.Time) bool {
	if e == nil {
		return false
	}
	for _, t := range startTimes {
		if e.StartTime.Equal(t) {
			return true
		}
	}
	return false
}
