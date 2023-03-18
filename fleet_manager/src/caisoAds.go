/*
 * caisoAds.go
 *
 * This file contains the feature that handles CAISO's ancillary service dispatches.
 *
 * Business Logic: a site should either follow the on-site "RIG" for local power commands or
 * SCADA commands coming through Washer->Fleet Manager->FM Scheduler->SC Scheduler->Site Controller
 * for remote power commands. A remote/local source flag determines which source Site Controller
 * will follow. The flag is controlled by Fleet Manager based on what settings CAISO is sending.
 * This feature is the "Fleet Manager->FM Scheduler" piece of the dataflow. It will send the remote
 * power command value as well as the remote/local source flag value through Fleet Manager's Scheduler
 * to Site Controller's Scheduler with the given start time and duration for when the settings are
 * valid.
 *
 */

package main

import (
	"fims"
	"fmt"
	"strings"
	"time"

	fg "github.com/flexgen-power/go_flexgen"
	log "github.com/flexgen-power/go_flexgen/logger"
	schedulerEvents "github.com/flexgen-power/scheduler/pkg/events"
)

// caisoAdsFeature holds configuration settings for the CAISO ADS feature.
type caisoAdsFeature struct {
	schedulerMode        string    // ID of the mode that Scheduler will use to manage the event. Delineates which variables/constants are tracked.
	commandId            string    // ID of the command's value. i.e. target_soc, kw_cmd, etc.
	adsOnFlagId          string    // ID of the ADS On flag variable, used when building Scheduler event
	eventDurationMinutes int       // duration, in minutes, that each CAISO ADS event should last
	lastID               string    // ID of the ISO dispatch batch
	lastStartTime        time.Time // start time associated with the dispatch batch
}

// caisoAdsId is the string that identifies the CAISO ADS feature in all JSONs, maps, etc.
const caisoAdsId string = "caisoAds"

// scheduleAdsEvent builds a Scheduler event representing a CAISO ADS command and sends it to Scheduler for dispatch.
func (ca *caisoAdsFeature) scheduleAdsEvent(batchId string, startTime time.Time, command float64) error {
	// for now, assume we only have one site. If that is not the case, this feature logic is currently not valid since
	// it does not distribute the power to multiple sites.
	if len(fleet) != 1 {
		return fmt.Errorf("scheduleAdsEvent called, but it requires exactly 1 site in the fleet. There are %d sites in the fleet", len(fleet))
	}

	// at this point, it has been verified that there is only one site in the fleet so get a pointer to that site.
	s, err := fleet.getRandomSite()
	if err != nil {
		return fmt.Errorf("failed to get random site from fleet: %w", err)
	}

	// build the Scheduler event
	event := schedulerEvents.CreateEvent(startTime, time.Duration(ca.eventDurationMinutes)*time.Minute, ca.schedulerMode)
	event.AddVariable(ca.commandId, command)
	adsOnFlag := false // TODO: logic for deciding adsOnFlag
	event.AddVariable(ca.adsOnFlagId, adsOnFlag)

	// structure the event within an event array that is within an "events" object
	msgBody := map[string]interface{}{
		"events": []map[string]interface{}{
			event.BuildObj(),
		},
	}

	// send the event as a POST so existing events are not overwritten
	log.Infof("Sending a CAISO ADS event to Scheduler with start time %v, duration %d, command %f, and local-remote flag %v", startTime, ca.eventDurationMinutes, command, adsOnFlag)
	_, err = f.SendAndVerify("all", fims.FimsMsg{
		Method:  "post",
		Uri:     fmt.Sprintf("/scheduler/%s", s.id),
		Replyto: "/fleet/caiso/eventVerification",
		Body:    msgBody,
	})
	if err != nil {
		return fmt.Errorf("failed sending caiso event to scheduler: %w", err)
	}
	return nil
}

