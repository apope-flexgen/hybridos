/**
 *
 * modes.go
 *
 * Map of different event types AKA modes.
 *
 */

package main

import (
	"fmt"
	"log"
	"reflect"
	"sync"

	fg "github.com/flexgen-power/go_flexgen"
)

// valueObj contains a field for each possible type of variable: float, int, bool, and string.
// Only one field should be valid at a time, since `valueObj` should only be used by one variable
// at a time.
type valueObj struct {
	value_float  float64
	value_int    int
	value_bool   bool
	value_string string
}

// mode represents a type of scheduler event.
//
// variables are setpoints for which every event must provide a value.
//
// constants are setpoints that share the same value for every event using the mode.
//
// colorCode is simply a stored piece of data to be shared with the UI.
type mode struct {
	variables []setpoint
	constants []setpoint
	colorCode string
}

// modeMap is a map where the keys are mode names and the values are pointers to the mode objects with those names.
type modeMap map[string]*mode

// a wrapper for the modeMap type to make it thread-safe when needed
type safeModeMap struct {
	data modeMap
	mu sync.RWMutex
}

// modes holds the modeMap containing all of Scheduler's valid modes.
var modes safeModeMap

func (m *mode) copy() (newMode *mode) {
	*newMode = *m
	return newMode
}

// sendConstants iterates through a mode's list of constants and sends them all as FIMS SETs to their respective URIs.
func (m *mode) sendConstants() {
	for _, c := range m.constants {
		c.sendSet()
	}
}

// sendGets sends a GET to every variable's/constant's URI
func (m *mode) sendGets() {
	for _, c := range m.constants {
		c.sendGet(fmt.Sprintf("/scheduler/activeEventStatus/%s", c.id))
	}
	for _, v := range m.variables {
		v.sendGet(fmt.Sprintf("/scheduler/activeEventStatus/%s", v.id))
	}
}

// buildObj creates a map[string]interface{} containing data on all modes. It is used for sending mode data via JSON.
func (mm modeMap) buildObj() map[string]interface{} {
	body := make(map[string]interface{})
	// add each mode to the list
	for name, modeStruct := range mm {
		modeObj := make(map[string]interface{})
		// add variables
		mVars := make([]interface{}, 0)
		for _, varObj := range modeStruct.variables {
			varObj.addToList(&mVars)
		}
		modeObj["variables"] = mVars
		// add constants
		mConsts := make([]interface{}, 0)
		for _, constObj := range modeStruct.constants {
			constObj.addToList(&mConsts)
		}
		modeObj["constants"] = mConsts
		// add color code
		modeObj["color_code"] = modeStruct.colorCode

		body[name] = modeObj
	}
	return body
}

// hasMode returns if a mode map has a mode with a name that matches the given string.
func (mm modeMap) hasMode(name string) bool {
	_, exists := mm[name]
	return exists
}

// overwrite replaces an old mode map with a new one, using logic that could potentially delete existing events based on changes made to mode configuration.
// An event will be deleted if its mode:
//     * is deleted
//     * has a new variable added to it
//     * has a variable type changed
// If a mode variable is deleted, there is no change to active events, but scheduled events have that variable removed from their variable list.
// If the URI of a mode variable is changed while an active event is using that mode, resend the entire event.
func overwriteModeMap(newModes modeMap) {
	modes.mu.Lock()
	defer modes.mu.Unlock()
	var modesToResend []string
	for modeName, oldMode := range modes.data {
		// if mode was deleted or new vars added, delete events
		newMode, modeStillExists := newModes[modeName]
		if !modeStillExists || oldMode.checkIfNewVars(newMode) {
			deleteEventsWithMode(modeName)
			continue
		}
		// handle deleted mode vars and var type changes, and mark URI changes
		for _, oldVar := range oldMode.variables {
			// check for deleted mode var
			newVarIndex := newMode.findModeVariable(oldVar.id)
			if newVarIndex == -1 {
				deleteVarOfMode(oldVar.id, modeName)
				continue
			}
			// check for var type changes
			newVar := newMode.variables[newVarIndex]
			if oldVar.varType != newVar.varType {
				deleteEventsWithMode(modeName)
				break
			}
			// check for URI changes
			if oldVar.uri != newVar.uri {
				modesToResend = append(modesToResend, modeName)
			}
		}
	}

	// overwrite the modes map
	modes.data = newModes

	if schedulerInstanceType != FLEET_MANAGER { // only a Site Controller instance of Scheduler should be sending setpoints
		// resend any active events that had URI changes
		for _, modeName := range modesToResend {
			resendActiveEventsWithMode(modeName)
		}
	}
}

