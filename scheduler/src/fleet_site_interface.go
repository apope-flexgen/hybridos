/**
 *
 * fleet_site_interface.go
 *
 * Interface between Schedulers on Fleet Manager boxes and Site Controller boxes.
 *
 * Due to regulations, Fleet Manager will always be the client and initiate the connection to its Site Controllers, which will always be servers.
 *
 */

package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/websockets"
)

// The data that must be sent from Fleet Manager to Site Controller upon WebSocket connection
// in order to facilitate the two Schedulers syncing with each other.
type webSocketSyncData struct {
	LastModification time.Time `json:"last_schedule_modification"`
	Modes            modeMap   `json:"modes"`
	Events           eventList `json:"events"`
}

// lastScheduleModification keeps track of when the schedule was last modified by the UI, SCADA, or Fleet-Site interface.
// The timestamp is used to determine who should update who when a Fleet Scheduler and a Site Scheduler connect to each other.
// Its initial value is Jan 1, 0000 so that an unedited schedule will not overwrite an edited schedule.
var lastScheduleModification time.Time

// Actions to take when a WebSocket client connects or disconnects.
func processSiteConnectionUpdate(update websockets.ClientConnectionUpdate) {
	// pub with update about connection status
	f.SendPub("/scheduler/connected", buildConnectionMap())

	if update.Connected {
		sendUpdateSyncToSiteController(update.ClientId)
	}
}

// Sends Site Controller the latest schedule, mode map, and last modification timestamp of Fleet Manager.
func sendUpdateSyncToSiteController(siteId string) {
	sched, ok := masterSchedule[siteId]
	if !ok {
		log.Errorf("Could not find schedule for client %s.", siteId)
		return
	}

	// send the Site Controller the latest modes, events, and last schedule modification timestamp that Fleet Manager has.
	// Site Controller will always accept the FM's modes. if SC has an older last modification timestamp, it will accept
	// the events as well. if SC's last modification timestamp is more recent, it will send its events to FM to update FM
	msg := websockets.Msg{
		Method: "set",
		Uri:    "update_sync",
		Body: webSocketSyncData{
			LastModification: lastScheduleModification,
			Modes:            modes,
			Events:           sched.aggregateAllEvents(),
		},
	}
	if err := websockets.Clients.WriteTo(siteId, msg); err != nil {
		log.Errorf("Error sending update sync to %s Site Controller: %v.", siteId, err)
		return
	}
}

// When a WebSocket connection is first established between a Fleet Manager and a Site Controller's Schedulers, Fleet Manager will send Site Controller
// an update sync message containing Fleet Manager's modes, events, and last modification timestamp. This function handles processing that update sync
// message.
//
// Begins by sending Site Controller's time zone to Fleet Manager. Will then read in Fleet Manager's modes.
// If Site Controller's last modification timestamp is more recent than Fleet Manager's, last step is to send Fleet Manager the up-to-date schedule.
// Otherwise, last step is to read in Fleet Manager's up-to-date schedule.
func handleUpdateSyncFromFleetManager(msg websockets.Msg) error {
	// send Site Controller's timezone to Fleet Manager
	if err := sendTimeZoneToFleetManager(); err != nil {
		log.Errorf("Error sending time zone to Fleet Manager: %v.", err)
	}

	var fleetManagerData webSocketSyncData
	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		return fmt.Errorf("failed to marshal update sync data: %w", err)
	}
	if err := json.Unmarshal(jsonBytes, &fleetManagerData); err != nil {
		return fmt.Errorf("failed to unmarshal update sync data: %w", err)
	}

	// fleet-connected Site Controller should always defer to Fleet Manager's modes
	if err := modes.handleMapSet(fleetManagerData.Modes, false); err != nil {
		return fmt.Errorf("failed to process Fleet Manager's modes: %w", err)
	}

	// compare timestamps and send SET or GET depending on logic described in this function's description
	if fleetManagerData.LastModification.Before(lastScheduleModification) {
		log.Infof("%s Site Controller last schedule modification timestamp %s is more recent than Fleet Manager's %s, so sending latest %s schedule to Fleet Manager.", schedCfg.LocalSchedule.Id, lastScheduleModification.String(), fleetManagerData.LastModification.String(), schedCfg.LocalSchedule.Id)
		if err = sendEventsFromSiteControllerToFleetManager(); err != nil {
			return fmt.Errorf("failed to send events to Fleet Manager: %w", err)
		}
	} else {
		log.Infof("Fleet Manager last schedule modification timestamp %s is more recent than %s Site Controller's %s, so accepting latest %s schedule from Fleet Manager.", fleetManagerData.LastModification.String(), schedCfg.LocalSchedule.Id, lastScheduleModification.String(), schedCfg.LocalSchedule.Id)
		if err = handleWebSocketEventsSet(schedCfg.LocalSchedule.Id, fleetManagerData.Events); err != nil {
			return fmt.Errorf("failed to send GET to Fleet Manager for events: %w", err)
		}
	}
	return nil
}