// handleCaisoAdsPacket parses out commands sent from CAISO and passes them to the CAISO ADS feature for scheduling.
func handleCaisoAdsPacket(msg fims.FimsMsg) error {
	// Replyto verification
	if msg.Replyto != "" {
		f.SendSet(msg.Replyto, "", true)
	}

	// if CAISO ADS feature is not available, ignore this packet
	caisoAds := features.caisoAds
	if caisoAds == nil {
		log.MsgWarn("Received CAISO ADS packet but feature is not available, so ignoring packet.")
		return nil
	}

	// verify variables came in map[string]interface{} form
	varMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected FIMS msg body to be type map[string]interface{}. Type of received FIMS msg body is %T", msg.Body)
	}

	// parse out CAISO ADS batch ID
	batchIdInterface, err := fg.ExtractValueWithType(varMap, "Id", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract Id from CAISO ADS packet: %w", err)
	}
	batchId := batchIdInterface.(string)
	// update batch record
	caisoAds.lastID = batchId

	// parse out start timestamp
	startTimeInterface, err := fg.ExtractValueWithType(varMap, "StartTime", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract start time from CAISO ADS packet: %w", err)
	}
	startTime, err := time.Parse(time.RFC3339, startTimeInterface.(string))
	if err != nil {
		return err
	}

	caisoAds.lastStartTime = startTime

	// parse out washer data map
	dataMapInterface, err := fg.ExtractValueWithType(varMap, "Data", fg.MAP_STRING_INTERFACE)
	if err != nil {
		return fmt.Errorf("failed to extract washer data map from CAISO ADS packet: %w", err)
	}
	dataMap := dataMapInterface.(map[string]interface{})

	// parse out CAISO ADS command
	commandInterface, err := fg.ExtractValueWithType(dataMap, caisoAds.commandId, fg.FLOAT64)
	if err != nil {
		return fmt.Errorf("failed to extract %s from CAISO ADS packet: %w", caisoAds.commandId, err)
	}
	command := commandInterface.(float64)

	// pass commands to feature for scheduling
	err = caisoAds.scheduleAdsEvent(batchId, startTime, command)
	if err != nil {
		return fmt.Errorf("failed to schedule ADS event: %w", err)
	}

	// send this latest batch to dbi for storage, used to make subsequent queries to the ISO
	caisoAds.backupToDbi()

	return err
}

func (ca *caisoAdsFeature) backupToDbi() {
	_, err := f.SendAndVerify("latest", fims.FimsMsg{
		Method:  "set",
		Uri:     fmt.Sprintf("/dbi/fleet_manager/features/features/%s", caisoAdsId),
		Replyto: "/fleet/caiso/latestBatchVerification",
		Body:    ca.buildObj(),
	})
	if err != nil {
		log.Errorf("Error backing up CAISO ADS feature settings to DBI: %v.", err)
	}
}

// Checks if the lastDispatchBatch is less than 24 hours old, and sends it along if so, else sends reserved -1
func sendLatestBatchResponse(uri string) error {
	// if CAISO ADS feature is not available, ignore this packet
	caisoAds := features.caisoAds
	if caisoAds == nil {
		log.MsgWarn("Received CAISO ADS packet but feature is not available, so ignoring packet.")
		return nil
	}

	// Assumes a 24 hour day
	if time.Since(caisoAds.lastStartTime) > time.Hour*24 {
		// If the batch is stale, send the reserved -1 which will request all batches in the last 24 hours from the ISO
		caisoAds.lastID = "-1"
	}
	log.Infof("Returned dispatch batch %s to %s", caisoAdsId, uri)
	f.SendSet(uri, "", caisoAds.lastID)
	return nil
}

// buildObj makes a map of the feature for easy JSON sending.
func (ca *caisoAdsFeature) buildObj() map[string]interface{} {
	obj := make(map[string]interface{})
	obj["schedulerMode"] = ca.schedulerMode
	obj["commandId"] = ca.commandId
	obj["adsOnFlagId"] = ca.adsOnFlagId
	obj["eventDurationMinutes"] = ca.eventDurationMinutes
	obj["lastID"] = ca.lastID
	obj["lastStartTime"] = ca.lastStartTime
	return obj
}

