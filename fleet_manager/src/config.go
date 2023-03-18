package main

import (
	"encoding/json"
	"fims"
	"fmt"

	"github.com/flexgen-power/go_flexgen/cfgFetch"
)

type config struct {
	DataProcessPeriodMs uint `json:"data_process_period_ms"`
	DataPublishPeriodMs uint `json:"data_publish_period_ms"`
}

var masterCfg config

// Wrapper around the cfgFetch package to either load configuration from DBI
// or read it from a JSON file.
func retrieveConfig(source string) (config, error) {
	decode := func(input interface{}) (interface{}, error) {
		return parseConfig(input)
	}
	cfgInterface, err := cfgFetch.RetrieveWithCustomDecode("fleet_manager", source, decode)
	if err != nil {
		return config{}, fmt.Errorf("failed to retrieve config: %w", err)
	}

	cfg, ok := cfgInterface.(config)
	if !ok {
		return config{}, fmt.Errorf("config retrieval expected to return config struct, but got %T", cfgInterface)
	}

	return cfg, nil
}

// Parses the given input (expects a map[string]interface{}) into a config struct
// and assigns default values to any missing fields.
func parseConfig(cfgInterface interface{}) (cfg config, err error) {
	cfgMap, ok := cfgInterface.(map[string]interface{})
	if !ok {
		return config{}, fmt.Errorf("expected map[string]interface{} but got %T", cfgInterface)
	}

	// optional wrapping in "configuration" key must be allowed since configuration may have come from DBI
	if wrappedCfgInterface, ok := cfgMap["configuration"]; ok {
		cfgMap, ok = wrappedCfgInterface.(map[string]interface{})
		if !ok {
			return config{}, fmt.Errorf("expected wrapped config to be map[string]interface{} but got %T", wrappedCfgInterface)
		}
	}

	cfgJson, err := json.Marshal(cfgMap)
	if err != nil {
		return config{}, fmt.Errorf("failed to marshal config map: %w. config map: %+v", err, cfgMap)
	}

	err = json.Unmarshal(cfgJson, &cfg)
	if err != nil {
		return config{}, fmt.Errorf("failed to unmarshal config JSON: %w", err)
	}

	// set default values for any missing config fields
	if cfg.DataProcessPeriodMs == 0 {
		cfg.DataProcessPeriodMs = 100
	}
	if cfg.DataPublishPeriodMs == 0 {
		cfg.DataPublishPeriodMs = 500
	}

	return cfg, nil
}

// Endpoint for any GETs to URIs beginning with /fleet/configuration.
func handleConfigurationGet(msg fims.FimsMsg) error {
	// /fleet/configuration
	if msg.Nfrags < 3 {
		err := f.SendSet(msg.Replyto, "", masterCfg)
		if err != nil {
			return fmt.Errorf("failed to reply to GET for configuration: %w", err)
		}
		return nil
	}

	switch msg.Frags[2] {
	case "data_process_period_ms":
		err := f.SendSet(msg.Replyto, "", masterCfg.DataProcessPeriodMs)
		if err != nil {
			return fmt.Errorf("failed to reply to GET for data process period: %w", err)
		}
	case "data_publish_period_ms":
		err := f.SendSet(msg.Replyto, "", masterCfg.DataPublishPeriodMs)
		if err != nil {
			return fmt.Errorf("failed to reply to GET for data publish period: %w", err)
		}
	default:
		return fmt.Errorf("%s is not a valid GET endpoint", msg.Uri)
	}
	return nil
}
