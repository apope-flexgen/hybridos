/**
 * Central Operating Process Supervisor (COPS)
 *
 * Created January 2021
 *
 * COPS is a process supervisor that checks on other processes
 * using a heartbeat to make sure they are still running. If a
 * process becomes hung or fails, COPS will take action. Failure
 * actions include sending an alarm notification to the events
 * module, printing a warning to the terminal, and sending a
 * kill command to the system process so that system services
 * can hopefully restart the process.
 *
 * COPS also records statistics about the health of the processes
 * that it monitors. COPS will publish these statistics periodically
 * over FIMS on the URI /cops/processStats. They can also be
 * accessed with a FIMS GET request.
 *
 * Running List of All Threads Running in COPS
 * - fimsListen / processFIMS
 * - manageHeartbeats
 * - patrolProcesses
 * - statisticsReport (long, perhaps opportunity for optimization)
 * - connectOverTCP / processC2C
 *
 */

package main

import (
	"fims"
	"flag"
	"fmt"
	"os"
	"path"
	"reflect"
	"strings"
	"time"

	"github.com/flexgen-power/go_flexgen/cfgfetch"
	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
)

// cfg is the struct to unmarshal cops.json into
type cfg struct {
	controllerName          string
	heartbeatFrequencyMS    int
	patrolFrequencyMS       int
	briefingFrequencyMS     int
	c2cMsgFrequencyMS       int
	temperatureSource       string
	enableRedundantFailover bool
	primaryIP               string
	primaryNetworkInterface string
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

// Contains release version info about a process
type processVersion struct {
	tag    string
	commit string
	build  string
}

// Information about each process used and updated by COPS
type processInfo struct {
	name                     string             // process name
	uri                      string             // URI of the data structure holding the COPS info at the process
	replyToURI               string             // URI that heartbeat replies from this process to COPS will use
	writeOutC2C              []string           // URIs that should be written out to C2C by primary to maintain data
	heartbeat                uint               // Current COPS heartbeat value
	pid                      int                // Process ID used to update processPtr
	processPtr               *os.Process        // Pointer to system process which can be used to kill process
	alive                    bool               // True if process is passing heartbeat checks. False if process is dead/hung
	killOnHang               bool               // if true, send a kill command to the process pointer if process fails check
	requiredForHealthyStatus bool               // if true, process is included in health checkup to determine if controller is healthy or unhealthy
	hangTimeAllowance        time.Duration      // duration of time process can be missing the heartbeat before declared hung/dead
	configRestart            bool               // Whether new configuration data requires the process to restart
	healthStats              processHealthStats // statistics to be published about the process's health
	version                  processVersion     // contains release version info about the process
}

// Error wrapper to distinguish configuration issues that should be fatal
type configurationError struct {
	err error
}

// Global variables
var cfgDir = "/usr/local/etc/config/cops"
var processJurisdiction map[string]*processInfo // Map of processes that will be checked to see if they are up and running
var f fims.Fims                                 // FIMS connection
var beginningTime time.Time                     // Timestamp of when COPS started running
var updatedProcesses []*processInfo
var heartRate, patrolRate, briefingRate, statsUpdateTicker *time.Ticker // Tickers for main loop

func main() {
	// parse command line
	cfgSource := parseFlags()

	// initialize logger config
	err := log.InitConfig("cops").Init("cops")
	if err != nil {
		fmt.Printf("COPS log initialization failed with error: %v\n", err)
		os.Exit(-1)
	}

	// Configuration & initialization
	fimsReceive := configureFIMS()

	log.Infof("Retrieving configuration from %s...", cfgSource)
	configuration, err := cfgfetch.Retrieve("cops", cfgSource)
	if err != nil {
		log.Fatalf("Error retrieving configuration data: %v.", err)
	}
	log.Infof("Retrieved the following configuration data: %+v.", configuration)

	if err = handleConfiguration(configuration); err != nil {
		log.Fatalf("Error handling configuration: %v.", err)
	}

	getDBIUpdateModeStatus()

	// Operating loop
	for {
		select {
		case msg := <-fimsReceive:
			processFIMS(msg)
		case <-heartRate.C:
			manageHeartbeats()
		case <-patrolRate.C:
			patrolProcesses()
		case <-briefingRate.C:
			go publishBriefing()
		case <-statsUpdateTicker.C:
			go updateResourceUsageData()
		}
	}
}

func parseFlags() (cfgSource string) {
	// cops config file
	cfgUsage := "Give a path to the config file or 'dbi' if config is stored in the database"
	flag.StringVar(&cfgSource, "c", "dbi", cfgUsage)
	flag.StringVar(&cfgSource, "config", "dbi", cfgUsage)
	// logger config file
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	flag.Parse()
	return cfgSource
}

// Configure FIMS connection
func configureFIMS() (fimsReceive chan fims.FimsMsg) {
	var err error
	// Connect to FIMS
	f, err = fims.Connect("COPS")
	fatalErrorCheck(err, "Unable to connect to FIMS server")

	// Subscribe to messages targeted at COPS
	err = f.Subscribe("/cops")
	fatalErrorCheck(err, "Unable to subscribe to FIMS")

	// Start a FIMS Receive channel that will be used to accept responses to GET requests
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
	return
}

// Configure internal variables
func configureCOPS(config cfg) error {
	beginningTime = time.Now()

	controllerName = config.controllerName
	if enableRedundantFailover = config.enableRedundantFailover; enableRedundantFailover {
		// Store the IP address of the primary
		primaryIP = config.primaryIP
		primaryNetworkInterface = config.primaryNetworkInterface

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

// Print a message if there is a fatal error and panic
func fatalErrorCheck(err error, message string) {
	if err != nil {
		log.Panicf("Panicking. Error: %v, Message: %s", err, message)
	}
}

// Sends GETs to all processes to request heartbeat values and PIDs
func getHeartbeats() {
	for _, process := range processJurisdiction {
		targetURI := path.Join(process.uri, "cops")
		f.SendGet(targetURI, process.replyToURI)
	}
}

// Returns if an interface is a map with string keys and interface{} elements
func isMapStringInterface(m interface{}) bool {
	var i interface{}
	return reflect.TypeOf(m) == reflect.MapOf(reflect.TypeOf("string"), reflect.TypeOf(&i).Elem())
}

// Returns if process is hung or dead
func (process processInfo) isHungOrDead() bool {
	return time.Since(process.healthStats.lastConfirmedAlive) > process.hangTimeAllowance
}

// Returns if process is marked as dead but is neither hung nor dead
func (process processInfo) isResurrected() bool {
	return !process.isHungOrDead() && !process.alive
}

// Returns if process that is marked as alive is still neither hung nor dead
func (process processInfo) isStillAlive() bool {
	return !process.isHungOrDead() && process.alive
}

// Returns if process that is marked as dead is still either hung or dead
func (process processInfo) isStillHungOrDead() bool {
	return process.isHungOrDead() && !process.alive
}

// Sends a system kill command targeting the passed-in process
func kill(process *processInfo) error {
	if process.processPtr == nil {
		return fmt.Errorf("process pointer is nil, not possible to send kill command to %s", process.name)
	}
	err := process.processPtr.Kill()
	if err != nil {
		return fmt.Errorf("error when trying to kill %s: %w", process.name, err)
	}
	return nil
}

// Kill the process after `delay` milliseconds have expired
func delayedkill(process *processInfo, delay int) {
	time.Sleep(time.Duration(delay) * time.Millisecond)
	err := kill(process)
	if err != nil {
		log.Errorf("Failed to send delayed kill signal to %s: %v", process.name, err)
	}
}

// Sends latest heartbeats to processes then requests the processes for their recorded heartbeats
func manageHeartbeats() {
	sendHeartbeats()
	getHeartbeats()
}

// Parse the heartbeat out of a SET message
func parseHeartbeat(body interface{}) (receivedHeartbeat uint, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, "cops_heartbeat", reflect.TypeOf(receivedHeartbeat))
	if errMsg != "" {
		return 0, errMsg
	}
	return uint(extractedVal.Uint()), ""
}

// Parse the PID out of a SET message
func parsePID(body interface{}) (receivedPID int, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, "pid", reflect.TypeOf(receivedPID))
	if errMsg != "" {
		return 0, errMsg
	}
	return int(extractedVal.Int()), ""
}

// Parse a string value from a map
func parseStringFromMap(key string, body interface{}) (receivedString string, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, key, reflect.TypeOf(receivedString))
	if errMsg != "" {
		return "", errMsg
	}
	return extractedVal.String(), ""
}

