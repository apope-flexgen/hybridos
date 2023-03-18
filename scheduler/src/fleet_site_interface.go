/**
 *
 * fleet_site_interface.go
 *
 * Interface between Schedulers on Fleet Manager boxes and Site Controller boxes
 *
 */

package main

import (
	"fmt"
	"log"
	"net/http"
	"reflect"
	"strings"
	"sync"
	"time"

	fg "github.com/flexgen-power/go_flexgen"
)

// configurationForm is just a map[string]interface{}, but it is still defined so that handler functions can be used for configuration actions.
type configurationForm map[string]interface{}

// These constants are used by the schedulerInstanceType global variable to indicate in what mode Scheduler is running.
const (
	SITE_WITH_SERVER = iota
	SITE_NO_SERVER
	FLEET_MANAGER
)

// schedulerInstanceType tracks if the Scheduler is representing a Fleet Manager,
// a Site Controller that is hosting an HTTPS server, or a Site Controller that is
// not hosting an HTTPS server.
//
// Scheduler starts up as a Site Controller w/o hosting a server, but configuration still needs to be completed to acquire the site id and enumerate it.
var schedulerInstanceType = SITE_NO_SERVER

// thisSiteId is only used if Scheduler has a SITE_WITH_SERVER or SITE_NO_SERVER instance type,
// as it is meant to hold the singular site id for a Site Controller instance of SCheduler.
var thisSiteId string

// thisTimezone is the time zone of the box on which Scheduler is running.
// It is only used if Scheduler is running as a Site Controller since Fleet
// Manager should be using the time zones of each of the sites it is managing.
var thisTimezone *time.Location

// thisPort is only used if Scheduler has a SITE_WITH_SERVER or SITE_NO_SERVER instance type, as it is meant to hold the singular port for a Site Controller instance of Scheduler to host its HTTPS server on
var thisPort string

// represents a Site Controller's connection to its Fleet Manager. It is only used if Scheduler has a SITE_WITH_SERVER instance type.
// Only one read and one write is allowed on a WebSocket at a time (one read + one write okay, two reads or two writes not okay) so must use atomic data structure.
var fleetManagerRW atomicWebSocketConn

// httpServer hosts a Site Controller's connection with its Fleet Manager. It is only used if Scheduler has a SITE_WITH_SERVER instance type.
var httpServer *http.Server

// httpServerExitDone will block the server shutdown thread from exiting until the server has had time to cleanup. It is only used if Scheduler has a SITE_WITH_SERVER instance type.
var httpServerExitDone *sync.WaitGroup

// lastScheduleModification keeps track of when the schedule was last modified by the UI, SCADA, or Fleet-Site interface.
// The timestamp is used to determine who should update who when a Fleet Man and Site Controller connect to each other.
// Its initial value is Jan 1, 0000 so that an unedited schedule will not overwrite an edited schedule.
var lastScheduleModification time.Time

///////////////////////////////////////////////////////////////////////////
////////////////////////////// Configuration //////////////////////////////
///////////////////////////////////////////////////////////////////////////

// configureFleetSiteInterface allocates space for the WebSocket channel and sets up the HTTP routes/handlers.
func configureFleetSiteInterface() {
	webSocketReceive = make(chan *webSocketMsg, 1)
	assignHTTPHandlers()
}

// buildSiteSchedulerConfigForm fills a map[string]interface{} with data for the Scheduler as a Site Scheduler.
func buildSiteSchedulerConfigForm(withPortNumber bool) map[string]interface{} {
	thisSite := masterSchedule.getSite(thisSiteId)
	if thisSite == nil {
		log.Println("No site with ID", thisSiteId, "found in master schedule")
		return map[string]interface{}{}
	}
	cf := make(map[string]interface{})
	cf["connection"] = "SC"
	cf["siteId"] = thisSiteId
	cf["siteName"] = thisSite.displayName
	if withPortNumber {
		if len(thisPort) <= 1 {
			cf["siteControllerPort"] = "ERROR"
		} else {
			cf["siteControllerPort"] = thisPort[1:]
		}
	}
	cf["setpointEnforcing"] = setpointEnforcing
	cf["setpointEnforcementFreqSeconds"] = setpointEnforcementFreqSeconds
	return cf
}

