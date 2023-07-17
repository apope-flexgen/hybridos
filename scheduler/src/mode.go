package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"strings"

	"github.com/flexgen-power/scheduler/internal/setpoint"
)

// Contains a group of setpoints that a scheduled event should execute.
type mode struct {
	Variables setpoint.List `json:"variables"`  // setpoints for which every event must provide a value
	Constants setpoint.List `json:"constants"`  // setpoints that share the same value for every event using the mode
	ColorCode string        `json:"color_code"` // stored for the UI
	Icon      string        `json:"icon"`       // stored for the UI
	Name      string        `json:"name"`       // stored for the UI
}

// Creates an ID for the mode using its name.
func (m *mode) generateId() (id string) {
	id = strings.ToLower(m.Name)
	id = strings.ReplaceAll(id, " ", "_")
	id = strings.ReplaceAll(id, "/", "_")
	return id
}

// Handles GETs to URIs beginning with /scheduler/modes/<mode ID>.
func (m *mode) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, m)
		return nil
	}

	switch msg.Frags[3] {
	case "name":
		sendReply(msg.Replyto, m.Name)
	case "color_code":
		sendReply(msg.Replyto, m.ColorCode)
	case "icon":
		sendReply(msg.Replyto, m.Icon)
	case "variables":
		if err := m.handleSetpointsGet(msg, m.Variables); err != nil {
			return fmt.Errorf("failed to handle GET to variables: %w", err)
		}
	case "constants":
		if err := m.handleSetpointsGet(msg, m.Constants); err != nil {
			return fmt.Errorf("failed to handle GET to constants: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles GETs to URIs beginning with /scheduler/modes/<mode ID>... /constants or /variables.
func (m *mode) handleSetpointsGet(msg fims.FimsMsg, setpoints setpoint.List) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// GET is to entire list
	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, setpoints)
		return nil
	}

	targetSetpointId := msg.Frags[4]
	targetSetpointIndex := setpoints.Find(targetSetpointId)
	if targetSetpointIndex < 0 {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("no mode with ID %s found", targetSetpointId)
	}
	targetSetpoint := setpoints[targetSetpointIndex]

	// GET is to a whole setpoint
	if msg.Nfrags == 5 {
		sendReply(msg.Replyto, targetSetpoint)
		return nil
	}

	// GET is to specific field of setpoint
	endpoint := msg.Frags[5]
	switch endpoint {
	case "name":
		sendReply(msg.Replyto, targetSetpoint.Name)
	case "type":
		sendReply(msg.Replyto, targetSetpoint.VarType)
	case "unit":
		sendReply(msg.Replyto, targetSetpoint.Unit)
	case "uri":
		sendReply(msg.Replyto, targetSetpoint.Uri)
	case "value":
		sendReply(msg.Replyto, targetSetpoint.Value)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles DELs to URIs beginning with /scheduler/modes/<mode ID>/...
func (m *mode) handleDel(msg fims.FimsMsg, modeId string) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	endpoint := msg.Frags[3]
	var replyObj interface{}
	switch endpoint {
	case "variables":
		var newVariablesList setpoint.List
		if msg.Nfrags == 4 {
			newVariablesList = make(setpoint.List, 0)
		} else {
			idOfVarToDelete := msg.Frags[4]
			newVariablesList = m.Variables.Copy()
			newVariablesList.Delete(idOfVarToDelete)
		}
		m.overwriteVariablesList(newVariablesList, modeId)
		replyObj = m.Variables
	case "constants":
		var newConstantsList setpoint.List
		if msg.Nfrags == 4 {
			newConstantsList = make(setpoint.List, 0)
		} else {
			idOfVarToDelete := msg.Frags[4]
			newConstantsList = m.Constants.Copy()
			newConstantsList.Delete(idOfVarToDelete)
		}
		m.Constants = newConstantsList // not as many side-effects for overwriting constants so can just directly overwrite the variable
		replyObj = m.Constants
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	sendReply(msg.Replyto, replyObj)
	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/%s", modeId, endpoint), replyObj)
	sendUpdatesAfterModeMapEdit(true)
	return nil
}

// Handles POSTs to URIs beginning with /scheduler/modes/<mode ID>/....
func (m *mode) handlePost(msg fims.FimsMsg, modeId string) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var newSetpoint setpoint.Setpoint
	if err = json.Unmarshal(jsonBytes, &newSetpoint); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to unmarshal new setpoint: %w", err)
	}
	newSetpoint.GenerateId()

	if err = newSetpoint.Validate(); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to validate new setpoint: %w", err)
	}

	endpoint := msg.Frags[3]
	var replyObj interface{}
	switch endpoint {
	case "variables":
		newVariablesList := m.Variables.Copy()
		if err = newVariablesList.Append(newSetpoint); err != nil {
			sendErrorResponse(msg.Replyto, "Setpoint Already Exists")
			return fmt.Errorf("failed to append new variable to variables list: %w", err)
		}
		m.overwriteVariablesList(newVariablesList, modeId)
		replyObj = m.Variables
	case "constants":
		newConstantsList := m.Constants.Copy()
		if err = newConstantsList.Append(newSetpoint); err != nil {
			sendErrorResponse(msg.Replyto, "Setpoint Already Exists")
			return fmt.Errorf("failed to append new constant to constants list: %w", err)
		}
		m.Constants = newConstantsList // not as many side-effects for overwriting constants so can just directly overwrite the variable
		replyObj = m.Constants
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	sendReply(msg.Replyto, replyObj)
	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/%s", modeId, endpoint), replyObj)
	sendUpdatesAfterModeMapEdit(true)
	return nil
}

