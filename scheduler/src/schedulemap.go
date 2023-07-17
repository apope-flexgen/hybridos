package main

import (
	"fims"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/flextime"
)

// a map where the keys are schedule IDs and the values are pointers to schedule objects.
type scheduleMap map[string]*schedule

// contains a map of all schedule IDs and schedule objects that Scheduler is managing.
var masterSchedule scheduleMap = scheduleMap{}

// deleteSchedules iterates through the passed-in list of schedule IDs and deletes them from the schedule map.
func (schedules scheduleMap) deleteSchedules(ids map[string]struct{}) {
	for id := range ids {
		delete(schedules, id)
	}
}

// deleteAllSchedulesExcept deletes all schedules in the schedule map except for the ones in the passed-in list
func (schedules scheduleMap) deleteAllSchedulesExcept(ids []string) {
	schedulesToDelete := schedules.getScheduleIds()
	for _, id := range ids {
		delete(schedulesToDelete, id)
	}
	schedules.deleteSchedules(schedulesToDelete)
}

// Handles SETs to URIs beginning with /scheduler/events.
func (schedules scheduleMap) handleFimsSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		for _, sched := range schedules {
			if err := sched.handleFimsSet(msg); err != nil {
				log.Errorf("Error handling %s's portion of SET to %s: %v.", sched.id, msg.Uri, err)
			}
		}
		sendReply(msg.Replyto, schedules.buildObj())
		return nil
	}

	scheduleId := msg.Frags[2]
	targetSchedule, ok := schedules[scheduleId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no schedule exists with ID %s", scheduleId)
	}
	if err := targetSchedule.handleFimsSet(msg); err != nil {
		return fmt.Errorf("failed to handle SET to schedule %s: %w", scheduleId, err)
	}
	return nil
}

// Routes POSTs with URIs beginning with /scheduler/events/<schedule ID> to the schedules they are targeting.
func (schedules scheduleMap) handleFimsPost(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	scheduleId := msg.Frags[2]
	targetSchedule, ok := schedules[scheduleId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no schedule exists with ID %s", scheduleId)
	}
	if err := targetSchedule.handleFimsPost(msg); err != nil {
		return fmt.Errorf("failed to handle POST to schedule %s: %w", scheduleId, err)
	}
	return nil
}

// Routes DELs with URIs beginning with /scheduler/events/<schedule ID> to the schedules that they are targeting.
func (schedules scheduleMap) handleFimsDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	scheduleId := msg.Frags[2]
	targetSchedule, ok := schedules[scheduleId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no schedule exists with ID %s", scheduleId)
	}
	if err := targetSchedule.handleFimsDel(msg); err != nil {
		return fmt.Errorf("failed to handle DEL to schedule %s: %w", scheduleId, err)
	}
	return nil
}

// Handles GETs with URIs that start with /scheduler/events.
func (schedules scheduleMap) handleFimsGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendReply(msg.Replyto, schedules.buildObj())
		return nil
	}

	scheduleId := msg.Frags[2]
	targetSchedule, ok := schedules[scheduleId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no schedule found with ID %s", scheduleId)
	}

	if err := targetSchedule.handleFimsGet(msg); err != nil {
		return fmt.Errorf("failed to handle GET to schedule with ID %s: %w", scheduleId, err)
	}
	return nil
}

// deleteEventsWithMode deletes any events in any schedule with the targeted mode, including ending active events with the mode.
func (schedules scheduleMap) deleteEventsWithMode(modeBeingDeleted string) {
	for _, sched := range schedules {
		sched.deleteEventsWithMode(modeBeingDeleted)
	}
}

// deleteVarOfMode iterates through every schedule and deletes the given variable from any events that use the given mode.
func (schedules scheduleMap) deleteVarOfMode(varID, changedMode string) {
	for _, sched := range schedules {
		sched.scheduledEvents.deleteVarOfMode(varID, changedMode)
	}
}

func (sm scheduleMap) buildObj() map[string]eventList {
	obj := make(map[string]eventList)
	for id, sched := range sm {
		obj[id] = sched.aggregateAllEvents()
	}
	return obj
}

