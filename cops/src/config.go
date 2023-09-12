// config.go implements reading/parsing config file for cops
package main

import (
	"fmt"
	"path"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
)

var cfgDir = "/usr/local/etc/config/cops"

// Error wrapper to distinguish configuration issues that should be fatal
type configurationError struct {
	err error
}

// cfg is the struct to unmarshal cops.json into
type cfg struct {
	controllerName          string
	heartbeatFrequencyMS    int
	patrolFrequencyMS       int
	briefingFrequencyMS     int
	c2cMsgFrequencyMS       int
	temperatureSource       string
	enableRedundantFailover bool
	primaryIP               []string
	primaryNetworkInterface []string
	thisCtrlrStaticIP       string
	otherCtrlrStaticIP      string
	pduIP                   string
	pduOutletEndpoint       string
	otherCtrlrOutlet        string
	processList             []processConfig
}

// Static information about each process that must be read from the config json and loaded into processInfo struct
type processConfig struct {
	name                     string
	uri                      string
	writeOutC2C              []string
	killOnHang               bool
	requiredForHealthyStatus bool
	hangTimeAllowanceMS      int
	configRestart            bool
}

// Handles configuring COPS with a map that contains configuration data
func handleConfiguration(body map[string]interface{}) error {
	// Define config struct with default values to fall back on in the case of parsing errors
	// Only variables with established default values are provided, else the variable is required and has no fallback
	config := cfg{
		heartbeatFrequencyMS:    1000,
		patrolFrequencyMS:       1000,
		briefingFrequencyMS:     5000,
		c2cMsgFrequencyMS:       50,
		enableRedundantFailover: false,
	}

	// Validate each configuration field
	heartbeatFreqInterface, err := parsemap.ExtractAsInt(body, "heartbeatFrequencyMS")
	// If error, use default value
	if err == nil {
		config.heartbeatFrequencyMS = heartbeatFreqInterface
	} else {
		log.Warnf("failed to extract heartbeatFrequencyMS from configuration, default value %d used.", config.heartbeatFrequencyMS)
	}

	patrolFreqInterface, err := parsemap.ExtractAsInt(body, "patrolFrequencyMS")
	// If error, use default value
	if err == nil {
		config.patrolFrequencyMS = patrolFreqInterface
	} else {
		log.Warnf("failed to extract patrolFrequencyMS from configuration, default value %d used.", config.patrolFrequencyMS)
	}

	briefingFreqInterface, err := parsemap.ExtractAsInt(body, "briefingFrequencyMS")
	// If error, use default value
	if err == nil {
		config.briefingFrequencyMS = briefingFreqInterface
	} else {
		log.Warnf("failed to extract briefingFrequencyMS from configuration, default value %d used.", config.briefingFrequencyMS)
	}

	cscMsgFreqInterface, err := parsemap.ExtractAsInt(body, "c2cMsgFrequencyMS")
	// If error, use default value
	if err == nil {
		config.c2cMsgFrequencyMS = cscMsgFreqInterface
	} else {
		log.Warnf("failed to extract c2cMsgFrequencyMS from configuration, default value %d used.", config.c2cMsgFrequencyMS)
	}

	tempInterface, err := parsemap.ExtractValueWithType(body, "temperatureSource", parsemap.STRING)
	// If error, unused and do not report
	if err == nil {
		config.temperatureSource = tempInterface.(string)
	}

	failoverInterface, err := parsemap.ExtractValueWithType(body, "enableRedundantFailover", parsemap.BOOL)
	// If error, use default value
	if err == nil {
		config.enableRedundantFailover = failoverInterface.(bool)
	} else {
		log.Warnf("failed to extract enableRedundantFailover from configuration, default value %t used.", config.enableRedundantFailover)
	}

	if err = configureFailover(body, &config); err != nil {
		return fmt.Errorf("failover configuration error: %w", err)
	}

	processListInterface, err := parsemap.ExtractValueWithType(body, "processList", parsemap.INTERFACE_SLICE)
	// Process list most be provided and include at least one entry
	if err != nil {
		return fmt.Errorf("failed to extract processList from configuration: %w", err)
	}
	processList := processListInterface.([]interface{})
	if len(processList) == 0 {
		return fmt.Errorf("extracted empty process list from configuration")
	}
	config.processList = make([]processConfig, 0, len(processList))
	for procId, process := range processList {
		procObj, ok := process.(map[string]interface{})
		if !ok {
			return fmt.Errorf("process object with ID %d was expected to be a map[string]interface{}, but actual type was %T", procId, procObj)
		}

		// Default process struct, skipping fields without established defaults
		procEntry := processConfig{
			killOnHang:               true,
			requiredForHealthyStatus: true,
			hangTimeAllowanceMS:      3000,
			configRestart:            true,
		}

		procNameInterface, err := parsemap.ExtractValueWithType(procObj, "name", parsemap.STRING)
		if err != nil {
			return fmt.Errorf("failed to extract name from process %d object configuration: %w", procId, err)
		}
		procEntry.name = procNameInterface.(string)

		procUriInterface, err := parsemap.ExtractValueWithType(procObj, "uri", parsemap.STRING)
		if err != nil {
			return fmt.Errorf("failed to extract uri from process %d object configuration: %w", procId, err)
		}
		procEntry.uri = procUriInterface.(string)

		procHangInterface, err := parsemap.ExtractValueWithType(procObj, "killOnHang", parsemap.BOOL)
		// If error, use default value
		if err == nil {
			procEntry.killOnHang = procHangInterface.(bool)
		} else {
			log.Warnf("failed to extract killOnHang from process %d object configuration, default value %t used.", procId, procEntry.killOnHang)
		}

		procHealthInterface, err := parsemap.ExtractValueWithType(procObj, "requiredForHealthyStatus", parsemap.BOOL)
		// If error, use default value
		if err == nil {
			procEntry.requiredForHealthyStatus = procHealthInterface.(bool)
		} else {
			log.Warnf("failed to extract requiredForHealthyStatus from process %d object configuration, default value %t used.", procId, procEntry.requiredForHealthyStatus)
		}

		procHangTimeInterface, err := parsemap.ExtractAsInt(procObj, "hangTimeAllowanceMS")
		// If error, use default value
		if err == nil {
			procEntry.hangTimeAllowanceMS = procHangTimeInterface
		} else {
			log.Warnf("failed to extract hangTimeAllowanceMS from process %d object configuration, default value %d used.", procId, procEntry.hangTimeAllowanceMS)
		}

		procRestartInterface, err := parsemap.ExtractValueWithType(procObj, "configRestart", parsemap.BOOL)
		// If error, use default value
		if err == nil {
			procEntry.configRestart = procRestartInterface.(bool)
		} else {
			log.Warnf("failed to extract configRestart from process %d object configuration, default value %t used.", procId, procEntry.configRestart)
		}

		writeOutInterface, err := parsemap.ExtractValueWithType(procObj, "writeOutC2C", parsemap.INTERFACE_SLICE)
		if err != nil {
			return fmt.Errorf("failed to extract writeOutC2C list from process %d object configuration: %w", procId, err)
		}
		procEntry.writeOutC2C = make([]string, 0, len(writeOutInterface.([]interface{})))
		for _, uriInterface := range writeOutInterface.([]interface{}) {
			if uri, ok := uriInterface.(string); ok {
				procEntry.writeOutC2C = append(procEntry.writeOutC2C, uri)
			} else {
				return fmt.Errorf("failed to extract writeOutC2C uri from process %d object configuration", procId)
			}
		}
		config.processList = append(config.processList, procEntry)
	}

	return configureCOPS(config)
}