// Extracts a specified field from a map[string]interface{}, checks that it is of the expected type, and returns it as a Value
func extractMapStringInterfaceValue(i interface{}, fieldName string, fieldType reflect.Type) (v reflect.Value, errMsg string) {
	if !isMapStringInterface(i) {
		errMsg = "interface{} is not a map[string]interface{}"
		return
	}
	field, ok := i.(map[string]interface{})[fieldName]
	if !ok {
		errMsg = fmt.Sprintf("%v field not found in map[string]interface{}", fieldName)
		return
	}
	rawVal := reflect.ValueOf(field)
	if !rawVal.Type().ConvertibleTo(fieldType) {
		errMsg = fmt.Sprintf("%v field in map[string]interface{} is not of proper type", fieldName)
		return
	}
	return rawVal.Convert(fieldType), ""
}

// Checks health status of each process and calls failure actions if necessary
func patrolProcesses() {
	for _, process := range processJurisdiction {
		if process.isStillHungOrDead() {
			log.Infof("Failure: Process %s still dead.", process.name)
		} else if process.isResurrected() {
			log.Infof("Success: Process %s resurrected.", process.name)
			process.alive = true
			process.sendPrimaryFlag(controllerMode == Primary)
		} else if process.isHungOrDead() {
			takeFailureAction(process)
		}
	}
}

