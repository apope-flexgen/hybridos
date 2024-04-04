/**
 *
 * schedule.go
 *
 * Methods for the `schedule` data type.
 * Some methods specific to the Fleet-Site interface are in fleet_site_interface.go.
 *
 */
package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"reflect"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	"github.com/flexgen-power/scheduler/internal/flextime"
	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/pkg/events"
)

type schedule struct {
	id              string         // same as map key corresponding to the schedule instance
	timezone        *time.Location // timezone of the machine associated with this schedule (https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
	scheduledEvents *eventList
	expiredEvents   *eventList
	activeEvent     *events.Event // activeEvent == nil if no active event
	cache           idcache.IdCache
}

// eventStatus identifies how an event fits into a schedule
type eventStatus int

// Types of events with respect to specific schedules
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

var ErrEventNotFound = errors.New("event not found")

// creates a schedule object and returns a pointer to it
func newSchedule(id string, timezone *time.Location) *schedule {
	return &schedule{
		id:              id,
		timezone:        timezone,
		scheduledEvents: &eventList{},
		expiredEvents:   &eventList{},
		activeEvent:     nil,
		cache:           idcache.New(),
	}
}

// Creates a schedule object in the schedule map if one does not already exist for the given ID.
func createScheduleIfNew(id, name string) {
	_, ok := masterSchedule[id]
	if ok {
		return
	}

	// if new schedule is the local schedule, use the local timezone. otherwise, leave timezone as nil until the timezone info comes through the WebSocket
	var timezone *time.Location // zero-values to nil
	if schedCfg.LocalSchedule.is(id) {
		timezone = localTimezone
	}
	sched := newSchedule(id, timezone)
	masterSchedule[id] = sched
}

// check conducts an event check for the schedule.
// Active events that have expired will be ended, and scheduled events that are ready
// will be executed.
func (sched *schedule) check() (scheduleChanged bool) {
	// if there is an active event: end it if it is over, or discontinue schedule check if it is not over
	if sched.hasActiveEvent() {
		if sched.activeEvent.IsDone(flextime.Now()) {
			finishedEvent := sched.endActiveEvent()
			sched.expiredEvents.add(finishedEvent)
			scheduleChanged = true
			log.Infof("Ended active event for schedule %s", sched.id)
		} else {
			return false
		}
	}

	// if no scheduled events, return early
	if sched.scheduledEvents.Len() == 0 {
		return scheduleChanged
	}

	// peek next event
	nextEvent := (*sched.scheduledEvents)[0]

	// if the next event is not ready to start, return early
	if !nextEvent.ReadyToStart(flextime.Now()) {
		return scheduleChanged
	}

	// split the next event instance from its series
	series := nextEvent       // series will be our reference to the data that is still on the event list
	nextEvent = series.Copy() // nextEvent will be our reference to the individual event instance that is now off the list
	nextEvent.Repeat = events.NewNonRepeatingSeries()

	// shift the series forward to its next event instance
	springForwardEvent, seriesOver := series.ShiftSeries()
	if springForwardEvent != nil {
		// if shifting the series meant it had to hop over a spring-forward event, the spring-forward event now needs to be rescheduled as an individual one-off event.
		// attempt to give it IDs. if the schedule is at max capacity, the spring-forward event cannot exist as a new independent event instance so we must forget it
		if successfullyTagged := sched.assignIdsToPeeledEvent(springForwardEvent); successfullyTagged {
			// if the spring-forward event is unable to be rescheduled (due to overlap), must discard it and remove its IDs from their respective caches
			if succcessfullyRescheduled := sched.scheduledEvents.handleSpringForwardEvent(springForwardEvent); !succcessfullyRescheduled {
				sched.cache.DeleteId(springForwardEvent.Id)
			}
		}
	}

	// if the series is over (nextEvent was last event in the series), remove it from the list and free its IDs
	if seriesOver {
		sched.cache.DeleteId(series.Id)
		sched.scheduledEvents.remove(0)
	}

	// if this instance of the event series was marked as an exception, it can be forgotten and not executed
	if series.HasException(nextEvent.StartTime) {
		// exception could mean that there's another event at this time, so recursively call schedule::check()
		sched.check()
		return true
	}

	// if the schedule is at max capacity, nextEvent cannot exist as a new independent event instance so we must forget it
	if successfullyTagged := sched.assignIdsToPeeledEvent(nextEvent); !successfullyTagged {
		return true
	}

	// only execute events for local schedules
	if schedCfg.LocalSchedule != nil && sched.id == schedCfg.LocalSchedule.Id {
		log.Infof("Executing event for schedule %s", sched.id)
		if err := executeEvent(nextEvent); err != nil {
			log.Errorf("Error while starting event for local schedule: %v", err)
		}

	}
	sched.activeEvent = nextEvent
	return true
}

