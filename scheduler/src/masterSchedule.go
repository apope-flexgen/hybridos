/**
 *
 * masterSchedule.go
 *
 * Methods for the `masterSchedule` data type.
 *
 */
package main

import (
	"errors"
	"fmt"
	"log"
	"strconv"
	"sync"
	"time"
)

// a map where the keys are strings of site IDs and the values are pointers to siteController objects.
type schedule map[string]*siteController

type safeSchedule struct {
	data schedule
	mu   sync.RWMutex
}

// contains a map of all site IDs and siteController objects that Scheduler is managing.
var masterSchedule safeSchedule

// variation on a "copy constructor" that does not include any events
func (ss *safeSchedule) createEmptyCopy() schedule {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	return ss.data.createEmptyCopy()
}

func (ss *safeSchedule) getNumSites() int {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	return len(ss.data)
}

func (ss *safeSchedule) getSites() []*siteController {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	sites := make([]*siteController, 0)
	for _, site := range ss.data {
		sites = append(sites, site)
	}
	return sites
}

func (ss *safeSchedule) getSite(id string) *siteController {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	site, ok := ss.data[id]
	if !ok {
		return nil
	}
	return site
}

// buildObj builds a map[string]interface{} to represent the entire schedule.
func (ss *safeSchedule) buildObj() map[string]interface{} {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	entireSchedule := make(map[string]interface{})
	for siteId, site := range ss.data {
		entireSchedule[siteId] = *site.buildObj()
	}
	return entireSchedule
}

// buildObj builds a map[string]interface{} to represent the entire schedule.
// start time is given as mins since midnight
func (ss *safeSchedule) buildLegacyObject() map[string]interface{} {
	ss.mu.RLock()
	defer ss.mu.RUnlock()
	entireSchedule := make(map[string]interface{})
	for siteId, site := range ss.data {
		entireSchedule[siteId] = *site.buildLegacyObject()
	}
	return entireSchedule
}

// addSite adds a new site to the schedule or replaces an existing site.
func (ss *safeSchedule) addSite(newSite *siteController) {
	ss.mu.Lock()
	defer ss.mu.Unlock()
	ss.data[newSite.id] = newSite
}

// deleteSites iterates through the passed-in list of site IDs and deletes them from the schedule.
func (ss *safeSchedule) deleteSites(ids map[string]struct{}) {
	ss.mu.Lock()
	defer ss.mu.Unlock()
	for id := range ids {
		delete(ss.data, id)
	}
}

// deleteAllSitesExcept deletes all sites in the schedule except for the sites in the passed-in list
func (ss *safeSchedule) deleteAllSitesExcept(ids []string) {
	// do not use mutex since all operations that read/write safeSchedule do so atomically.
	// locking the mutex here is unnecessary and would result in permanent lock once atomic functions called
	sitesToDelete := ss.data.getSiteIds()
	for _, id := range ids {
		delete(sitesToDelete, id)
	}
	ss.deleteSites(sitesToDelete)
}

// buildSiteScheduleObj builds an object that only includes a single site's schedule.
//
// siteId indicates which site should be included.
func buildSiteScheduleObj(siteId string) map[string]interface{} {
	obj := make(map[string]interface{})
	site := masterSchedule.getSite(siteId)
	obj[siteId] = *site.buildObj()
	return obj
}

// iterates through each site in the master schedule and checks the next event.
// If it is time for the event, the event is removed from the list of scheduled events and executed.
func checkMasterSchedule() {
	// handle any sites that have crossed midnight in their time zones (need to roll over their schedules to the "new tomorrow")
    scheduleChanged := handleDayTransitions(getCurrentTime(), lastCheck)

	// iterate through each site and process its schedule
	for _, site := range masterSchedule.getSites() {
		scheduleChanged = site.check() || scheduleChanged
	}

	// if schedule changed, send out updates
	if scheduleChanged {
		sendUpdatesAfterScheduleEdit(INTERNAL)
	}

	// update the lastCheck timestamp
	lastCheck = getCurrentTime()
}