// getScheduleIds returns a list of IDs for all schedules in the map.
func (s scheduleMap) getScheduleIds() map[string]struct{} {
	idList := make(map[string]struct{})
	for id := range s {
		idList[id] = struct{}{}
	}
	return idList
}

func (sm1 scheduleMap) equals(sm2 scheduleMap) (areEqual bool, reasonNotEqual string) {
	if len(sm1) != len(sm2) {
		return false, fmt.Sprintf("map 1 length %d does not match map 2 length %d", len(sm1), len(sm2))
	}

	for id, sched1 := range sm1 {
		sched2, ok := sm2[id]
		if !ok {
			return false, fmt.Sprintf("ID %s found in map 1 but not map 2", id)
		}
		if areEqual, reasonNotEqual := sched1.equals(sched2); !areEqual {
			return false, fmt.Sprintf("schedules with ID %s are not equal: %s", id, reasonNotEqual)
		}
	}
	return true, ""
}

// updates each schedule by deleting old events, expiring ended events, and
// executing the next event in the local schedule if it is ready
func checkMasterSchedule() {
	var scheduleChanged bool
	// iterate through each schedule and process it
	for _, sched := range masterSchedule {
		scheduleChanged = sched.check() || scheduleChanged
		scheduleChanged = sched.pruneOldEventsAndExceptions() || scheduleChanged
	}

	// if master schedule changed, send out updates
	if scheduleChanged {
		sendUpdatesAfterMasterScheduleEdit(false)
	}
}

// verifyActiveEventStatus sends GETs to all URIs of the active event's variables/constants so that Scheduler can verify
// that their values are correct.
func verifyActiveEventStatus() error {
	// get pointer to schedule object
	localSchedule, ok := masterSchedule[schedCfg.LocalSchedule.Id]
	if !ok {
		return fmt.Errorf("local schedule with ID '%s' not found", schedCfg.LocalSchedule.Id)
	}

	// send GETs for the schedule's active event's setpoints' status values
	if err := localSchedule.verifyActiveEventStatus(); err != nil {
		return fmt.Errorf("failed to verify active event setpoint statuses: %w", err)
	}
	return nil
}

// Builds a map of schedule IDs to a string representing the schedule's time zone.
func buildTimezoneMap() map[string]string {
	tzMap := make(map[string]string)
	for _, sched := range masterSchedule {
		if sched.timezone == nil {
			tzMap[sched.id] = ""
		} else {
			tzMap[sched.id] = sched.timezone.String()
		}
	}
	return tzMap
}

// resendActiveEventsWithMode iterates through every schedule and re-executes any active events that use the given mode.
func resendActiveEventsWithMode(changedMode string) {
	// if no local schedule configured, will not have to resend any setpoints
	if schedCfg.LocalSchedule == nil {
		return
	}
	sched := masterSchedule[schedCfg.LocalSchedule.Id]
	if sched == nil {
		log.Errorf("Error when trying to resend active event setpoints: no schedule found with ID %s", schedCfg.LocalSchedule.Id)
		return
	}
	sched.resendActiveEventsWithMode(changedMode)
}

// Ends all active events and deletes all scheduled events.
func clearMasterSchedule() {
	for _, sched := range masterSchedule {
		sched.clear()
	}
	sendUpdatesAfterMasterScheduleEdit(true)
}

// sendUpdatesAfterMasterScheduleEdit takes care of sending an updated master schedule to all interested parties.
// TODO: our current method of sending updates sends very large objects even if the change was only to smaller objects. make updates more precise
func sendUpdatesAfterMasterScheduleEdit(externalEditor bool) {
	// since all controllers are internally tracking the master schedule, internal updates do not need to be sent across WebSockets
	if externalEditor {
		updateLastScheduleModification()
		updateScheduleOfFleetSiteCounterpart()
	}

	// update the SCADA interface
	refreshSiteEnums()
	updateScadaRegs()

	// backup, log, and publish new events
	broadcastStateData("events", masterSchedule.buildObj())
}

// Sets lastScheduleModification to the current time and backs it up to DBI.
func updateLastScheduleModification() {
	lastScheduleModification = flextime.Now()
	f.SendSet("/dbi/scheduler/last_schedule_modification/timestamp", "", lastScheduleModification)
}