// Configure internal variables
func configureCOPS(config cfg) error {
	beginningTime = time.Now()

	controllerName = config.controllerName
	if enableRedundantFailover = config.enableRedundantFailover; enableRedundantFailover {
		// Store the IP addresses of the primary
		for _, ip := range config.primaryIP {
			primaryIP = append(primaryIP, ip)
		}
		for _, netInterface := range config.primaryNetworkInterface {
			primaryNetworkInterface = append(primaryNetworkInterface, netInterface)
		}

		// Store the IP of the PDU
		pduIP = config.pduIP
		pduOutletEndpoint = config.pduOutletEndpoint
		otherCtrlrOutlet = config.otherCtrlrOutlet
	}

	tempSource = config.temperatureSource

	// Set process variables
	processJurisdiction = make(map[string]*processInfo)
	for _, process := range config.processList {
		var processEntry processInfo
		processEntry.name = process.name
		processEntry.uri = process.uri
		processEntry.writeOutC2C = process.writeOutC2C
		processEntry.killOnHang = process.killOnHang
		processEntry.requiredForHealthyStatus = process.requiredForHealthyStatus
		processEntry.hangTimeAllowance = time.Duration(process.hangTimeAllowanceMS) * time.Millisecond
		processEntry.configRestart = process.configRestart
		// Set default initial values
		processEntry.replyToURI = path.Join("/cops/heartbeat/", process.name)
		processEntry.alive = true
		processEntry.healthStats.lastConfirmedAlive = beginningTime
		processEntry.healthStats.lastRestart = beginningTime
		processEntry.healthStats.totalRestarts = -1 // First startup will increment this to zero
		processEntry.healthStats.copsRestarts = 0
		processJurisdiction[process.name] = &processEntry
	}

	dr.configure(float64(config.heartbeatFrequencyMS))
	if enableRedundantFailover {
		err := configureC2C(config)
		if err != nil {
			return fmt.Errorf("error configuring C2C: %w", err)
		}
		go startControllerModeNegotiation()
	}
	if !enableRedundantFailover {
		takeOverAsPrimary()
	}
	heartRate, patrolRate, briefingRate, statsUpdateTicker = startAllTickers(config)
	return nil
}

// Returns all COPS tickers when given their frequencies
func startAllTickers(config cfg) (heartRate, patrolRate, briefingRate, statsUpdateRate *time.Ticker) {
	heartRate = startTicker(config.heartbeatFrequencyMS)
	patrolRate = startTicker(config.patrolFrequencyMS)
	briefingRate = startTicker(config.briefingFrequencyMS)
	statsUpdateRate = startTicker(3000)
	return
}

// Uses a millisecond value from the config file to start a Ticker
func startTicker(rawTickFreqMS int) *time.Ticker {
	tickFreq := time.Duration(rawTickFreqMS) * time.Millisecond
	return time.NewTicker(tickFreq)
}

func (m *configurationError) Error() string {
	return m.err.Error()
}
