package ftd

import (
	"errors"
	"fmt"
	"strings"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/go_flexgen/parsemap"
)

// Root configuration struct
type Config struct {
	Lane1 *LaneConfig `json:"1"` // Lowest priority lane configuration
	Lane2 *LaneConfig `json:"2"`
	Lane3 *LaneConfig `json:"3"`
	Lane4 *LaneConfig `json:"4"`
	Lane5 *LaneConfig `json:"5"` // Highest priority lane configuration
}

// Configuration specific to a data lane
type LaneConfig struct {
	DbName            string `json:"db_name"`
	ArchivePeriod     int    `json:"period"`
	ArchivePath       string `json:"archive"`
	Parquet           bool   `json:"parquet"`
	NumArchiveWorkers int    `json:"num_archive_workers"`
	Uris              []UriConfig
}

// Configuration specific to data from a particular uri
type UriConfig struct {
	BaseUri       string   `json:"uri"`
	Sources       []string `json:"sources"`
	Fields        []string `json:"fields"`
	Group         string   `json:"group"`
	Method        []string `json:"message_methods"`
	DestinationDb string   `json:"destination"` // "influx" or "mongo"
	Measurement   string   `json:"measurement"`
}

// Translates configuration data into internal data structure.
// Validates config data and returns error if invalid.
func ExtractRootConfiguration(body map[string]interface{}) (Config, error) {
	var cfg Config

	for i := 1; i <= 5; i++ {
		var laneCfg *LaneConfig = nil

		laneCfgInterface := body[fmt.Sprint(i)]
		laneCfgObj, ok := laneCfgInterface.(map[string]interface{})
		if !ok {
			log.Infof("Data lane %d configured as inactive since it doesn't assert to a map[string]interface{}", i)
		} else if laneCfgObj == nil {
			log.Infof("Data lane %d configured as inactive since it is nil", i)
		} else {
			laneCfgStruct, err := extractLaneConfiguration(laneCfgObj)
			if err != nil {
				return cfg, fmt.Errorf("failed to extract lane configuration for lane %d, %w", i, err)
			}
			laneCfg = &laneCfgStruct
		}

		switch i {
		case 1:
			cfg.Lane1 = laneCfg
		case 2:
			cfg.Lane2 = laneCfg
		case 3:
			cfg.Lane3 = laneCfg
		case 4:
			cfg.Lane4 = laneCfg
		case 5:
			cfg.Lane5 = laneCfg
		}
	}

	return cfg, nil
}

