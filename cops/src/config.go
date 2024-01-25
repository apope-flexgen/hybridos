// config.go implements reading/parsing config file for cops
package main

import (
	"encoding/json"
	"fmt"
	"net"
	"os"
	"path"
	"strconv"
	"time"

	"github.com/flexgen-power/go_flexgen/cfgfetch"
	log "github.com/flexgen-power/go_flexgen/logger"
)

var cfgDir = "/usr/local/etc/config/cops"
var config = Config{}

// Error wrapper to distinguish configuration issues that should be fatal
type configurationError struct {
	err error
}

// Config is the struct to unmarshal cops.json into
type Config struct {
	Name                        string    `json:"controllerName"`
	HeartbeatFrequencyMS        int       `json:"heartbeatFrequencyMS"`
	PatrolFrequencyMS           int       `json:"patrolFrequencyMS"`
	BriefingFrequencyMS         int       `json:"briefingFrequencyMS"`
	C2cMsgFrequencyMS           int       `json:"c2cMsgFrequencyMS"`
	ConnectionHangtimeAllowance int       `json:"connectionHangtimeAllowance"`
	TemperatureSource           string    `json:"temperatureSource"`
	Syswatch                    bool      `json:"syswatch"` // Enable system hardware stats reporting.
	EnableRedundantFailover     bool      `json:"enableRedundantFailover"`
	PrimaryIP                   []string  `json:"primaryIP,omitempty"`
	PrimaryNetworkInterface     []string  `json:"primaryNetworkInterface,omitempty"`
	ThisCtrlrStaticIP           string    `json:"thisCtrlrStaticIP,omitempty"`
	OtherCtrlrStaticIP          string    `json:"otherCtrlrStaticIP,omitempty"`
	PduIP                       string    `json:"pduIP,omitempty"`
	PduOutletEndpoint           string    `json:"pduOutletEndpoint"`
	OtherCtrlrOutlet            string    `json:"otherCtrlrOutlet"`
	AllowActions                bool      `json:"allowActions"` // Global for whether or not to allow taking an action
	ProcessList                 []Process `json:"processList"`
}

type Process struct {
	Name                     string   `json:"name"`
	Uri                      string   `json:"uri"`
	AllowActions             bool     `json:"allowActions"`
	EnableConnectionStatus   bool     `json:"connectionStatus"` // toggle for enabling reporting a client-server connection status
	WriteOutC2C              []string `json:"writeOutC2C"`
	KillOnHang               bool     `json:"killOnHang"`
	RequiredForHealthyStatus bool     `json:"requiredForHealthyStatus"` // Determines whether or not heartbeats are enabled for a process.
	HangTimeAllowanceMS      int      `json:"hangTimeAllowanceMS"`
	ConfigRestart            bool     `json:"configRestart"`
	replyToURI               string
	heartbeat                uint
	pid                      int
	processPtr               *os.Process
	alive                    bool
	healthStats              processHealthStats
	version                  processVersion
}

// Pretty print the config to stdout for debugging purposes.
func printConfig(config Config) error {
	cfgJSON, err := json.MarshalIndent(config, "", "  ")
	if err != nil {
		return fmt.Errorf("pretty formatting json: %w", err)
	}
	log.Infof("config: %+v", string(cfgJSON))
	return nil
}

// Handle a configuration body received from DBI.
func handleConfigBody(m map[string]interface{}) error {
	// Marshal the map to JSON
	bytes, err := json.Marshal(m)
	if err != nil {
		return fmt.Errorf("marshaling config map: %w", err)
	}

	// Unmarshal bytes into config struct
	if err := json.Unmarshal(bytes, &config); err != nil {
		return fmt.Errorf("unmarshaling config body to struct: %w", err)
	}

	if err := config.validate(); err != nil {
		return fmt.Errorf("validating new dbi config: %w", err)
	}

	return configureCOPS(config)
}

// Parse config.json in an appropriate struct and validate it.
func parse(cfgSource string) error {

	if cfgSource == "dbi" {
		// Handle config from DBI.
		cfg, err := cfgfetch.Retrieve(processName, cfgSource)
		if err != nil {
			return fmt.Errorf("retrieving dbi config body: %w", err)
		}

		if err := handleConfigBody(cfg); err != nil {
			return fmt.Errorf("handling DBI config body: %w", err)
		}

	} else if cfgSource != "" {

		log.Infof("Retrieving config from: %s", cfgSource)
		configBytes, err := os.ReadFile(cfgSource)
		if err != nil {
			return fmt.Errorf("reading config file: %w", err)
		}

		if err := json.Unmarshal(configBytes, &config); err != nil {
			return fmt.Errorf("unmarshaling config: %w", err)
		}

		// Validate configuration file.
		if err := config.validate(); err != nil {
			return fmt.Errorf("validating config file: %w", err)
		}

		// Configure cops with the provided json file.
		if err := configureCOPS(config); err != nil {
			return fmt.Errorf("configuring COPS: %w", err)
		}

	}

	return nil
}