// Deletes expired events that ended before today and exception timestamps that occurred before today,
// all with respect to the schedule's local time zone.
func (sched *schedule) pruneOldEventsAndExceptions() (scheduleChanged bool) {
	// if schedule has not had its time zone configured yet, do not try to prune
	if sched.timezone == nil {
		return false
	}

	midnight := flextime.GetMidnightInNDays(0, flextime.Now().In(sched.timezone))

	// delete old events. iterate over copy so range-based for loop does not get corrupted as events are deleted
	expiredEventsCopy := sched.expiredEvents.copy()
	for _, eCopy := range *expiredEventsCopy {
		if eCopy.StartTime.Add(eCopy.TimeDuration()).Before(midnight) {
			scheduleChanged = true
			log.Infof("Pruning old event with ID %v.", eCopy.Id)
			sched.deleteEventWithId(eCopy.Id)
		}
	}

	// delete old exceptions
	for _, e := range *sched.scheduledEvents {
		scheduleChanged = e.Repeat.DeleteExceptionsBefore(midnight) || scheduleChanged
	}
	return scheduleChanged
}

// Searches expired events, scheduled events, and active event for an event with the given ID and deletes it if found.
func (sched *schedule) deleteEventWithId(id idcache.Id) {
	sched.cache.DeleteId(id)
	sched.expiredEvents.deleteEventWithId(id)
	sched.scheduledEvents.deleteEventWithId(id)
	if sched.hasActiveEvent() && sched.activeEvent.Id == id {
		sched.endActiveEvent()
	}
}

func (sched *schedule) getEventWithId(id idcache.Id) (event *events.Event, err error) {
	if _, event = sched.scheduledEvents.getEventWithId(id); event != nil {
		return event, nil
	}
	if _, event = sched.expiredEvents.getEventWithId(id); event != nil {
		return event, nil
	}
	if sched.activeEvent != nil && sched.activeEvent.Id == id {
		return sched.activeEvent, nil
	}
	return nil, ErrEventNotFound
}

// Removes all events from the schedule.
func (sched *schedule) clear() {
	sched.cache = idcache.New()
	sched.expiredEvents = &eventList{}
	sched.scheduledEvents = &eventList{}
	if sched.activeEvent != nil {
		sched.endActiveEvent()
	}
}

// verifyActiveEventStatus sends GETs for all active event variables/constants with reply-to URI of the format /scheduler/active_event_status/<variable ID>.
// When the value is returned, it will be checked against the setpoint to make sure it is the same value.
func (sched *schedule) verifyActiveEventStatus() error {
	// if schedule does not have an active event, there is nothing that has to be verified
	if !sched.hasActiveEvent() {
		return nil
	}

	// get the mode being used by the active event, which will contain the URIs of all variables/constants
	activeMode, ok := modes[sched.activeEvent.Mode]
	if !ok {
		return fmt.Errorf("active event's mode %s was not found in the mode map", sched.activeEvent.Mode)
	}

	// send a GET for every variable/constant
	activeMode.sendGets()
	return nil
}

// hasActiveEvent returns if a schedule has a currently active event.
func (sched *schedule) hasActiveEvent() bool {
	return sched.activeEvent != nil
}

// Marks the active event as over and sends default setpoints if this schedule is the local schedule
// and there is no continuous transition to the next event in the schedule.
//
// Returns the formerly-active event so caller can decide what to do with it (put in expired events
// list, ignore, etc.).
func (sched *schedule) endActiveEvent() *events.Event {
	e := sched.activeEvent
	sched.activeEvent = nil

	// only local schedules should send default setpoints
	if schedCfg.LocalSchedule == nil || sched.id != schedCfg.LocalSchedule.Id {
		return e
	}

	// if there is continuous transition to next event, do not send default setpoints
	if len(*sched.scheduledEvents) > 0 {
		if nextEvent := (*sched.scheduledEvents)[0]; e.HasContinuousTransition(nextEvent) {
			return e
		}
	}

	modes.sendDefaultSetpoints()
	return e
}