// verifyActiveEventStatus sends GETs to all URIs of the active event's variables/constants so that Scheduler can verify
// that their values are correct. Returns an error if there is not exactly one site in the schedule or if that site does
// not have an active event.
func verifyActiveEventStatus() error {
	// this function assumes Scheduler is a Site Controller, so there should only be one site.
	// if there is not one site, this assumption is broken and the function should not be executed.
	if numSites := masterSchedule.getNumSites(); numSites != 1 {
		return errors.New("expected one and only one site in the schedule, but there are " + strconv.Itoa(numSites))
	}

	// get pointer to site object
	thisSite := masterSchedule.getSite(thisSiteId)
	if thisSite == nil {
		return errors.New("thisSite (" + thisSiteId + ") not found in schedule")
	}

	// send GETs for the site's active event's setpoints' status values
	if err := thisSite.verifyActiveEventStatus(); err != nil {
		return fmt.Errorf("failed to verify active event setpoint statuses: %w", err)
	}
	return nil
}

// handleDayTransitions will check each site to see if midnight has been crossed in the site's time zone. If so, the site's repeatDayIfMidnight method is called.
func handleDayTransitions(now, last time.Time) (scheduleChanged bool) {
	scheduleChanged = false
	for _, site := range masterSchedule.getSites() {
		// do not attempt to transition sites that have not been fully configured yet
		if site.timezone == nil {
			continue
		}
		currentTime := now.In(site.timezone)
		lastCheckTime := last.In(site.timezone)
		if currentTime.Day() != lastCheckTime.Day() {
			scheduleChanged = true
			site.repeatDayIfMidnight(currentTime)
		}
	}
	return
}

// buildTimezoneMap builds a map with the following format:
// {
//     "siteId1": "timezone1",
//     "siteId2": "timezone2",
//     ...
//     "siteIdN": "timezoneN"
// }
func buildTimezoneMap() map[string]interface{} {
	tzMap := make(map[string]interface{})
	for _, site := range masterSchedule.getSites() {
		tzMap[site.id] = site.timezone.String()
	}
	return tzMap
}

// deleteEventsWithMode deletes any events in the whole schedule with the targeted mode, including ending active events with the mode.
func deleteEventsWithMode(modeBeingDeleted string) {
	for _, site := range masterSchedule.getSites() {
		site.deleteEventsWithMode(modeBeingDeleted)
	}
}

// equals returns if schedule 1 equals schedule 2.
func (s1 schedule) equals(s2 schedule) bool {
	// if the two schedules do not hold the same number of sites, they cannot be equal
	if len(s1) != len(s2) {
		log.Println("Lengths of master schedules do not match")
		return false
	}
	for siteId, siteInfo1 := range s1 {
		siteInfo2, ok := s2[siteId]
		if !ok {
			log.Println(siteId, "has no associated site object in schedule")
			return false
		}
		if !siteInfo1.equals(siteInfo2) {
			log.Println("Site", siteId, "failed equality test")
			return false
		}
	}
	return true
}

// deleteVarOfMode iterates through every site and deletes the given variable from any events that use the given mode.
func deleteVarOfMode(varID, changedMode string) {
	for _, site := range masterSchedule.getSites() {
		site.scheduledEvents.deleteVarOfMode(varID, changedMode)
	}
}

// resendActiveEventsWithMode iterates through every site and re-executes any active events that use the given mode.
func resendActiveEventsWithMode(changedMode string) {
	for _, site := range masterSchedule.getSites() {
		site.resendActiveEventsWithMode(changedMode)
	}
}

// ends all active events and deletes all scheduled events.
func clearMasterSchedule(editor editingInterface) {
	for _, site := range masterSchedule.getSites() {
		if site.activeEvent != nil {
			site.endActiveEvent()
		}
		site.scheduledEvents = &eventHeap{}
		site.expiredEvents = &eventHeap{}
	}
	sendUpdatesAfterScheduleEdit(editor)
}

// getSiteIds returns a list of all the site ids in the schedule.
func (s schedule) getSiteIds() map[string]struct{} {
	idList := make(map[string]struct{})
	for id := range s {
		idList[id] = struct{}{}
	}
	return idList
}

func (s schedule) print() {
	for _, site := range s {
		log.Println("site:", site.id)
		log.Printf("\n")
		log.Println("Active Event:")
		if site.activeEvent == nil {
			log.Println("None")
			log.Printf("\n")
		} else {
			site.activeEvent.Print()
			log.Printf("\n")
		}
		log.Println("Scheduled Events:")
		site.scheduledEvents.print()
		log.Printf("\n")
	}
}

