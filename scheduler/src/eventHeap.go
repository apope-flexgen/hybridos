/**
 *
 * eventHeap.go
 *
 * Methods for the eventHeap data type, which is a min-heap storing a site's schedule.
 * The earliest event in the schedule will be at index 0 in the heap.
 *
 */
package main

import (
	"container/heap"
	"errors"
	"fmt"
	"log"
	"time"

	events "github.com/flexgen-power/scheduler/pkg/events"
)

// eventHeap implements heap.Interface and holds event data for a single site.
type eventHeap []*events.Event

// newEventHeap returns a pointer to a new eventHeap.
func newEventHeap() *eventHeap {
	return &eventHeap{}
}

// Len returns the number of events in the eventHeap.
func (eHeap eventHeap) Len() int {
	return len(eHeap)
}

// Less returns if event i starts before event j.
func (eHeap eventHeap) Less(i, j int) bool {
	return eHeap[i].StartTime.Before(eHeap[j].StartTime)
}

// Swap switches the order of two events in the eventHeap.
// It does not modify startTime, just the index in the array.
func (eHeap eventHeap) Swap(i, j int) {
	eHeap[i], eHeap[j] = eHeap[j], eHeap[i]
}

// Push inserts an event onto an eventHeap.
func (eHeap *eventHeap) Push(e interface{}) {
	*eHeap = append(*eHeap, e.(*events.Event))
}

// Pop removes and returns the last event in the array.
// Note: the last event in the array is not necessarily the event with the latest start time.
func (eHeap *eventHeap) Pop() interface{} {
	old := *eHeap
	n := len(old)
	e := old[n-1]
	*eHeap = old[0 : n-1]
	return e
}

// Peek returns the earliest event in an eventHeap without removing it.
func (eHeap *eventHeap) peek() (*events.Event, error) {
	if eHeap.Len() == 0 {
		return nil, errors.New("eventHeap::Peek() ~ Tried to peek empty heap")
	}
	e := heap.Pop(eHeap).(*events.Event)
	heap.Push(eHeap, e)
	return e, nil
}

// PeekLast returns the latest event in an eventHeap without removing it.
func (eHeap *eventHeap) peekLast() (*events.Event, error) {
	numEvents := eHeap.Len()
	if numEvents == 0 {
		return nil, errors.New("eventHeap::peekLast() ~ Tried to peek empty heap")
	}
	eArray := make([]*events.Event, numEvents)
	// Pop all of the events in order to get the last one
	for i := 0; i < numEvents; i++ {
		eArray[i] = heap.Pop(eHeap).(*events.Event)
	}
	e := eArray[numEvents-1]
	// Push them back onto the heap
	for i := 0; i < numEvents; i++ {
		heap.Push(eHeap, eArray[i])
	}
	// Return the last event
	return e, nil
}

// print prints formatted event data to the log.
func (eHeap *eventHeap) print() {
	for i, e := range *eHeap {
		log.Printf("Event %v:\n", i)
		e.Print()
	}
}

// get returns a pointer to the event that starts at the specified time, as well as the event's index.
func (eHeap *eventHeap) get(t time.Time) (*events.Event, int) {
	for i, v := range *eHeap {
		if v.StartTime.Equal(t) {
			return v, i
		}
	}
	return nil, -1
}

// has returns a bool indicating if the eventHeap has an event that starts
// at the same time as the passed-in event.
func (eHeap *eventHeap) hasEventWithSameStartAs(e *events.Event) bool {
	eFound, _ := eHeap.get(e.StartTime)
	return eFound != nil
}

// Set updates an existing event at a specified time with new settings.
func (eHeap *eventHeap) Set(x *events.Event) error {
	e, _ := eHeap.get(x.StartTime)
	if e != nil {
		e.Update(x)
		return nil
	}
	return errors.New("eventHeap::Set() ~ Could not find event at given start time")
}

// copy deep copies a given eventHeap.
func (eHeap eventHeap) copy() *eventHeap {
	// If the given heap is empty, return an empty heap
	if eHeap.Len() == 0 {
		return &eventHeap{}
	}
	var newHeap eventHeap
	// Copy each event in eHeap and add copies to newHeap
	for _, e := range eHeap {
		newHeap = append(newHeap, e.Copy())
	}
	// Ensure new heap is valid
	heap.Init(&newHeap)
	return &newHeap
}

// deleteEventsWithMode deletes any events from an eventHeap that use the indicated mode.
func (eHeap *eventHeap) deleteEventsWithMode(modeBeingDeleted string) {
	for i, e := range *eHeap {
		if e.Mode == modeBeingDeleted {
			heap.Remove(eHeap, i)
			eHeap.deleteEventsWithMode(modeBeingDeleted) // use recursion to delete multiple events with the mode since indices will change on every heap.Remove call
			break
		}
	}
}