// replaceActiveEvent ends the schedule's active event if there is one,
// sets the passed-in event to be the schedule's new active event, and
// executes the new active event
func (sched *schedule) replaceActiveEvent(e *events.Event) {
	if sched.activeEvent != nil {
		sched.endActiveEvent()
	}
	sched.activeEvent = e
	// only execute events local schedules.
	// aka do not execute Site Controller events on Fleet Manager and vice versa
	if schedCfg.LocalSchedule != nil && sched.id == schedCfg.LocalSchedule.Id {
		if err := executeEvent(e); err != nil {
			log.Errorf("Error while replacing active event: %v", err)
		}
	}
}

func (sched *schedule) getIdsOfEventsWithMode(targetMode string) []idcache.Id {
	ids := make([]idcache.Id, 0)
	ids = append(ids, sched.expiredEvents.getIdsOfEventsWithMode(targetMode)...)
	ids = append(ids, sched.scheduledEvents.getIdsOfEventsWithMode(targetMode)...)
	if sched.activeEvent != nil && sched.activeEvent.Mode == targetMode {
		ids = append(ids, sched.activeEvent.Id)
	}
	return ids
}

// Ends active event if it uses mode being deleted, and deletes any events from schedule that use the mode.
func (sched *schedule) deleteEventsWithMode(modeBeingDeleted string) {
	idsOfEventsToDelete := sched.getIdsOfEventsWithMode(modeBeingDeleted)
	for _, id := range idsOfEventsToDelete {
		sched.deleteEventWithId(id)
	}
}

// re-executes a schedule's active event if it uses the given mode. used if there is a URI change to a mode variable
func (sched *schedule) resendActiveEventsWithMode(changedMode string) {
	if sched.activeEvent != nil && sched.activeEvent.Mode == changedMode {
		if err := executeEvent(sched.activeEvent); err != nil {
			log.Errorf("Error while resending active events with mode: %v", err)
		}
	}
}

// Routes FIMS SETs that have URIs beginning with /scheduler/events.
func (sched *schedule) handleFimsSet(msg fims.FimsMsg) error {
	// SET is to /scheduler/events or /scheduler/events/<schedule ID>
	if msg.Nfrags < 4 {
		if err := sched.handleFimsSetToSchedule(msg); err != nil {
			return fmt.Errorf("failed to handle FIMS SET to schedule: %w", err)
		}
		return nil
	}

	// SET is to specific event
	if err := sched.handleFimsWriteToEvent(msg); err != nil {
		return fmt.Errorf("failed to handle FIMS SET to event: %w", err)
	}
	return nil
}

// Handles SETs to /scheduler/events/<schedule ID> and partially handles SETs to /scheduler/events (only portion of body labeled with this schedule's ID).
func (sched *schedule) handleFimsSetToSchedule(msg fims.FimsMsg) error {
	input := msg.Body
	// SETs to /scheduler/events will be treated as independent SETs to each schedule
	if msg.Nfrags < 3 {
		inputScheduleMap, ok := msg.Body.(map[string]interface{})
		if !ok {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("expected map[string]interface{} but got %T", msg.Body)
		}

		// this schedule might not have been included in the larger SET. that is OK, and just ignore if so
		if input, ok = inputScheduleMap[sched.id]; !ok {
			return nil
		}
	}

	inputEventList, err := parseEventList(input, sched.timezone)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to parse event list: %w", err)
	}

	if err = sched.handleFullScheduleSet(inputEventList); err != nil {
		if errors.Is(err, idcache.ErrFullCache) {
			sendErrorResponse(msg.Replyto, "Schedule Is Full")
		} else {
			sendErrorResponse(msg.Replyto, "Overlapping Events")
		}
		return fmt.Errorf("failed to handle full schedule SET: %w", err)
	}

	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, sched.aggregateAllEvents())
	}
	return nil
}