// Handles SETs to URIs beginning with /scheduler/modes/<mode ID>.
func (m *mode) handleSet(msg fims.FimsMsg, modeId string) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	if msg.Nfrags == 3 {
		var newModeSettings mode
		if err = json.Unmarshal(jsonBytes, &newModeSettings); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal input: %w", err)
		}

		if err = newModeSettings.validate(); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to validate new settings: %w", err)
		}

		m.overwrite(newModeSettings, modeId)
		sendReply(msg.Replyto, m)
		sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s", modeId), m)
		sendUpdatesAfterModeMapEdit(true)
		return nil
	}

	endpoint := msg.Frags[3]
	var replyObj interface{}
	switch msg.Frags[3] {
	case "name":
		if err = json.Unmarshal(jsonBytes, &m.Name); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal name: %w", err)
		}
		replyObj = m.Name
	case "color_code":
		if err = json.Unmarshal(jsonBytes, &m.ColorCode); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal color_code: %w", err)
		}
		replyObj = m.ColorCode
	case "icon":
		if err = json.Unmarshal(jsonBytes, &m.Icon); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal icon: %w", err)
		}
		replyObj = m.Icon
	case "constants":
		if err = m.handleConstantsSet(msg.Frags, jsonBytes, msg.Replyto, modeId); err != nil {
			return fmt.Errorf("failed to handle constants SET: %w", err)
		}
		return nil
	case "variables":
		if err = m.handleVariablesSet(msg.Frags, jsonBytes, msg.Replyto, modeId); err != nil {
			return fmt.Errorf("failed to handle variables SET: %w", err)
		}
		return nil
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	sendReply(msg.Replyto, replyObj)
	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/%s", modeId, endpoint), replyObj)
	sendUpdatesAfterModeMapEdit(true)
	return nil
}

