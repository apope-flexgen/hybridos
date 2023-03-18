/**
 *
 * siteController.go
 *
 * Methods for the `siteController` data type.
 * Some methods specific to the Fleet-Site interface are in fleet_site_interface.go.
 *
 */
package main

import (
	"container/heap"
	"errors"
	"fmt"
	"log"
	"time"

	"github.com/flexgen-power/scheduler/pkg/events"
)

type siteController struct {
	id              string // same as map key corresponding to the siteController instance
	displayName     string
	timezone        *time.Location // timezone of the box on which this site's Site Controller is running (https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)
	scheduledEvents *eventHeap
	expiredEvents   *eventHeap
	activeEvent     *events.Event // activeEvent == nil if no active event
	ws              *webSocketConfig
}

// creates a new site object and returns a pointer to it
func newSite(id string) *siteController {
	return &siteController{
		id:              id,
		displayName:     id, // default to using ID, but will likely be changed later
		timezone:        thisTimezone,
		scheduledEvents: &eventHeap{},
		expiredEvents:   &eventHeap{},
		activeEvent:     nil,
	}
}

// variation on a "copy constructor" that does not include any events
func (site *siteController) createEmptyCopy() *siteController {
	return &siteController{
		id:              site.id,
		displayName:     site.displayName,
		timezone:        site.timezone,
		scheduledEvents: &eventHeap{},
		expiredEvents:   &eventHeap{},
		activeEvent:     nil,
	}
}

// createSiteIfNew creates a site object in the master schedule if one does not already exist for the given id.
// If Scheduler is a Fleet Manager, it will wipe the time zone so that the Site Controller can set the proper time zone.
func createSiteIfNew(id string) *siteController {
	// check if site object already exists and create it if not
	site := masterSchedule.getSite(id)
	if site == nil {
		site = newSite(id)
		masterSchedule.addSite(site)
	}

	// The site object creation that just happened set the time zone as this host machine's time zone.
	// If this is Fleet Manager, the time zone of the Site Controller is not necessarily our time zone.
	// The time zone will be set upon connection, but until connection happens use "" as placeholder
	if schedulerInstanceType == FLEET_MANAGER {
		site.timezone = nil
	}

	return site
}

// check conducts an event check on just a single site schedule for the current day.
// Active events that have expired will be ended, and scheduled events that are ready
// will be executed.
func (site *siteController) check() (scheduleChanged bool) {
	scheduleChanged = false

	// if there is an active event: end it if it is over, or discontinue site check if it is not over
	if site.hasActiveEvent() {
		if eventIsDone(site.activeEvent) {
			finishedEvent := site.endActiveEvent()
			heap.Push(site.expiredEvents, finishedEvent)
			scheduleChanged = true
		} else {
			return false
		}
	}

	// if at this point, then there is no active event so check next event in schedule
	if site.scheduledEvents.Len() > 0 {
		// peek next event
		topEvent, err := site.scheduledEvents.peek()
		if err != nil {
			log.Printf("scheduler.go::check() ~ Error when peeking next event for site %v: %v\n", site.id, err)
			return
		}

		// take action on next event if necessary
		if eventStartTimePassed(topEvent) { // if event is not active but start time has already passed, it is an expired event and should be skipped
			heap.Pop(site.scheduledEvents)
			site.check() // since skipping this event, recursively do another check to see if next event in list is ready
			return true
		} else if eventReadyToStart(topEvent) {
			if schedulerInstanceType != FLEET_MANAGER { // only a Site Controller instance of Scheduler should be sending setpoints
				executeEvent(topEvent)
			}
			site.activeEvent = topEvent
			heap.Pop(site.scheduledEvents)
			return true
		}
	}
	return
}

// verifyActiveEventStatus sends GETs for all active event variables/constants with reply-to URI of the format /scheduler/activeEventStatus/<variable ID>.
// When the value is returned, it will be checked against the setpoint to make sure it is the same value.
func (site *siteController) verifyActiveEventStatus() error {
	// if site does not have an active event, there is nothing that has to be verified
	if !site.hasActiveEvent() {
		return nil
	}

	// get the mode being used by the active event, which will contain the URIs of all variables/constants
	activeMode := modes.getMode(site.activeEvent.Mode)
	if activeMode == nil {
		return errors.New("active event's mode " + site.activeEvent.Mode + " was not found in the mode map")
	}

	// send a GET for every variable/constant
	activeMode.sendGets()
	return nil
}