// Validate the configuration - verify required fields exist. Populate optional fields with defaults if applicable.
func (c *Config) validate() error {
	// Default rates in milliseconds
	defaultHeartBeatMS := 1000
	defaultPatrolMS := 1000
	defaultBriefingMS := 5000
	defaultC2CmsgMS := 50
	defaultConnMS := 3500

	// Use hostname as name if no name provided and redundant failover is not enabled
	if c.Name == "" && !c.EnableRedundantFailover {
		name, err := os.Hostname()
		if err != nil {
			log.Errorf("No controller name provided. Could not obtain default host name: %v", err)
		}

		log.Infof("No controller name provided. Using default: %v. ", name)
		c.Name = name
	}

	// Verify we have appropriate timer frequencies
	if c.HeartbeatFrequencyMS == 0 {
		log.Infof("Using default heartbeat frequency value: %v.", defaultHeartBeatMS)
		c.HeartbeatFrequencyMS = defaultHeartBeatMS
	}

	if c.PatrolFrequencyMS == 0 {
		log.Infof("Using default patrol frequency value: %v.", defaultPatrolMS)
		c.PatrolFrequencyMS = defaultPatrolMS
	}

	if c.BriefingFrequencyMS == 0 {
		log.Infof("Using default briefing frequency value: %v.", defaultBriefingMS)
		c.BriefingFrequencyMS = defaultBriefingMS
	}

	if c.C2cMsgFrequencyMS == 0 {
		log.Infof("Using default c2c messaging frequency value: %v.", defaultC2CmsgMS)
		c.C2cMsgFrequencyMS = defaultC2CmsgMS
	}

	if c.ConnectionHangtimeAllowance == 0 {
		log.Infof("Using default c2c messaging frequency value: %v.", defaultC2CmsgMS)
		c.C2cMsgFrequencyMS = defaultConnMS
	}

	// Validate process list
	for key, process := range c.ProcessList {
		if err := process.validate(); err != nil {
			return fmt.Errorf("process %v: %w", key, err)
		}
	}

	return c.validateFailoverConfig()
}

// Validate a given process from configuration.
func (p *Process) validate() error {

	// Validate each process provided in the list
	if p.Name == "" {
		return fmt.Errorf("process name not provided")
	}

	if p.Uri == "" {
		log.Warnf("URI for process %s not provided.", p.Name)
	}

	if p.HangTimeAllowanceMS == 0 {
		p.HangTimeAllowanceMS = 3000
	}

	return nil
}