func (ca *caisoAdsFeature) publish() {
	pubUri := fmt.Sprintf("/fleet/features/%s", caisoAdsId)
	err := f.SendPub(pubUri, map[string]interface{}{
		"schedulerMode":        ca.schedulerMode,
		"commandId":            ca.commandId,
		"adsOnFlagId":          ca.adsOnFlagId,
		"eventDurationMinutes": ca.eventDurationMinutes,
		"lastID":               ca.lastID,
		"lastStartTime":        ca.lastStartTime,
	})
	if err != nil {
		log.Errorf("Error publishing to %s: %v", pubUri, err)
	}
}

// takes CAISO ADS feature configuration settings and parses them into the feature object
func parseCaisoAdsFeature(inputCfg interface{}) (ca *caisoAdsFeature, err error) {
	ca = &caisoAdsFeature{}

	// verify type is map[string]interface{}
	varMap, ok := inputCfg.(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("expected map[string]interface{}, got %T", inputCfg)
	}

	// extract ID of Scheduler mode that feature will use
	schedulerModeInterface, err := fg.ExtractValueWithType(varMap, "schedulerMode", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract schedulerMode: %w", err)
	}
	ca.schedulerMode = schedulerModeInterface.(string)

	// extract ID of command that feature will look for and build Scheduler msg from
	commandIdInterface, err := fg.ExtractValueWithType(varMap, "commandId", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract commandId: %w", err)
	}
	ca.commandId = commandIdInterface.(string)

	// extract ID of ADS On flag that will be used when building Scheduler event
	adsOnFlagIdInterface, err := fg.ExtractValueWithType(varMap, "adsOnFlagId", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract adsOnFlagId: %w", err)
	}
	ca.adsOnFlagId = adsOnFlagIdInterface.(string)

	// extract duration that will be assigned to CAISO ADS Scheduler events
	eventDurationMinutesInterface, err := fg.ExtractValueWithType(varMap, "eventDurationMinutes", fg.FLOAT64)
	if err != nil {
		return nil, fmt.Errorf("failed to extract eventDurationMinutes: %w", err)
	}
	ca.eventDurationMinutes = int(eventDurationMinutesInterface.(float64))

	// parse out CAISO ADS batch ID if present
	batchIdInterface, err := fg.ExtractValueWithType(varMap, "lastID", fg.STRING)
	if err != nil {
		// Use default reserved id
		ca.lastID = "-1"
		log.MsgWarn("No CAISO latest dispatch batch id in config")
	} else {
		ca.lastID = batchIdInterface.(string)
	}

	// parse out timestamp if present
	timeInterface, err := fg.ExtractValueWithType(varMap, "lastStartTime", fg.STRING)
	if err != nil {
		// use empty time as default case
		log.MsgWarn("No CAISO lastest timestamp in config")
	} else {
		ca.lastStartTime, err = time.Parse(time.RFC3339, timeInterface.(string))
		if err != nil {
			return nil, fmt.Errorf("failed to parse lastest timestamp: %w", err)
		}
	}
	return ca, nil
}

func handleCaisoSet(msg fims.FimsMsg) error {
	if msg.Uri == "/fleet/features/caisoAds/dispatchBatch" {
		return handleCaisoAdsPacket(msg)
	}

	if strings.HasPrefix(msg.Uri, "/fleet/features/caisoAds/latestBatchVerification") || strings.HasPrefix(msg.Uri, "/fleet/features/caisoAds/eventVerification") {
		// Verification handled by go_fims, but valid endpoint
		return nil
	}

	if msg.Uri == "/fleet/features/caisoAds" {
		caisoConfig, ok := msg.Body.(map[string]interface{})
		if !ok {
			return fmt.Errorf("expected FIMS msg body to be type map[string]interface{}. Type of received FIMS msg body is %T", msg.Body)
		}
		caisoFeature, err := parseCaisoAdsFeature(caisoConfig)
		if err != nil {
			return fmt.Errorf("error configuring CAISO feature: %w", err)
		}
		// overwrite features with new config settings
		features.caisoAds = caisoFeature
		caisoFeature.backupToDbi()
		return nil
	}

	return fmt.Errorf("URI %s is not a valid endpoint in CAISO", msg.Uri)
}