// repeatDayIfMidnight takes all events in the final day of the schedule and makes copies of them for the day after.
// The expired events list is also cleared.
func (site *siteController) repeatDayIfMidnight(currentTime time.Time) {
	// get timestamp for tomorrow's midnight
	midnightTomorrow := getMidnightInNDays(1, currentTime)

	// wipe the expired events list
	site.expiredEvents = newEventHeap()

	// instantiate new eventHeap for holding tomorrow's copied events while iterating through today. Do not want to start iterating over newly added events
	newEvents := newEventHeap()

	// make a list that will hold any events occurring during skipped hour if tomorrow is a Spring Forward day
	springForwardSkippedEvents := newEventHeap()

	// if on the last day transition, there were skipped Spring Forward events put into the springForwardSkipped events list,
	// those will now be found in the scheduledEvents list with timestamps that occur tomorrow. Remove those events from the
	// schedule and put them in the new events list so that they can be organized with the copies of today's events
	previouslySkippedEvents := newEventHeap()
	for _, e := range *site.scheduledEvents {
		// add the event to the previouslySkippedEvents list if it is scheduled to occur tomorrow
		if !e.StartTime.Before(midnightTomorrow) {
			heap.Push(previouslySkippedEvents, e)
		}
	}
	// remove all previously skipped events from the schedule and place them in the new events list
	for previouslySkippedEvents.Len() > 0 {
		e := heap.Pop(previouslySkippedEvents)
		_, i := site.scheduledEvents.get(e.(*events.Event).StartTime)
		heap.Remove(site.scheduledEvents, i)
		heap.Push(newEvents, e.(*events.Event))
	}

	// iterate through each of today's events and copy each one to tomorrow
	for _, existingEvent := range *site.scheduledEvents {
		// if this event is an instance of a Spring Forward event that was shifted to just after the time hop,
		// it should not be copied since its actual copy (with the non-shifted start time) is going to be found in previouslySkippedEvents
		if existingEvent.NonRolling {
			continue
		}

		// make a copy of the event and verify its start timestamp is in the right time zone
		e := existingEvent.Copy()
		e.StartTime = e.StartTime.In(site.timezone)

		// use timestamp that has tomorrow's year/month/day to make new start time with event's start hour/month/second/ns
		newStart := time.Date(midnightTomorrow.Year(), midnightTomorrow.Month(), midnightTomorrow.Day(), e.StartTime.Hour(), e.StartTime.Minute(), e.StartTime.Second(), e.StartTime.Nanosecond(), midnightTomorrow.Location())

		// if tomorrow is Spring Forward day and this event occurs during skipped hour (this condition identified by time.Date assigning newStart a
		// different Hour than was passed to the func), come back to it later.
		// otherwise, give the event its new start time
		if newStart.Hour() != e.StartTime.Hour() {
			heap.Push(springForwardSkippedEvents, e)
			continue
		} else {
			e.StartTime = newStart
		}

		// add event to tomorrow's event list.
		// if there are overlaps, do not add the event. this would happen if the event was scheduled for the extra hour of a Fall Back day,
		//     or if it was scheduled for a Spring Forward day and now a previously skipped event is blocking it
		if !newEvents.checkForOverlaps(e) {
			heap.Push(newEvents, e)
		}
	}

	// if there are any skipped Spring Forward events, the latest one gets moved to just after the time jump
	lastSkippedEvent, err := springForwardSkippedEvents.peekLast()
	if err == nil {
		// make a copy since all event objects in the skipped events list will be moved to the next day
		makeupEvent := lastSkippedEvent.Copy()

		// it should be non-rolling since an event with the normal day start time will be added to the following day
		makeupEvent.NonRolling = true

		// move last skipped event to just after the time jump
		// TO DO: perhaps we should reconsider putting a 2:30AM event at 3:30AM instead of right at 3AM. 3:30AM is more intuitive and follows Google's lead. 3AM makes sense in a very specific use case of trying to maximize a charge/discharge event's duration
		makeupEvent.StartTime = time.Date(midnightTomorrow.Year(), midnightTomorrow.Month(), midnightTomorrow.Day(), makeupEvent.StartTime.Hour()+1, 0, 0, 0, midnightTomorrow.Location())

		// if the new position causes overlap, trim the makeup event (if overlapping on end side) or delete makeup event (if overlapping on start side)
		for _, e := range *newEvents {
			if makeupEvent.Overlaps(e) {
				if makeupEvent.StartTime.Before(e.StartTime) {
					makeupEvent.Duration = e.StartTime.Sub(makeupEvent.StartTime)
				} else {
					makeupEvent = nil
					break
				}
			}
		}

		// as long as the makeup event was not deleted due to overlap on start side, add it to tomorrow's events
		if makeupEvent != nil {
			heap.Push(newEvents, makeupEvent)
		}

		// now take all skipped Spring Forward events and schedule them for the next day so they do not get lost.
		// get timestamp that occurs at midnight after tomorrow.
		nextMorning := midnightTomorrow.Add(time.Duration(25) * time.Hour)
		// do not need to worry about this timestamp slipping due to a Daylight Saving Day starting at midnight,
		//	since at this part of the code we know that DST began prior to nextMidnight
		nextMidnight := time.Date(nextMorning.Year(), nextMorning.Month(), nextMorning.Day(), 0, 0, 0, 0, nextMorning.Location())
		for _, skippedEvent := range *springForwardSkippedEvents {
			skippedEvent.StartTime = time.Date(nextMidnight.Year(), nextMidnight.Month(), nextMidnight.Day(), skippedEvent.StartTime.Hour(), skippedEvent.StartTime.Minute(), skippedEvent.StartTime.Second(), skippedEvent.StartTime.Nanosecond(), nextMidnight.Location())
			heap.Push(newEvents, skippedEvent)
		}
	}

	// if the last event of today overlaps with the first event of tomorrow, trim it
	lastEventToday, _ := site.scheduledEvents.peekLast()
	firstEventTomorrow, _ := newEvents.peek()
	if (lastEventToday != nil && firstEventTomorrow != nil) && lastEventToday.Overlaps(firstEventTomorrow) {
		lastEventToday.Duration = firstEventTomorrow.StartTime.Sub(lastEventToday.StartTime)
	}

	// concatenate today's event list with tomorrow's event list
	site.scheduledEvents.appendHeap(newEvents)
}

