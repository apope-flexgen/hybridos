package main

import (
	"encoding/json"
	"errors"
	"fims"
	"fmt"
	"net"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	"github.com/flexgen-power/scheduler/internal/websockets"
)

// Wraps the configuration for both a WebSocket server's configuration and a WebSocket client's configuration.
type webSocketsConfig struct {
	ServerConfig  *webSocketServerConfig     `json:"server,omitempty"`
	ClientConfigs *webSocketClientConfigList `json:"clients,omitempty"`
}

// Contains all information needed to host a WebSocket server.
type webSocketServerConfig struct {
	Enabled bool `json:"enabled"` // used to turn the HTTPS server on/off
	Port    int  `json:"port"`    // port on which the HTTPS server will be served
}

// Contains the configuration settings for a single WebSocket client.
type webSocketClientConfig struct {
	Id   string `json:"id"`
	Name string `json:"name"`
	Ip   string `json:"ip"`
	Port int    `json:"port"`
}

// Contains the collection of WebSocket client configuration settings.
type webSocketClientConfigList []webSocketClientConfig

// returns IDs of all schedules the client config list contains
func (cfgList webSocketClientConfigList) getIds() []string {
	idList := make([]string, 0, len(cfgList))
	for _, cfg := range cfgList {
		idList = append(idList, cfg.Id)
	}
	return idList
}

func reconfigureWebSockets(cfg webSocketsConfig) {
	reconfigureWebSocketServer(cfg.ServerConfig)
	reconfigureWebSocketClients(cfg.ClientConfigs)
	f.SendPub("/scheduler/connected", buildConnectionMap())
}

func reconfigureWebSocketClients(cfgList *webSocketClientConfigList) {
	schedCfg.WebSockets.ClientConfigs = cfgList
	if cfgList == nil {
		return
	}

	// connect to sites
	for _, clientCfg := range *cfgList {
		createScheduleIfNew(clientCfg.Id, clientCfg.Name)
		websockets.Clients.Add(websockets.NewClient(clientCfg.Id, clientCfg.Ip, clientCfg.Port))
	}

	// only connect to the Site Controller if in primary controller mode, as a secondary controller should not have web connections
	if isPrimaryScheduler {
		websockets.Clients.LaunchAll()
	}
}

// resets the WebSocket server with new config settings
func reconfigureWebSocketServer(cfg *webSocketServerConfig) {
	schedCfg.WebSockets.ServerConfig = cfg
	// disabled server or secondary controller should not start the server
	if cfg == nil || !cfg.Enabled || !isPrimaryScheduler {
		return
	}

	err := websockets.StartServer(cfg.Port)
	if err != nil {
		log.Errorf("Error: failed to start server: %v.", err)
	}
}