// Starting point for handling any and all incoming FIMS messages
func processFIMS(msg fims.FimsMsg) {
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Error processing FIMS message: %v", r)
		}
	}()
	switch msg.Method {
	case "set":
		err := handleSet(msg)
		if err != nil {
			if configErr, ok := err.(*configurationError); ok {
				log.Fatalf("Fatal error: %v", configErr)
			} else {
				log.Errorf("Error: %v", err)
			}
		}
	case "get":
		handleGet(msg)
	default:
		log.Warnf("Received FIMS message that was not a SET. No action taken.")
	}
}

// Put all FIMS GET hooks here
func handleGet(msg fims.FimsMsg) {
	switch msg.Uri {
	case "/cops/processStats":
		f.SendSet(msg.Replyto, "", buildHealthStatsMap())
	case "/cops/healthScore":
		f.SendSet(msg.Replyto, "", dr.healthCheckup())
	case "/cops/status":
		f.SendSet(msg.Replyto, "", statusNames[controllerMode])
	case "/cops/controller_name":
		f.SendSet(msg.Replyto, "", controllerName)
	case "/cops":
		f.SendSet(msg.Replyto, "", buildBriefingReport())
	default:
		log.Errorf("Error in handleSet: message URI doesn't match any known patterns. msg.uri: %s", msg.Uri)
	}
}

