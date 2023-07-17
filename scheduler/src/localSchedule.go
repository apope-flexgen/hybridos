package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"strings"
	"time"
)

// config object used for configuring the local schedule
type localScheduleConfig struct {
	Id                  string                    `json:"id"`
	Name                string                    `json:"name"`
	ClothedSetpoints    bool                      `json:"clothed_setpoints"`
	SetpointEnforcement setpointEnforcementConfig `json:"setpoint_enforcement"`
}

// Time zone of the box on which Scheduler is running.
var localTimezone *time.Location

// Sets the ID to be the same as the name but with all letters lower-cased and
// replacing slashes and spaces with underscores.
func (cfg *localScheduleConfig) generateId() {
	cfg.Id = strings.ToLower(cfg.Name)
	cfg.Id = strings.ReplaceAll(cfg.Id, " ", "_")
	cfg.Id = strings.ReplaceAll(cfg.Id, "/", "_")
}

// Handles FIMS SETs to URIs beginning with /scheduler/configuration/local_schedule.
func handleLocalScheduleSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	if msg.Nfrags == 3 {
		var newCfg *localScheduleConfig
		if err = json.Unmarshal(jsonBytes, &newCfg); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal input: %w", err)
		}

		if newCfg.Id == "" {
			newCfg.generateId()
		}

		if err = newCfg.validate(schedCfg.SchedulerType); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to validate local schedule settings: %w", err)
		}
		reconfigureLocalSchedule(newCfg)

		sendBackup("/dbi/scheduler/configuration/local_schedule", newCfg)
		sendReply(msg.Replyto, newCfg)
		return nil
	}

	if schedCfg.LocalSchedule == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return errors.New("cannot operate on non-existent local schedule")
	}

	endpoint := msg.Frags[3]
	newCfg := *schedCfg.LocalSchedule
	var replyObject interface{}
	switch endpoint {
	case "setpoint_enforcement":
		if err = handleSetpointEnforcementSet(msg); err != nil {
			return fmt.Errorf("failed to handle setpoint enforcement SET: %w", err)
		}
		return nil
	case "name":
		if err = json.Unmarshal(jsonBytes, &newCfg.Name); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new name: %w", err)
		}
		replyObject = newCfg.Name
	case "id":
		if err = json.Unmarshal(jsonBytes, &newCfg.Id); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new ID: %w", err)
		}
		replyObject = newCfg.Id
	case "clothed_setpoints":
		if err = json.Unmarshal(jsonBytes, &newCfg.ClothedSetpoints); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new clothed setpoints flag: %w", err)
		}
		replyObject = newCfg.ClothedSetpoints
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if err := newCfg.validate(schedCfg.SchedulerType); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to validate new local schedule config settings: %w", err)
	}

	schedCfg.LocalSchedule = &newCfg
	// TODO: evaluate if we really do need to do an entire reconfiguration
	reconfigureScheduler(schedCfg)

	sendBackup(fmt.Sprintf("/dbi/scheduler/configuration/local_schedule/%s", endpoint), replyObject)
	sendReply(msg.Replyto, replyObject)
	return nil
}

func (cfg *localScheduleConfig) validate(schedulerType string) error {
	if cfg == nil {
		if schedulerType == SITE_SCHEDULER {
			return errors.New("local_schedule is nil or not provided")
		}
		// Fleet Scheduler allowed to not have local schedule
		return nil
	}
	if len(cfg.Id) == 0 {
		return errors.New("empty ID string")
	}
	if strings.ContainsAny(cfg.Id, "/ ") {
		return fmt.Errorf("invalid id '%s' - cannot contain spaces or slashes", cfg.Id)
	}
	if len(cfg.Name) == 0 {
		return errors.New("empty name string")
	}
	cfg.SetpointEnforcement.validate()
	return nil
}

// Handles GETs to URIs beginning with /scheduler/configuration/local_schedule.
func (cfg *localScheduleConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	if cfg == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return errors.New("local_schedule is nil")
	}

	switch msg.Frags[3] {
	case "id":
		sendReply(msg.Replyto, cfg.Id)
	case "name":
		sendReply(msg.Replyto, cfg.Name)
	case "clothed_setpoints":
		sendReply(msg.Replyto, cfg.ClothedSetpoints)
	case "setpoint_enforcement":
		if err := cfg.SetpointEnforcement.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle GET to setpoint_enforcement: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Returns true if the local schedule is configured and has the same ID as the given argument.
func (cfg *localScheduleConfig) is(checkId string) bool {
	if cfg == nil {
		return false
	}
	return cfg.Id == checkId
}

// Uses the new settings to create, delete, or modify the local schedule.
func reconfigureLocalSchedule(newCfg *localScheduleConfig) {
	// if a local schedule currently exists, delete it if there is no new local schedule or the new local schedule's ID is different
	if schedCfg.LocalSchedule != nil && (newCfg == nil || newCfg.Id != schedCfg.LocalSchedule.Id) {
		masterSchedule.deleteSchedules(map[string]struct{}{schedCfg.LocalSchedule.Id: {}})
		schedCfg.LocalSchedule = nil
	}

	// make the new local schedule if it did not already exist
	if schedCfg.LocalSchedule == nil && newCfg != nil {
		createScheduleIfNew(newCfg.Id, newCfg.Name)
	}

	// update recorded localSchedule configuration
	schedCfg.LocalSchedule = newCfg
	// update setpoint enforcement feature with new configuration settings
	reconfigureSetpointEnforcement(newCfg.SetpointEnforcement)
	// send out schedule update
	sendUpdatesAfterMasterScheduleEdit(true)
}
