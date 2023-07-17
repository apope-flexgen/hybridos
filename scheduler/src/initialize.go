package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/internal/websockets"
)

// Contains a field for each of the documents that get stored in DBI.
type storedState struct {
	Cfg              schedulerConfig      `json:"configuration"`
	Timezones        map[string]string    `json:"timezones"`
	LastModification lastModTimeWrapper   `json:"last_schedule_modification"`
	Modes            modeMap              `json:"modes"`
	EventLists       map[string]eventList `json:"events"`
}

// temporary solution until DBI can support primitive types
type lastModTimeWrapper struct {
	Timestamp time.Time `json:"timestamp"`
}

// Filters initialization documents into a state data object.
// If there was no valid basic configuration, sits and waits for it to be sent.
// Finally, initializes Scheduler with the validated initial state.
func initializeScheduler(initializationDocs map[string]json.RawMessage, fimsReceive chan fims.FimsMsg) error {
	state, err := buildInitialStateFromDocs(initializationDocs)
	if err != nil {
		return fmt.Errorf("failed to build initial state from documents: %w", err)
	}

	// check if the initial state includes a basic configuration. if not, sit and wait for valid configuration to be sent
	if err := state.Cfg.validate(); err != nil {
		log.Infof("No valid configuration given. Will not begin operation until valid configuration is sent to /scheduler/configuration.")
		state.Cfg = waitForBasicConfiguration(fimsReceive)
		log.Infof("Valid configuration has been received.")
	}
	loadInitialState(state)
	broadcastState(state)
	return nil
}

// Takes the initialization documents, which were fetched from either DBI or disk,
// and filters out any invalid documents. Normalizes into a state data object.
//
// If invalid data is found in documents, logs an error and proceeds with default values.
func buildInitialStateFromDocs(initializationDocs map[string]json.RawMessage) (state *storedState, err error) {
	// default values
	state = &storedState{
		Cfg:        schedulerConfig{}, // empty scheduleType string in schedulerConfig will be the signal that state is unconfigured
		Timezones:  make(map[string]string),
		Modes:      NewDefaultModeMap(),
		EventLists: make(map[string]eventList),
	}

	// empty map would be the case if it is Scheduler's first time being launched
	if len(initializationDocs) == 0 {
		return state, nil
	}

	if err = readConfigurationDocIntoState(initializationDocs, state); err != nil {
		return nil, fmt.Errorf("failed to read configuration document: %w", err)
	}

	// initialize time zone map with empty strings for all time zones.
	// time zone for local schedule will get overwritten in load state step of startup sequence
	for _, scheduleId := range state.Cfg.getListOfScheduleIds() {
		state.Timezones[scheduleId] = ""
	}

	// allow pre-loading DBI with only basic configuration
	if len(initializationDocs) == 1 {
		return state, nil
	}

	if err = readModesDocIntoState(initializationDocs, state); err != nil {
		return nil, fmt.Errorf("failed to read modes document: %w", err)
	}

	// allow pre-loading DBI with only basic configuration and modes
	if len(initializationDocs) == 2 {
		return state, nil
	}

	if err = readTimezoneDocIntoState(initializationDocs, state); err != nil {
		return nil, fmt.Errorf("failed to read time zone document: %w", err)
	}

	if err = readEventsDocIntoState(initializationDocs, state); err != nil {
		return nil, fmt.Errorf("failed to read events document: %w", err)
	}

	if err = readLastModificationDocIntoState(initializationDocs, state); err != nil {
		return nil, fmt.Errorf("failed to read last schedule modification document: %w", err)
	}
	return state, nil
}

func readConfigurationDocIntoState(initializationDocs map[string]json.RawMessage, state *storedState) error {
	generalConfigurationJson, ok := initializationDocs["configuration"]
	if !ok {
		return errors.New("configuration document not found")
	}
	if err := json.Unmarshal(generalConfigurationJson, &state.Cfg); err != nil {
		return fmt.Errorf("failed to unmarshal configuration document: %w", err)
	}
	return nil
}