// Handles SETs to URIs beginning with /scheduler/modes/<mode ID>/constants.
func (m *mode) handleConstantsSet(uriFrags []string, body []byte, replyTo string, modeId string) error {
	if len(uriFrags) < 4 {
		sendErrorResponse(replyTo, "Invalid URI")
		return ErrInvalidUri
	}

	// SET is to entire constants list
	if len(uriFrags) == 4 {
		var newConstants setpoint.List
		if err := json.Unmarshal(body, &newConstants); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal constants array: %w", err)
		}

		if err := newConstants.Validate(); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to validate constants array: %w", err)
		}
		m.Constants = newConstants

		sendReply(replyTo, m.Constants)
		sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/constants", modeId), m.Constants)
		return nil
	}

	targetConstantId := uriFrags[4]
	targetConstantIndex := m.Constants.Find(targetConstantId)
	if targetConstantIndex < 0 {
		sendErrorResponse(replyTo, "Resource Not Found")
		return fmt.Errorf("did not find constant with ID %s", targetConstantId)
	}

	// SET is to whole constant setpoint
	if len(uriFrags) == 5 {
		var newConstant setpoint.Setpoint
		if err := json.Unmarshal(body, &newConstant); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new constant settings: %w", err)
		}
		// ID must be preserved at this SET level
		newConstant.Id = m.Constants[targetConstantIndex].Id

		if err := newConstant.Validate(); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to validate new constant settings: %w", err)
		}

		m.Constants[targetConstantIndex] = newConstant
		sendReply(replyTo, newConstant)
		sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/constants", modeId), m.Constants)
		return nil
	}

	newSettings := m.Constants[targetConstantIndex]

	endpoint := uriFrags[5]
	var replyObj interface{}
	switch endpoint { // varType field does not have individual SET endpoint because changing varType without changing value is not a valid operation
	case "name":
		if err := json.Unmarshal(body, &newSettings.Name); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal name: %w", err)
		}
		replyObj = newSettings.Name
	case "unit":
		if err := json.Unmarshal(body, &newSettings.Unit); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal unit: %w", err)
		}
		replyObj = newSettings.Unit
	case "uri":
		if err := json.Unmarshal(body, &newSettings.Uri); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal uri: %w", err)
		}
		replyObj = newSettings.Uri
	case "value":
		if err := json.Unmarshal(body, &newSettings.Value); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal value: %w", err)
		}
		replyObj = newSettings.Value
	default:
		sendErrorResponse(replyTo, "Invalid URI")
		return ErrInvalidUri
	}

	if err := newSettings.Validate(); err != nil {
		sendErrorResponse(replyTo, "Invalid Data")
		return fmt.Errorf("failed to validate new %s: %w", endpoint, err)
	}

	m.Constants[targetConstantIndex] = newSettings

	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/constants", modeId), m.Constants)
	sendReply(replyTo, replyObj)
	return nil
}

// Handles SETs to URIs beginning with /scheduler/modes/<mode ID>/variables.
func (m *mode) handleVariablesSet(uriFrags []string, body []byte, replyTo string, modeId string) error {
	if len(uriFrags) < 4 {
		sendErrorResponse(replyTo, "Invalid URI")
		return ErrInvalidUri
	}

	if len(uriFrags) == 4 {
		var newVariables setpoint.List
		if err := json.Unmarshal(body, &newVariables); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal variables array: %w", err)
		}

		if err := newVariables.Validate(); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to validate variables array: %w", err)
		}

		m.overwriteVariablesList(newVariables, modeId)

		sendReply(replyTo, m.Variables)
		sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/variables", modeId), m.Variables)
		return nil
	}

	targetVariableId := uriFrags[4]
	targetVariableIndex := m.Variables.Find(targetVariableId)
	if targetVariableIndex < 0 {
		sendErrorResponse(replyTo, "Resource Not Found")
		return fmt.Errorf("did not find variable with ID %s", targetVariableId)
	}

	// SET is to whole variable setpoint
	if len(uriFrags) == 5 {
		var newVariable setpoint.Setpoint
		if err := json.Unmarshal(body, &newVariable); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new variable settings: %w", err)
		}
		// ID must be preserved at this SET level
		newVariable.Id = m.Variables[targetVariableIndex].Id

		if err := newVariable.Validate(); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to validate new variable settings: %w", err)
		}

		// overwrite using copy of variables list so overwriteVariablesList function can be reused
		newVariablesList := m.Variables.Copy()
		newVariablesList[targetVariableIndex] = newVariable
		m.overwriteVariablesList(newVariablesList, modeId)

		sendReply(replyTo, newVariable)
		sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/variables", modeId), m.Variables)
		return nil
	}

	newVariable := m.Variables[targetVariableIndex]

	endpoint := uriFrags[5]
	var replyObj interface{}
	switch endpoint { // varType field does not have individual SET endpoint because changing varType without changing value is not a valid operation
	case "name":
		if err := json.Unmarshal(body, &newVariable.Name); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal name: %w", err)
		}
		replyObj = newVariable.Name
	case "unit":
		if err := json.Unmarshal(body, &newVariable.Unit); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal unit: %w", err)
		}
		replyObj = newVariable.Unit
	case "uri":
		if err := json.Unmarshal(body, &newVariable.Uri); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal uri: %w", err)
		}
		replyObj = newVariable.Uri
	case "value":
		if err := json.Unmarshal(body, &newVariable.Value); err != nil {
			sendErrorResponse(replyTo, "Invalid Data")
			return fmt.Errorf("failed to unmarshal value: %w", err)
		}
		replyObj = newVariable.Value
	default:
		sendErrorResponse(replyTo, "Invalid URI")
		return ErrInvalidUri
	}

	if err := newVariable.Validate(); err != nil {
		sendErrorResponse(replyTo, "Invalid Data")
		return fmt.Errorf("failed to validate new %s: %w", endpoint, err)
	}

	// overwrite using copy of variables list so overwriteVariablesList function can be reused
	newVariablesList := m.Variables.Copy()
	newVariablesList[targetVariableIndex] = newVariable
	m.overwriteVariablesList(newVariablesList, modeId)

	sendBackup(fmt.Sprintf("/dbi/scheduler/modes/%s/variables", modeId), m.Variables)
	sendReply(replyTo, replyObj)
	return nil
}

