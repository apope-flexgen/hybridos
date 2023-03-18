/**
 *
 * fims_handler.go
 *
 * Infrastructure for using the go_fims package to handle and send FIMS messages.
 *
 */

package main

import (
	"errors"
	"fims"
	"fmt"
	"log"
	"os"
	"reflect"
	"strconv"
	"strings"
	"time"

	fg "github.com/flexgen-power/go_flexgen"
)

var f *fims.Fims
var cops_heartbeat uint

// configureFims configures a FIMS connection
func configureFims() (fimsReceive chan fims.FimsMsg) {
	// Connect to FIMS
	fimsObj, err := fims.Connect("Scheduler")
	fatalErrorCheck(err, "Unable to connect to FIMS server")
	f = &fimsObj

	// Subscribe to messages targeted at Scheduler
	err = f.Subscribe("/scheduler")
	fatalErrorCheck(err, "Unable to subscribe to FIMS")

	// Start a FIMS Receive channel that will be used to hold incoming FIMS messages
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
	return
}

// processFims is the starting point for handling any and all incoming FIMS messages.
func processFims(msg fims.FimsMsg) {
	// put a recover here as insurance in case a FIMS message comes through that unexpectedly causes seg fault.
	// will keep Scheduler from totally crashing
	defer func() {
		if r := recover(); r != nil {
			log.Println("Error processing FIMS message to URI", msg.Uri, ":", r)
		}
	}()

	switch msg.Method {
	case "set":
		handleSet(msg)
	case "get":
		handleGet(msg)
	case "post":
		handlePost(msg)
	case "del":
		handleDel(msg)
	case "pub":
		// Publishes are ignored
	default:
		log.Println("Received FIMS message with invalid method. No action taken.")
	}
}

// handleSet routes all received FIMS messages that have the method set to their appropriate handler function
func handleSet(msg fims.FimsMsg) {
	var err error
	switch {
	case strings.HasPrefix(msg.Uri, "/scheduler/configuration"):
		err = handleConfigurationSet(msg)
	case strings.HasPrefix(msg.Uri, "/scheduler/scada"):
		err = handleScadaSet(msg)
	case msg.Nfrags >= 2 && masterSchedule.getSite(msg.Frags[1]) != nil:
		err = handleFimsEventsEdit(msg, SET)
	case msg.Nfrags == 3 && strings.HasPrefix(msg.Uri, "/scheduler/activeEventStatus"):
		err = handleActiveEventStatusUpdate(msg.Frags[2], msg.Body)
	case msg.Uri == "/scheduler/lastScheduleModification":
		err = handleLastSchedModSet(msg)
	case msg.Uri == "/scheduler/operation/primary_controller":
		err = handlePrimaryFlagSet(msg)
	case msg.Uri == "/scheduler/operation/dbi_update":
		err = handleDbiUpdateFlagSet(msg)
	case msg.Uri == "/scheduler/cops/cops_heartbeat":
		err = handleCopsHeartbeatSet(msg)
	case msg.Uri == "/scheduler/events":
		err = handleFimsFullScheduleEdit(msg, COPS, SET)
	case msg.Uri == "/scheduler/modes":
		err = handleFimsModesEdit(msg, UI, SET)
	case msg.Uri == "/scheduler/config":
		err = handleDbiDataEdit(msg, SET)
	case msg.Uri == "/scheduler/enableSchedulerEvents":
		err = handleEnableSet(msg)
	case msg.Uri == "/scheduler/timezones":
		err = handleTimezonesSet(msg)
	default:
		err = errors.New("URI is not a valid SET endpoint")
	}
	if err != nil {
		log.Println("Error processing SET to", msg.Uri, ":", err)
	}
}