// Attempts to overwrite the existing schedule's events with the new input events.
func (sched *schedule) handleFullScheduleSet(inputEventList eventList) error {
	newCache := idcache.New()
	for i := 0; i < inputEventList.Len(); i++ {
		id, err := newCache.GenerateId()
		if err != nil {
			return fmt.Errorf("when trying to generate %d new unique IDs, failed to generate ID %d: %w", inputEventList.Len(), i+1, err)
		}
		inputEventList[i].Id = id
		inputEventList[i].Repeat.Id = id
	}

	if err := sched.overwrite(inputEventList, newCache, true); err != nil {
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}
	return nil
}

// Handles POSTs to URIs beginning with /scheduler/events/<schedule ID>.
func (sched *schedule) handleFimsPost(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// POST is to /scheduler/events/<schedule ID>
	if msg.Nfrags == 3 {
		if err := sched.handleFimsPostToSchedule(msg); err != nil {
			return fmt.Errorf("failed to handle FIMS POST to schedule: %w", err)
		}
		return nil
	}

	// POST is to specific event
	if err := sched.handleFimsWriteToEvent(msg); err != nil {
		return fmt.Errorf("failed to handle FIMS POST to event: %w", err)
	}
	return nil
}

// Handles POSTs to /scheduler/events/<schedule ID>.
func (sched *schedule) handleFimsPostToSchedule(msg fims.FimsMsg) error {
	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	inputEvent, err := events.ParseEvent(jsonBytes, sched.timezone, modes.buildMapOfVariables())
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to parse input event: %w", err)
	}

	if flextime.TimeIsBeforeCurrentMinute(inputEvent.StartTime) {
		sendErrorResponse(msg.Replyto, "Event Starts In Past")
		return fmt.Errorf("event start time %v is in past", inputEvent.StartTime)
	}

	inputEventList := eventList{}
	inputEventList.add(&inputEvent)

	if err = sched.handleFullSchedulePost(inputEventList); err != nil {
		if errors.Is(err, idcache.ErrFullCache) {
			sendErrorResponse(msg.Replyto, "Schedule Is Full")
		} else {
			sendErrorResponse(msg.Replyto, "Overlapping Events")
		}
		return fmt.Errorf("failed to handle full schedule POST: %w", err)
	}

	sendReply(msg.Replyto, sched.aggregateAllEvents())
	return nil
}