// buildFleetManConfigForm fills a map[string]interface{} with data for the Scheduler as a Fleet Manager.
func buildFleetManConfigForm() map[string]interface{} {
	cf := make(map[string]interface{})
	cf["connection"] = "FM"
	sites := make([]interface{}, 0)
	for _, site := range masterSchedule.getSites() {
		sites = append(sites, site.buildWsConfigObj())
	}
	cf["sites"] = sites
	return cf
}

// configureSiteController closes old HTTPS connections and establishes Scheduler
// as a Site Controller with the id provided in the configurationForm. If the
// port field is found, Scheduler hosts an HTTPS server to allow for a connection
// to Fleet Manager.
func configureSiteController(cf configurationForm) error {
	// extract site id
	id, err := fg.ExtractValueWithType(cf.castToMap(), "siteId", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract siteId from configuration form: %v", err.Error())
	}

	// extract site name
	name, err := fg.ExtractValueWithType(cf.castToMap(), "siteName", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract siteName from configuration form: %v", err.Error())
	}

	// extract setpointEnforcing flag. Default to false if not found
	setpointEnforcingInterface, err := fg.ExtractValueWithType(cf.castToMap(), "setpointEnforcing", fg.BOOL)
	if err != nil {
		log.Printf("'setpointEnforcing' not found in configuration form for site %s. Defaulting to false.\n", name.(string))
		setpointEnforcingInterface = false
	}
	newSetpointEnforcing := setpointEnforcingInterface.(bool)

	// extract setpointEnforcementFreqSeconds. Only required if setpointEnforcing is true
	newSetpointEnforcementFreqSeconds, err := extractIntThatMightBeFloat64(cf.castToMap(), "setpointEnforcementFreqSeconds")
	if err != nil {
		if newSetpointEnforcing {
			return fmt.Errorf("setpointEnforcing was set to true but no setpointEnforcementFreqSeconds found")
		} else {
			newSetpointEnforcementFreqSeconds = 360000 // 100 hours. arbitrarily large since it will not be a useful select case in main() while setpointEnforcing is false
		}
	}

	// if configured to host an HTTPS server, extract the port number. Otherwise, default it to ""
	var port string
	var portFound bool
	// extract the port on which to host the HTTP server
	port, err = cf.extractPort()
	if err != nil {
		portFound = false
		port = "" // set the port to "" so old port number gets wiped
	} else {
		portFound = true
	}

	// lock in extracted config data
	thisSiteId = id.(string)
	thisPort = port
	setpointEnforcing = newSetpointEnforcing
	setpointEnforcementFreqSeconds = newSetpointEnforcementFreqSeconds
	setpointEnforcementFreq := time.Duration(time.Second) * time.Duration(setpointEnforcementFreqSeconds)
	setpointEnforcementTicker = time.NewTicker(setpointEnforcementFreq)

	// configure as Connected Site Controller
	closeExistingConnections()
	thisSite := createSiteIfNew(thisSiteId)
	thisSite.displayName = name.(string)
	masterSchedule.deleteAllSitesExcept([]string{thisSiteId})
	if portFound {
		schedulerInstanceType = SITE_WITH_SERVER
		// start server to allow for new connection to Fleet Manager
		if isPrimaryScheduler { // a secondary mode controller should not start a server
			err = startServer(port)
			if err != nil {
				return fmt.Errorf("failed to start server: %v", err.Error())
			}
		}
	} else {
		schedulerInstanceType = SITE_NO_SERVER
	}
	log.Println("Configured Scheduler as Site Controller for site", thisSiteId)
	return nil
}