// handleGet checks the endpoint of the GET message and replies with the appropriate data.
func handleGet(msg fims.FimsMsg) {
	if msg.Nfrags < 2 {
		log.Printf("fims_handler::handleGet ~ %v URI fragments received, expected at least 2\n", len(msg.Frags))
		return
	}
	endpoint := msg.Frags[msg.Nfrags-1]
	switch {
	case endpoint == "events":
		sendLegacyEvents(msg.Replyto)
	case endpoint == "modes":
		sendModes(msg.Replyto)
	case msg.Frags[1] == "scada":
		handleScadaGet(msg)
	case endpoint == "cops":
		sendHeartBeat(msg.Replyto)
	case msg.Frags[1] == "configuration":
		handleConfigurationGet(msg)
	case endpoint == "connected":
		f.SendSet(msg.Replyto, "", buildConnectionMap())
	case endpoint == "timezones":
		f.SendSet(msg.Replyto, "", buildTimezoneMap())
	case endpoint == "enableSchedulerEvents":
		sendEnableSchedulerEvents(msg.Replyto)
	case masterSchedule.getSite(endpoint) != nil:
		f.SendSet(msg.Replyto, "", map[string]interface{}{
			"events": *masterSchedule.getSite(endpoint).buildObj(),
		})
	default:
		log.Printf("%v is an invalid Scheduler GET endpoint", msg.Uri)
	}
}

// handlePost routes all received FIMS messages that have the method post to their appropriate handler functions.
func handlePost(msg fims.FimsMsg) {
	var err error
	switch {
	case msg.Nfrags >= 2 && masterSchedule.getSite(msg.Frags[1]) != nil:
		err = handleFimsEventsEdit(msg, POST)
	case msg.Uri == "/scheduler/modes":
		err = handleFimsModesEdit(msg, UI, POST)
	case msg.Uri == "/scheduler/config":
		err = handleDbiDataEdit(msg, POST)
	default:
		err = errors.New("URI is not a valid POST endpoint")
	}
	if err != nil {
		log.Println("Error processing POST to", msg.Uri, ":", err)
	}
}

// handleDel routes all received FIMS messages that have the method del to their appropriate handler functions.
func handleDel(msg fims.FimsMsg) {
	var err error
	switch {
	case msg.Nfrags >= 2 && masterSchedule.getSite(msg.Frags[1]) != nil:
		err = handleFimsEventsEdit(msg, DEL)
	case msg.Uri == "/scheduler/modes":
		err = handleFimsModesEdit(msg, UI, DEL)
	case msg.Uri == "/scheduler/config":
		err = handleDbiDataEdit(msg, DEL)
	default:
		err = errors.New("URI is not a valid DEL endpoint")
	}
	if err != nil {
		log.Println("Error processing DEL to", msg.Uri, ":", err)
	}
}

// handleConfigurationSet is the handler function for a FIMS SET to the URI /scheduler/configuration.
// A configuration form has been received from the UI.
func handleConfigurationSet(msg fims.FimsMsg) error {
	if msg.Nfrags >= 3 && msg.Frags[2] == "scada" {
		return handleScadaConfigSet(msg)
	}

	// verify type of FIMS msg body
	body, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of SET to /scheduler/configuration is not map[string]interface{}. Real type is %v", reflect.TypeOf(msg.Body))
	}

	// extract connection type from configuration form
	connectionType, err := fg.ExtractValueWithType(&body, "connection", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to parse configuration form: %v", err.Error())
	}

	// check which connection type is being configured and call the corresponding handler function
	switch connectionType {
	case "SC":
		err = configureSiteController(body)
		if err != nil {
			return fmt.Errorf("failed to configure Scheduler as Site Controller: %v", err.Error())
		}
		broadcastNewTimezones() // only called in SC case and not FM case because FM must get time zones from DBI or directly from sites. SC gets it straight from Linux during configuration
	case "FM":
		err = configureFleetManager(body)
		if err != nil {
			return fmt.Errorf("failed to configure Scheduler as Fleet Manager: %v", err.Error())
		}
	default:
		return fmt.Errorf("connection declared in configuration form is invalid connection type")
	}

	// SCADA interface configuration can optionally be included with general configuration, but not required
	newScadaConfig, err := fg.ExtractValueWithType(&body, "scadaConfig", fg.MAP_STRING_INTERFACE)
	if err == nil {
		if err := handleScadaConfigStructSet(newScadaConfig); err != nil {
			log.Println("Error handling scadaConfig portion of configuration data:", err)
		} else {
			configureScadaInterface()
			updateScadaRegs()
		}
	}

	// send configuration data to DBI for backup
	// also write events to DBI in case configuration deleted any old sites
	if !strings.HasSuffix(msg.Uri, "_dbi") {
		sendConfigData("/dbi/scheduler/configuration")
		sendUpdatesAfterScheduleEdit(INTERNAL)
	}
	return nil
}