// Handles SETs/POSTs to URIs beginning with /scheduler/events/<scheduler ID>/<event ID>.
func (sched *schedule) handleFimsWriteToEvent(msg fims.FimsMsg) error {
	eventIdString := msg.Frags[3]
	eventId, err := idcache.ParseIdFromString(eventIdString)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("failed to parse event ID from %s: %w", eventIdString, err)
	}

	allEvents := sched.aggregateAllEvents()
	_, targetEvent := allEvents.getEventWithId(idcache.Id(eventId))
	if targetEvent == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("could not find event with ID %d", eventId)
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var replyObject interface{}
	switch msg.Method {
	case "set":
		targetEventIsActiveEvent := sched.activeEvent != nil && sched.activeEvent.Id == targetEvent.Id
		if replyObject, err = targetEvent.HandleSet(msg.Frags, jsonBytes, sched.timezone, modes.buildMapOfVariables(), targetEventIsActiveEvent); err != nil {
			if errors.Is(err, events.ErrInvalidUri) {
				sendErrorResponse(msg.Replyto, "Invalid URI")
			} else {
				sendErrorResponse(msg.Replyto, "Invalid Data")
			}
			return fmt.Errorf("failed to apply SET to target event: %w", err)
		}
	case "post":
		if replyObject, err = targetEvent.HandlePost(msg.Frags, jsonBytes, sched.timezone, modes.buildMapOfVariables()); err != nil {
			if errors.Is(err, events.ErrInvalidUri) {
				sendErrorResponse(msg.Replyto, "Invalid URI")
			} else {
				sendErrorResponse(msg.Replyto, "Invalid Data")
			}
			return fmt.Errorf("failed to apply POST to target event: %w", err)
		}
	case "del":
		if msg.Nfrags < 5 {
			allEvents.deleteEventWithId(targetEvent.Id)
			sched.cache.DeleteId(targetEvent.Id)
			replyObject = allEvents
		} else if replyObject, err = targetEvent.HandleDel(msg.Frags, jsonBytes, sched.timezone, modes.buildMapOfVariables()); err != nil {
			if errors.Is(err, events.ErrInvalidUri) {
				sendErrorResponse(msg.Replyto, "Invalid URI")
			} else {
				sendErrorResponse(msg.Replyto, "Invalid Data")
			}
			return fmt.Errorf("failed to apply DEL to target event: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("invalid method %s", msg.Method)
	}

	if err := sched.overwrite(allEvents, sched.cache, true); err != nil {
		sendErrorResponse(msg.Replyto, "Overlapping Events")
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}

	sendReply(msg.Replyto, replyObject)
	return nil
}

func (sched *schedule) handleFullSchedulePost(inputEventList eventList) error {
	// generate a new ID for each new event
	newCache := sched.cache.Copy()
	for i := 0; i < inputEventList.Len(); i++ {
		newId, err := newCache.GenerateId()
		if err != nil {
			return fmt.Errorf("when trying to generate %d new unique IDs, failed to generate ID %d: %w", inputEventList.Len(), i+1, err)
		}
		inputEventList[i].Id = newId
		inputEventList[i].Repeat.Id = newId
	}

	// make a copy of existing schedule so that existing schedule is not changed until new schedule has been verified
	newSchedule := sched.aggregateAllEvents()

	// add the POST'd events
	newSchedule.addList(&inputEventList)

	if err := sched.overwrite(newSchedule, newCache, true); err != nil {
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}
	return nil
}

// A PATCH request accepts a list of events and will add all of the events to the schedule, similar to a POST.
// However, if an input event has the same start time as an existing event, the existing event will be updated
// with the input event's settings (as opposed to rejecting the input event which is what a POST would do).
func (sched *schedule) handleFullSchedulePatch(inputEventList eventList) error {
	// do overlap check upfront just in case there are two events in the input list with the same start time.
	// that would result in the first event overwriting the existing event then the second event overwriting the first event.
	// that would be hard to debug
	if overlapFound, e1, e2 := inputEventList.hasOverlaps(); overlapFound {
		return fmt.Errorf("input event list has overlapping events %v with repeat settings %v and %v with repeat settings %v", e1, e1.Repeat, e2, e2.Repeat)
	}

	// for each input event, identify if there is an existing event with the same start time as it.
	// if an existing event is found, update that event with the input event's settings and maintain the existing ID.
	// otherwise, create a new ID for the input event add it to the schedule as a new event
	newCache := sched.cache.Copy()
	newSchedule := sched.aggregateAllEvents()
	for i, inputEvent := range inputEventList {
		if existingEventWithSameStartTime, _ := newSchedule.get(inputEvent.StartTime); existingEventWithSameStartTime != nil {
			existingEventWithSameStartTime.Update(inputEvent)
			continue
		}
		newId, err := newCache.GenerateId()
		if err != nil {
			return fmt.Errorf("failed to generate new unique ID for input event %d: %w", i+1, err)
		}
		inputEvent.Id = newId
		inputEvent.Repeat.Id = newId
		newSchedule.add(inputEvent)
	}

	if err := sched.overwrite(newSchedule, newCache, true); err != nil {
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}
	return nil
}

// Handles DELs with URIs that start with /scheduler/events/<schedule ID>.
func (sched *schedule) handleFimsDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// DEL is to /scheduler/events/<schedule ID>. delete all events in the schedule
	if msg.Nfrags == 3 {
		sched.clear()
		sendUpdatesAfterMasterScheduleEdit(true)
		sendReply(msg.Replyto, sched.aggregateAllEvents())
		return nil
	}

	// DEL is to specific event
	if err := sched.handleFimsWriteToEvent(msg); err != nil {
		return fmt.Errorf("failed to handle FIMS DEL to event: %w", err)
	}
	return nil
}

func (sched *schedule) deleteMultipleEvents(idsOfEventsToDelete []idcache.Id) error {
	if idsOfEventsToDelete == nil {
		return errors.New("received nil array")
	}

	// make a copy of existing schedule so that existing schedule is not changed until new schedule has been verified
	newSchedule := sched.aggregateAllEvents()

	// delete any events from existing schedule and ID cache that have IDs found in DEL payload
	newCache := sched.cache.Copy()
	for _, id := range idsOfEventsToDelete {
		newSchedule.deleteEventWithId(id)
		newCache.DeleteId(id)
	}

	if err := sched.overwrite(newSchedule, newCache, true); err != nil {
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}
	return nil
}