// Further validate failover configuration.
func (config *Config) validateFailoverConfig() error {
	defaultServerName := "cops_server"
	defaultClientName := "cops_client"

	// Don't worry about validation if not enabled
	if !config.EnableRedundantFailover {
		return nil
	}

	// Handle encryption and failover validation
	if err := config.handleEncryptionConfig(); err != nil {
		return fmt.Errorf("encryption config: %w", err)
	}

	if len(config.PrimaryIP) < 1 {
		return fmt.Errorf("at least one primary IP address must be provided when failover is enabled")
	}
	for _, ip := range config.PrimaryIP {
		if net.ParseIP(ip) == nil {
			return fmt.Errorf("invalid primary IP address provided: %s", ip)
		}
	}
	if len(config.PrimaryNetworkInterface) != len(config.PrimaryIP) {
		return fmt.Errorf("a network interface must be provided for each ip given when failover is enabled. Got %d ips and %d interfaces", len(config.PrimaryIP), len(config.PrimaryNetworkInterface))
	}
	for i, netInterface := range config.PrimaryNetworkInterface {
		if netInterface == "" {
			return fmt.Errorf("no primary network interface provided, entry number %d", i)
		}
	}
	if net.ParseIP(config.ThisCtrlrStaticIP) == nil {
		return fmt.Errorf("invalid static IP provided for this controller: %s", config.ThisCtrlrStaticIP)
	}
	if net.ParseIP(config.OtherCtrlrStaticIP) == nil {
		return fmt.Errorf("invalid static IP provided for other controller: %s", config.OtherCtrlrStaticIP)
	}
	if net.ParseIP(config.PduIP) == nil {
		return fmt.Errorf("invalid pdu IP provided: %s", config.PduIP)
	}
	if config.PduOutletEndpoint == "" {
		// SNMP endpoint for PDU outlet control. Should be the same for all PDU models in the field (AP7901b)
		defaultPduOutletEndpoint := ".1.3.6.1.4.1.318.1.1.4.4.2.1.3."
		log.Infof("Setting default PDU outlet endpoint: %v", defaultPduOutletEndpoint)
		config.PduOutletEndpoint = defaultPduOutletEndpoint
	}
	otherOutletInt, err := strconv.Atoi(config.OtherCtrlrOutlet)
	if err != nil {
		return fmt.Errorf("outlet value: %s for the other controller should be able to be parsed as an integer", config.OtherCtrlrOutlet)
	}
	// TODO: currently only 8 outlets supported, but could be up to 24 in the future with different models
	if otherOutletInt < 1 || otherOutletInt > 8 {
		return fmt.Errorf("outlet value for the other controller is out of range, should be [1 - 8] but got %d", otherOutletInt)
	}

	// Generate controller name based on whether this is server/client if no name provided
	if config.Name == "" {
		// return True if this is the server (this ip < other ip)
		cmp, err := isIPLessThan(config.ThisCtrlrStaticIP, config.OtherCtrlrStaticIP)
		if err != nil {
			return fmt.Errorf("comparing thisCtrlrStaticIP, otherCtrlrStaticIP: %w", err)
		}

		if cmp {
			log.Infof("Setting controller name to default server name: %v.", defaultServerName)
			config.Name = defaultServerName
		} else {
			log.Infof("Setting controller name to default client name: %v.", defaultClientName)
			config.Name = defaultClientName
		}
	}

	return nil
}

// Configure internal variables
func configureCOPS(config Config) error {
	beginningTime = time.Now()

	controllerName = config.Name
	if enableRedundantFailover = config.EnableRedundantFailover; enableRedundantFailover {
		// Store the IP addresses of the primary
		for _, ip := range config.PrimaryIP {
			primaryIP = append(primaryIP, ip)
		}
		for _, netInterface := range config.PrimaryNetworkInterface {
			primaryNetworkInterface = append(primaryNetworkInterface, netInterface)
		}

		// Store the IP of the PDU
		pduIP = config.PduIP
		pduOutletEndpoint = config.PduOutletEndpoint
		otherCtrlrOutlet = config.OtherCtrlrOutlet
	}

	tempSource = config.TemperatureSource

	// Set process variables
	processJurisdiction = make(map[string]*processInfo)
	for _, process := range config.ProcessList {
		var processEntry processInfo
		processEntry.name = process.Name
		processEntry.uri = process.Uri
		processEntry.connected = false
		processEntry.lastUpdate = time.Now()
		processEntry.allowActions = process.AllowActions
		processEntry.enableConnectionStatus = process.EnableConnectionStatus
		processEntry.writeOutC2C = process.WriteOutC2C
		processEntry.killOnHang = process.KillOnHang
		processEntry.requiredForHealthyStatus = process.RequiredForHealthyStatus
		processEntry.hangTimeAllowance = time.Duration(process.HangTimeAllowanceMS) * time.Millisecond
		processEntry.configRestart = process.ConfigRestart
		// Set default initial values
		processEntry.replyToURI = path.Join("/cops/heartbeat/", process.Name)
		processEntry.alive = true
		processEntry.healthStats.lastConfirmedAlive = beginningTime
		processEntry.healthStats.totalRestarts = -1 // First startup will increment this to zero
		processEntry.healthStats.copsRestarts = 0
		processJurisdiction[process.Name] = &processEntry
	}

	// Initialize dependency list for each process
	for _, p := range processJurisdiction {
		if err := p.updateDependencies(); err != nil {
			return fmt.Errorf("updating %s dependency list: %w", p.name, err)
		}
	}

	dr.configure(float64(config.HeartbeatFrequencyMS))
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
func startAllTickers(config Config) (heartRate, patrolRate, briefingRate, statsUpdateRate *time.Ticker) {
	heartRate = startTicker(config.HeartbeatFrequencyMS)
	patrolRate = startTicker(config.PatrolFrequencyMS)
	briefingRate = startTicker(config.BriefingFrequencyMS)
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