// Put all FIMS SET hooks here
func handleSet(msg fims.FimsMsg) error {
	switch {
	// Set received from UI or command line enabling or disabling Update mode
	case msg.Uri == "/cops/update_mode":
		return handleUpdateMode(msg)
	case msg.Uri == "/cops/configuration":
		// verify type of FIMS msg body
		body, ok := msg.Body.(map[string]interface{})
		if !ok {
			return &configurationError{fmt.Errorf("expected DBI configuration of the form map[string]interface{}, but received %T", body)}
		}
		// Ignore empty response {} from DBI
		if len(body) == 0 {
			return &configurationError{fmt.Errorf("expected DBI configuration document, but it was empty")}
		}
		return &configurationError{handleConfiguration(body)}
	// Check for DBI update response first
	// Check uri is one of /cops/<process_name>/dbi_update_complete or <writeouturi>/dbi_update
	case msg.Nfrags == 3 && strings.HasSuffix(msg.Uri, "dbi_update_complete"):
		_, processKnown := processJurisdiction[msg.Frags[1]]
		if processKnown {
			handleDBIUpdate(msg)
			return nil
		}

	case msg.Nfrags == 3 && msg.Frags[1] == "heartbeat":
		process := processJurisdiction[msg.Frags[2]]
		if err := handleHeartbeatReply(msg, process); err != nil {
			return fmt.Errorf("failed to handle heartbeat reply from process %s: %w", msg.Frags[2], err)
		}
		return nil

	// This check of the uri against <writeoutui>/dbi_update needs to happen before the check used
	// for Setpoints messages against just the writeOut uri, otherwise this check won't be reached
	case strings.HasSuffix(msg.Uri, "dbi_update"):
		for _, process := range processJurisdiction {
			// If more efficiency is needed, the writeout uris could be put into a hashset at configuration to speed up this check
			for _, writeOutUri := range process.writeOutC2C {
				if strings.HasPrefix(msg.Uri, writeOutUri) {
					handleDBIUpdate(msg)
					return nil
				}
			}
		}
	// Cops heartbeat response and setpoint
	// Check uri is of form /cops/<process_name>
	// TODO: give heartbeats a more specific endpoint, but this will require changes in other services

	case msg.Nfrags > 1:
		process, processKnown := processJurisdiction[msg.Frags[1]]
		if processKnown {
			// Check uri contains one of <writeouturi>
			for _, writeOutUri := range process.writeOutC2C {
				if strings.Contains(msg.Uri, writeOutUri) {
					handleSetpointWriteout(msg, process)
					return nil
				}
			}
		}

	default:
		// If not caught by any of the above checks, then the set is invalid
		log.Errorf("Error in handleSet: message URI doesn't match any known patterns. msg.uri: %s", msg.Uri)
	}
	return nil
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

// Handles the response back from dbi with the latest configuration data
func handleDBIUpdate(msg fims.FimsMsg) {
	// As the primary, the other controller (secondary) has requested our latest config on its startup
	// This controller sent the request to DBI, and has now received the response for each uri (document) requested

	// Send the document we got from DBI for this uri
	// Sets will be received and written through C2C one-by-one, with the last set containing the uri keyword /dbi_update_complete
	sendC2CMsg(fimsToC2C("SetDBIUpdate", msg.Uri, msg.Body))
}

// Handles writing out setpoint data. If in Update mode, also tracks any processes that have been updated in order to restart them as needed
func handleSetpointWriteout(msg fims.FimsMsg, process *processInfo) {
	// Writeout our setpoint sent from our process to the other process
	if controllerMode == Primary {
		// Convert the setpoint message to a C2C message and send to the secondary process
		sendC2CMsg(fimsToC2C("Setpoints", msg.Uri, msg.Body))
		return
	} else if controllerMode == Update {
		// If updating, record all processes that have received updates so they can be restarted on exiting the mode
		// This will include both the configuration changes uploaded to this Update controller,
		// and any setpoints that may be received from the primary while updating
		updatedProcesses = append(updatedProcesses, process)
	}
}

// Handle a set to update mode coming from either ansible or dbi
func handleUpdateMode(msg fims.FimsMsg) error {
	var interfaceBody interface{}
	// Parse response from dbi that will be parsed as map[string]interface{}
	if mapBody, ok := msg.Body.(map[string]interface{}); ok {
		// Ignore empty response when no document exists
		if len(mapBody) == 0 {
			return nil
		}
		if interfaceBody, ok = mapBody["update_mode"]; !ok {
			return fmt.Errorf("failed to extract update_mode entry from map %v", mapBody)
		}
	} else {
		interfaceBody = msg.Body
	}
	// Parse single bool sent from ansible or by hand
	body, ok := interfaceBody.(bool)
	if !ok {
		return fmt.Errorf("expected Update Mode value to be a bool, but received %T instead", msg.Body)
	}

	// Save the update mode bool received
	f.SendSet("/dbi/cops/update_mode", "", map[string]interface{}{"update_mode": body})

	// Update mode enabled
	if body {
		log.Infof("Locking this controller into update mode")
		controllerMode = Update
	} else if controllerMode == Update {
		log.Infof("Exiting this controller from update mode")
		// Received exit update mode signal
		for _, process := range updatedProcesses {
			// If the process requires a restart in order to properly configure such as site controller, restart it
			if process.configRestart {
				// The process may have no fully started up with a PID yet, delay it's restart then kill to ensure
				// all of the latest configuration has been read
				err := kill(process)
				if err != nil {
					log.Errorf("Failed to send kill signal to %s in attempt to ensure latest configuration: %v", process.name, err)
				}
			} else {
				// Notify the process's (optional) dbi_update endpoint
				// This endpoint is used for processes that support reading configuration data on the fly such as scheduler
				f.SendSet(path.Join(process.uri, "/operation/dbi_update"), "", true)
			}
		}
		// Clear the list of updated processes, processes still maintained in processJurisdiction
		updatedProcesses = nil
		// Now that this controller has fully configured with new data, take over as primary
		// This will also sync the db of other controller (now secondary) with this controller's updated db
		takeOverAsPrimary()
	}
	return nil
}

// Handles the given heartbeat reply message and associated process
func handleHeartbeatReply(msg fims.FimsMsg, process *processInfo) error {
	if process == nil {
		return fmt.Errorf("error: process is nil when calling function handleHeartbeatReply")
	}
	receivedHeartbeat, errMsg := parseHeartbeat(msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseHeartbeat: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.updateHeartbeat(receivedHeartbeat)
	readPID, errMsg := parsePID(msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parsePID: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.updatePID(readPID)
	readVersionTag, errMsg := parseStringFromMap("version_tag", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.tag = readVersionTag
	readVersionCommit, errMsg := parseStringFromMap("version_commit", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.commit = readVersionCommit
	readVersionBuild, errMsg := parseStringFromMap("version_build", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.build = readVersionBuild
	return nil
}

// Sends an alarm notification to the events module via FIMS
func sendAlarm(message string) {
	// Build notification JSON
	body := make(map[string]interface{})
	body["source"] = "COPS"
	body["message"] = message
	body["severity"] = 3 // severity of 3 is for alarms

	// Send notification
	_, err := f.Send(fims.FimsMsg{
		Method: "post",
		Uri:    "/events",
		Body:   body,
	})
	if err != nil {
		log.Errorf("Error sending event notificiation: %v", err)
	}
}

// Sends SETs to all processes with their heartbeat values
func sendHeartbeats() {
	for _, process := range processJurisdiction {
		targetURI := path.Join(process.uri, "cops/cops_heartbeat")
		f.SendSet(targetURI, "", process.heartbeat)
	}
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

// Takes necessary failure actions on a process that has been newly declared hung or dead
func takeFailureAction(process *processInfo) {
	log.Infof("Failure: Process %s declared hung or dead. Last confirmed alive at %s. Sending alarm notification to events.", process.name, process.healthStats.lastConfirmedAlive.String())
	processHungMsg := fmt.Sprintf("Process %s is hung or dead (last confirmed alive at %v).", process.name, process.healthStats.lastConfirmedAlive.Round(time.Millisecond))
	if process.killOnHang {
		sendAlarm(processHungMsg + " Sending kill signal.")
		err := kill(process)
		if err != nil {
			log.Errorf("Failed to send kill signal to %s: %v", process.name, err)
			sendAlarm(fmt.Sprintf("Failed to send kill signal to process %s.", process.name))
		} else {
			sendAlarm(fmt.Sprintf("Successfully sent kill signal to process %s.", process.name))
			process.healthStats.copsRestarts++
		}
	} else {
		sendAlarm(processHungMsg)
	}
	process.alive = false
}

// Put "defer timeTaken(time.Now(), "funcName")" at the beginning of a long function to measure execution time
//lint:ignore U1000 ignore "unused function" warning as this function is kept to be used by coders during development
func timeTaken(t time.Time, name string) {
	elapsed := time.Since(t)
	log.Infof("TIME: %s took %s\n", name, elapsed)
}

// Update the COPS process heartbeat value if a received heartbeat value matches it
func (process *processInfo) updateHeartbeat(receivedHeartbeat uint) {
	if process.heartbeat == receivedHeartbeat {
		process.heartbeat++
		process.recordResponse()
	}
}

// Update the process pointer if a new PID is received. Record restart data
func (process *processInfo) updatePID(readPID int) {
	if process.pid != readPID {
		var err error
		process.pid = readPID
		process.processPtr, err = os.FindProcess(readPID)
		fatalErrorCheck(err, "Error updating process pointer for "+process.name)
		process.recordRestart()
		if controllerMode == Primary {
			process.sendPrimaryFlag(true)
		}
	}
}

// Check if the controller was previously put into update mode
// If no response is received, simply continue
func getDBIUpdateModeStatus() {
	err := f.SendGet("/dbi/cops/update_mode", "/cops/update_mode")
	if err != nil {
		log.Errorf("error getting update_mode status from dbi: %v", err)
	}
}

// As the primary, the other controller (secondary) has requested our latest setpoints on its startup
func getDBIUpdate() {
	for _, process := range processJurisdiction {
		// Get all the latest config from DBI
		for _, writeOutUri := range process.writeOutC2C {
			// Get whatever is stored in DBI for each of the process's write out uris
			_, data := path.Split(writeOutUri)
			// The response will be returned back to this COPS with the final fragment dbi_update from our replyto
			// Then written through to the other controller via C2C
			f.SendGet(path.Join("/dbi/", process.name, data), path.Join(writeOutUri, "/dbi_update"))
		}
		// The above will produce an undetermined number of responses back to cops with the keyword dbi_update
		// To resolve the issue of uncertainty as to when the last response has been received, send one additional response
		// that will be handled last, and mark the end of the documents response for this process
		// Will produce the response: SET /cops/<process name>/dbi_update_complete {}
		f.SendGet("/dbi/update/complete", path.Join("/cops/", process.name, "/dbi_update_complete"))
	}
}

// Pub all process health statistics over FIMS
func publishBriefing() {
	// prevent a failure here from crashing all of COPS
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Error with briefing publish: %v", r)
		}
	}()
	// publish summary data
	f.SendPub("/cops/summary", buildBriefingReport())
	// publish process health statistics
	for _, process := range processJurisdiction {
		f.SendPub("/cops/stats/"+process.name, process.buildStatsReport())
	}
}

// Builds a map containing general information desired by external processes
func buildBriefingReport() map[string]interface{} {
	briefingReport := make(map[string]interface{})
	// Add the controller info
	if len(controllerName) != 0 {
		briefingReport["controller_name"] = controllerName
	}
	briefingReport["status"] = statusNames[controllerMode]
	// Add the temperature only if a source is provided
	if len(tempSource) != 0 {
		systemTemp, err := readSystemTemp()
		if err != nil {
			log.Errorf("Error: %v", err)
		} else {
			briefingReport["system_temp"] = systemTemp
		}
	}
	return briefingReport
}

// `getProcess` searches the processJurisdiction for a process with the given
// name and returns a pointer to it. If not found, returns nil.
func getProcess(name string) *processInfo {
	for _, process := range processJurisdiction {
		if process.name == name {
			return process
		}
	}
	return nil
}

func (m *configurationError) Error() string {
	return m.err.Error()
}
