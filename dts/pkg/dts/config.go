package dts

import (
	"encoding/json"
	"fmt"
	"os"
	"strings"

	"github.com/flexgen-power/hybridos/go_flexgen/cfgfetch"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

// structure to store config file
type Config struct {
	InputPath                      string                  `json:"input_path"`                         // Path at which dts looks for archives
	NumExtractWorkers              int                     `json:"num_extract_workers"`                // Number of workers extracting data files from archives running in parallel
	NumValidateWorkers             int                     `json:"num_validate_workers"`               // Number of validation workers running in parallel
	NumInfluxPrepareBatchesWorkers int                     `json:"num_influx_prepare_batches_workers"` // Number of workers preparing batch points in parallel in the influx writer
	NumInfluxSendBatchesWorkers    int                     `json:"num_influx_send_batches_workers"`    // Number of workers sending batches in parallel to influx
	DbHealthCheckDelayS            float64                 `json:"db_health_check_delay_seconds"`      // Delay in seconds between db client health checks when trying to write
	Extensions                     []string                `json:"ext"`                                // Extensions of archives to look for
	InfluxAddr                     string                  `json:"influx_address"`                     // External IP:Port for influx -- optional (defaults to localhost:8086)
	MongoAddr                      string                  `json:"mongo_address"`                      // External IP:Port for mongo -- optional (defaults to localhost:27017)
	RetryConnectPeriodSeconds      float64                 `json:"retry_connect_period_seconds"`       // Period with which we retry connecting to databases
	RetentionPolicies              []RetentionPolicyConfig `json:"retention_policies"`                 // Retention policies to create for Influx databases
	ContinuousQueries              []ContinuousQueryConfig `json:"continuous_queries"`                 // Continuous queries to create for Influx databases
}

// config for Influx retention policies
type RetentionPolicyConfig struct {
	Name     string `json:"name"`     // Name of the RP
	Duration string `json:"duration"` // Influx duration string for the RP duration
	Default  bool   `json:"default"`  // True if this RP is the default RP
}

// config for Influx continuous queries
type ContinuousQueryConfig struct {
	Name     string `json:"name"`     // Name of CQ
	GroupBy  string `json:"group_by"` // String passed to GROUP BY phrase of CQ, defines grouping used by aggregations
	Resample string `json:"resample"` // String passed to RESAMPLE phrase of CQ if it should be included, defines how to do recalculations used to account for batching period
	RP       string `json:"rp"`       // Name of RP to store the CQ into
	Select   string `json:"select"`   // String passed to SELECT phrase of CQ, defines query which becomes the CQ's result

	// Fields with default values that usually shouldn't be overwritten
	Measurement string `json:"measurement"` // Destination measurement in which the CQ results are stored
	From        string `json:"from"`        // Measurement on which the CQ is applied
	Into        string `json:"into"`        // Destination DB in which the CQ results are stored
}

var (
	GlobalConfig Config // global config struct parsed from config file if passed
)

// Parses configuration from the source provided: either a filepath or "dbi"
func ParseConfig(cfgSource string) error {
	// extract configuration based on source
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
		err = json.Unmarshal(configBytes, &GlobalConfig)
		if err != nil {
			return fmt.Errorf("could not unmarshal json bytes to config struct: %w", err)
		}
	} else {
		log.MsgInfo("Config source set to read from a file")
		// Read config from files
		configBytes, err := os.ReadFile(cfgSource)
		if err != nil {
			return fmt.Errorf("could not read config file: %w", err)
		}
		err = json.Unmarshal(configBytes, &GlobalConfig)
		if err != nil {
			return fmt.Errorf("could not unmarshal file to config struct: %w", err)
		}
	}

	return validateConfig()
}

// Unmarshal a root config with default values
func (cfg *Config) UnmarshalJSON(data []byte) error {
	type MethodlessConfigAlias Config // type alias with no methods is needed to prevent recursive calls to UnmarshalJSON

	// default values
	tmpCfg := &MethodlessConfigAlias{
		RetryConnectPeriodSeconds: 1,
		InfluxAddr:                "localhost:8086",
		MongoAddr:                 "localhost:27017",
		NumExtractWorkers:         1,
	}

	err := json.Unmarshal(data, tmpCfg)
	if err != nil {
		return err
	}

	*cfg = Config(*tmpCfg)
	return nil
}

// Unmarshal a CQ config with default values
func (cfg *ContinuousQueryConfig) UnmarshalJSON(data []byte) error {
	type MethodlessConfigAlias ContinuousQueryConfig // type alias with no methods is needed to prevent recursive calls to UnmarshalJSON

	// default values
	tmpCfg := &MethodlessConfigAlias{
		Measurement: ":MEASUREMENT", // uses Influx backreferencing to store the result into a measurement named according to the FROM measurement
		From:        "/.*/",         // uses a match-everything regular expression to apply the CQ to all measurements
		Into:        "",             // if left empty, we should later dynamically set the destination DB to the same DB the CQ is applied to
	}

	err := json.Unmarshal(data, tmpCfg)
	if err != nil {
		return err
	}

	*cfg = ContinuousQueryConfig(*tmpCfg)
	return nil
}

// return an error if config is invalid
func validateConfig() error {
	if len(GlobalConfig.InputPath) == 0 {
		return fmt.Errorf("input path not specified")
	}
	if len(GlobalConfig.Extensions) == 0 {
		return fmt.Errorf("extension not specified")
	}
	if GlobalConfig.NumValidateWorkers <= 0 {
		return fmt.Errorf("number of validator workers configured is %d, but at least 1 is required to function", GlobalConfig.NumValidateWorkers)
	}
	if GlobalConfig.DbHealthCheckDelayS < 0 {
		return fmt.Errorf("configured write delay is %f seconds, but the write delay must be non-negative", GlobalConfig.DbHealthCheckDelayS)
	}
	if GlobalConfig.RetryConnectPeriodSeconds <= 0 {
		return fmt.Errorf("configured retry connection period is %f seconds, but it must be positive", GlobalConfig.DbHealthCheckDelayS)
	}

	// check that extensions are known
	for _, ext := range GlobalConfig.Extensions {
		if ext != ".tar.gz" && ext != ".batchpqtgz" {
			return fmt.Errorf("configured extension %s is unknown", ext)
		}
	}

	// check that there is at most one default retention policy and retention policy names aren't duplicated
	numDefaultRPs := 0
	rpNameSet := map[string]struct{}{}
	for _, rp := range GlobalConfig.RetentionPolicies {
		if _, exists := rpNameSet[rp.Name]; exists {
			return fmt.Errorf("retention policy name %s is repeated, duplicate names are not allowed", rp.Name)
		}
		rpNameSet[rp.Name] = struct{}{}

		if rp.Default {
			numDefaultRPs++
		}
	}
	if numDefaultRPs > 1 {
		return fmt.Errorf("number of default retentions is %d, but must be at most 1", numDefaultRPs)
	}

	// check that retention policies used by continuous queries are all included in the retention policies list
	for _, cq := range GlobalConfig.ContinuousQueries {
		if _, known := rpNameSet[cq.RP]; !known {
			return fmt.Errorf("continuous query %s uses unknown retention policy %s", cq.Name, cq.RP)
		}
	}

	return nil
}
