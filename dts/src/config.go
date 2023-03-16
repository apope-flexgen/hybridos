package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"strings"

	"github.com/flexgen-power/go_flexgen/cfgfetch"
	log "github.com/flexgen-power/go_flexgen/logger"
)

// structure to store config file
type Config struct {
	Retention                      string  `json:"retention"`                          // Retention period to set while creating database
	InputPath                      string  `json:"input_path"`                         // Path at which dts looks for archives
	FailedValidatePath             string  `json:"failed_validate_path"`               // Path to which dts saves failed archives
	FailedWritePath                string  `json:"failed_write_path"`                  // Path to which archives that failed a write are moved
	FwdPath                        string  `json:"forward"`                            // Path to which dts saves successful archives
	NumValidateWorkers             int     `json:"num_validate_workers"`               // Number of validation workers running in parallel
	NumInfluxPrepareBatchesWorkers int     `json:"num_influx_prepare_batches_workers"` // Number of workers preparing batch points in parallel in the influx writer
	NumInfluxSendBatchesWorkers    int     `json:"num_influx_send_batches_workers"`    // Number of workers sending batches in parallel to influx
	DbHealthCheckDelayS            float64 `json:"db_health_check_delay_seconds"`      // Delay in seconds between db client health checks when trying to write
	Extension                      string  `json:"ext"`                                // Extension of archives to look for
	InfluxAddr                     string  `json:"influx_address"`                     // External IP:Port for influx -- optional (defaults to localhost:8086)
	MongoAddr                      string  `json:"mongo_address"`                      // External IP:Port for mongo -- optional (defaults to localhost:27017)
}

var (
	config Config // global config struct parsed from config file if passed
)

// Parses configuration from the source provided: either a filepath or "dbi"
func parseConfig(cfgSource string) error {
	if len(cfgSource) == 0 || strings.EqualFold(cfgSource, "dbi") {
		log.MsgInfo("Config source set to read from dbi")
		// Query DBI for configuration
		configBody, err := cfgfetch.Retrieve("dts", "dbi")
		if err != nil {
			return fmt.Errorf("failed to get config from dbi: %w", err)
		}
		configBytes, err := json.Marshal(configBody)
		if err != nil {
			return fmt.Errorf("failed to marshal dbi response into json bytes: %w", err)
		}
		err = json.Unmarshal(configBytes, &config)
		if err != nil {
			return fmt.Errorf("could not unmarshal json bytes to config struct: %w", err)
		}
	} else {
		log.MsgInfo("Config source set to read from a file")
		// Read config from files
		configBytes, err := ioutil.ReadFile(cfgSource)
		if err != nil {
			return fmt.Errorf("could not read config file: %w", err)
		}
		err = json.Unmarshal(configBytes, &config)
		if err != nil {
			return fmt.Errorf("could not unmarshal file to config struct: %w", err)
		}
	}

	return validateConfig()
}

func validateConfig() error {
	log.Infof("configured with retention policy: %s", config.Retention)
	if len(config.InputPath) == 0 {
		return fmt.Errorf("input path not specified")
	}
	if len(config.FailedValidatePath) == 0 {
		return fmt.Errorf("validate err path not specified")
	}
	if len(config.FailedWritePath) == 0 {
		return fmt.Errorf("write err path not specified")
	}
	if len(config.Extension) == 0 {
		return fmt.Errorf("extension not specified")
	}
	if config.NumValidateWorkers <= 0 {
		return fmt.Errorf("number of validator workers configured is %d, but at least 1 is required to function", config.NumValidateWorkers)
	}
	if config.DbHealthCheckDelayS < 0 {
		return fmt.Errorf("configured write delay is %f seconds, but the write delay must be non-negative", config.DbHealthCheckDelayS)
	}
	return nil
}