// parseFullSchedule iterates through a map of sites in JSON form and
// returns them in siteController form.
func parseFullSchedule(inputSchedule map[string]interface{}) (schedule, error) {
	// allocate memory for a new schedule
	newSchedule := schedule{}

	// parse the schedule of each site
	for siteId, inputEventsJsonInterface := range inputSchedule {
		inputEventsJson, ok := inputEventsJsonInterface.([]interface{})
		if !ok {
			return nil, fmt.Errorf("schedule of site %s is not a []interface{}, but instead is a %T", siteId, inputEventsJsonInterface)
		}

		// get the timezone of the site if it already exists in the schedule
		var siteTimezone *time.Location
		if referenceSite := masterSchedule.getSite(siteId); referenceSite == nil {
			return nil, fmt.Errorf("failed to find reference site %s for FIMS msg", siteId)
		} else {
			siteTimezone = referenceSite.timezone
		}

		// allocate memory for site object and schedule heap
		site := newSite(siteId)
		inputEvents := newEventHeap()

		// parse the site schedule out of the JSON representation
		err := parseEventListIntoHeap(inputEventsJson, inputEvents, -1, siteTimezone)
		if err != nil {
			return nil, fmt.Errorf("failed to parse input events of %s from FIMS msg: %w", siteId, err)
		}
		site.scheduledEvents = inputEvents

		// add the site to the schedule
		newSchedule[siteId] = site
	}
	return newSchedule, nil
}

// editMultipleSchedules takes an input schedule and performs the passed-in method once on each
// of the site schedules within the input schedule. Note, it does not carry out the method on the
// entire masterSchedule. So a SET with only one of masterSchedule's sites in it will not delete the other
// sites. Edits happen on a site-by-site basis.
func editMultipleSchedules(inputSchedule schedule, editor editingInterface, method editingMethod) {
	// iterate through each site in the input schedule and edit it
	for siteId, inputSite := range inputSchedule {
		// get the pointer to the existing schedule's site
		site := masterSchedule.getSite(siteId)
		if site == nil {
			log.Println("There is no site with ID", siteId, "in the schedule so it will not be edited")
			continue
		}

		// edit the current site's schedule
		site.editSchedule(inputSite.scheduledEvents, CALLER, method, -1)
	}

	// only send out updates if this is the only edit that is happening
	// otherwise, the function caller will take care of sending updates
	if editor != CALLER {
		sendUpdatesAfterScheduleEdit(editor)
	}
}

// sendUpdatesAfterScheduleEdit takes care of sending an updated schedule to all interested parties.
func sendUpdatesAfterScheduleEdit(editor editingInterface) {
	if editor == CALLER { // CALLER should not be used as a final editor, only to defer this function call to a higher-up function
		return
	}

	if editor != INTERNAL {
		checkMasterSchedule() // if the schedule was changed by an external interface, it should be checked

		// if an external interface that was not the database modifies the schedule, mark it as modified
		if editor != DBI { // database does not count because the edits coming from database are secondhand edits
			updateLastScheduleModification()
		}

		if editor != FLEET_SITE { // since all controllers are internally tracking the schedule, internal updates do not need to be sent across websockets
			updateScheduleOfFleetSiteCounterpart()
		}
	}

	if editor != DBI {
		// do not use `sendEvents` to avoid isPrimaryScheduler check
		f.SendSet("/dbi/scheduler/events", "", map[string]interface{}{
			"schedule": masterSchedule.buildObj(),
		})
	}

	updateScadaRegs()  // update the SCADA registers in all cases
	refreshSiteEnums() // update the enumerated SCADA values for sites in all cases

	pubEvents() // update UI with PUB
}

// `updateLastScheduleModification` sets `lastScheduleModification` to the current time
// and sends an update to interested parties.
func updateLastScheduleModification() {
	lastScheduleModification = getCurrentTime()
	sendLastScheduleModification("/dbi/scheduler/lastScheduleModification") // to DBI even if Secondary
}

// variation on a "copy constructor" that does not include any events
func (sched schedule) createEmptyCopy() schedule {
	newSchedule := schedule{}
	for id, site := range sched {
		newSchedule[id] = site.createEmptyCopy()
	}
	return newSchedule
}