func readTimezoneDocIntoState(initializationDocs map[string]json.RawMessage, state *storedState) error {
	timezonesJson, ok := initializationDocs["timezones"]
	if !ok {
		return errors.New("time zone document not found")
	}
	if err := json.Unmarshal(timezonesJson, &state.Timezones); err != nil {
		return fmt.Errorf("failed to unmarshal time zones: %w", err)
	}
	for id, timezoneString := range state.Timezones {
		if _, err := time.LoadLocation(timezoneString); err != nil {
			log.Errorf("Error loading location from time zone string %s: %v. Removing it from time zone map.", timezoneString, err)
			delete(state.Timezones, id)
		}
	}
	return nil
}

func readModesDocIntoState(initializationDocs map[string]json.RawMessage, state *storedState) error {
	modesJson, ok := initializationDocs["modes"]
	if !ok {
		return errors.New("modes document not found")
	}
	if err := json.Unmarshal(modesJson, &state.Modes); err != nil {
		return fmt.Errorf("failed to unmarshal modes: %w", err)
	}
	if err := state.Modes.validate(); err != nil {
		return fmt.Errorf("failed to validate modes: %w", err)
	}
	return nil
}

func readEventsDocIntoState(initializationDocs map[string]json.RawMessage, state *storedState) error {
	eventsJson, ok := initializationDocs["events"]
	if !ok {
		return errors.New("events document not found")
	}
	if err := json.Unmarshal(eventsJson, &state.EventLists); err != nil {
		return fmt.Errorf("failed to unmarshal events: %w", err)
	}
	for id, eList := range state.EventLists {
		timezoneString, ok := state.Timezones[id]
		if !ok {
			return fmt.Errorf("no associated time zone for event list %s", id)
		}
		if timezoneString == "" {
			continue
		}
		tz, err := time.LoadLocation(timezoneString)
		if err != nil {
			return fmt.Errorf("time zone string %s for event list %s is not loadable: %w", timezoneString, id, err)
		}
		if err = eList.validate(tz, state.Modes.buildMapOfVariables()); err != nil {
			return fmt.Errorf("failed to validate event list %s: %w", id, err)
		}
		cache := idcache.New()
		for _, event := range eList {
			if cache.CheckId(event.Id) {
				return fmt.Errorf("duplicate ID %d found in event list %s", event.Id, id)
			}
			cache.Add(event.Id)
		}
	}
	return nil
}

func readLastModificationDocIntoState(initializationDocs map[string]json.RawMessage, state *storedState) error {
	lastSchedModJson, ok := initializationDocs["last_schedule_modification"]
	if !ok {
		return errors.New("last_schedule_modification document not found")
	}
	if err := json.Unmarshal(lastSchedModJson, &state.LastModification); err != nil {
		return fmt.Errorf("failed to unmarshal last_schedule_modification: %w", err)
	}
	return nil
}

// Infinitely listens for FIMS message until valid basic configuration is sent to /scheduler/configuration.
// Will not respond to any other FIMS messages except for GETs to /scheduler/configuration, to which it will reply with an empty object.
func waitForBasicConfiguration(fimsReceive chan fims.FimsMsg) (schedulerConfig) {
	for {
		msg := <-fimsReceive
		if msg.Uri != "/scheduler/configuration" {
			continue
		}
		if msg.Method == "get" {
			sendReply(msg.Replyto, map[string]interface{}{})
			continue
		}
		if msg.Method != "set" {
			sendErrorResponse(msg.Replyto, "While waiting for configuration, only valid methods are SET and GET.")
			continue
		}
		jsonBytes, err := json.Marshal(msg.Body)
		if err != nil {
			log.Errorf("Error marshaling configuration: %v.", err)
			sendErrorResponse(msg.Replyto, "Invalid JSON")
			continue
		}
		cfg := schedulerConfig{}
		if err = json.Unmarshal(jsonBytes, &cfg); err != nil {
			log.Errorf("Error unmarshaling configuration: %v.", err)
			sendErrorResponse(msg.Replyto, "Invalid JSON")
			continue
		}
		if err = cfg.validate(); err != nil {
			log.Errorf("Invalid configuration received: %v.", err)
			sendErrorResponse(msg.Replyto, "Invalid Data")
			continue
		}
		return cfg
	}
}