// returns a copy of the given mode map
func (mm modeMap) copy() modeMap {
	newModeMap := make(modeMap)
	for name, ptr := range mm {
		newModeMap[name] = ptr.copy()
	}
	return newModeMap
}

// appendModeMap iterates through an input mode map and appends each of its modes to the old mode map.
// If there are overlapping modes, the existing mode will be edited to match the new mode.
// See `overwrite` for logic on what happens to events when their mode is edited.
func (oldModes modeMap) appendModeMap(newModes modeMap) modeMap {
	for modeName, newMode := range newModes {
		oldModes[modeName] = newMode
	}
	return oldModes
}

// subtractModeMap iterates through an input mode map and deletes each of its modes from the old mode map.
// If one of the input mode map's modes is not found in the old map, it is simply ignored.
func (oldModes modeMap) subtractModeMap(modesToSubtract modeMap) modeMap {
	for modeName := range modesToSubtract {
		delete(oldModes, modeName)
	}
	return oldModes
}

// checkIfNewVars returns if the new configuration of a mode adds a new variable to the mode.
func (oldMode *mode) checkIfNewVars(newMode *mode) bool {
	for _, modeVar := range (*newMode).variables {
		if oldMode.findModeVariable(modeVar.id) == -1 {
			return true
		}
	}
	return false
}

// findModeVariable returns the index of a mode variable, or -1 if the variable is not found.
// NOTE: this brute force search is inefficient and mode setpoints could be refactored to maps instead of slices
func (m *mode) findModeVariable(id string) int {
	for i, modeVar := range (*m).variables {
		if id == modeVar.id {
			return i
		}
	}
	return -1
}

// findModeConstant returns the index of a mode constant, or -1 if the constant is not found.
func (m *mode) findModeConstant(id string) int {
	for i, modeConst := range (*m).constants {
		if id == modeConst.id {
			return i
		}
	}
	return -1
}

// printModesMap prints formatted mode data to the log.
//lint:ignore U1000 only used when dev wants to see it
func (mm modeMap) printModesMap() {
	for modeName, modeObj := range mm {
		log.Println("Mode:", modeName)
		log.Println("Variables:")
		for _, varObj := range modeObj.variables {
			log.Println(" ", varObj.id)
			log.Println("   ", varObj.uri)
			log.Println("   ", varObj.varType)
			switch varObj.varType {
			case "Float":
				log.Println("   ", varObj.value.value_float)
			case "Int":
				log.Println("   ", varObj.value.value_int)
			case "Bool":
				log.Println("   ", varObj.value.value_bool)
			case "String":
				log.Println("   ", varObj.value.value_string)
			}
		}
		log.Println("Constants:")
		for _, varObj := range modeObj.constants {
			log.Println(" ", varObj.id)
			log.Println("   ", varObj.uri)
			log.Println("   ", varObj.varType)
			switch varObj.varType {
			case "Float":
				log.Println("   ", varObj.value.value_float)
			case "Int":
				log.Println("   ", varObj.value.value_int)
			case "Bool":
				log.Println("   ", varObj.value.value_bool)
			case "String":
				log.Println("   ", varObj.value.value_string)
			}
		}
		log.Printf("\n")
	}
}

// editModes takes a mode map as input and modifies the current Scheduler mode map with it.
// If the method is SET, the current mode map gets entirely overwritten.
// If the method is POST, the input mode map is appended to and edits the current mode map.
// If the resulting mode map does not have a default mode, `editModes` will return a non-nil error.
func editModes(inputModes modeMap, editor editingInterface, method editingMethod) error {
	var newModeMap modeMap
	switch method {
	case SET:
		// input mode map replaces the old mode map entirely
		newModeMap = inputModes
	case POST:
		// append the input mode map to a copy of the old mode map
		newModeMap = modes.copy()
		newModeMap.appendModeMap(inputModes)
	case DEL:
		// subtract the input mode map from a copy of the old mode map
		newModeMap = modes.copy()
		newModeMap.subtractModeMap(inputModes)
	default:
		return fmt.Errorf("object editing method %v is invalid", method)
	}

	// default is a required mode
	if !newModeMap.hasMode("default") {
		return fmt.Errorf("required 'default' mode not found")
	}

	// replace old modes map with new one
	overwriteModeMap(newModeMap)

	// only send out updates if this is the only edit that is happening
	// otherwise, the function caller will take care of sending updates
	if editor != CALLER {
		sendUpdatesAfterModeMapEdit(editor)
	}
	return nil
}