// equals compares an eventHeap to another eventHeap and returns if they equal each other or not.
func (eHeap1 *eventHeap) equals(eHeap2 *eventHeap) bool {
	// if the eventHeaps have a different number of events, they are not equal
	if eHeap1.Len() != eHeap2.Len() {
		log.Println("events.go::eventHeap::equals ~ Day schedules do not have same number of events")
		return false
	}
	// check each event of eHeap1 against the event that has the same start time in eHeap2
	for _, e1 := range *eHeap1 {
		e2, _ := eHeap2.get(e1.StartTime)
		// if e2 == nil, then eHeap2 does not have an event matching the start time of eHeap1's event
		if e2 == nil {
			log.Println("events.go::eventHeap::equals ~ eHeap2 does not have an event starting at", e1.StartTime)
			return false
		}
		if !e1.Equals(e2) {
			log.Println("events.go::eventHeap::equals ~ Event starting at", e1.StartTime, "failed equality check")
			return false
		}
	}
	return true
}

// deleteVarOfMode iterates through each event of an eventHeap and
// deletes the given variable from any events that use the given mode.
func (eHeap *eventHeap) deleteVarOfMode(varID, changedMode string) {
	for _, e := range *eHeap {
		if e.Mode == changedMode {
			delete((*e).Variables, varID)
		}
	}
}

// checkForOverlaps checks an event against all events in an eventHeap.
func (eHeap *eventHeap) checkForOverlaps(eNew *events.Event) bool {
	for _, e := range *eHeap {
		if eNew.Overlaps(e) {
			return true
		}
	}
	return false
}

// hasOverlaps is an internal check to see if any events within the eventHeap are overlapping with each other.
func (eHeap *eventHeap) hasOverlaps() bool {
	for i, e := range *eHeap {
		for j := i + 1; j < eHeap.Len(); j++ {
			if e.Overlaps((*eHeap)[j]) {
				return true
			}
		}
	}
	return false
}

// buildObj builds a []interface{} object to represent an eventHeap in JSON format.
func (eHeap *eventHeap) buildObj() *[]interface{} {
	obj := make([]interface{}, 0)
	eHeap.addToList(&obj)
	return &obj
}

// addToList adds an eventHeap's events to an existing list.
func (eHeap *eventHeap) addToList(list *[]interface{}) {
	for _, eventObj := range *eHeap {
		eventObj.AddToList(list)
	}
}

// addToLegacyList adds an eventHeap's events to an existing list.
// start time is given as mins since midnight
func (eHeap *eventHeap) addToLegacyList(list *[]interface{}) {
	for _, eventObj := range *eHeap {
		eventObj.AddToLegacyList(list)
	}
}

// appendHeap takes an input eventHeap and adds all of its events to a receiver eventHeap.
// If there are overlapping events, the receiver eventHeap's event is edited to match the input event.
func (eHeap1 *eventHeap) appendHeap(eHeap2 *eventHeap) {
	for _, e := range *eHeap2 {
		eHeap1.appendEvent(e)
	}
}

// appendEvent takes an input event and adds it to a receiver eventHeap.
// If the event matches one of the events already in the receiver eventHeap,
// the receiver eventHeap's event is edited to match the input event.
func (eHeap *eventHeap) appendEvent(e *events.Event) {
	if matchingEvent, _ := eHeap.get(e.StartTime); matchingEvent != nil {
		matchingEvent.Update(e)
	} else {
		heap.Push(eHeap, e)
	}
}

// subtractHeap take an input eventHeap and removes any matching events from a receiver eventHeap.
func (eHeap1 *eventHeap) subtractHeap(eHeap2 *eventHeap) {
	result := newEventHeap()
	for _, e := range *eHeap1 {
		if !eHeap2.hasEventWithSameStartAs(e) {
			heap.Push(result, e.Copy())
		}
	}
	*eHeap1 = *result
}

// parseEventListIntoHeap expects a specific format of object (see set_events.sh script in test folder for details).
// The objects in the new event list will be parsed and added to the given eventHeap.
func parseEventListIntoHeap(newEvents []interface{}, existingEvents *eventHeap, dayIndex int, timeZone *time.Location) error {
	// iterate through each element of the new event list and parse it
	for i, eInterface := range newEvents {
		// verify event details are presented in map
		eMap, ok := eInterface.(map[string]interface{})
		if !ok {
			// If not a list of map[string]interface{}, check if it's a nested list
			eList, ok := eInterface.([]interface{})
			if !ok {
				return fmt.Errorf("event with index %d is not a map[string]interface or nested map[string]interface. Actual type is %T", i, eInterface)
			}
			// Because this is a nested structure, we know that the day index is the index of the inner list within the outer list
			err := parseEventListIntoHeap(eList, existingEvents, i, timeZone)
			if err != nil {
				return err
			}
		} else {
			// parse event
			e, err := parseEvent(&eMap, dayIndex, timeZone)
			if err != nil {
				return fmt.Errorf("failed to parse event with index %d: %w", i, err)
			}

			// add event to the eventHeap
			existingEvents.appendEvent(e)
		}
	}
	return nil
}