// Initializes Scheduler with a state that has already been validated.
func loadInitialState(state *storedState) {
	// time zones and events are stored in schedule map which does not follow same format as JSON so cannot do a simple assignment here
	schedCfg = state.Cfg
	lastScheduleModification = state.LastModification.Timestamp
	modes = state.Modes

	// build schedule map, launch setpoint enforcement ticker, and build WebSocket client map
	if schedCfg.LocalSchedule != nil {
		masterSchedule[schedCfg.LocalSchedule.Id] = newSchedule(schedCfg.LocalSchedule.Id, localTimezone)
		reconfigureSetpointEnforcement(schedCfg.LocalSchedule.SetpointEnforcement)
	}
	if schedCfg.WebSockets.ClientConfigs != nil {
		for _, client := range *schedCfg.WebSockets.ClientConfigs {
			masterSchedule[client.Id] = newSchedule(client.Id, nil)
			websockets.Clients.Add(websockets.NewClient(client.Id, client.Ip, client.Port))
		}
	}

	// read in all time zones
	if schedCfg.LocalSchedule != nil {
		// update time zone of local schedule in case box's time zone was changed since the last time Scheduler was run
		state.Timezones[schedCfg.LocalSchedule.Id] = localTimezone.String()
	}
	for scheduleId, timezoneString := range state.Timezones {
		sched, ok := masterSchedule[scheduleId]
		if !ok {
			// if not found, the time zone is unused and should be discarded
			delete(state.Timezones, scheduleId)
			continue
		}
		tz, err := time.LoadLocation(timezoneString)
		if err != nil {
			// if err, then assume string is "" which implies site had not connected to Fleet Manager prior to restart
			continue
		}
		sched.timezone = tz
	}

	// add events to their respective schedules
	for id, storedEvents := range state.EventLists {
		sched, ok := masterSchedule[id]
		if !ok {
			continue
		}
		sched.cache = storedEvents.buildIdCache()
		sched.integrateEventList(storedEvents)
	}

	// in the case that eventLists state is empty (such as first time Scheduler is launched), update it with empty schedules
	state.EventLists = masterSchedule.buildObj()

	// configure SCADA interface
	refreshModeEnums()
	refreshSiteEnums()
	reconfigureScadaInterface(schedCfg.Scada)
}

// Writes all state data to DBI, logs all, and publishes all.
func broadcastState(state *storedState) {
	broadcastStateData("configuration", state.Cfg)
	broadcastStateData("timezones", state.Timezones)
	broadcastStateData("last_schedule_modification", state.LastModification)
	broadcastStateData("modes", state.Modes)
	broadcastStateData("events", state.EventLists)
}

// Writes the given state data to DBI, logs it, and publishes it.
func broadcastStateData(id string, data interface{}) {
	sendBackup(fmt.Sprintf("/dbi/scheduler/%s", id), data)

	bytes, err := json.Marshal(data)
	if err != nil {
		log.Errorf("Error marshaling %s state data.", id)
	} else {
		log.Infof("%s: %s", id, string(bytes))
	}

	f.SendPub(fmt.Sprintf("/scheduler/%s", id), data)
}

func buildState() *storedState {
	return &storedState{
		Cfg:              schedCfg,
		Timezones:        buildTimezoneMap(),
		LastModification: lastModTimeWrapper{Timestamp: lastScheduleModification},
		Modes:            modes,
		EventLists:       masterSchedule.buildObj(),
	}
}