func (sched *schedule) aggregateAllEvents() eventList {
	allEvents := eventList{}
	allEvents.addList(sched.expiredEvents.copy())
	allEvents.addList(sched.scheduledEvents.copy())
	if sched.activeEvent != nil {
		allEvents.add(sched.activeEvent.Copy())
	}
	return allEvents
}

// Handles GETs with URIs that start with /scheduler/events/<schedule ID>.
func (sched *schedule) handleFimsGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendReply(msg.Replyto, sched.aggregateAllEvents())
		return nil
	}

	eventIdString := msg.Frags[3]
	eventId, err := idcache.ParseIdFromString(eventIdString)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return fmt.Errorf("failed to parse event ID from %s: %w", eventIdString, err)
	}

	targetEvent, err := sched.getEventWithId(idcache.Id(eventId))
	if err != nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("failed to find event with ID %d: %w", eventId, err)
	}

	replyObject, err := targetEvent.HandleGet(msg.Frags)
	if err != nil {
		if errors.Is(err, events.ErrInvalidUri) {
			sendErrorResponse(msg.Replyto, "Invalid URI")
		} else {
			sendErrorResponse(msg.Replyto, "Resource Not Found")
		}
		return fmt.Errorf("failed to get reply object for get to event %v: %w", eventId, err)
	}

	sendReply(msg.Replyto, replyObject)
	return nil
}

func (sched *schedule) overwrite(inputEventList eventList, inputCache idcache.IdCache, externalEditor bool) error {
	if err := inputEventList.validate(sched.timezone, modes.buildMapOfVariables()); err != nil {
		return fmt.Errorf("invalid input event list: %w", err)
	}

	// overwrite the existing ID cache with the new one
	sched.cache = inputCache

	// wipe scheduled/expired event lists since new events will replace them
	sched.scheduledEvents = &eventList{}
	sched.expiredEvents = &eventList{}

	// write all input events to schedule
	activeEventIncluded := sched.integrateEventList(inputEventList)

	// if there is no active event in the new schedule but there was one in the old one, need to end it
	if !activeEventIncluded && sched.hasActiveEvent() {
		sched.endActiveEvent()
	}

	// backup, log, and publish updates
	sendUpdatesAfterMasterScheduleEdit(externalEditor)
	return nil
}

// Assigns each event in the input list to the schedule's expired list, scheduled list,
// or active event pointer based on its start time and duration.
func (sched *schedule) integrateEventList(eList eventList) (activeEventIncluded bool) {
	for _, e := range eList {
		activeEventIncluded = sched.organizeEvent(e) || activeEventIncluded
	}
	return activeEventIncluded
}

// Evaluates whether the given event is expired, active, or scheduled based on its start time, duration,
// and the current time. Then, sorts the event into the schedule accordingly.
func (sched *schedule) organizeEvent(e *events.Event) (isActiveEvent bool) {
	switch sched.classifyEvent(e) {
	case EXPIRED_EVENT:
		// save expired instance to expired events list with "one-off" repeat settings
		series := e.Copy()
		e.Repeat = events.NewNonRepeatingSeries()
		// if the event was not able to be assigned IDs, it will be lost
		if successfullyTagged := sched.assignIdsToPeeledEvent(e); successfullyTagged {
			sched.expiredEvents.add(e)
		}
		// shift series up to next occurrence and recursively organize it (will keep happening until series catches up with present time or ends)
		springForwardEvent, seriesOver := series.ShiftSeries()
		if springForwardEvent != nil {
			isActiveEvent = isActiveEvent || sched.organizeEvent(springForwardEvent)
		}
		if !seriesOver {
			isActiveEvent = isActiveEvent || sched.organizeEvent(series)
		}
	case RETRO_ACTIVE_EVENT:
		sched.replaceActiveEvent(e)
		isActiveEvent = true
	case ACTIVE_EVENT:
		updateActiveEvent(sched.activeEvent, e)
		isActiveEvent = true
	case SCHEDULED_EVENT:
		sched.scheduledEvents.add(e)
	}
	return isActiveEvent
}

