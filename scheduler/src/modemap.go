package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/setpoint"
)

// Keys are mode IDs and values are pointers to the mode objects with those IDs.
type modeMap map[string]*mode

// Holds the modeMap containing all of Scheduler's valid modes.
// Must always have a 'default' mode.
var modes modeMap = NewDefaultModeMap()

const (
	defaultModeId        = "default"
	defaultModeColorCode = "gray"
	defaultModeIcon      = "Build"
	defaultModeName      = "Default"
)

func NewDefaultModeMap() modeMap {
	return modeMap{
		defaultModeId: &mode{
			Variables: setpoint.List{},
			Constants: setpoint.List{},
			ColorCode: defaultModeColorCode,
			Icon:      defaultModeIcon,
			Name:      defaultModeName,
		},
	}
}

// Sends the updated mode map to all interested parties.
func sendUpdatesAfterModeMapEdit(externalEditor bool) {
	// update Site Schedulers if we are Fleet Scheduler
	if schedCfg.SchedulerType == FLEET_SCHEDULER {
		updateModesOfSiteControllers()
	}

	// update the SCADA enumerated values for modes
	refreshModeEnums()

	if isPrimaryScheduler {
		f.SendPub("/scheduler/modes", modes)
	}

	// editing modes might have changed the schedule so send updates for that as well
	sendUpdatesAfterMasterScheduleEdit(externalEditor)
}

// Handles GETs to URIs beginning with /scheduler/modes.
func (mm modeMap) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 2 {
		sendReply(msg.Replyto, mm)
		return nil
	}

	targetModeId := msg.Frags[2]
	targetMode, ok := mm[targetModeId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("did not find mode with ID %s in map", targetModeId)
	}

	if err := targetMode.handleGet(msg); err != nil {
		return fmt.Errorf("failed to handle GET to mode %s", targetModeId)
	}
	return nil
}

// Handles SETs to URIs beginning with /scheduler/modes.
func (mm modeMap) handleSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if schedCfg.SchedulerType == SITE_SCHEDULER && schedCfg.WebSockets.ServerConfig.Enabled {
		sendErrorResponse(msg.Replyto, "Cannot Edit Modes As Fleet-Connected Site Controller")
		return errors.New("cannot edit modes as fleet-connected Site Controller")
	}

	if msg.Nfrags == 2 {
		if err := mm.handleMapSet(msg.Body, true); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to edit mode map with input modes: %w", err)
		}
		sendReply(msg.Replyto, mm)
		return nil
	}

	targetModeId := msg.Frags[2]
	targetMode, ok := modes[targetModeId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no mode with ID %s", targetModeId)
	}

	if err := targetMode.handleSet(msg, targetModeId); err != nil {
		return fmt.Errorf("failed to handle SET to %s mode: %w", targetModeId, err)
	}
	return nil
}

// After validating the given new modes map, overwrites the existing mode map
// and sends updates to all entities that care about mode map edits.
func (mm modeMap) handleMapSet(inputJsonInterface interface{}, externalEditor bool) error {
	jsonBytes, err := json.Marshal(inputJsonInterface)
	if err != nil {
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	newMap := make(modeMap)
	if err = json.Unmarshal(jsonBytes, &newMap); err != nil {
		return fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err := newMap.validate(); err != nil {
		return fmt.Errorf("failed to validate new mode map: %w", err)
	}

	mm.overwrite(newMap)
	sendBackup("/dbi/scheduler/modes", mm)
	sendUpdatesAfterModeMapEdit(externalEditor)
	return nil
}

// Handles POSTs to URIs beginning with /scheduler/modes.
func (mm modeMap) handlePost(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if schedCfg.SchedulerType == SITE_SCHEDULER && schedCfg.WebSockets.ServerConfig.Enabled {
		sendErrorResponse(msg.Replyto, "Cannot Edit Modes As Fleet-Connected Site Controller")
		return errors.New("cannot edit modes as fleet-connected Site Controller")
	}

	if msg.Nfrags == 2 {
		if err := mm.handleNewModePost(msg); err != nil {
			return fmt.Errorf("failed to add mode to mode map: %w", err)
		}
		return nil
	}

	targetModeId := msg.Frags[2]
	targetMode, ok := mm[targetModeId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no mode found with ID %s", targetModeId)
	}
	if err := targetMode.handlePost(msg, targetModeId); err != nil {
		return fmt.Errorf("failed to handle POST to mode %s: %w", targetModeId, err)
	}
	return nil
}

// Parses and validates a new mode, assigns it an ID, and adds it to the modes map.
func (mm modeMap) handleNewModePost(msg fims.FimsMsg) error {
	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var newMode mode
	if err = json.Unmarshal(jsonBytes, &newMode); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to unmarshal input: %w", err)
	}
	if err := newMode.validate(); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to validate new mode settings: %w", err)
	}

	newModeId := newMode.generateId()
	if _, ok := mm[newModeId]; ok {
		sendErrorResponse(msg.Replyto, "Mode Already Exists")
		return fmt.Errorf("mode with ID %s already exists", newModeId)
	}

	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s", newModeId), newMode)

	mm[newModeId] = &newMode
	sendReply(msg.Replyto, mm)
	sendUpdatesAfterModeMapEdit(true)
	return nil
}

