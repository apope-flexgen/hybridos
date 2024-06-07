package ftd

import (
	"encoding/json"
	"fmt"
	"strings"
)

// Root configuration struct
type Config struct {
	ClientName string      `json:"client"`      // Name of client whose data we're gathering
	SiteName   string      `json:"site"`        // Name of site whose data we're gathering
	Lane1      *LaneConfig `json:"1,omitempty"` // Lowest priority lane configuration
	Lane2      *LaneConfig `json:"2,omitempty"`
	Lane3      *LaneConfig `json:"3,omitempty"`
	Lane4      *LaneConfig `json:"4,omitempty"`
	Lane5      *LaneConfig `json:"5,omitempty"` // Highest priority lane configuration
}

// Configuration specific to a data lane
type LaneConfig struct {
	DbName            string      `json:"db_name"`
	ArchivePeriod     int         `json:"period"`
	ArchivePath       string      `json:"archive"`
	BatchParquetGZ    bool        `json:"batchpqtgz,omitempty"`
	NumArchiveWorkers int         `json:"num_archive_workers"`
	Uris              []UriConfig `json:"uris"`
}

// Configuration specific to data from a particular uri
type UriConfig struct {
	BaseUri         string                 `json:"uri"`
	Sources         []string               `json:"sources,omitempty"`
	Fields          []string               `json:"fields,omitempty"`            // specifies fields to filter for, or empty if all fields should be captured
	BitStringFields []BitStringFieldConfig `json:"bit_string_fields,omitempty"` // configures fields which should be interpreted as bitstrings
	Group           string                 `json:"group,omitempty"`
	Method          []string               `json:"message_methods,omitempty"`
	DestinationDb   string                 `json:"destination,omitempty"` // "influx" or "mongo"
	Measurement     string                 `json:"measurement"`
}

// Configuration for how to encode a bit string field
type BitStringFieldConfig struct {
	FieldName  string   `json:"field"`
	BitStrings []string `json:"bit_strings"` // standardized names assigned to each bit
}

// Translates configuration data into internal data structure.
// Validates config data and returns error if invalid.
func ExtractRootConfiguration(body map[string]interface{}) (Config, error) {
	var cfg Config

	configBytes, err := json.Marshal(body)
	if err != nil {
		return cfg, fmt.Errorf("failed to marshal config body into json bytes: %w", err)
	}
	err = json.Unmarshal(configBytes, &cfg)
	if err != nil {
		return cfg, fmt.Errorf("failed to unmarshal config: %w", err)
	}

	err = validateRootConfig(cfg)
	if err != nil {
		return cfg, fmt.Errorf("config failed validation: %w", err)
	}

	return cfg, nil
}

// Returns an error if the config fails validation checks
func validateRootConfig(cfg Config) error {
	laneCfgs := []*LaneConfig{
		cfg.Lane1,
		cfg.Lane2,
		cfg.Lane3,
		cfg.Lane4,
		cfg.Lane5,
	}
	for laneIndex, laneCfg := range laneCfgs {
		err := validateLaneConfig(laneCfg)
		if err != nil {
			return fmt.Errorf("lane %d failed validation: %w", laneIndex, err)
		}
	}
	return nil
}

func validateLaneConfig(laneCfg *LaneConfig) error {
	if laneCfg == nil {
		return nil // nothing to do if lane has no configuration
	}
	if laneCfg.ArchivePeriod <= 0 {
		return fmt.Errorf("archive period is not positive")
	}
	if laneCfg.NumArchiveWorkers < 1 {
		return fmt.Errorf("number of archive workers must be at least 1")
	}
	if len(laneCfg.Uris) == 0 {
		return fmt.Errorf("no uris configured")
	}
	uriAndMethodHasSources := make(map[string]bool)
	for uriIndex, uriCfg := range laneCfg.Uris {
		if uriCfg.Measurement == "" {
			return fmt.Errorf("uri %s at index %d has no measurement", uriCfg.BaseUri, uriIndex)
		}

		// do not allow the same uri to be listed both with and without sources for the same method, it must be one or the other
		sourcesGiven := len(uriCfg.Sources) > 0
		for _, methodName := range uriCfg.Method {
			if hadSources, exists := uriAndMethodHasSources[uriCfg.BaseUri+"_"+methodName]; exists && (hadSources != sourcesGiven) {
				return fmt.Errorf("uri %s at index %d mixes having and not having sources for the same method", uriCfg.BaseUri, uriIndex)
			} else {
				uriAndMethodHasSources[uriCfg.BaseUri+"_"+methodName] = sourcesGiven
			}
		}
	}
	return nil
}

// Unmarshal a lane config with default values
func (cfg *LaneConfig) UnmarshalJSON(data []byte) error {
	type MethodlessConfigAlias LaneConfig // type alias with no methods is needed to prevent recursive calls to UnmarshalJSON

	// default values
	tmpCfg := &MethodlessConfigAlias{
		BatchParquetGZ: false,
		ArchivePath:    "/home/hybridos/data",
	}

	err := json.Unmarshal(data, tmpCfg)
	if err != nil {
		return err
	}

	*cfg = LaneConfig(*tmpCfg)
	return nil
}

// Unmarshal a uri config with default values
func (cfg *UriConfig) UnmarshalJSON(data []byte) error {
	type MethodlessConfigAlias UriConfig // type alias with no methods is needed to prevent recursive calls to UnmarshalJSON

	// default values
	tmpCfg := &MethodlessConfigAlias{
		DestinationDb: "influx",
		Sources:       []string{}, // empty sources indicates to capture all sources
		Fields:        []string{}, // empty fields indicates to capture all fields
		Group:         "",         // empty string indicates no grouping
		Method:        []string{"pub", "post"},
	}

	err := json.Unmarshal(data, tmpCfg)
	if err != nil {
		return err
	}

	*cfg = UriConfig(*tmpCfg)
	return nil
}

// Cleans up URI for easier storage and processing,
// removes any edge spaces,
// removes tailing '/' to avoid complexities, and
// ensures URI starts with '/' and ALWAYS ends with alphanumeric.
//
// TODO: compare this functionality with that of path.Clean to see
// if this can just be replaced with path.Clean
func cleanUri(str string) string {
	ind := len(str) - 1
	found := false
	for ind > 0 {
		if str[ind] == '/' || str[ind] == ' ' {
			found = true
			ind--
		} else {
			break
		}
	}
	var retVal string = str
	if found {
		retVal = str[0:(ind + 1)]
	}
	retVal = strings.TrimSpace(retVal)
	if retVal[0] != '/' {
		retVal = "/" + retVal
	}
	return retVal
}
