package main

import (
	"encoding/json"
	"fims"
	"fmt"
	"time"
)

// Determines if / how often Scheduler sends GETs to setpoints during active events
// and resends SETs if the GET reply does not match active setpoint's value.
type setpointEnforcementConfig struct {
	Enabled          bool `json:"enabled"`           // turns this feature on/off
	FrequencySeconds uint `json:"frequency_seconds"` // how often, in seconds, the GETs are to be sent out
}

// Only a var because Go does not support constant structs.
// Should be treated as a const.
var defaultSetpointEnforcementConfig = setpointEnforcementConfig{
	Enabled:          false,
	FrequencySeconds: defaultSetpointEnforcementFrequencySeconds,
}

const defaultSetpointEnforcementFrequencySeconds = 360000

// setpointEnforcementTicker keeps track of how often the active event should have its status values verified, if there is an active event.
// If there is no local schedule or the setpoint enforcement feature is turned off, this ticker will be set to have a very slow frequency
// in order to reduce overhead.
var setpointEnforcementTicker *time.Ticker = time.NewTicker(time.Second * time.Duration(defaultSetpointEnforcementFrequencySeconds))

// Reconfigures the setpoint-enforcement feature with the given settings.
func reconfigureSetpointEnforcement(cfg setpointEnforcementConfig) {
	schedCfg.LocalSchedule.SetpointEnforcement = cfg
	setpointEnforcementFreq := time.Duration(time.Second) * time.Duration(cfg.FrequencySeconds)
	setpointEnforcementTicker = time.NewTicker(setpointEnforcementFreq)
}

// Handles FIMS SETs to URIs beginning with /scheduler/configuration/local_schedule/setpoint_enforcement.
func handleSetpointEnforcementSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	if msg.Nfrags == 4 {
		var newCfg setpointEnforcementConfig
		if err = json.Unmarshal(jsonBytes, &newCfg); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal setpoint enforcement settings: %w", err)
		}
		newCfg.validate()

		sendBackup("/dbi/scheduler/configuration/local_schedule/setpoint_enforcement", newCfg)
		sendReply(msg.Replyto, newCfg)
		reconfigureSetpointEnforcement(newCfg)
		return nil
	}

	endpoint := msg.Frags[4]
	newCfg := schedCfg.LocalSchedule.SetpointEnforcement
	var replyObject interface{}
	switch endpoint {
	case "enabled":
		if err = json.Unmarshal(jsonBytes, &newCfg.Enabled); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal enabled flag: %w", err)
		}
		replyObject = newCfg.Enabled
	case "frequency_seconds":
		if err = json.Unmarshal(jsonBytes, &newCfg.FrequencySeconds); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal frequency seconds: %w", err)
		}
		replyObject = newCfg.FrequencySeconds
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	newCfg.validate()
	sendBackup(fmt.Sprintf("/dbi/scheduler/configuration/local_schedule/setpoint_enforcement/%s", endpoint), replyObject)
	schedCfg.LocalSchedule.SetpointEnforcement = newCfg
	reconfigureScheduler(schedCfg)
	sendReply(msg.Replyto, replyObject)
	return nil
}

// Since setpoint_enforcement is optional and will be zero-valued if not included, default to
// low frequency to keep routine in main func from occurring too often.
func (cfg *setpointEnforcementConfig) validate() {
	if cfg.FrequencySeconds == 0 {
		cfg.FrequencySeconds = defaultSetpointEnforcementFrequencySeconds
	}
}

func (cfg *setpointEnforcementConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	switch msg.Frags[4] {
	case "enabled":
		sendReply(msg.Replyto, cfg.Enabled)
	case "frequency_seconds":
		sendReply(msg.Replyto, cfg.FrequencySeconds)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}