// Hanldes FIMS POSTs to URIs that begin with /scheduler/configuration/web_sockets.
func (cfg *webSocketsConfig) handlePost(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	switch msg.Frags[3] {
	case "clients":
		if err := cfg.ClientConfigs.handlePost(msg); err != nil {
			return fmt.Errorf("failed to handle POST to client configurations: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS DELs to URIs that begin with /scheduler/configuration/web_sockets.
func (cfg *webSocketsConfig) handleDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	switch msg.Frags[3] {
	case "clients":
		if err := cfg.ClientConfigs.handleDel(msg); err != nil {
			return fmt.Errorf("failed to handle DEL to client configurations: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS GETs to URIs that begin with /scheduler/configuration/web_sockets.
func (cfg *webSocketsConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 3 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	switch msg.Frags[3] {
	case "clients":
		if err := cfg.ClientConfigs.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSockets clients GET: %w", err)
		}
	case "server":
		if err := cfg.ServerConfig.handleGet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSockets server GET: %w", err)
		}
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS SETs to URIs that begin with /scheduler/configuration/web_sockets.
func handleWebSocketsConfigSet(msg fims.FimsMsg) (err error) {
	if msg.Nfrags < 3 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// SET is to /scheduler/configuration/web_sockets
	if msg.Nfrags == 3 {
		newCfg, err := parseWebSocketsConfig(msg.Body)
		if err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to parse Scheduler WebSockets config object: %w", err)
		}
		schedCfg.WebSockets = newCfg
		// TODO: evaluate if we really do need to do an entire reconfiguration
		reconfigureScheduler(schedCfg)
		sendReply(msg.Replyto, schedCfg.WebSockets)
		return nil
	}

	switch msg.Frags[3] {
	case "clients":
		if schedCfg.SchedulerType == SITE_SCHEDULER {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return errors.New("cannot SET WebSocket clients as Site Scheduler")
		}
		if err := handleWebSocketClientsConfigSet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSocket clients config SET: %w", err)
		}
	case "server":
		if schedCfg.SchedulerType == FLEET_SCHEDULER {
			sendErrorResponse(msg.Replyto, "Invalid URI")
			return errors.New("cannot SET WebSocket server as Fleet Scheduler")
		}
		if err := handleWebSocketServerConfigSet(msg); err != nil {
			return fmt.Errorf("failed to handle WebSocket server config SET: %w", err)
		}
	default:
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS GETs to URIs that begin with /scheduler/configuration/web_sockets/server.
func (cfg *webSocketServerConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 || cfg == nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	if cfg == nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return errors.New("configuration is nil")
	}

	switch msg.Frags[4] {
	case "enabled":
		sendReply(msg.Replyto, cfg.Enabled)
	case "port":
		sendReply(msg.Replyto, cfg.Port)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS SETs to URIs that begin with /scheduler/configuration/web_sockets/server.
func handleWebSocketServerConfigSet(msg fims.FimsMsg) (err error) {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		// SET is to /scheduler/configuration/web_sockets/server
		newCfg, err := parseWebSocketServerConfig(msg.Body)
		if err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to parse Scheduler WebSocket server config object: %w", err)
		}
		schedCfg.WebSockets.ServerConfig = &newCfg
		// TODO: do not need to totally reconfigure entire Scheduler since only one connection is being edited
		reconfigureScheduler(schedCfg)
		sendReply(msg.Replyto, schedCfg.WebSockets.ServerConfig)
		return nil
	}

	switch msg.Frags[4] {
	case "enabled":
		enabled, ok := msg.Body.(bool)
		if !ok {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to parse enabled value from body. expected bool but got %T", msg.Body)
		}
		schedCfg.WebSockets.ServerConfig.Enabled = enabled
		sendReply(msg.Replyto, enabled)
	case "port":
		port, err := parsemap.CastToInt(msg.Body)
		if err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to cast %v to int: %w", msg.Body, err)
		}
		if port <= 0 {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("port must be positive but got %d", port)
		}
		schedCfg.WebSockets.ServerConfig.Port = port
		sendReply(msg.Replyto, port)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	// TODO: do not need to totally reconfigure entire Scheduler since only one connection is being edited
	reconfigureScheduler(schedCfg)
	return nil
}

// Handles FIMS DELs to URIs beginning with /scheduler/configuration/web_sockets/clients.
func (cfg *webSocketClientConfigList) handleDel(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		// DEL to /scheduler/configuration/web_sockets/clients deletes all clients
		*cfg = make(webSocketClientConfigList, 0)
	} else {
		targetClientId := msg.Frags[4]
		cfg.deleteClient(targetClientId)
	}

	// TODO: evaluate if we really do need to do an entire reconfiguration.
	// this func also does backup to DBI but sends entire configuration instead of just the new client list so that should be made to be more precise
	reconfigureScheduler(schedCfg)

	sendReply(msg.Replyto, cfg)
	return nil
}

// Deletes the client with the given ID from the list.
func (cfg *webSocketClientConfigList) deleteClient(id string) {
	for i, client := range *cfg {
		if client.Id == id {
			*cfg = append((*cfg)[:i], (*cfg)[i+1:]...)
			return
		}
	}
}

// Handles FIMS POSTs to /scheduler/configuration/web_sockets/clients.
func (cfg *webSocketClientConfigList) handlePost(msg fims.FimsMsg) error {
	if msg.Nfrags != 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var newClientCfg webSocketClientConfig
	if err = json.Unmarshal(jsonBytes, &newClientCfg); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if newClientCfg.Id == "" {
		newClientCfg.generateId()
	}

	if err = newClientCfg.validate(); err != nil {
		sendErrorResponse(msg.Replyto, "Invalid Data")
		return fmt.Errorf("invalid config: %w", err)
	}

	// add new config to shallow copy of config list and validate it (checks that no clients have the same ID)
	newCfgList := append(*cfg, newClientCfg)
	if err = newCfgList.validate(); err != nil {
		sendErrorResponse(msg.Replyto, "Client Already Exists")
		return fmt.Errorf("failed to validate config list with new config merged in: %w", err)
	}

	schedCfg.WebSockets.ClientConfigs = &newCfgList

	// TODO: evaluate if we really do need to do an entire reconfiguration.
	// this func also does backup to DBI but sends entire configuration instead of just the new client list so that should be made to be more precise
	reconfigureScheduler(schedCfg)

	sendReply(msg.Replyto, newCfgList)
	return nil
}

// Handles FIMS GETs to URIs that begin with /scheduler/configuration/web_sockets/clients.
func (cfg *webSocketClientConfigList) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 || cfg == nil {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	targetClientId := msg.Frags[4]
	targetClient := cfg.getClientCfg(targetClientId)
	if targetClient == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("could not find a WebSocket client configuration with ID %s", targetClientId)
	}

	if err := targetClient.handleGet(msg); err != nil {
		return fmt.Errorf("failed to handle GET to WebSocket client %s's configuration: %w", targetClientId, err)
	}
	return nil
}

// Handles FIMS SETs to URIs that begin with /scheduler/configuration/web_sockets/clients.
func handleWebSocketClientsConfigSet(msg fims.FimsMsg) (err error) {
	if msg.Nfrags < 4 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 4 {
		if err = schedCfg.WebSockets.handleSetToEntireClientList(msg.Body, msg.Replyto); err != nil {
			return fmt.Errorf("failed to handle SET to client list: %w", err)
		}
	}

	targetClientId := msg.Frags[4]
	targetClient := schedCfg.WebSockets.ClientConfigs.getClientCfg(targetClientId)
	if targetClient == nil {
		sendErrorResponse(msg.Replyto, "Resource Not Found")
		return fmt.Errorf("could not find a WebSocket client configuration with ID %s", targetClientId)
	}

	if err := targetClient.handleSet(msg); err != nil {
		return fmt.Errorf("failed to handle SET to WebSocket client %s's configuration: %w", targetClientId, err)
	}

	// TODO: really do not need to reconfigure entire Scheduler for just a single client edit
	reconfigureScheduler(schedCfg)
	return nil
}

func (cfg *webSocketsConfig) handleSetToEntireClientList(input interface{}, replyTo string) error {
	jsonBytes, err := json.Marshal(input)
	if err != nil {
		sendErrorResponse(replyTo, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	var newCfgList webSocketClientConfigList
	if err = json.Unmarshal(jsonBytes, &newCfgList); err != nil {
		sendErrorResponse(replyTo, "Invalid Data")
		return fmt.Errorf("failed to unmarshal input: %w", err)
	}

	for i, client := range newCfgList {
		if client.Id == "" {
			newCfgList[i].generateId()
		}
	}

	if err = newCfgList.validate(); err != nil {
		sendErrorResponse(replyTo, "Invalid Data")
		return fmt.Errorf("invalid config: %w", err)
	}

	schedCfg.WebSockets.ClientConfigs = &newCfgList

	// TODO: evaluate if we really do need to do an entire reconfiguration.
	// this func also does backup to DBI but sends entire configuration instead of just the new client list so that should be made to be more precise
	reconfigureScheduler(schedCfg)

	sendReply(replyTo, newCfgList)
	return nil
}

func (cfg *webSocketsConfig) validate(schedulerType string) error {
	if schedulerType == SITE_SCHEDULER {
		if err := cfg.validateSiteSchedulerWebSocketsCfg(); err != nil {
			return fmt.Errorf("invalid cfg for Site Scheduler: %w", err)
		}
	} else {
		if err := cfg.validateFleetSchedulerWebSocketsCfg(); err != nil {
			return fmt.Errorf("invalid cfg for Fleet Scheduler: %w", err)
		}
	}
	return nil
}

func (cfg *webSocketsConfig) validateSiteSchedulerWebSocketsCfg() error {
	cfg.ClientConfigs = nil
	if cfg.ServerConfig == nil {
		// default server config for Site Scheduler is disabled server with port 9000
		cfg.ServerConfig = &webSocketServerConfig{
			Enabled: false,
			Port:    9000,
		}
	}
	if err := cfg.ServerConfig.validate(); err != nil {
		return fmt.Errorf("invalid server config: %w", err)
	}
	return nil
}

func (cfg *webSocketsConfig) validateFleetSchedulerWebSocketsCfg() error {
	cfg.ServerConfig = nil
	if cfg.ClientConfigs == nil {
		cfg.ClientConfigs = &webSocketClientConfigList{}
	}
	if err := cfg.ClientConfigs.validate(); err != nil {
		return fmt.Errorf("invalid clients config: %w", err)
	}
	return nil
}

func parseWebSocketsConfig(inputJsonInterface interface{}) (cfg webSocketsConfig, err error) {
	jsonBytes, err := json.Marshal(inputJsonInterface)
	if err != nil {
		return cfg, fmt.Errorf("failed to marshal input: %w", err)
	}

	if err = json.Unmarshal(jsonBytes, &cfg); err != nil {
		return cfg, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = cfg.validate(schedCfg.SchedulerType); err != nil {
		return cfg, fmt.Errorf("invalid config: %w", err)
	}
	return cfg, nil
}

func (cfg *webSocketServerConfig) validate() error {
	if cfg == nil {
		return errors.New("nil pointer")
	}
	if cfg.Port <= 0 {
		return fmt.Errorf("port must be positive but got %d", cfg.Port)
	}
	return nil
}

func parseWebSocketServerConfig(inputJsonInterface interface{}) (cfg webSocketServerConfig, err error) {
	jsonBytes, err := json.Marshal(inputJsonInterface)
	if err != nil {
		return cfg, fmt.Errorf("failed to marshal input: %w", err)
	}

	if err = json.Unmarshal(jsonBytes, &cfg); err != nil {
		return cfg, fmt.Errorf("failed to unmarshal input: %w", err)
	}

	if err = cfg.validate(); err != nil {
		return cfg, fmt.Errorf("invalid config: %w", err)
	}
	return cfg, nil
}

func (cfgList webSocketClientConfigList) validate() error {
	if cfgList == nil {
		return errors.New("clients slice is nil")
	}
	ids := make(map[string]struct{})
	for i, cfg := range cfgList {
		if err := cfg.validate(); err != nil {
			return fmt.Errorf("client with index %d is invalid: %w", i, err)
		}
		if _, alreadyExists := ids[cfg.Id]; alreadyExists {
			return fmt.Errorf("client ID %s found more than once in config list %v", cfg.Id, cfgList)
		}
		ids[cfg.Id] = struct{}{}
	}
	return nil
}

func (cfgList webSocketClientConfigList) getClientCfg(searchId string) *webSocketClientConfig {
	for i, client := range cfgList {
		if client.Id == searchId {
			return &cfgList[i]
		}
	}
	return nil
}

// Sets the ID to be the same as the name but with all letters lower-cased and
// replacing slashes and spaces with underscores.
func (cfg *webSocketClientConfig) generateId() {
	cfg.Id = strings.ToLower(cfg.Name)
	cfg.Id = strings.ReplaceAll(cfg.Id, " ", "_")
	cfg.Id = strings.ReplaceAll(cfg.Id, "/", "_")
}

func (cfg *webSocketClientConfig) validate() error {
	if len(cfg.Id) == 0 {
		return errors.New("empty ID string")
	}
	if strings.ContainsAny(cfg.Id, "/ ") {
		return errors.New("ID cannot contain any slashes or spaces")
	}
	if len(cfg.Name) == 0 {
		return errors.New("empty name string")
	}
	if net.ParseIP(cfg.Ip) == nil {
		return fmt.Errorf("%s is invalid IP address", cfg.Ip)
	}
	if cfg.Port <= 0 {
		return fmt.Errorf("port must be positive but got %d", cfg.Port)
	}
	return nil
}

// Handles FIMS GETs to URIs that begin with /scheduler/configuration/web_sockets/clients/<client ID>.
func (cfg *webSocketClientConfig) handleGet(msg fims.FimsMsg) error {
	if msg.Nfrags < 5 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	if msg.Nfrags == 5 {
		sendReply(msg.Replyto, cfg)
		return nil
	}

	switch msg.Frags[5] {
	case "name":
		sendReply(msg.Replyto, cfg.Name)
	case "ip":
		sendReply(msg.Replyto, cfg.Ip)
	case "port":
		sendReply(msg.Replyto, cfg.Port)
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}
	return nil
}

// Handles FIMS SETs to URIs that begin with /scheduler/configuration/web_sockets/clients/<client ID>.
func (cfg *webSocketClientConfig) handleSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 5 {
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	jsonBytes, err := json.Marshal(msg.Body)
	if err != nil {
		sendErrorResponse(msg.Replyto, "Invalid JSON")
		return fmt.Errorf("failed to marshal input: %w", err)
	}

	if msg.Nfrags == 5 {
		var newSettings webSocketClientConfig
		if err = json.Unmarshal(jsonBytes, &newSettings); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid JSON")
			return fmt.Errorf("failed to unmarshal input to client configuration object: %w", err)
		}

		// edit to individual client cfg will not change ID. to change ID, must delete the client cfg and add a brand new one
		newSettings.Id = cfg.Id
		if err = newSettings.validate(); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to validate new client configuration settings: %w", err)
		}

		*cfg = newSettings
		// DBI does not currently support indexing into arrays via URIs. so for now, allow the call to `reconfigureScheduler`
		// (that happens when this function returns) to do a higher-level backup to DBI
		sendReply(msg.Replyto, cfg)
		return nil
	}

	endpoint := msg.Frags[5]
	var replyObject interface{}
	switch endpoint {
	case "name":
		var newName string
		if err = json.Unmarshal(jsonBytes, &newName); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new name: %w", err)
		}
		if newName == "" {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return errors.New("name cannot be empty string")
		}
		cfg.Name = newName
		replyObject = newName
	case "ip":
		var newIp string
		if err = json.Unmarshal(jsonBytes, &newIp); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new IP: %w", err)
		}
		if net.ParseIP(newIp) == nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("%s is invalid IP address", newIp)
		}
		cfg.Ip = newIp
		replyObject = newIp
	case "port":
		var newPort int
		if err = json.Unmarshal(jsonBytes, &newPort); err != nil {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("failed to unmarshal new port: %w", err)
		}
		if newPort <= 0 {
			sendErrorResponse(msg.Replyto, "Invalid Data")
			return fmt.Errorf("port must be positive but got %d", newPort)
		}
		cfg.Port = newPort
		replyObject = newPort
	default:
		sendErrorResponse(msg.Replyto, "Invalid URI")
		return ErrInvalidUri
	}

	// DBI does not currently support indexing into arrays via URIs. so for now, allow the call to `reconfigureScheduler`
	// (that happens when this function returns) to do a higher-level backup to DBI
	sendReply(msg.Replyto, replyObject)
	return nil
}