// configureFleetManager parses all the configured sites and creates objects for them if they do not already exist.
// HTTPS clients are made for each site and Scheduler begins attempting to connect to each of the Site Controllers.
// If coming from another schedulerInstanceType, the old HTTPS connections will be closed.
func configureFleetManager(cf configurationForm) error {
	// parse site list for configuration forms, site IDs, and site names
	siteConfigs, siteIds, siteNames, err := cf.parseSiteList()
	if err != nil {
		return fmt.Errorf("failed to parse site list: %v", err.Error())
	}

	// close existing connections
	closeExistingConnections()

	// delete old sites not found in configuration, and create new sites
	masterSchedule.deleteAllSitesExcept(siteIds)
	for i, siteId := range siteIds {
		newSite := createSiteIfNew(siteId)
		newSite.displayName = siteNames[i]
	}

	// since configuring as Fleet Manager, Site Controller strings should be wiped.
	// setpointEnforcing should be set false (since event execution currently only belongs to Site Controllers)
	thisSiteId = ""
	thisPort = ""
	setpointEnforcing = false
	setpointEnforcementFreq := time.Duration(time.Second) * 360000 // 100 hours. arbitrarily large since it will not be a useful select case in main() while acting as Fleet Manager
	setpointEnforcementTicker = time.NewTicker(setpointEnforcementFreq)

	// mark as now being Fleet Manager Scheduler
	schedulerInstanceType = FLEET_MANAGER

	// connect to sites
	loadWsConfigs(&siteConfigs)
	if isPrimaryScheduler { // only connect to the Site Controller if in primary controller mode, as a secondary controller should not have web connections
		launchWsClients()
	}
	log.Println("Configured Scheduler as Fleet Manager")
	return nil
}

// parseSiteList extracts a list of the site-specific configuration forms as well
// as a list of the site IDs and a list of the site names from a Fleet Manager config form.
func (cf *configurationForm) parseSiteList() ([]configurationForm, []string, []string, error) {
	// extract list of sites to manage
	siteList, err := fg.ExtractValueWithType(cf.castToMap(), "sites", fg.INTERFACE_SLICE)
	if err != nil {
		return nil, nil, nil, fmt.Errorf("failed to extract site list from config form: %v", err.Error())
	}

	// instantiate lists of site config forms, site IDs, and site names
	siteConfigs := make([]configurationForm, 0)
	siteIds := make([]string, 0)
	siteNames := make([]string, 0)

	// iterate through site list and parse config form, id, and name
	for i, siteConfigInterface := range siteList.([]interface{}) {
		// verify map data type
		siteConfig, ok := siteConfigInterface.(map[string]interface{})
		if !ok {
			log.Printf("siteConfig with index %v in site list is not a configurationForm. Actual type is %v", i, reflect.TypeOf(siteConfigInterface))
			continue
		}

		// extract site id
		siteId, err := fg.ExtractValueWithType(&siteConfig, "siteId", fg.STRING)
		if err != nil {
			log.Printf("failed to extract siteId from siteConfig with index %v: %v. Ignoring the site config form", i, err.Error())
			continue
		}

		// extract site name
		siteName, err := fg.ExtractValueWithType(&siteConfig, "siteName", fg.STRING)
		if err != nil {
			log.Printf("failed to extract siteName from siteConfig with index %v: %v. Ignoring the site config form", i, err.Error())
			continue
		}

		// add site config form, id, and name to lists
		siteConfigs = append(siteConfigs, configurationForm(siteConfig))
		siteIds = append(siteIds, siteId.(string))
		siteNames = append(siteNames, siteName.(string))
	}
	return siteConfigs, siteIds, siteNames, nil
}

// loadWsConfigs iterates through each site config in a list and parses websocket connection
// information from the site config to the respective siteController object.
func loadWsConfigs(siteConfigs *[]configurationForm) {
	for i, siteConfig := range *siteConfigs {
		// use data in configuration form to configure websocket connection information
		err := siteConfig.loadWsDataIntoSite()
		if err != nil {
			log.Println("Error parsing site configuration with index", i, ":", err)
			log.Println("Skipping this site")
		}
	}
}