// hasActiveEvent returns if a site has a currently active event.
func (site *siteController) hasActiveEvent() bool {
	return site.activeEvent != nil
}

// endActiveEvent marks the active event as over and sends default setpoints (if Site Controller) to restore site state.
func (site *siteController) endActiveEvent() *events.Event {
	e := site.activeEvent
	site.activeEvent = nil
	// check the next event to determine if there is a smooth transition. If not, send the default mode setpoints
	// only a Site Controller instance of Scheduler should be sending setpoints
	if nextEvent, _ := site.scheduledEvents.peek(); !e.HasContinuousTransition(nextEvent) && schedulerInstanceType != FLEET_MANAGER {
		sendDefaultSetpoints()
	}
	return e
}

// replaceActiveEvent ends the site's active event if there is one,
// sets the passed-in event to be the site's new active event, and
// executes the new active event
func (site *siteController) replaceActiveEvent(e *events.Event) {
	if site.activeEvent != nil {
		site.endActiveEvent()
	}
	site.activeEvent = e
	if schedulerInstanceType != FLEET_MANAGER { // only a Site Controller instance of Scheduler should be sending setpoints
		executeEvent(e)
	}
}

// ends active event if it uses mode being deleted, and deletes any events from schedule that use the mode
func (site *siteController) deleteEventsWithMode(modeBeingDeleted string) {
	// end a site's active event if it is using the mode that is being deleted
	if site.activeEvent != nil && site.activeEvent.Mode == modeBeingDeleted {
		site.endActiveEvent()
	}
	// iterate through schedule and delete events using the mode
	site.scheduledEvents.deleteEventsWithMode(modeBeingDeleted)
}

// returns if schedule of site1 matches schedule of site2
func (site1 *siteController) equals(site2 *siteController) bool {
	if site1.id != site2.id {
		log.Println("scheduler.go::siteController::equals ~ Site IDs do not match")
		return false
	}
	// either both should have no active event or both should have an active event
	if (site1.activeEvent == nil && site2.activeEvent != nil) || (site1.activeEvent != nil && site2.activeEvent == nil) {
		log.Println("scheduler.go::siteController::equals ~ Mismatch between existence of active events")
		return false
	}
	// active events should equal each other
	if site1.activeEvent != nil && !site1.activeEvent.Equals(site2.activeEvent) {
		log.Println("scheduler.go::siteController::equals ~ Active events do not equal each other")
		return false
	}
	// compare scheduled events
	if !site1.scheduledEvents.equals(site2.scheduledEvents) {
		log.Println("scheduler.go::siteController::equals ~ Schedules failed equality check")
		return false
	}
	return true
}

// re-executes a site's active event if it uses the given mode. used if there is a URI change to a mode variable
func (site *siteController) resendActiveEventsWithMode(changedMode string) {
	if site.activeEvent.Mode == changedMode {
		executeEvent(site.activeEvent)
	}
}