// parseModesFromMap parses a modes map out of a map[string]interface{}.
// The input map is expected to have a specific structure (see modes.json in test folder for example).
// If that structure is not followed, a non-nil error will be returned.
func parseModesFromMap(modesMapHolder *map[string]interface{}) (modeMap, error) {
	// allocate memory for new modes map
	receivedModes := make(modeMap)

	// extract new modes map object from parent
	newModesMap, err := fg.ExtractValueWithType(modesMapHolder, "modes", fg.MAP_STRING_INTERFACE)
	if err != nil {
		return nil, fmt.Errorf("failed to parse out mode list: %v", err.Error())
	}

	// iterate through new modes map and parse each individual mode
	for modeName, modeDetailsInterface := range newModesMap.(map[string]interface{}) {
		// verify data type of mode details
		modeDetails, ok := modeDetailsInterface.(map[string]interface{})
		if !ok {
			return nil, fmt.Errorf("failed to parse %v mode, as it is not type map[string]interface{} and is instead type %v", modeName, reflect.TypeOf(modeDetailsInterface))
		}

		// parse mode
		modeInstance, err := parseModeFromMap(modeName, &modeDetails)
		if err != nil {
			return nil, fmt.Errorf("failed to parse %v mode: %v", modeName, err.Error())
		}

		// add mode to new modes map
		receivedModes[modeName] = modeInstance
	}

	return receivedModes, nil
}

// parseModeFromMap helps `parseModesFromMap` by parsing a single mode object's details from its map[string]interface{} representation.
func parseModeFromMap(modeName string, modeDetails *map[string]interface{}) (*mode, error) {
	// instantiate new mode object
	var modeInstance mode

	// parse color code
	modeColorCode, err := fg.ExtractValueWithType(modeDetails, "color_code", fg.STRING)
	if modeName != "default" {
		if err != nil {
			return nil, fmt.Errorf("failed to parse color code of %v mode: %v", modeName, err.Error())
		}
		modeInstance.colorCode = modeColorCode.(string)
	}

	// parse variables
	err = modeInstance.parseModeSetpointList(*modeDetails, "variables")
	if err != nil {
		return nil, fmt.Errorf("failed to parse variables of mode %v: %v", modeName, err.Error())
	}

	// parse constants
	err = modeInstance.parseModeSetpointList(*modeDetails, "constants")
	if err != nil {
		return nil, fmt.Errorf("failed to parse constants of mode %v: %v", modeName, err.Error())
	}

	return &modeInstance, nil
}

// parseModeSetpointList parses a list of setpoints, either variables or constants.
// The string setpointType is expected to be either "variables" or "constants" and denotes which kind of setpoints
// the function caller is attempting to parse.
func (modeInstance *mode) parseModeSetpointList(modeDetails map[string]interface{}, setpointType string) error {
	// extract setpoint list from mode details
	setpointList, err := fg.ExtractValueWithType(&modeDetails, setpointType, fg.INTERFACE_SLICE)
	if err != nil {
		return fmt.Errorf("failed to extract setpoint list from mode details: %v", err.Error())
	}

	// iterate over list of setpoints and parse each individual setpoint
	for _, spObjInterface := range setpointList.([]interface{}) {
		// parse setpoint
		modeSetpoint, err := parseModeSetpoint(spObjInterface)
		if err != nil {
			return fmt.Errorf("failed to parse setpoint: %v", err.Error())
		}

		// append to appropriate list of setpoints
		switch setpointType {
		case "variables":
			modeInstance.variables = append(modeInstance.variables, modeSetpoint)
		case "constants":
			modeInstance.constants = append(modeInstance.constants, modeSetpoint)
		default:
			return fmt.Errorf("%v is unsupported type of setpoint", setpointType)
		}
	}
	return nil
}