// loadWsDataIntoSite parses an individual site config form for its websocket connection
// configuration data and loads it into the respective siteController object's wsConfig.
func (cf *configurationForm) loadWsDataIntoSite() error {
	// extract site id
	siteId, err := fg.ExtractValueWithType(cf.castToMap(), "siteId", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract siteId: %v", err.Error())
	}

	// get site pointer
	site := masterSchedule.getSite(siteId.(string))
	if site == nil {
		return fmt.Errorf("%v not found in schedule", siteId)
	}

	// extract IP address of Site Controller
	address, err := cf.extractIP()
	if err != nil {
		return fmt.Errorf("failed to extract IP address from config form: %v", err.Error())
	}

	// Save the websocket configuration data
	site.ws = newWsConfig(address)
	return nil
}

// launchWsClients iterates through all sites and attempts to connect to their Site Controllers over an HTTPS websocket.
func launchWsClients() {
	for _, site := range masterSchedule.getSites() {
		go func(site *siteController) {
			// connect to the Site Controller via HTTPS
			connError := site.connect(true)
			if connError != nil {
				log.Println("Failed to connect to", site.id, "Site Controller -", connError)
				log.Println("Will try to reattempt connection once per second for 10 seconds then slow down to once per 30 seconds")
			}

			// read incoming messages from the Site Controller
			// if connection failed, this will also attempt to reconnect
			site.readWithConnection()
		}(site)
	}
}

// isConnected returns if the connection to the Site Controller is available or not.
func (site *siteController) isConnected() bool {
	if site.ws == nil {
		return false
	}
	return site.ws.isConnected()
}

// buildWsConfigObj adds a Site Controller representation's id, IP address, and port to an object and returns it
func (site *siteController) buildWsConfigObj() map[string]interface{} {
	obj := make(map[string]interface{})
	obj["siteId"] = site.id
	obj["siteName"] = site.displayName
	ipAndPort := strings.Split(site.ws.address, ":")
	if len(ipAndPort) != 2 {
		obj["siteControllerIP"] = "ERROR"
		obj["siteControllerPort"] = "ERROR"
	}
	obj["siteControllerIP"] = ipAndPort[0]
	obj["siteControllerPort"] = ipAndPort[1]
	return obj
}

// extractIP extracts the IP address and port number from a configuration form.
func (cf *configurationForm) extractIP() (string, error) {
	// extract IP address
	ipAddrInterface, ok := (*cf)["siteControllerIP"]
	if !ok {
		return "", fmt.Errorf("configuration form does not have siteControllerIP field")
	}

	// verify IP address var type
	ipAddr, ok := ipAddrInterface.(string)
	if !ok {
		return "", fmt.Errorf("siteControllerIP field in configuration form is not a string")
	}

	// extract port number
	portNum, err := cf.extractPort()
	if err != nil {
		return "", err
	}

	// concatenate IP address with port number (colon already prepended onto port number)
	fullAddress := fmt.Sprintf("%v%v", ipAddr, portNum)

	// validate IP address
	err = checkIPAddrFormat(fullAddress)
	if err != nil {
		return "", err
	} else {
		return fullAddress, nil
	}
}

// extractPort extracts just the port number from the configuration form and prepends it with a ':'.
func (cf *configurationForm) extractPort() (string, error) {
	// extract port number
	portNumInterface, ok := (*cf)["siteControllerPort"]
	if !ok {
		return "", fmt.Errorf("configuration form does not have siteControllerPort field")
	}

	// parse port number if it was entered as a number
	portNumFloat, ok := portNumInterface.(float64)
	if ok {
		return fmt.Sprintf(":%v", portNumFloat), nil
	}

	// parse port number if it was entered as a string
	portNumStr, ok := portNumInterface.(string)
	if ok {
		// prepend a ':' character if it is not already there
		if !strings.HasPrefix(portNumStr, ":") {
			portNumStr = strings.Join([]string{":", portNumStr}, "")
		}

		return portNumStr, nil
	}

	return "", fmt.Errorf("siteControllerPort field in configuration form is not a float64 or a string. With 9000 as example, options are 9000, \"9000\", or \":9000\"")
}