func (sched *schedule) getIdsOfEventsWithStartTimes(startTimes []time.Time) []idcache.Id {
	ids := make([]idcache.Id, 0)
	for _, e := range *sched.expiredEvents {
		if e.HasOneOfStartTimes(startTimes) {
			ids = append(ids, e.Id)
		}
	}
	for _, e := range *sched.scheduledEvents {
		if e.HasOneOfStartTimes(startTimes) {
			ids = append(ids, e.Id)
		}
	}
	if sched.activeEvent.HasOneOfStartTimes(startTimes) {
		ids = append(ids, sched.activeEvent.Id)
	}
	return ids
}

// classifyEvent compares an event with the state of the schedule and identifies if the event is
// an active event that has already been triggered, an active event that has not yet been
// triggered (meaning this is the first time we must have seen it), an expired event, or a
// scheduled event.
func (sched *schedule) classifyEvent(e *events.Event) eventStatus {
	if sched.activeEvent != nil && sched.activeEvent.StartTime.Equal(e.StartTime) {
		return ACTIVE_EVENT
	} else if e.PassedStartTime(flextime.Now()) {
		if e.StartTime.Add(e.TimeDuration()).Before(flextime.Now()) {
			return EXPIRED_EVENT
		} else {
			return RETRO_ACTIVE_EVENT
		}
	} else {
		return SCHEDULED_EVENT
	}
}

// When an event is peeled off from a series, it needs to be given its own ID.
// If the schedule is at max capacity, then there is nothing that can be done and the peeled event should be forgotten by the schedule.
func (sched *schedule) assignIdsToPeeledEvent(e *events.Event) (successfullyTagged bool) {
	id, err := sched.cache.GenerateId()
	if err != nil {
		log.Errorf("Error generating new ID for peeled event %+v: %v. Event will be discarded.", *e, err)
		return false
	}
	e.Id = id
	e.Repeat.Id = id
	return true
}

// handleActiveEventStatusUpdate will check the matching ID setpoint of the event's active event.
// If the control value does not match the status value received, an update SET is sent out.
func (sched *schedule) handleActiveEventStatusUpdate(varId string, statusUpdateBody interface{}) error {
	// if schedule has no active event, this must be a lagging status update from when there was an active event so just ignore it
	if !sched.hasActiveEvent() {
		return nil
	}

	// get the mode being used by the active event, which will contain the URIs of all variables/constants
	activeMode, ok := modes[sched.activeEvent.Mode]
	if !ok {
		return fmt.Errorf("active event's mode %s was not found in the mode map", sched.activeEvent.Mode)
	}

	// status value may come in clothed body or naked body
	statusValue := parsemap.UnwrapVariable(statusUpdateBody)

	// determine if status value is variable or constant
	// if not found in either, then it is likely a lagging update from a previous active event and can just be ignored
	varIndex := activeMode.Variables.Find(varId)
	constIndex := activeMode.Constants.Find(varId)
	if constIndex != -1 {
		// is constant
		sp := activeMode.Constants[constIndex]
		if !parsemap.InterfaceEquals(sp.Value, statusValue) {
			sp.SendSet(f, sp.Value, schedCfg.LocalSchedule.ClothedSetpoints)
		}
	} else if varIndex != -1 {
		// is variable
		sp := activeMode.Variables[varIndex]
		controlValue, ok := sched.activeEvent.Variables[varId]
		if !ok {
			return errors.New("active event does not have variable with ID " + varId + " even though mode says it should")
		}
		if !parsemap.InterfaceEquals(controlValue, statusValue) {
			sp.SendSet(f, controlValue, schedCfg.LocalSchedule.ClothedSetpoints)
		}
	}
	return nil
}

func (sched *schedule) setTimezone(tz *time.Location) error {
	if tz == nil {
		return errors.New("time zone cannot be nil")
	}
	sched.timezone = tz
	for _, e := range *sched.scheduledEvents {
		e.StartTime = e.StartTime.In(sched.timezone)
	}
	return nil
}