// handleLastSchedModSet is the handler function for a FIMS SET to the URI /scheduler/lastScheduleModification.
// DBI or COPS is updating the lastScheduleModification timestamp.
func handleLastSchedModSet(msg fims.FimsMsg) error {
	// verify FIMS body is a map[string]interface{}
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("FIMS body is not a map[string]interface. Actual type is %v", reflect.TypeOf(msg.Body))
	}

	// the timestamp will have been marshalled into a string
	timeString, err := fg.ExtractValueWithType(&bodyMap, "lastScheduleModification", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract lastScheduleModification from FIMS body: %v", err.Error())
	}

	// read the timestamp. when it was built with time.Now() then later marshalled into a string, the golang time package used RFC3339 format
	parsedTime, err := time.Parse(time.RFC3339, timeString.(string))
	if err != nil {
		return err
	}

	// update last schedule modification timestamp
	lastScheduleModification = parsedTime
	return nil
}

// handlePrimaryFlagSet is the handler function for a FIMS SET to the URI /scheduler/operation/primary_controller.
// COPS set the Scheduler to be primary if the received flag is true.
func handlePrimaryFlagSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(bool)
	if !ok {
		return fmt.Errorf("type assertion of bool on /scheduler/operation/primary_controller message body failed")
	}

	// update isPrimaryScheduler if it is being changed
	if isPrimaryScheduler != body {
		if body {
			getDbiData() // if taking over as primary, send GET to DBI to configure Fleet-Site connection
		}
		isPrimaryScheduler = body
	}
	return nil
}

// handleDbiUpdateSet is the handler function for a FIMS SET to the URI /scheduler/operation/dbi_update.
// COPS signaled that the mode and event data should be updated from DBI. This only needs to be done if Scheduler is in secondary mode.
func handleDbiUpdateFlagSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(bool)
	if !ok {
		return fmt.Errorf("type assertion of bool on /scheduler/operation/dbi_update message body failed")
	}

	// get send update requests to DBI if flag is true and in secondary controller mode
	if body && !isPrimaryScheduler {
		getDbiData()
	}
	return nil
}

// handleCopsHeartbeatSet is the handler function for a FIMS SET to the URI /scheduler/cops/cops_heartbeat.
// COPS has sent an updated heartbeat value that Scheduler should store so that when COPS sends a GET for it, Scheduler can verify it is alive.
func handleCopsHeartbeatSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(float64) // ints seem to be converted to floats somewhere in the FIMS pipeline
	if !ok {
		return fmt.Errorf("type assertion of uint on %s message body failed", msg.Uri)
	}

	// update the internal COPS heartbeat value
	cops_heartbeat = uint(body)
	return nil
}

// handleEnableSet is the handler function for a FIMS SET to the URI /scheduler/enable and expects a boolean value with which
// to set the variable enableSchedulerEvents.
func handleEnableSet(msg fims.FimsMsg) error {
	// unwrap val in case it is clothed
	bodyVal := unwrapVariable(msg.Body)

	// verify type of FIMS msg newFlag
	newFlag, ok := bodyVal.(bool)
	if !ok {
		return errors.New("type assertion of bool on /scheduler/enable message body failed")
	}

	// update the internal enableSchedulerEvents value
	updateEnableSchedulerEvents(newFlag)
	return nil
}

// handleTimezonesSet is the handler function for a FIMS SET to the URI /scheduler/timezones and expects a map where the
// keys are site IDs and the values are time zone strings (i.e. America/New_York, Europe/London, etc.).
//
// The map is allowed to be either naked or wrapped in a {"timezones": {<map>}} object.
func handleTimezonesSet(msg fims.FimsMsg) error {
	// verify data type of FIMS msg body
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return errors.New("body of FIMS msg is not a map[string]interface{}")
	}

	// try to parse wrapped time zone map. if failed, then expect naked map
	timezoneMapInterface, err := fg.ExtractValueWithType(&bodyMap, "timezones", fg.MAP_STRING_INTERFACE)
	if err != nil {
		timezoneMapInterface = bodyMap
	}
	timezoneMap := timezoneMapInterface.(map[string]interface{})

	// iterate through each element of the map and parse out the site time zone
	for siteId, tzInterface := range timezoneMap {
		// parse out site
		site := masterSchedule.getSite(siteId)
		if site == nil {
			log.Println("Received time zone map with site ID", siteId, "but no site with that ID exists in the master schedule.")
			continue
		}

		// parse out time zone string
		tz, ok := tzInterface.(string)
		if !ok {
			log.Printf("Time zone map element with site ID %s does not hold a string. Holds a %T instead.\n", siteId, tzInterface)
			continue
		}

		// load *time.Location that corresponds with the received time zone string
		loc, err := time.LoadLocation(tz)
		if err != nil {
			log.Println("Failed to load location for time zone string", tz, "found in time zone map under site ID", siteId, ":", err)
		}

		// assign new *time.Location to the site object
		site.timezone = loc
	}
	return nil
}