// Handles DELs to URIs beginning with /scheduler/modes.
func (mm modeMap) handleDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if schedCfg.SchedulerType == SITE_SCHEDULER && schedCfg.WebSockets.ServerConfig.Enabled {
		sendErrorResponse(msg.Replyto, "Cannot Edit Modes As Fleet-Connected Site Controller")
		return errors.New("cannot edit modes as fleet-connected Site Controller")
	}

	if msg.Nfrags == 2 {
		mm.deleteAllModesExceptDefault()
		sendBackup("/dbi/scheduler/modes", mm)
		sendReply(msg.Replyto, mm)
		sendUpdatesAfterModeMapEdit(true)
		return nil
	}

	targetModeId := msg.Frags[2]

	if msg.Nfrags == 3 {
		mm.deleteMode(targetModeId)
		sendBackup("/dbi/scheduler/modes", mm)
		sendReply(msg.Replyto, mm)
		sendUpdatesAfterModeMapEdit(true)
		return nil
	}

	targetMode, ok := mm[targetModeId]
	if !ok {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("could not find mode with ID %s", targetModeId)
	}
	if err := targetMode.handleDel(msg, targetModeId); err != nil {
		return fmt.Errorf("failed to handle DEL to mode %s: %w", targetModeId, err)
	}
	return nil
}

func (mm modeMap) deleteAllModesExceptDefault() {
	for id := range mm {
		if id != defaultModeId {
			delete(mm, id)
		}
	}
}

// Replaces an old mode map with a new one. Handles the deletion/updates of events
// that use deleted/modified modes.
func (mm modeMap) overwrite(newModes modeMap) {
	for id, existingMode := range mm {
		// delete modes that are not in the new map
		newModeSettings, ok := newModes[id]
		if !ok {
			mm.deleteMode(id)
			continue
		}

		// update modes that are in both maps
		existingMode.overwrite(*newModeSettings, id)
	}

	// add new modes that were not in old map
	for id, newMode := range newModes {
		mm[id] = newMode
	}
}

// Deletes the mode from the mode map and deletes all
// events that have the mode from the schedule.
func (mm modeMap) deleteMode(id string) {
	masterSchedule.deleteEventsWithMode(id)
	delete(mm, id)
}

// Returns a map of mode IDs to the slice of variable setpoints for the mode.
// This method will no longer be needed once modes get moved into their own package.
func (mm modeMap) buildMapOfVariables() map[string]setpoint.List {
	mapOfVariables := make(map[string]setpoint.List)
	for modeId, mode := range mm {
		mapOfVariables[modeId] = mode.Variables
	}
	return mapOfVariables
}

// Ensures mode map has a default mode and validates all individual modes.
func (mm modeMap) validate() error {
	if mm == nil {
		return errors.New("nil map")
	}
	if _, ok := mm[defaultModeId]; !ok {
		return errors.New("required default mode not found")
	}
	for id, m := range mm {
		if id == "" {
			return errors.New("found mode with empty ID which is not allowed")
		}
		if err := m.validate(); err != nil {
			return fmt.Errorf("mode %s is invalid: %w", id, err)
		}
	}
	return nil
}

// Sends default mode's constants.
func (mm modeMap) sendDefaultSetpoints() {
	defaultMode, ok := mm[defaultModeId]
	if !ok {
		log.Errorf("Default mode not found in modes map.")
		return
	}
	defaultMode.sendConstants()
}
