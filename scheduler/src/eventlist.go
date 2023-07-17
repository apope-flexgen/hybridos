package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"sort"
	"time"

	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/internal/setpoint"
	events "github.com/flexgen-power/scheduler/pkg/events"
)

type eventList []*events.Event

var ErrOverlappingEvents = errors.New("overlapping events")

// Returns the number of events in the eventList.
// Required to implement sort.Interface.
func (eList *eventList) Len() int {
	return len(*eList)
}

// Returns if event i is "less than" event j.
// The order of checks that take place to determine this: start time, not having exception at start time, duration, mode name, ID.
// Required to implement sort.Interface.
func (eList *eventList) Less(i, j int) bool {
	event1 := (*eList)[i]
	event2 := (*eList)[j]
	// if event start times are not equal, event that starts first is "Less"
	if event1.StartTime.Before(event2.StartTime) {
		return true
	}
	if event1.StartTime.After(event2.StartTime) {
		return false
	}

	// if start times are equal, assumption is that one or both have exceptions at the start time
	// to prevent overlap. in this case, the event that will actually occur at the given start time
	// should be considered "Less"
	event1HasException := event1.HasException(event1.StartTime)
	event2HasException := event2.HasException(event2.StartTime)
	if event1HasException != event2HasException { // think of this as a XOR
		return !event1HasException
	}

	// at this point, start times are equal and assuming both events have exceptions at this start time.
	// next tie-breaker is duration
	if event1.Duration != event2.Duration {
		return event1.Duration < event2.Duration
	}

	// out of meaningful tie-breakers. try mode name then just do an ID comparison
	if event1.Mode != event2.Mode {
		return event1.Mode < event2.Mode
	}
	return event1.Id < event2.Id
}

// Switches the position of two events in the eventList.
// Required to implement sort.Interface.
func (eList *eventList) Swap(i, j int) {
	(*eList)[i], (*eList)[j] = (*eList)[j], (*eList)[i]
}

func (eList *eventList) add(event *events.Event) {
	*eList = append(*eList, event)
	sort.Sort(eList)
}

func (eList *eventList) remove(index int) {
	if index < 0 || index >= len(*eList) {
		return
	}
	*eList = append((*eList)[:index], (*eList)[index+1:]...)
}

// Returns a pointer to the event that starts at the specified time, as well as the event's index.
func (eList *eventList) get(t time.Time) (*events.Event, int) {
	for i, v := range *eList {
		if v.StartTime.Equal(t) {
			return v, i
		}
	}
	return nil, -1
}

// If the spring-forward event, which has been offset forward by an hour, overlaps with other events that were already in that position, resolve the conflict.
// If the overlap occurs on the ending side of the spring-forward event, trim the event.
// If the overlap occurs on the starting side of the spring-forward event, do not add it to the schedule.
// Return whether or not the spring-forward event was able to be re-integrated into the schedule.
func (eList *eventList) handleSpringForwardEvent(springForwardEvent *events.Event) (successfullyRescheduled bool) {
	for _, e := range *eList {
		if springForwardEvent.Overlaps(e) {
			if springForwardEvent.StartTime.Before(e.StartTime) {
				springForwardEvent.Duration = uint(e.StartTime.Sub(springForwardEvent.StartTime).Minutes())
			} else {
				return false
			}
		}
	}
	eList.add(springForwardEvent)
	return true
}

// Updates an existing event at a specified time with new settings.
func (eList *eventList) Set(x *events.Event) error {
	e, _ := eList.get(x.StartTime)
	if e != nil {
		e.Update(x)
		return nil
	}
	return errors.New("could not find an event with the given start time")
}

// Deep copies a given eventList.
func (eList eventList) copy() *eventList {
	newList := make(eventList, 0, len(eList))
	for _, e := range eList {
		newList = append(newList, e.Copy())
	}
	return &newList
}

func (eList *eventList) getIdsOfEventsWithMode(targetMode string) []idcache.Id {
	ids := make([]idcache.Id, 0)
	for _, e := range *eList {
		if e.Mode == targetMode {
			ids = append(ids, e.Id)
		}
	}
	return ids
}