// castToMap simply type casts *configurationForm to *map[string]interface{}
func (cf *configurationForm) castToMap() *map[string]interface{} {
	return (*map[string]interface{})(cf)
}

// buildConnectedStatuses creates a map of the site(s) and what their connection status is
// with the format:
// {
// 		"<siteId1>": connectedStatus1,
// 		"<siteId2>": connectedStatus2,
// 		...
// 		"<siteIdN>": connectedStatusN,
// }
// where connectedStatus is a boolean
func buildConnectionMap() map[string]bool {
	connectionMap := make(map[string]bool)
	switch schedulerInstanceType {
	case FLEET_MANAGER:
		for _, site := range masterSchedule.getSites() {
			connectionMap[site.id] = site.isConnected()
		}
	case SITE_WITH_SERVER:
		connectionMap[thisSiteId] = fleetManagerRW.isConnected()
	}
	return connectionMap
}

//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Websocket messages and messaging //////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// processWebSocketMsg processes a message received through the websocket coming from either the Fleet Manager
// (if this is Site Controller instance) or a Site Controller (if this is Fleet Man instance).
func processWebSocketMsg(msg *webSocketMsg) {
	var err error
	switch msg.Method {
	case "set":
		err = processWebSocketSet(msg)
	case "get":
		err = processWebSocketGet(msg)
	default:
		err = fmt.Errorf("invalid method")
	}
	if err != nil {
		log.Println("Error processing received WebSocket message:", err)
	}
}

// processWebSocketSet processes any websocket messages that are SETs based on the endpoint in the last fragment of the URI.
func processWebSocketSet(msg *webSocketMsg) error {
	// break the target URI into fragments
	frags := strings.Split(msg.Uri, "/")
	if len(frags) == 0 {
		return fmt.Errorf("uri was split into 0 fragments")
	}

	// check last fragment to determine if this is SET to events or modes
	switch frags[len(frags)-1] {
	case "events":
		return handleWebSocketEventsSet(msg)
	case "modes":
		return handleWebSocketModesSet(msg)
	case "updateSync":
		return handleWebSocketUpdateSync(msg)
	case "timezone":
		return handleWebSocketTimezoneSet(msg)
	default:
		return fmt.Errorf("unsupported websocket SET endpoint")
	}
}

// handleWebSocketEventsSet handles SETs containing events that come through the websocket.
func handleWebSocketEventsSet(msg *webSocketMsg) error {
	// verify data type of websocket msg body
	siteMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("parsing full schedule failed")
	}

	// parse the received site map for the input schedule data
	inputSchedule, err := parseFullSchedule(siteMap)
	if err != nil {
		return fmt.Errorf("failed to parse schedule from site map: %v", err.Error())
	}

	// SET the schedule with the received input schedule data
	editMultipleSchedules(inputSchedule, FLEET_SITE, SET)
	return nil
}

// handleWebSocketModesSet handles SETs containing modes that come through the websocket.
func handleWebSocketModesSet(msg *webSocketMsg) error {
	// verify data type of FIMS msg
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of FIMS msg is not a map[string]interface{}")
	}

	// parse the mode map out of the FIMS msg body
	newModes, err := parseModesFromMap(&bodyMap)
	if err != nil {
		return fmt.Errorf("failed to parse new modes from FIMS msg: %v", err.Error())
	}

	// validate the parsed mode map and overwrite the old mode map
	err = editModes(newModes, FLEET_SITE, SET)
	if err != nil {
		return fmt.Errorf("failed to set mode map with newly parsed modes: %v", err.Error())
	}
	return nil
}

// handleWebSocketTimezoneSet handles SETs containing the timezone of a single Site Controller's box
func handleWebSocketTimezoneSet(msg *webSocketMsg) error {
	// extract Site Controller representation of the site that has sent us its time zone
	site, err := msg.getTargetSite()
	if err != nil {
		return err
	}

	// verify that the data payload is a string
	timezone, ok := msg.Body.(string)
	if !ok {
		return fmt.Errorf("body of time zone websocket msg is not a string. Actual type is %T", msg.Body)
	}

	// load the time.Location corresponding to the received string
	loc, err := time.LoadLocation(timezone)
	if err != nil {
		return fmt.Errorf("failed to load *time.Location from time zone name %s: %w", timezone, err)
	}
	site.timezone = loc
	broadcastNewTimezones()
	return nil
}