// Translates lane configuration data into internal data structure.
// Validates config data and returns error if invalid.
func extractLaneConfiguration(body map[string]interface{}) (LaneConfig, error) {
	var cfg LaneConfig

	// parse DB name
	dbNameInterface, err := parsemap.ExtractValueWithType(body, "db_name", parsemap.STRING)
	if err != nil {
		return cfg, fmt.Errorf("failed to extract db_name from configuration, %w", err)
	}
	cfg.DbName = dbNameInterface.(string)

	parquetInterface, err := parsemap.ExtractValueWithType(body, "parquet", parsemap.BOOL)
	if err != nil {
		cfg.Parquet = false // default
	} else {
		cfg.Parquet = parquetInterface.(bool)
	}

	periodInterface, err := parsemap.ExtractAsInt(body, "period")
	if err != nil {
		return cfg, fmt.Errorf("failed to extract period from configuration, %w", err)
	}
	if periodInterface <= 0 {
		return cfg, fmt.Errorf("configured period is not positive, %w", err)
	}
	cfg.ArchivePeriod = periodInterface

	archiveInterface, err := parsemap.ExtractValueWithType(body, "archive", parsemap.STRING)
	if err == nil {
		cfg.ArchivePath = archiveInterface.(string)
	} else {
		cfg.ArchivePath = "/home/hybridos/data"
		log.Warnf("Failed to extract archive from configuration, default value %s used", cfg.ArchivePath)
	}

	numArchiveWorkers, err := parsemap.ExtractAsInt(body, "num_archive_workers")
	if err != nil {
		return cfg, fmt.Errorf("failed to extract num_archive_workers from configuration: %w", err)
	}
	if numArchiveWorkers < 1 {
		return cfg, fmt.Errorf("expected num_archive_workers to be >=1 but read %d instead", numArchiveWorkers)
	}
	cfg.NumArchiveWorkers = numArchiveWorkers

	urisInterface, err := parsemap.ExtractValueWithType(body, "uris", parsemap.INTERFACE_SLICE)
	if err != nil {
		return cfg, fmt.Errorf("failed to extract uris list from configuration: %w", err)
	}
	urisList := urisInterface.([]interface{})
	uriLen := len(urisList)
	if uriLen == 0 {
		return cfg, errors.New("no FIMS URIs provided to subscribe. null config")
	}
	cfg.Uris = make([]UriConfig, 0, uriLen)
	uriAndMethodHasSources := make(map[string]bool)
	for index, uri := range urisList {
		uriObj, ok := uri.(map[string]interface{})
		if !ok {
			return cfg, fmt.Errorf("uri entry with ID %d was expected to be a map[string]interface{}, but actual type was %T", index, uriObj)
		}

		// Default values
		uriEntry := UriConfig{
			DestinationDb: "influx",
			Sources:       []string{},
			Fields:        []string{},
			Group:         "",
			Method:        []string{},
		}

		uriInterface, err := parsemap.ExtractValueWithType(uriObj, "uri", parsemap.STRING)
		if err != nil {
			return cfg, fmt.Errorf("failed to extract uri from uris entry %d configuration: %w", index, err)
		}
		uriEntry.BaseUri = uriInterface.(string)

		sourceListInterface, ok := uriObj["sources"]
		var sourceList []interface{}
		// If sources isn't in the map, use default value
		if !ok {
			log.Infof("Did not find a source list for entry %d in uri configuration, defaulting to capturing all sources", index)
			sourceList = []interface{}{} // Set empty to make sure we skip iterating over it
		} else {
			sourceList, ok = sourceListInterface.([]interface{})
			if !ok {
				return cfg, fmt.Errorf("failed assertion of source list to []interface{} for entry %d in uri configuration", index)
			}
			uriEntry.Sources = make([]string, 0, len(sourceList))
		}

		for _, source := range sourceList {
			if sourceName, ok := source.(string); ok {
				uriEntry.Sources = append(uriEntry.Sources, sourceName)
			} else {
				return cfg, fmt.Errorf("failed to extract sourceName for entry %d in storage configuration due to incorrect datatype %T, expected string", index, source)
			}
		}

		fieldsListInterface, ok := uriObj["fields"]
		var fieldsList []interface{}
		// If fields isn't in the map, use default value
		if !ok {
			log.Infof("Did not find a fields list for entry %d in uri configuration, defaulting to capturing all fields", index)
			fieldsList = []interface{}{} // Set empty to make sure we skip iterating over it
		} else {
			fieldsList, ok = fieldsListInterface.([]interface{})
			if !ok {
				return cfg, fmt.Errorf("failed assertion of fields list to []interface{} for entry %d in uri configuration", index)
			}
			uriEntry.Fields = make([]string, 0, len(fieldsList))
		}

		for _, field := range fieldsList {
			if fieldName, ok := field.(string); ok {
				uriEntry.Fields = append(uriEntry.Fields, fieldName)
			} else {
				return cfg, fmt.Errorf("failed to extract fieldName for entry %d in storage configuration due to incorrect datatype %T, expected string", index, field)
			}
		}

		groupInterface, ok := uriObj["group"]
		if ok {
			group, isString := groupInterface.(string)
			if !isString {
				return cfg, fmt.Errorf("URI %s of index %d has group field of type %T but expected string", uri, index, groupInterface)
			}
			uriEntry.Group = group
		}

		methodListInterface, ok := uriObj["message_methods"]
		var methodList []interface{}
		// If message_methods isn't in the map, use default value
		if !ok {
			log.Infof("Did not find a method list for entry %d in uri configuration, defaulting to capturing pub and post message methods", index)
			methodList = []interface{}{"pub", "post"} // Set to pub and post to make sure we have backwards compatibility for older configs
		} else {
			methodList, ok = methodListInterface.([]interface{})
			if !ok {
				return cfg, fmt.Errorf("failed assertion of method list to []interface{} for entry %d in uri configuration", index)
			}
			uriEntry.Method = make([]string, 0, len(methodList))
		}

		for _, method := range methodList {
			if methodName, ok := method.(string); ok {
				uriEntry.Method = append(uriEntry.Method, methodName)
			} else {
				return cfg, fmt.Errorf("failed to extract methodName for entry %d in storage configuration due to incorrect datatype %T, expected string", index, method)
			}
		}

		measurementInterface, err := parsemap.ExtractValueWithType(uriObj, "measurement", parsemap.STRING)
		if err != nil {
			return cfg, fmt.Errorf("failed to extract measurement for URI- %s at index %d with error %v", uriEntry.BaseUri, index, err)
		}
		uriEntry.Measurement = measurementInterface.(string)

		destinationInterface, err := parsemap.ExtractValueWithType(uriObj, "destination", parsemap.STRING)
		// If error, fall back to default value
		if err == nil {
			uriEntry.DestinationDb = destinationInterface.(string)
		} else {
			log.Warnf("Failed to extract destination for URI- %s at index %d, default value %s used", uriEntry.BaseUri, index, uriEntry.DestinationDb)
		}

		// Do not allow the same uri to be listed both with and without sources for the same method, it must be one or the other
		sourcesGiven := len(uriEntry.Sources) > 0
		for _, methodName := range uriEntry.Method {
			if hadSources, exists := uriAndMethodHasSources[uriEntry.BaseUri+"_"+methodName]; exists && (hadSources != sourcesGiven) {
				return cfg, fmt.Errorf("uri entry with id %d mixes having and not having sources for the same method", index)
			} else {
				uriAndMethodHasSources[uriEntry.BaseUri+"_"+methodName] = sourcesGiven
			}
		}

		cfg.Uris = append(cfg.Uris, uriEntry)
	}
	return cfg, nil
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
