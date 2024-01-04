package ftd

import (
	"errors"
	"fmt"
	"strings"

	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
	"github.com/flexgen-power/hybridos/go_flexgen/parsemap"
)

type Config struct {
	DbName            string `json:"db_name"`
	ArchivePeriod     int    `json:"period"`
	ArchivePath       string `json:"archive"`
	NumArchiveWorkers int    `json:"num_archive_workers"`
	Uris              []UriConfig
}

type UriConfig struct {
	BaseUri       string   `json:"uri"`
	Sources       []string `json:"sources"`
	Group         string   `json:"group"`
	DestinationDb string   `json:"destination"` // "influx" or "mongo"
	Measurement   string   `json:"measurement"`
}

var GlobalConfig Config // data structure for data parsed from configuration

// Translates configuration data into internal data structure.
// Validates config data and returns error if invalid.
func HandleConfiguration(body map[string]interface{}) error {
	// parse DB name
	dbNameInterface, err := parsemap.ExtractValueWithType(body, "db_name", parsemap.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract db_name from configuration, %w", err)
	}
	GlobalConfig.DbName = dbNameInterface.(string)

	periodInterface, err := parsemap.ExtractAsInt(body, "period")
	if err != nil {
		return fmt.Errorf("failed to extract period from configuration, %w", err)
	}
	if periodInterface <= 0 {
		return fmt.Errorf("configured period is not positive, %w", err)
	}
	GlobalConfig.ArchivePeriod = periodInterface

	archiveInterface, err := parsemap.ExtractValueWithType(body, "archive", parsemap.STRING)
	if err == nil {
		GlobalConfig.ArchivePath = archiveInterface.(string)
	} else {
		GlobalConfig.ArchivePath = "/home/hybridos/data"
		log.Warnf("Failed to extract archive from configuration, default value %s used", GlobalConfig.ArchivePath)
	}

	numArchiveWorkers, err := parsemap.ExtractAsInt(body, "num_archive_workers")
	if err != nil {
		return fmt.Errorf("failed to extract num_archive_workers from configuration: %w", err)
	}
	if numArchiveWorkers < 1 {
		return fmt.Errorf("expected num_archive_workers to be >=1 but read %d instead", numArchiveWorkers)
	}
	GlobalConfig.NumArchiveWorkers = numArchiveWorkers

	urisInterface, err := parsemap.ExtractValueWithType(body, "uris", parsemap.INTERFACE_SLICE)
	if err != nil {
		return fmt.Errorf("failed to extract uris list from configuration: %w", err)
	}
	urisList := urisInterface.([]interface{})
	uriLen := len(urisList)
	if uriLen == 0 {
		return errors.New("no FIMS URIs provided to subscribe. null config")
	}
	GlobalConfig.Uris = make([]UriConfig, 0, uriLen)
	uriHasSources := make(map[string]bool)
	for index, uri := range urisList {
		uriObj, ok := uri.(map[string]interface{})
		if !ok {
			return fmt.Errorf("uri entry with ID %d was expected to be a map[string]interface{}, but actual type was %T", index, uriObj)
		}

		// Default values
		uriEntry := UriConfig{
			DestinationDb: "influx",
			Sources:       []string{},
			Group:         "",
		}

		uriInterface, err := parsemap.ExtractValueWithType(uriObj, "uri", parsemap.STRING)
		if err != nil {
			return fmt.Errorf("failed to extract uri from uris entry %d configuration: %w", index, err)
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
				return fmt.Errorf("failed assertion of source list to []interface{} for entry %d in uri configuration", index)
			}
			uriEntry.Sources = make([]string, 0, len(sourceList))
		}

		for _, source := range sourceList {
			if sourceName, ok := source.(string); ok {
				uriEntry.Sources = append(uriEntry.Sources, sourceName)
			} else {
				return fmt.Errorf("failed to extract sourceName for entry %d in storage configuration due to incorrect datatype %T, expected string", index, source)
			}
		}
		// Do not allow the same uri to be listed both with and without sources, it must be one or the other
		sourcesGiven := len(uriEntry.Sources) > 0
		if hadSources, exists := uriHasSources[uriEntry.BaseUri]; exists && (hadSources != sourcesGiven) {
			return fmt.Errorf("uri entry with id %d mixes having and not having sources", index)
		} else {
			uriHasSources[uriEntry.BaseUri] = sourcesGiven
		}

		groupInterface, ok := uriObj["group"]
		if ok {
			group, isString := groupInterface.(string)
			if !isString {
				return fmt.Errorf("URI %s of index %d has group field of type %T but expected string", uri, index, groupInterface)
			}
			uriEntry.Group = group
		}

		measurementInterface, err := parsemap.ExtractValueWithType(uriObj, "measurement", parsemap.STRING)
		if err != nil {
			return fmt.Errorf("failed to extract measurement for URI- %s at index %d with error %v", uriEntry.BaseUri, index, err)
		}
		uriEntry.Measurement = measurementInterface.(string)

		destinationInterface, err := parsemap.ExtractValueWithType(uriObj, "destination", parsemap.STRING)
		// If error, fall back to default value
		if err == nil {
			uriEntry.DestinationDb = destinationInterface.(string)
		} else {
			log.Warnf("Failed to extract destination for URI- %s at index %d, default value %s used", uriEntry.BaseUri, index, uriEntry.DestinationDb)
		}
		GlobalConfig.Uris = append(GlobalConfig.Uris, uriEntry)
	}
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