// Overwrites the settings of the receiver mode with the settings of the mode passed-in as argument.
// Also handles applying these updates to existing events.
func (m *mode) overwrite(newSettings mode, modeId string) {
	needToResendActiveEvents := updateEventsBasedOnVariableListEdit(m.Variables, newSettings.Variables, modeId)
	*m = newSettings
	// need to resend active events AFTER overwriting mode settings so that
	// re-execution of active event uses correct mode settings
	if needToResendActiveEvents {
		resendActiveEventsWithMode(modeId)
	}
}

// Overwrites the variables list of the mode with the passed-in variables list.
// Also handles applying these updates to existing events.
func (m *mode) overwriteVariablesList(newVariables setpoint.List, modeId string) {
	needToResendActiveEvents := updateEventsBasedOnVariableListEdit(m.Variables, newVariables, modeId)
	m.Variables = newVariables
	// need to resend active events AFTER overwriting mode settings so that
	// re-execution of active event uses correct mode settings
	if needToResendActiveEvents {
		resendActiveEventsWithMode(modeId)
	}
}

// When editing a mode's variables, this function will apply those settings changes to existing events.
// Returns a boolean indicating if the changes mean active events with the edited mode should be
// re-executed. The calling function should make sure to re-execute the active events AFTER overwriting
// the active mode object with the new variables.
func updateEventsBasedOnVariableListEdit(oldVariables, newVariables setpoint.List, modeId string) (needToResendActiveEvents bool) {
	// if mode variables were added, delete events
	if oldVariables.IsMissingSetpointsThatAreIn(newVariables) {
		masterSchedule.deleteEventsWithMode(modeId)
		return false
	}

	for _, oldVar := range oldVariables {
		// if mode variables were deleted, do not delete events
		// but delete the variable from event variable maps
		newVarIndex := newVariables.Find(oldVar.Id)
		if newVarIndex == -1 {
			masterSchedule.deleteVarOfMode(oldVar.Id, modeId)
			continue
		}
		newVar := newVariables[newVarIndex]

		// if mode variable types were changed, delete events
		if oldVar.VarType != newVar.VarType {
			masterSchedule.deleteEventsWithMode(modeId)
			break
		}

		// if URI of a mode variable is changed, re-execute any active events with this mode
		if oldVar.Uri != newVar.Uri {
			needToResendActiveEvents = true
		}
	}

	return needToResendActiveEvents
}

// Iterates through a mode's list of constants and sends them all as FIMS SETs to their respective URIs.
func (m *mode) sendConstants() {
	for _, c := range m.Constants {
		c.SendSet(f, c.Value, schedCfg.LocalSchedule.ClothedSetpoints)
	}
}

// Sends a GET to every variable's/constant's URI.
func (m *mode) sendGets() {
	for _, c := range m.Constants {
		f.SendGet(c.Uri, fmt.Sprintf("/scheduler/active_event_status/%s", c.Id))
	}
	for _, v := range m.Variables {
		f.SendGet(v.Uri, fmt.Sprintf("/scheduler/active_event_status/%s", v.Id))
	}
}

// Ensures all fields are filled in and validates all setpoints.
func (m *mode) validate() error {
	if m == nil {
		return errors.New("pointer is nil")
	}
	if m.Name == "" {
		return errors.New("name string is empty")
	}
	if m.ColorCode == "" {
		return errors.New("color_code string is empty")
	}
	if m.Icon == "" {
		return errors.New("icon string is empty")
	}
	if err := m.Variables.Validate(); err != nil {
		return fmt.Errorf("failed to validate variables: %w", err)
	}
	if err := m.Constants.Validate(); err != nil {
		return fmt.Errorf("failed to validate constants: %w", err)
	}
	return nil
}