func (sched1 *schedule) equals(sched2 *schedule) (areEqual bool, reasonNotEqual string) {
	if sched1.id != sched2.id {
		return false, fmt.Sprintf("sched1 ID %s does not match sched2 ID %s", sched1.id, sched2.id)
	}

	if sched1.activeEvent == nil && sched2.activeEvent != nil {
		return false, "sched2 has active event but sched1 does not"
	}
	if sched1.activeEvent != nil && sched2.activeEvent == nil {
		return false, "sched1 has active event but sched2 does not"
	}

	if sched1.activeEvent != nil {
		if areEqual, reasonNotEqual := sched1.activeEvent.Equals(sched2.activeEvent); !areEqual {
			return false, fmt.Sprintf("active events are not equal: %s", reasonNotEqual)
		}
	}

	if areEqual, reasonNotEqual := sched1.scheduledEvents.equals(*sched2.scheduledEvents); !areEqual {
		return false, fmt.Sprintf("scheduled events lists are not equal: %s", reasonNotEqual)
	}

	if areEqual, reasonNotEqual := sched1.expiredEvents.equals(*sched2.expiredEvents); !areEqual {
		return false, fmt.Sprintf("expired events lists are not equal: %s", reasonNotEqual)
	}
	return true, ""
}

// Sends an event's setpoints to their respective URIs.
func executeEvent(e *events.Event) error {
	m, ok := modes[e.Mode]
	if !ok {
		return fmt.Errorf("Mode %s not found in modes map", e.Mode)
	}

	m.sendConstants()

	var attempted, failed int
	for _, sp := range m.Variables {
		attempted++
		variableValue, ok := e.Variables[sp.Id]
		if !ok {
			log.Errorf("Did not find %s mode's variable setpoint %s in event variable map. Event details: %+v.", m.Name, sp.Id, *e)
			failed++
			continue
		}
		if sp.IsTemplate {
			if err := sp.SendTemplatedSet(f, variableValue, schedCfg.LocalSchedule.ClothedSetpoints); err != nil {
				log.Errorf("Error while sending templated set: %v", err)
				failed++
				continue
			}
		} else {
			sp.SendSet(f, variableValue, schedCfg.LocalSchedule.ClothedSetpoints)
		}
	}

	if failed != 0 {
		return fmt.Errorf("attempted %d setpoints but failed %d", attempted, failed)
	}

	return nil
}

// updateActiveEvent is a special case of update for active events.
// Start time and mode are required to be the same.
func updateActiveEvent(eOld *events.Event, eNew *events.Event) {
	// verify start time and mode are the same
	if !eOld.StartTime.Equal(eNew.StartTime) {
		log.Errorf("Active event's start time %v does not match start time of event that is meant to be editing it %v.", eOld.StartTime, eNew.StartTime)
		return
	}
	if eOld.Mode != eNew.Mode {
		log.Errorf("Active event's mode %s does not match mode of event that is meant to be editing it %s.", eOld.Mode, eNew.Mode)
		return
	}
	// update duration
	eOld.Duration = eNew.Duration
	// get updated mode from mode map
	m, ok := modes[eNew.Mode]
	if !ok {
		log.Errorf("Mode %s not found in modes map while trying to update active event.", eOld.Mode)
		return
	}
	// if the values of any variables are different in the new event settings, send out the new value
	for _, modeVar := range m.Variables {
		// get old value of variable that is stored in the event's variable map
		oldVarVal, ok := eOld.Variables[modeVar.Id]
		// if old event's variable map does not include one of the mode's
		// variables, there has been an error because adding a new variable to
		// the active event's mode should have ended the active event. if this
		// case somehow happens, just skip this setpoint
		if !ok {
			log.Errorf("Mode variable %s does not match any variables in old event variable map while trying to update active event.", modeVar.Id)
			continue
		}
		// get updated variable value.
		// if received event's variable map does not include one of the mode's
		// variables, the input data was not properly validated. if this
		// case somehow happens, just skip this setpoint
		newVarVal, ok := eNew.Variables[modeVar.Id]
		if !ok {
			log.Errorf("Mode variable %s does not match any variables in new event variable map while trying to update active event.", modeVar.Id)
			continue
		}

		// if the variable value has not changed, no need to send a new SET
		if reflect.DeepEqual(oldVarVal, newVarVal) {
			return
		}

		// if we have made it to this point, then a variable value has been changed. send an update
		modeVar.SendSet(f, newVarVal, schedCfg.LocalSchedule.ClothedSetpoints)
		// also update event object's stored value
		eOld.Variables[modeVar.Id] = newVarVal
	}
}