func (eList *eventList) deleteEventWithId(id idcache.Id) {
	index, _ := eList.getEventWithId(id)
	if index >= 0 {
		eList.remove(index)
	}
}

// Returns the index and a pointer to the event with the given ID.
// If no event has the given ID, returns -1 and nil.
func (eList *eventList) getEventWithId(id idcache.Id) (index int, event *events.Event) {
	for i, e := range *eList {
		if e.Id == id {
			return i, e
		}
	}
	return -1, nil
}

func (eList *eventList) getStartTimes() []time.Time {
	times := make([]time.Time, 0, len(*eList))
	for _, e := range *eList {
		times = append(times, e.StartTime)
	}
	return times
}

// Iterates through each event of an eventList and deletes the
// given variable from any events that use the given mode.
func (eList *eventList) deleteVarOfMode(varID, changedMode string) {
	for _, e := range *eList {
		if e.Mode == changedMode {
			delete((*e).Variables, varID)
		}
	}
}

// Checks if any events within the list are overlapping with each other.
func (eList *eventList) hasOverlaps() (overlapFound bool, e1, e2 *events.Event) {
	for i, e1 := range *eList {
		for j := i + 1; j < eList.Len(); j++ {
			e2 := (*eList)[j]
			if e1.Overlaps(e2) {
				return true, e1, e2
			}
		}
	}
	return false, nil, nil
}

// Adds all events from the given eventList to the receiver eventList.
func (eList1 *eventList) addList(eList2 *eventList) {
	for _, e := range *eList2 {
		eList1.add(e)
	}
}

func parseEventList(input interface{}, timeZone *time.Location) (eList eventList, err error) {
	jsonBytes, err := json.Marshal(input)
	if err != nil {
		return nil, fmt.Errorf("failed to marshal input: %w", err)
	}

	if err = json.Unmarshal(jsonBytes, &eList); err != nil {
		return nil, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = eList.validate(timeZone, modes.buildMapOfVariables()); err != nil {
		return nil, fmt.Errorf("failed to validate event list: %w", err)
	}
	return eList, nil
}

// Checks that the list is instantiated and all contained events are valid.
// Will also sort the list's events in order (see Event::Less method).
func (eList *eventList) validate(timeZone *time.Location, mapOfVariables map[string]setpoint.List) error {
	if eList == nil {
		return errors.New("nil pointer")
	}
	if *eList == nil {
		return errors.New("nil slice")
	}
	for i, e := range *eList {
		if err := e.Validate(timeZone, mapOfVariables); err != nil {
			return fmt.Errorf("event with index %d is invalid: %w", i, err)
		}
	}
	if hasOverlaps, e1, e2 := eList.hasOverlaps(); hasOverlaps {
		return fmt.Errorf("%w: %+v with repeat settings %+v and %+v with repeat settings %+v", ErrOverlappingEvents, *e1, *e1.Repeat, *e2, *e2.Repeat)
	}
	sort.Sort(eList)
	return nil
}

// Adds every event's ID to a new ID cache.
func (eList *eventList) buildIdCache() idcache.IdCache {
	cache := idcache.New()
	for _, e := range *eList {
		cache.Add(e.Id)
	}
	return cache
}

func (eList1 eventList) equals(eList2 eventList) (areEqual bool, reasonNotEqual string) {
	if eList1 == nil {
		if eList2 == nil {
			return true, ""
		}
		return false, "eList1 is nil but eList2 is not"
	}
	if eList2 == nil {
		return false, "eList2 is nil but eList1 is not"
	}

	if eList1.Len() != eList2.Len() {
		return false, fmt.Sprintf("eList1 has length %d but eList2 has length %d", eList1.Len(), eList2.Len())
	}

	for i, event := range eList1 {
		if areEqual, reasonNotEqual := event.Equals(eList2[i]); !areEqual {
			return false, fmt.Sprintf("events at index %d are not equal: %s", i, reasonNotEqual)
		}
	}
	return true, ""
}