// processWebSocketGet handles replying to any GETs received through the websocket based on the endpoint in the last fragment of the URI.
func processWebSocketGet(msg *webSocketMsg) error {
	// break the target URI into fragments
	frags := strings.Split(msg.Uri, "/")
	if len(frags) == 0 {
		return fmt.Errorf("uri was split into 0 fragments")
	}

	// check last fragment to determine if this is GET to events or modes
	switch frags[len(frags)-1] {
	case "events":
		return handleWebSocketEventsGet(msg)
	case "modes":
		return sendModesThroughWebSocket(msg.ReplyTo)
	default:
		return fmt.Errorf("%v is invalid endpoint for websocket GET", msg.Uri)
	}
}

// handleWebSocketEventsGet handles replying to requests for event data.
func handleWebSocketEventsGet(msg *webSocketMsg) error {
	switch schedulerInstanceType {
	case SITE_WITH_SERVER:
		// if we are a Site Controller with server, Fleet Manager is asking for our entire schedule (consisting of just one site)
		return sendEventsThroughWebSocket(msg.ReplyTo, "all")
	case FLEET_MANAGER:
		// if we are a Fleet Manager, Site Controller would only be asking for its own schedule
		// expected URI format here is /masterSchedule/<siteId>/events. leading '/' causes 4 frags
		frags := strings.Split(msg.Uri, "/")
		if len(frags) >= 4 {
			return sendEventsThroughWebSocket(msg.ReplyTo, frags[2])
		} else {
			return fmt.Errorf("invalid URI (%v) for events GET to a Site with server", msg.Uri)
		}
	default:
		// this case if disconnected Site Controller, but should never happen so print error and ignore
		return fmt.Errorf("schedulerInstanceType is %v and thus cannot handle websocket GETs for events", schedulerInstanceType)
	}
}

// sendEventsThroughWebSocket sends events to the given uri.
// If scope is "all", the entire schedule gets sent.
// Otherwise, only the schedule for the site indicated by scope is sent.
func sendEventsThroughWebSocket(uri, scope string) error {
	var msg webSocketMsg
	msg.Method = "set"
	msg.Uri = uri
	if scope == "all" {
		msg.Body = masterSchedule.buildObj()
	} else {
		msg.Body = buildSiteScheduleObj(scope)
	}
	return msg.send()
}

// getEventsThroughWebSocket sends a GET message through the websocket with the given URI and reply-to URI.
func getThroughWebSocket(uri, replyTo string) error {
	var msg webSocketMsg
	msg.Method = "get"
	msg.Uri = uri
	msg.ReplyTo = replyTo
	return msg.send()
}

// sendModesThroughWebSocket sends all modes to the given URI through the websocket. This function should only be used by Fleet Manager.
func sendModesThroughWebSocket(uri string) error {
	var msg webSocketMsg
	msg.Method = "set"
	msg.Uri = uri
	msg.Body = map[string]interface{}{"modes": modes.buildObj()}
	return msg.send()
}

// send checks who a websocket message is being sent to and calls the appropriate sender function for that recipient.
func (msg webSocketMsg) send() error {
	if msg.isToFleetManager() {
		return writeToFleetManager(msg)
	} else if msg.isToSiteController() {
		sc, err := msg.getTargetSite()
		if err == nil {
			return sc.write(msg)
		}
		return err
	}
	return fmt.Errorf("unsupported uri for sending event schedule via websocket")
}

// isToFleetManager checks if a websocket message is targeted to Fleet Manager.
func (msg webSocketMsg) isToFleetManager() bool {
	return strings.HasPrefix(msg.Uri, "/masterSchedule")
}