// builds object to represent a site's schedule
func (site *siteController) buildObj() *[]interface{} {
	obj := site.scheduledEvents.buildObj()
	site.addActiveEventToList(obj)
	site.expiredEvents.addToList(obj)
	return obj
}

// builds object to represent a site's schedule in mins since midnight
// splits entries across multiple lists, with each list corresponding to a particular day
// start time is given as mins since midnight
func (site *siteController) buildLegacyObject() *[]interface{} {
	var siteSchedule []interface{}
	todayEventsList := make([]interface{}, 0)
	tomorrowEventsList := make([]interface{}, 0)
	site.expiredEvents.addToLegacyList(&todayEventsList)
	site.addActiveEventToLegacyList(&todayEventsList)

	// add each day's schedule to the site schedule object
	for _, sEvent := range *site.scheduledEvents {
		// position in the list of days per schedule
		dayIndex := sEvent.StartTime.Day() - getCurrentTime().In(site.timezone).Day()

		switch dayIndex {
		case 0:
			sEvent.AddToLegacyList(&todayEventsList)
		case 1:
			sEvent.AddToLegacyList(&tomorrowEventsList)
		}
	}
	siteSchedule = append(siteSchedule, todayEventsList)
	siteSchedule = append(siteSchedule, tomorrowEventsList)

	return &siteSchedule
}

// adds site's active event to an event list, if an active site exists
func (site *siteController) addActiveEventToList(list *[]interface{}) {
	if site.activeEvent != nil {
		site.activeEvent.AddToList(list)
	}
}

// adds site's active event to an event list, if an active site exists
// start time is given as mins since midnight
func (site *siteController) addActiveEventToLegacyList(list *[]interface{}) {
	if site.activeEvent != nil {
		site.activeEvent.AddToLegacyList(list)
	}
}

// copies all events occuring on the opposite day and returns them
// 0 indicates to get tomorrow's events, and 1 indicates to get today's events
func (site *siteController) getOtherDayEvents(day int) *eventHeap {
	daySchedule := newEventHeap()
	for _, event := range *site.expiredEvents {
		if event.StartTime.Day()-getCurrentTime().In(site.timezone).Day() != day {
			daySchedule.appendEvent(event)
		}
	}
	if site.hasActiveEvent() && site.activeEvent.StartTime.Day()-getCurrentTime().In(site.timezone).Day() != day {
		daySchedule.appendEvent(site.activeEvent)
	}
	for _, event := range *site.scheduledEvents {
		if event.StartTime.Day()-getCurrentTime().In(site.timezone).Day() != day {
			daySchedule.appendEvent(event)
		}
	}
	return daySchedule
}

// editSchedule accepts an input set of events and will edit the schedule
// with the input events according to the designated editing method given.
// day indicates a UI set for a specific day, indicating that the other day has not been overwritten and should be preserved
func (site *siteController) editSchedule(inputEvents *eventHeap, editor editingInterface, method editingMethod, day int) error {
	// build new schedule by combining the current schedule with the input schedule based on the editing method
	var newSchedule *eventHeap
	switch method {
	case SET:
		switch editor {
		case UI:
			if day == -1 {
				return fmt.Errorf("invalid day index -1 received")
			}
			// The UI only sends one day's worth of events at a time
			// Therefore treat this set as a POST that will update any existing events, and prevent deletion of the other day's events
			newSchedule = site.getOtherDayEvents(day).copy()
			newSchedule.appendHeap(inputEvents)
		default:
			// input events replace the existing events entirely
			newSchedule = inputEvents
		}
	case POST:
		// input events are added to / edit the existing events
		newSchedule = site.scheduledEvents.copy()
		newSchedule.appendHeap(site.expiredEvents.copy())
		if site.hasActiveEvent() && !inputEvents.hasEventWithSameStartAs(site.activeEvent) {
			newSchedule.appendEvent(site.activeEvent.Copy())
		}
		newSchedule.appendHeap(inputEvents)
	case DEL:
		// existing events that match an input event are removed
		newSchedule = site.scheduledEvents.copy()
		newSchedule.appendHeap(site.expiredEvents.copy())
		if site.hasActiveEvent() {
			newSchedule.appendEvent(site.activeEvent.Copy())
		}
		newSchedule.subtractHeap(inputEvents)
	default:
		return fmt.Errorf("object editing method %v is invalid", method)
	}

	// validate the new schedule by checking for overlaps
	if site.scheduledEvents.hasOverlaps() {
		return errors.New("cannot edit schedule as new schedule would have overlapping events")
	}

	// overwrite the existing site schedule with the new site schedule
	site.overwriteSchedule(newSchedule)

	// only send out updates if this is the only edit that is happening
	// otherwise, the function caller will take care of sending updates
	if editor != CALLER {
		sendUpdatesAfterScheduleEdit(editor)
	}
	return nil
}