// handleActiveEventStatusUpdate handles FIMS SETs to URIs with the format /scheduler/activeEventStatus/<variable ID>.
// Takes a variable ID and its value in the form of a JSON body.
// Compares the value to the setpoint it should be at, and sends an update SET if the value does not match the setpoint.
func handleActiveEventStatusUpdate(varId string, updateBody interface{}) error {
	// this function assumes Scheduler is a Site Controller, so there should only be one site.
	// if there is not one site, this assumption is broken and the function should not be executed.
	if masterSchedule.getNumSites() != 1 {
		return errors.New("expected one and only one site in the schedule, but there are " + strconv.Itoa(masterSchedule.getNumSites()))
	}

	// get pointer to site object
	thisSite := masterSchedule.getSite(thisSiteId)
	if thisSite == nil {
		return errors.New("thisSite (" + thisSiteId + ") not found in schedule")
	}

	if err := thisSite.handleActiveEventStatusUpdate(varId, updateBody); err != nil {
		return fmt.Errorf("failed to handle active event status update: %w", err)
	}
	return nil
}

// handleFimsEventsEdit is the handler function for a FIMS msg to URIs with one of two formats, either:
//	/scheduler/<site id>/<day index>  	which expects a single day's worth of events, used only by the UI, or
//	/scheduler/<site id>  				which expects a full site's worth of events, used by all other editors over fims
// The input site schedule is parsed and used to edit the current site schedule.
func handleFimsEventsEdit(msg fims.FimsMsg, method editingMethod) error {
	// verify number of URI fragments
	if msg.Nfrags < 2 {
		return fmt.Errorf("invalid number of URI fragments. Expecting at least 2 or 3, got %d", msg.Nfrags)
	}

	// extract site from URI
	site := masterSchedule.getSite(msg.Frags[1])
	if site == nil {
		return fmt.Errorf("site %s not found in schedule", msg.Frags[1])
	}

	// Editor of the schedule
	var scheduleEditor editingInterface

	// extract day ID from URI if 3 fragments given (events per day use case)
	var day int
	if msg.Nfrags == 3 {
		// This is the UI sending a single day's worth of events
		dayID := msg.Frags[2]
		if !strings.Contains(dayID, "_") {
			return fmt.Errorf("invalid URI. Third fragment for events edit from UI should be of the form day_#")
		}

		// extract day number from day ID
		dayNumberString := strings.Split(dayID, "_")[1]
		var err error
		day, err = strconv.Atoi(dayNumberString)
		if err != nil {
			return fmt.Errorf("error parsing %v for day number", dayNumberString)
		}

		scheduleEditor = UI
	} else {
		// This any service sending a specific site's full schedule over fims
		// Day index will be determined by the position of the event in the schedule (full schedule use case)
		day = -1
		scheduleEditor = CONTROLLING_PROCESS
	}

	// verify data type of FIMS msg body
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return errors.New("body of FIMS msg is not a map[string]interface{}")
	}

	// extract site schedule
	siteScheduleObj, err := fg.ExtractValueWithType(&bodyMap, "events", fg.INTERFACE_SLICE)
	if err != nil {
		return fmt.Errorf("failed to extract site schedule object from FIMS body map: %w", err)
	}

	// parse the site schedule out of the FIMS msg body
	inputEvents := newEventHeap()
	err = parseEventListIntoHeap(siteScheduleObj.([]interface{}), inputEvents, day, site.timezone)
	if err != nil {
		return fmt.Errorf("failed to parse input events from FIMS msg: %w", err)
	}

	// edit the site schedule
	err = site.editSchedule(inputEvents, scheduleEditor, method, day)
	if err != nil {
		return fmt.Errorf("failed to edit site schedule with input site schedule: %w", err)
	}

	// reply with modified schedule
	if msg.Replyto != "" {
		sendEvents(msg.Replyto)
	}

	return nil
}