// parseModeSetpoint parses an individual setpoint, for a mode, that follows the format of the `setpoint` struct.
func parseModeSetpoint(spDetailsInterface interface{}) (setpoint, error) {
	// instantiate new setpoint
	var modeSetpoint setpoint

	// verify type of setpoint details
	spDetails, ok := spDetailsInterface.(map[string]interface{})
	if !ok {
		return modeSetpoint, fmt.Errorf("type assertion of map[string]interface{} on spDetailsInterface failed. Is type %v", reflect.TypeOf(spDetailsInterface))
	}

	// parse id
	id, err := fg.ExtractValueWithType(&spDetails, "id", fg.STRING)
	if err != nil {
		return modeSetpoint, fmt.Errorf("failed to extract id from setpoint details: %v", err.Error())
	}
	modeSetpoint.id = id.(string)

	// parse name
	name, err := fg.ExtractValueWithType(&spDetails, "name", fg.STRING)
	if err != nil {
		return modeSetpoint, fmt.Errorf("failed to extract name from setpoint details: %v", err.Error())
	}
	modeSetpoint.name = name.(string)

	// parse unit
	unit, err := fg.ExtractValueWithType(&spDetails, "unit", fg.STRING)
	if err != nil {
		return modeSetpoint, fmt.Errorf("failed to extract unit from setpoint details: %v", err.Error())
	}
	modeSetpoint.unit = unit.(string)

	// parse uri
	uri, err := fg.ExtractValueWithType(&spDetails, "uri", fg.STRING)
	if err != nil {
		return modeSetpoint, fmt.Errorf("failed to extract uri from setpoint details: %v", err.Error())
	}
	modeSetpoint.uri = uri.(string)

	// parse type
	varType, err := fg.ExtractValueWithType(&spDetails, "type", fg.STRING)
	if err != nil {
		return modeSetpoint, fmt.Errorf("failed to extract type from setpoint details: %v", err.Error())
	}
	modeSetpoint.varType = varType.(string)

	// parse value
	var varValue valueObj
	switch varType {
	case "Float":
		varFloat, err := fg.ExtractValueWithType(&spDetails, "value", fg.FLOAT64)
		if err != nil {
			return modeSetpoint, fmt.Errorf("failed to extract value from setpoint details: %v", err.Error())
		}
		varValue.value_float = varFloat.(float64)
	case "Int":
		// ints are converted to float64 by golang during marshalling process
		floatBuff, err := fg.ExtractValueWithType(&spDetails, "value", fg.FLOAT64)
		if err != nil {
			return modeSetpoint, fmt.Errorf("failed to extract value from setpoint details: %v", err.Error())
		}
		varValue.value_int = int(floatBuff.(float64))
	case "Bool":
		varBool, err := fg.ExtractValueWithType(&spDetails, "value", fg.BOOL)
		if err != nil {
			return modeSetpoint, fmt.Errorf("failed to extract value from setpoint details: %v", err.Error())
		}
		varValue.value_bool = varBool.(bool)
	case "String":
		varString, err := fg.ExtractValueWithType(&spDetails, "value", fg.STRING)
		if err != nil {
			return modeSetpoint, fmt.Errorf("failed to extract value from setpoint details: %v", err.Error())
		}
		varValue.value_string = varString.(string)
	default:
		return modeSetpoint, fmt.Errorf("failed to extract value from setpoint details: invalid variable type")
	}
	modeSetpoint.value = varValue
	return modeSetpoint, nil
}

// sendUpdatesAfterModeMapEdit takes care of sending the updated mode map to all interested parties.
func sendUpdatesAfterModeMapEdit(editor editingInterface) { // update DBI with the new mode map
	if editor != DBI {
		sendModes("/dbi/scheduler/modes")
	}

	// update Site Controllers if we are Fleet Manager
	if editor != FLEET_SITE {
		if schedulerInstanceType == FLEET_MANAGER {
			updateModesOfSiteControllers()
		}
	}

	// update the SCADA enumerated values for modes
	refreshModeEnums()

	// update UI
	pubModes()

	// editing modes might have changed the schedule so send updates for that as well
	sendUpdatesAfterScheduleEdit(editor)
}

func (smm *safeModeMap) copy() modeMap {
	smm.mu.RLock()
	defer smm.mu.RUnlock()
	return smm.data.copy()
}

func (smm *safeModeMap) getMode(name string) *mode {
	smm.mu.RLock()
	defer smm.mu.RUnlock()
	if m, ok := smm.data[name]; ok {
		return m
	}
	return nil
}

func (smm *safeModeMap) getModeNames() []string {
	smm.mu.RLock()
	defer smm.mu.RUnlock()
	names := make([]string, 0)
	for name := range smm.data {
		names = append(names, name)
	}
	return names
}

func (smm *safeModeMap) buildObj() map[string]interface{} {
	smm.mu.RLock()
	defer smm.mu.RUnlock()
	return smm.data.buildObj()
}