// overwriteSchedule accepts input events and will replace the existing site
// schedule with the input site schedule. Expired/active events will be managed accordingly.
func (site *siteController) overwriteSchedule(inputEvents *eventHeap) error {
	// allocate memory for new day schedule and expired events
	newSiteSchedule := newEventHeap()
	newExpiredEvents := newEventHeap()

	// need to take into account active event and expired events
	activeEventIncluded := false
	for _, e := range *inputEvents {
		switch site.classifyEvent(e) {
		case EXPIRED_EVENT:
			heap.Push(newExpiredEvents, e)
		case RETRO_ACTIVE_EVENT:
			activeEventIncluded = true
			site.replaceActiveEvent(e)
		case ACTIVE_EVENT:
			activeEventIncluded = true
			updateActiveEvent(site.activeEvent, e)
		case SCHEDULED_EVENT:
			heap.Push(newSiteSchedule, e)
		}
	}

	// if there is no active event in the new schedule but there was one in the old one, need to end it
	if !activeEventIncluded && site.hasActiveEvent() {
		site.endActiveEvent()
	}

	// swap in new schedule and expired events
	site.scheduledEvents = newSiteSchedule
	site.expiredEvents = newExpiredEvents
	return nil
}

// classifyEvent compares an event with the state of a site and identifies if the event is
// an active event that has already been triggered, an active event that has not yet been
// triggered (meaning this is the first time we must have seen it), an expired event, or a
// scheduled event.
//
// It is assumed that the event belongs to the current day, because otherwise it would always
// be a scheduled event.
func (site *siteController) classifyEvent(e *events.Event) eventStatus {
	if site.activeEvent != nil && site.activeEvent.StartTime.Equal(e.StartTime) {
		return ACTIVE_EVENT
	} else if eventStartTimePassed(e) {
		if e.StartTime.Add(e.Duration).Before(getCurrentTime()) {
			return EXPIRED_EVENT
		} else {
			return RETRO_ACTIVE_EVENT
		}
	} else {
		return SCHEDULED_EVENT
	}
}

// handleActiveEventStatusUpdate will check the matching ID setpoint of the event's active event.
// If the control value does not match the status value received, an update SET is sent out.
func (site *siteController) handleActiveEventStatusUpdate(varId string, updateBody interface{}) error {
	// if site has no active event, this must be a lagging status update from when there was an active event so just ignore it
	if !site.hasActiveEvent() {
		return nil
	}

	// status value may come in clothed body or naked body. Try clothed body and if not, assume naked
	var statusValue interface{}
	statusValueMap, ok := updateBody.(map[string]interface{})
	if !ok {
		statusValue = updateBody
	} else {
		statusValueField, ok := statusValueMap["value"]
		if !ok {
			return errors.New("received update value came in a map[string]interface{} but did not have a 'value' field")
		}
		statusValue = statusValueField
	}

	// get the mode being used by the active event, which will contain the URIs of all variables/constants
	activeMode := modes.getMode(site.activeEvent.Mode)
	if activeMode == nil {
		return errors.New("active event's mode " + site.activeEvent.Mode + " was not found in the mode map")
	}

	// determine if status value is variable or constant
	// if not found in either, then it is likely a lagging update from a previous active event and can just be ignored
	varIndex := activeMode.findModeVariable(varId)
	constIndex := activeMode.findModeConstant(varId)
	if constIndex != -1 {
		// is constant
		sp := activeMode.constants[constIndex]
		controlValue := sp.get()
		if !interfaceEquals(controlValue, statusValue) {
			sp.sendSet()
		}
	} else if varIndex != -1 {
		// is variable
		sp := activeMode.variables[varIndex]
		controlValue, ok := site.activeEvent.Variables[varId]
		if !ok {
			return errors.New("active event does not have variable with ID " + varId + " even though mode says it should")
		}
		if !interfaceEquals(controlValue, statusValue) {
			ok = sp.set(controlValue)
			if !ok {
				return fmt.Errorf("control value is not the correct type. Expected %s but control value is %T", sp.varType, controlValue)
			}
			sp.sendSet()
		}
	}
	return nil
}