// handleDbiDataEdit is the handler function for a FIMS message to the URI /scheduler/config.
// DBI has sent data to update Scheduler. This data may be mode data or schedule data.
func handleDbiDataEdit(msg fims.FimsMsg, method editingMethod) error {
	// verify data type of FIMS msg body
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of msg from DBI is not a map[string]interface{}. Is type %T", bodyMap)
	}

	// iterate through keys of msg body and handle each piece of data separately
	keys := extractKeys(bodyMap)
	for _, k := range keys {
		switch k {
		case "modes":
			err := handleFimsModesEdit(msg, DBI, method)
			if err != nil {
				log.Println("Error handling modes object in msg from DBI:", err)
				log.Println("Continuing to next object in msg if there is one")
			}
		case "schedule":
			err := handleFimsFullScheduleEdit(msg, DBI, method)
			if err != nil {
				log.Println("Error handling schedule object in msg from DBI:", err)
				log.Println("Continuing to next object in msg if there is one")
			}
		}
	}
	return nil
}

// handleFimsFullScheduleEdit takes a FIMS message that contains an all-sites input schedule from DBI.
// The input schedule is parsed and used to edit the old schedule based on the passed-in editing method.
func handleFimsFullScheduleEdit(msg fims.FimsMsg, editor editingInterface, method editingMethod) error {
	// verify data type of FIMS msg body
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return errors.New("body of FIMS msg is not a map[string]interface{}")
	}

	// extract site map from FIMS msg body
	siteMapInterface, err := fg.ExtractValueWithType(&bodyMap, "schedule", fg.MAP_STRING_INTERFACE)
	if err != nil {
		return fmt.Errorf("failed to parse site map from FIMS msg body: %w", err)
	}
	siteMap := siteMapInterface.(map[string]interface{})

	// parse the received site map for the input schedule data
	inputSchedule, err := parseFullSchedule(siteMap)
	if err != nil {
		return fmt.Errorf("failed to parse schedule from site map: %w", err)
	}

	// edit the schedule with the received input schedule data
	editMultipleSchedules(inputSchedule, editor, method)

	// Reply with modified events
	if msg.Replyto != "" {
		sendEvents(msg.Replyto)
	}
	return nil
}

// handleFimsModesEdit takes a FIMS message that contains an input mode map.
// The input mode map is parsed and used to edit the old Scheduler mode map based on the passed-in editing method.
func handleFimsModesEdit(msg fims.FimsMsg, editor editingInterface, method editingMethod) error {
	// verify data type of FIMS msg
	bodyMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("body of FIMS msg is not a map[string]interface{}")
	}

	// parse the mode map out of the FIMS msg body
	newModes, err := parseModesFromMap(&bodyMap)
	if err != nil {
		return fmt.Errorf("failed to parse input modes from FIMS msg: %v", err.Error())
	}

	// edit the mode map
	err = editModes(newModes, editor, method)
	if err != nil {
		return fmt.Errorf("failed to edit mode map with input modes: %v", err.Error())
	}

	// reply with modified modes
	if msg.Replyto != "" {
		sendModes(msg.Replyto)
	}

	return nil
}

// Send GETs to DBI to get any backed up data that might be there from a previous runtime
func getDbiData() {
	f.SendGet("/dbi/scheduler/configuration", "/scheduler/configuration_dbi")
	f.SendGet("/dbi/scheduler/timezones", "/scheduler/timezones")
	f.SendGet("/dbi/scheduler/lastScheduleModification", "/scheduler/lastScheduleModification")
	f.SendGet("/dbi/scheduler/modes", "/scheduler/config")
	f.SendGet("/dbi/scheduler/events", "/scheduler/config")
}

// sendEvents sends the entire schedule to the specified URI if in primary controller mode, since secondary controller does not send FIMS messages.
func sendEvents(uri string) {
	if isPrimaryScheduler {
		f.SendSet(uri, "", map[string]interface{}{
			"schedule": masterSchedule.buildObj(),
		})
	}
}