// isToSiteController checks if a websocket message is targeted to a Site Controller.
func (msg webSocketMsg) isToSiteController() bool {
	return strings.HasPrefix(msg.Uri, "/siteController")
}

// getTargetSite extracts the target site id from a websocket message's URI.
func (msg webSocketMsg) getTargetSite() (*siteController, error) {
	frags := strings.Split(msg.Uri, "/")
	if len(frags) >= 3 { // first frag is empty since uri starts with '/'
		siteId := frags[2]
		site := masterSchedule.getSite(siteId)
		if site == nil {
			return nil, fmt.Errorf("%v does not exist in schedule", siteId)
		}
		return site, nil
	}
	return nil, fmt.Errorf("invalid Site Controller URI")
}

// updateScheduleOfFleetSiteCounterpart sends the schedule over the websocket.
// If Scheduler is a Fleet Manager instance, each Site Controller in the fleet has only its own schedule sent to it.
// If Scheduler is a Site Controller, its entire schedule is sent to Fleet Manager.
func updateScheduleOfFleetSiteCounterpart() {
	// send appropriate schedules to counterpart(s)
	if schedulerInstanceType == FLEET_MANAGER {
		for _, site := range masterSchedule.getSites() {
			if site.isConnected() {
				err := sendEventsThroughWebSocket(fmt.Sprintf("/siteController/%v/events", site.id), site.id)
				if err != nil {
					log.Println("Error sending events to", site.id, "Site Controller:", err)
				}
			}
		}
	} else if schedulerInstanceType == SITE_WITH_SERVER {
		if fleetManagerRW.isConnected() {
			err := sendEventsThroughWebSocket("/masterSchedule/events", "all")
			if err != nil {
				log.Println("Error sending events to Fleet Manager:", err)
			}
		}
	}
	// if disconnected Site Controller, no counterpart to update
}

// updateModesOfSiteControllers sends modes to all Site Controllers in the fleet.
func updateModesOfSiteControllers() {
	for _, site := range masterSchedule.getSites() {
		if site.isConnected() {
			err := sendModesThroughWebSocket(fmt.Sprintf("/siteController/%s/modes", site.id))
			if err != nil {
				log.Println("Error trying to update mode of", site.id, "Site Controller:", err)
			}
		}
	}
}

// handleWebSocketUpdateSync handles parsing a sync message with a time in the body that indicates the last time the sending Scheduler's schedule had an edit.
// This function should be called as a Site Controller with server, receiving the sync from its Fleet Manager.
// If Fleet Manager was the last to be edited, Site Controller sends a GET, but if Site Controller was the last to be edited, Site Controller sends a SET.
// Also use this handler to trigger sending Site Controller's timezone to Fleet Manager
func handleWebSocketUpdateSync(msg *webSocketMsg) error {
	// send Site Controller's timezone to Fleet Manager
	if s := masterSchedule.getSite(thisSiteId); s != nil {
		var msg webSocketMsg
		msg.Method = "set"
		msg.Uri = fmt.Sprintf("/masterSchedule/%v/timezone", thisSiteId)
		msg.Body = s.timezone.String()
		msg.send()
	}

	// the timestamp will have been marshalled into a string by Fleet Manager
	timeString, ok := msg.Body.(string)
	if !ok {
		return fmt.Errorf("type assertion of string on update sync msg body failed. Actual type is %v", reflect.TypeOf(msg.Body))
	}

	// read the timestamp. when it was built with time.Now() then later marshalled into a string, the golang time package used RFC3339 format
	fleetManagerLastUpdateTime, err := time.Parse(time.RFC3339, timeString)
	if err != nil {
		return err
	}

	// compare timestamps and send SET or GET depending on logic described in this function's description
	if fleetManagerLastUpdateTime.Before(lastScheduleModification) {
		return sendEventsThroughWebSocket(msg.ReplyTo, "all")
	} else {
		return getThroughWebSocket(msg.ReplyTo, fmt.Sprintf("/siteController/%v/events", thisSiteId))
	}
}