// Creates a map of connection IDs to their connection statuses (true for connected; false for disconnected).
func buildConnectionMap() map[string]bool {
	connectionMap := make(map[string]bool)
	for _, sched := range masterSchedule {
		if schedCfg.LocalSchedule != nil && sched.id == schedCfg.LocalSchedule.Id {
			connectionMap[sched.id] = websockets.ServerConn.IsConnected()
		} else {
			connectionMap[sched.id] = websockets.Clients.ClientIsConnected(sched.id)
		}
	}
	return connectionMap
}

// Processes a message received through a WebSocket connection.
func processWebSocketMsg(msg websockets.Msg) {
	var err error
	switch msg.Method {
	case "set":
		err = processWebSocketSet(msg)
	default:
		err = fmt.Errorf("invalid method")
	}
	if err != nil {
		log.Errorf("Error processing WebSocket %s message: %v.", msg.Method, err)
	}
}

// Processes any WebSocket messages that are SETs.
func processWebSocketSet(msg websockets.Msg) error {
	// break the target URI into fragments
	frags := strings.Split(msg.Uri, "/")

	// check last fragment to determine if this is SET to events or modes
	var err error
	switch frags[0] {
	case "timezone":
		if len(frags) < 2 {
			return errors.New("first fragment was timezone but there was no second fragment")
		}
		err = handleWebSocketTimezoneSet(frags[1], msg.Body)
	case "update_sync":
		err = handleUpdateSyncFromFleetManager(msg)
	case "events":
		if len(frags) < 2 {
			return errors.New("first fragment was events but there was no second fragement")
		}
		err = handleWebSocketEventsSet(frags[1], msg.Body)
	case "modes":
		err = modes.handleMapSet(msg.Body, false)
	}
	if err != nil {
		return fmt.Errorf("failed to handle WebSocket SET to URI %s: %w", msg.Uri, err)
	}
	return nil
}

// Handles WebSocket SETs with URIs beginning with events.
func handleWebSocketEventsSet(targetSite string, inputEventListInterface interface{}) error {
	sched, ok := masterSchedule[targetSite]
	if !ok {
		return fmt.Errorf("could not find schedule with ID %s", targetSite)
	}

	inputEventList, err := parseEventList(inputEventListInterface, sched.timezone)
	if err != nil {
		return fmt.Errorf("failed to parse input: %w", err)
	}

	// events coming from Fleet/Site counterpart should maintain same IDs
	if err := sched.overwrite(inputEventList, inputEventList.buildIdCache(), false); err != nil {
		return fmt.Errorf("failed to overwrite schedule: %w", err)
	}
	return nil
}

// Handles WebSocket SETs containing the timezone of a single Site Controller's box.
func handleWebSocketTimezoneSet(targetSiteId string, timezoneInterface interface{}) error {
	// verify that the data payload is a string
	timezoneString, ok := timezoneInterface.(string)
	if !ok {
		return fmt.Errorf("body of time zone WebSocket msg is not a string. Actual type is %T", timezoneInterface)
	}

	// load the time.Location corresponding to the received string
	tz, err := time.LoadLocation(timezoneString)
	if err != nil {
		return fmt.Errorf("failed to load *time.Location from time zone name %s: %w", timezoneString, err)
	}

	sched, ok := masterSchedule[targetSiteId]
	if !ok {
		return fmt.Errorf("failed to get schedule %s", targetSiteId)
	}
	if err := sched.setTimezone(tz); err != nil {
		return fmt.Errorf("failed to set %s's time zone: %w", sched.id, err)
	}

	broadcastNewTimezones()
	return nil
}

