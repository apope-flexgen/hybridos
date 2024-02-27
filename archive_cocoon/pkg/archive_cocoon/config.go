package archive_cocoon

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
	InputPath         string `json:"input_path"`          // Path at which cocoon looks for archives
	OutputPath        string `json:"output_path"`         // Path at which cocoon writes parquet files
	FailedConvertPath string `json:"failed_convert_path"` // Path to which cocoon saves failed archives
	FwdPath           string `json:"forward"`             // Path to which cocoon saves successful archives
	NumConvertWorkers int    `json:"num_convert_workers"` // Number of conversion workers running in parallel
	Extension         string `json:"ext"`                 // Extension of archives to look for

	// NOTE: these may possibly be unecessary (check my other note in converter.go/convertUntil)
	Client string `json:"client_name"` // name of the client for parquet file naming purposes
	Site   string `json:"site_name"`   // name of the site for parquet file naming purposes
}

var (
	GlobalConfig Config // global config struct parsed from config file if passed
)

// Parses configuration from the source provided: either a filepath or "dbi"
func ParseConfig(cfgSource string) error {
	// set default config values
	GlobalConfig = Config{
		FailedConvertPath: "",
		FwdPath:           "",
		NumConvertWorkers: 1,
	}
	// read config from dbi or from file
	if len(cfgSource) == 0 || strings.EqualFold(cfgSource, "dbi") {
		log.MsgInfo("Config source set to read from dbi")
		// Query DBI for configuration
		configBody, err := cfgfetch.Retrieve("archive_cocoon", "dbi")
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
		configBytes, err := ioutil.ReadFile(cfgSource)
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

func validateConfig() error {
	if len(GlobalConfig.InputPath) == 0 {
		return fmt.Errorf("input path not specified")
	}
	if len(GlobalConfig.OutputPath) == 0 {
		return fmt.Errorf("output path not specified")
	}
	if len(GlobalConfig.Extension) == 0 {
		return fmt.Errorf("extension not specified")
	}
	if GlobalConfig.NumConvertWorkers <= 0 {
		return fmt.Errorf("number of validator workers configured is %d, but at least 1 is required to function", GlobalConfig.NumConvertWorkers)
	}
	return nil
}
