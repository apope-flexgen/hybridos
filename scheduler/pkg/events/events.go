package events

/*
 * The events package provides a way for non-Scheduler applications to build Scheduler events to
 * send to the Scheduler application. The API frees the user from having to know the internal implementation
 * of Scheduler events.
 *
 */

import (
	"log"
	"time"
)

// Event is the lowest-level object containing fundamental data about a Scheduler event
type Event struct {
	StartTime  time.Time              // timestamp of when the event should start.
	Duration   time.Duration          // how long before the event should end.
	Mode       string                 // name of the Scheduler mode the event is using.
	Variables  map[string]interface{} // map of event variable IDs and values.
	NonRolling bool                   // whether or not the event should not be copied to a new day's schedule at rollover
}

// CreateEvent constructs an Event with the given start time, duration, and mode.
func CreateEvent(startTime time.Time, duration time.Duration, mode string) *Event {
	return &Event{
		StartTime:  startTime,
		Duration:   duration,
		Mode:       mode,
		Variables:  make(map[string]interface{}),
		NonRolling: true,
	}
}

// AddVariable takes a variable ID and value and adds them to the event's variables map.
// If the variable already exists in the variables map, its value is overwritten.
func (e *Event) AddVariable(id string, val interface{}) {
	e.Variables[id] = val
}

// BuildObj converts an event to a map[string]interface{} for easy FIMS sending as a JSON.
func (e *Event) BuildObj() map[string]interface{} {
	eMap := make(map[string]interface{})
	eMap["start_timestamp"] = e.StartTime
	eMap["duration"] = e.Duration.Minutes()
	eMap["mode"] = e.Mode
	varMap := make(map[string]interface{})
	// add each variable to the event
	for varID, varValue := range e.Variables {
		varMap[varID] = varValue
	}
	eMap["variables"] = varMap
	eMap["nonRolling"] = e.NonRolling
	return eMap
}

// Returns true if the event's start time has the given calendar day after being
// shifted to the given time zone.
func (e *Event) HasCalendarDay(calendarDay int, timezone *time.Location) bool {
	return e.StartTime.In(timezone).Day() == calendarDay
}

// BuildLegacyObj converts an event to a map[string]interface{} for easy FIMS sending as a JSON.
// The timestamp is given in mins since midnight
func (e *Event) BuildLegacyObj(now, midnightDayOf time.Time) map[string]interface{} {
	eMap := make(map[string]interface{})
	eMap["start_time"] = e.StartTime.Sub(midnightDayOf).Minutes()
	eMap["duration"] = e.Duration.Minutes()
	eMap["mode"] = e.Mode
	varMap := make(map[string]interface{})
	// add each variable to the event
	for varID, varValue := range e.Variables {
		varMap[varID] = varValue
	}
	eMap["variables"] = varMap
	eMap["nonRolling"] = e.NonRolling
	return eMap
}

// newEvent allocates memory for a new event object
func NewEvent() *Event {
	var e Event
	e.Variables = make(map[string]interface{})
	return &e
}

// Update gives an event new settings
func (e *Event) Update(x *Event) {
	e.Duration = x.Duration
	e.StartTime = x.StartTime
	e.Mode = x.Mode
	e.Variables = x.Variables
	e.NonRolling = x.NonRolling
}

// converts an event to a map[string]interface{} and adds it to a list of events for easy FIMS sending
func (e *Event) AddToList(list *[]interface{}) {
	eMap := e.BuildObj()
	*list = append(*list, eMap)
}

// converts an event to a map[string]interface{} and adds it to a list of events for easy FIMS sending
// start time is given as mins since midnight
func (e *Event) AddToLegacyList(list *[]interface{}, now, midnightDayOf time.Time) {
	eMap := e.BuildLegacyObj(now, midnightDayOf)
	*list = append(*list, eMap)
}

func (e *Event) Print() {
	log.Println("start time:", e.StartTime)
	log.Println("duration:", e.Duration)
	log.Println("mode:", e.Mode)
	log.Println("variables:")
	i := 1
	for k, v := range e.Variables {
		log.Println("variable", i)
		log.Println("id:", k)
		log.Println("uri:", v)
		i++
	}
	log.Println("nonRolling:", e.NonRolling)
	log.Printf("\n")
}

// Deep copy a given event
func (e *Event) Copy() *Event {
	// If given event is nil, return nil
	if e == nil {
		return nil
	}
	var newEvent Event
	newEvent.StartTime = e.StartTime
	newEvent.Duration = e.Duration
	newEvent.Mode = e.Mode
	newEvent.Variables = make(map[string]interface{})
	for key, val := range e.Variables {
		newEvent.Variables[key] = val
	}
	newEvent.NonRolling = e.NonRolling
	return &newEvent
}

// returns if two events are overlapping
func (e1 *Event) Overlaps(e2 *Event) bool {
	return (e1.StartTime.Before(e2.StartTime) && e1.StartTime.Add(e1.Duration).After(e2.StartTime)) || (e2.StartTime.Before(e1.StartTime) && e2.StartTime.Add(e2.Duration).After(e1.StartTime))
}

// equals compares an event to another event and returns if they match or not
func (e1 *Event) Equals(e2 *Event) bool {
	// Compare start time, duration, mode, and number of variables
	if e2 == nil || !e1.StartTime.Equal(e2.StartTime) || e1.Duration != e2.Duration || e1.Mode != e2.Mode || len(e1.Variables) != len(e2.Variables) || e1.NonRolling != e2.NonRolling {
		return false
	}
	// Check each variable value, also checking if the same variables are in each map
	for k, v1 := range e1.Variables {
		if v2, ok := e2.Variables[k]; ok {
			if v1 != v2 {
				return false
			}
		} else {
			return false
		}
	}
	return true
}

// determines whether there is continuity between events, i.e. the other event starts exactly when this one ends (on the minute)
// and the two events share the same mode
func (thisEvent *Event) HasContinuousTransition(otherEvent *Event) bool {
	if thisEvent == nil || otherEvent == nil {
		return false
	}

	// End time of this event, ensuring 1 min precision
	thisEventEndTime := thisEvent.StartTime.Add(thisEvent.Duration)
	// Start time of other event, ensuring 1 min precision in event 1 timezone
	otherEventStartTime := otherEvent.StartTime.In(thisEvent.StartTime.Location())
	return thisEventEndTime.Equal(otherEventStartTime) && thisEvent.Mode == otherEvent.Mode
}