// sendEvents sends the entire schedule to the specified URI if in primary controller mode, since secondary controller does not send FIMS messages.
// start time is given as mins since midnight
func sendLegacyEvents(uri string) {
	if isPrimaryScheduler {
		f.SendSet(uri, "", map[string]interface{}{
			"schedule": masterSchedule.buildLegacyObject(),
		})
	}
}

// pubEvents sends a PUB with the entire schedule. Used to update the UI
func pubEvents() {
	if isPrimaryScheduler {
		uri := "/scheduler/events"
		f.SendPub(uri, map[string]interface{}{
			"schedule": masterSchedule.buildLegacyObject(),
		})
	}
}

// sendModes sends the entire modes object to the specified URI
func sendModes(uri string) {
	// If in secondary mode, we don't want any mode data to be sent out
	if isPrimaryScheduler {
		f.SendSet(uri, "", map[string]interface{}{
			"modes": modes.buildObj(),
		})
	}
}

// pubModes sends a PUB with the mode map. Used to update the UI
func pubModes() {
	if isPrimaryScheduler {
		f.SendPub("/scheduler/modes", map[string]interface{}{
			"modes": modes.buildObj(),
		})
	}
}

// broadcastNewTimezones sends a SET to DBI and broadcasts a PUB with a map of each site's time zone.
func broadcastNewTimezones() {
	timeZoneMap := buildTimezoneMap()
	f.SendPub("/scheduler/timezones", timeZoneMap)
	f.SendSet("/dbi/scheduler/timezones", "", map[string]interface{}{
		"timezones": timeZoneMap,
	})
}

// Sends COPS heartbeat info
func sendHeartBeat(uri string) {
	body := make(map[string]interface{})
	body["cops_heartbeat"] = cops_heartbeat
	body["pid"] = os.Getpid()
	if schedulerVersion.tag != "" {
		body["version_tag"] = schedulerVersion.tag
	} else {
		body["version_tag"] = "Invalid"
	}
	if schedulerVersion.commit != "" {
		body["version_commit"] = schedulerVersion.commit
	} else {
		body["version_commit"] = "Invalid"
	}
	if schedulerVersion.build != "" {
		body["version_build"] = schedulerVersion.build
	} else {
		body["version_build"] = "Invalid"
	}

	f.SendSet(uri, "", body)
}

// sendLastScheduleModification sends `lastScheduleModification` to the given URI.
func sendLastScheduleModification(uri string) {
	f.SendSet(uri, "", map[string]interface{}{"lastScheduleModification": lastScheduleModification})
}

// sendEnableSchedulerEvents sends enableSchedulerEvents to the given URI.
func sendEnableSchedulerEvents(uri string) {
	f.SendSet(uri, "", map[string]interface{}{"enableSchedulerEvents": enableSchedulerEvents})
}

// handles all GETs to URIs starting with // /scheduler/configuration
func handleConfigurationGet(msg fims.FimsMsg) {
	if msg.Nfrags == 2 { // /scheduler/configuration
		if err := sendConfigData(msg.Replyto); err != nil {
			log.Println("Error sending configuration data to", msg.Replyto, ":", err)
		}
		return
	}

	if msg.Frags[2] == "scada" {
		if err := handleScadaConfigGet(msg); err != nil {
			log.Println("Error sending SCADA configuration data to", msg.Replyto, ":", err)
		}
		return
	}

	log.Println(msg.Uri, "is invalid Scheduler GET endpoint")
}

// sendConfigData sends the configuration data as a config form to the specified URI.
func sendConfigData(uri string) error {
	var body map[string]interface{}
	switch schedulerInstanceType {
	case SITE_NO_SERVER:
		body = buildSiteSchedulerConfigForm(false)
	case SITE_WITH_SERVER:
		body = buildSiteSchedulerConfigForm(true)
	case FLEET_MANAGER:
		body = buildFleetManConfigForm()
	default:
		return fmt.Errorf("schedulerInstanceType is an invalid value")
	}
	if body == nil {
		return fmt.Errorf("built body for config data is nil")
	}
	body["scadaConfig"] = scadaConfig.buildObj()
	if isPrimaryScheduler {
		return f.SendSet(uri, "", body)
	}
	return nil
}