func sendEventsFromFleetManagerToSiteController(targetSiteId string) error {
	sched, ok := masterSchedule[targetSiteId]
	if !ok {
		return fmt.Errorf("could not send non-existent schedule %s through WebSocket", targetSiteId)
	}
	msg := websockets.Msg{
		Method: "set",
		Uri:    fmt.Sprintf("events/%s", targetSiteId),
		Body:   sched.aggregateAllEvents(),
	}
	if err := websockets.Clients.WriteTo(targetSiteId, msg); err != nil {
		return fmt.Errorf("failed to write to %s client: %w", targetSiteId, err)
	}
	return nil
}

func sendEventsFromSiteControllerToFleetManager() error {
	sched, ok := masterSchedule[schedCfg.LocalSchedule.Id]
	if !ok {
		return fmt.Errorf("could not send non-existent schedule %s through WebSocket", schedCfg.LocalSchedule.Id)
	}
	msg := websockets.Msg{
		Method: "set",
		Uri:    fmt.Sprintf("events/%s", schedCfg.LocalSchedule.Id),
		Body:   sched.aggregateAllEvents(),
	}
	if err := websockets.ServerConn.WriteJSON(msg); err != nil {
		return fmt.Errorf("failed to write to Site Controller: %w", err)
	}
	return nil
}

// sendModesThroughWebSocket sends all modes to the given URI through the WebSocket. This function should only be used by Fleet Manager.
func sendModesThroughWebSocket(targetSiteId string) error {
	msg := websockets.Msg{
		Method: "set",
		Uri:    "modes",
		Body:   modes,
	}
	if err := websockets.Clients.WriteTo(targetSiteId, msg); err != nil {
		return fmt.Errorf("failed to write to %s client: %w", targetSiteId, err)
	}
	return nil
}

func sendTimeZoneToFleetManager() error {
	sched, ok := masterSchedule[schedCfg.LocalSchedule.Id]
	if !ok {
		return fmt.Errorf("could not find local schedule %s", schedCfg.LocalSchedule.Id)
	}
	msg := websockets.Msg{
		Method: "set",
		Uri:    fmt.Sprintf("timezone/%s", sched.id),
		Body:   sched.timezone.String(),
	}
	if err := websockets.ServerConn.WriteJSON(msg); err != nil {
		return fmt.Errorf("failed to write to Fleet Manager: %w", err)
	}
	return nil
}

// updateScheduleOfFleetSiteCounterpart sends the schedule over the WebSocket.
// If Scheduler is a Fleet Manager instance, each Site Controller in the fleet has only its own schedule sent to it.
// If Scheduler is a Site Controller, its entire schedule is sent to Fleet Manager.
func updateScheduleOfFleetSiteCounterpart() {
	// send appropriate schedules to counterpart(s)
	if schedCfg.SchedulerType == FLEET_SCHEDULER {
		for _, sched := range masterSchedule {
			if (schedCfg.LocalSchedule == nil || sched.id != schedCfg.LocalSchedule.Id) && websockets.Clients.ClientIsConnected(sched.id) {
				if err := sendEventsFromFleetManagerToSiteController(sched.id); err != nil {
					log.Errorf("Error sending events to %s Site Controller: %v.", sched.id, err)
				}
			}
		}
		return
	}
	// == SITE_SCHEDULER
	if websockets.ServerConn.IsConnected() {
		if err := sendEventsFromSiteControllerToFleetManager(); err != nil {
			log.Errorf("Error sending events to Fleet Manager: %v.", err)
		}
	}
}

// updateModesOfSiteControllers sends modes to all Site Controllers in the fleet.
func updateModesOfSiteControllers() {
	for _, sched := range masterSchedule {
		if websockets.Clients.ClientIsConnected(sched.id) {
			err := sendModesThroughWebSocket(sched.id)
			if err != nil {
				log.Errorf("Error trying to update mode of %s Site Controller: %v.", sched.id, err)
			}
		}
	}
}
