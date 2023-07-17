package main

import (
	"encoding/json"
	"fims"
	"fmt"

	"github.com/flexgen-power/scheduler/internal/websockets"
)

// config object used for reading in, validating, and setting new configuration settings for Scheduler
type schedulerConfig struct {
	SchedulerType string               `json:"scheduler_type"`
	LocalSchedule *localScheduleConfig `json:"local_schedule,omitempty"`
	WebSockets    webSocketsConfig     `json:"web_sockets"`
	Scada         scadaConfiguration   `json:"scada"`
}

// object to hold configuration settings
var schedCfg schedulerConfig

// These constants are used by the scheduler_type configuration field to indicate in what mode Scheduler is running.
const (
	SITE_SCHEDULER  = "SC"
	FLEET_SCHEDULER = "FM"
)

// Updates configuration settings in the middle of a run, then updates DBI with the new schedule,
// in case it was altered by the reconfiguring.
func reconfigureScheduler(cfg schedulerConfig) {
	schedCfg = cfg
	websockets.CloseExistingConnections()
	// add any new schedules and delete any existing schedules that were not included in new config
	scheduleIds := make([]string, 0)
	if cfg.WebSockets.ClientConfigs != nil {
		scheduleIds = append(scheduleIds, cfg.WebSockets.ClientConfigs.getIds()...)
	}
	if cfg.LocalSchedule != nil {
		createScheduleIfNew(cfg.LocalSchedule.Id, cfg.LocalSchedule.Name)
		scheduleIds = append(scheduleIds, cfg.LocalSchedule.Id)
	}
	masterSchedule.deleteAllSchedulesExcept(scheduleIds)
	// reconfigure Scheduler features with new configuration settings
	if cfg.LocalSchedule != nil {
		reconfigureSetpointEnforcement(cfg.LocalSchedule.SetpointEnforcement)
	}
	reconfigureWebSockets(cfg.WebSockets)
	reconfigureScadaInterface(cfg.Scada)
	// send update to DBI
	broadcastState(buildState())
	sendUpdatesAfterMasterScheduleEdit(true)
}

// Handles all POSTs to URIs starting with /scheduler/configuration.
func (cfg *schedulerConfig) handlePost(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	switch msg.Frags[2] {
	case "web_sockets":
		if err := cfg.WebSockets.handlePost(msg); err != nil {
			return fmt.Errorf("failed to handle POST to WebSockets: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles all DELs to URIs starting with /scheduler/configuration.
func (cfg *schedulerConfig) handleDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	switch msg.Frags[2] {
	case "web_sockets":
		if err := cfg.WebSockets.handleDel(msg); err != nil {
			return fmt.Errorf("failed to handle DEL to WebSockets: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles all GETs to URIs starting with /scheduler/configuration.
func (cfg *schedulerConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 2 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	switch msg.Frags[2] {
	case "local_schedule":
		if err := cfg.LocalSchedule.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle local schedule configuration GET: %w", err)
		}
	case "web_sockets":
		if err := cfg.WebSockets.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSockets GET: %w", err)
		}
	case "scada":
		if err := cfg.Scada.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle SCADA configuration GET: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// parses a map of all configuration settings and reconfigures Scheduler with the given settings
func handleFullConfigurationSet(msg fims.FimsMsg) error {
	// parse and validate received configuration settings
	cfg, err := parseSchedulerConfig(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to parse configuration object: %w", err)
	}
	// use validated configuration settings to reconfigure Scheduler
	reconfigureScheduler(cfg)

	sendReply(msg.Replyto, schedCfg)
	return nil
}

// parses a JSON representation of a schedulerConfig into a struct
func parseSchedulerConfig(inputJsonInterface interface{}) (cfg schedulerConfig, err error) {
	jsonBytes, err := json.Marshal(inputJsonInterface)
	if err != nil {
		return cfg, fmt.Errorf("failed to marshal input: %w", err)
	}

	if err = json.Unmarshal(jsonBytes, &cfg); err != nil {
		return cfg, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = cfg.validate(); err != nil {
		return cfg, fmt.Errorf("invalid scheduler config: %w", err)
	}

	return cfg, nil
}

func (cfg *schedulerConfig) validate() error {
	if cfg.SchedulerType != SITE_SCHEDULER && cfg.SchedulerType != FLEET_SCHEDULER {
		return fmt.Errorf("received value of %s for scheduler_type, but only valid options are '%s' and '%s'", cfg.SchedulerType, SITE_SCHEDULER, FLEET_SCHEDULER)
	}

	if err := cfg.LocalSchedule.validate(cfg.SchedulerType); err != nil {
		return fmt.Errorf("invalid local_schedule config: %w", err)
	}

	if err := cfg.WebSockets.validate(cfg.SchedulerType); err != nil {
		return fmt.Errorf("invalid web_sockets config: %w", err)
	}

	if cfg.LocalSchedule != nil && cfg.WebSockets.ClientConfigs != nil {
		for _, client := range *cfg.WebSockets.ClientConfigs {
			if client.Id == cfg.LocalSchedule.Id {
				return fmt.Errorf("local schedule ID %s also exists in WebSocket clients", client.Id)
			}
		}
	}

	// zero value of scada config is a valid config so no need to validate

	return nil
}

// Returns a list of all IDs encompassed by the configuration (local schedule + all WebSocket clients).
func (cfg *schedulerConfig) getListOfScheduleIds() []string {
	ids := make([]string, 0)
	if cfg.LocalSchedule != nil {
		ids = append(ids, cfg.LocalSchedule.Id)
	}
	if cfg.WebSockets.ClientConfigs == nil {
		return ids
	}
	for _, client := range *cfg.WebSockets.ClientConfigs {
		ids = append(ids, client.Id)
	}
	return ids
}
