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
	"errors"
	"fims"
	"fmt"
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
	lastId               string    // ID of the ISO dispatch batch
	lastStartTime        time.Time // start time associated with the dispatch batch
}

// caisoAdsId is the string that identifies the CAISO ADS feature in all JSONs, URIs, etc.
const caisoAdsId string = "caiso_ads"

// scheduleAdsEvent builds a Scheduler event representing a CAISO ADS command and sends it to Scheduler for dispatch.
func (caiso *caisoAdsFeature) scheduleAdsEvent(startTime time.Time, command float64) error {
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
	event := schedulerEvents.CreateEvent(startTime, time.Duration(caiso.eventDurationMinutes)*time.Minute, caiso.schedulerMode)
	event.AddVariable(caiso.commandId, command)
	adsOnFlag := false // TODO: logic for deciding adsOnFlag
	event.AddVariable(caiso.adsOnFlagId, adsOnFlag)

	// structure the event within an event array that is within an "events" object
	msgBody := map[string]interface{}{
		"events": []map[string]interface{}{
			event.BuildObj(),
		},
	}

	// send the event as a POST so existing events are not overwritten
	log.Infof("Sending a CAISO ADS event to Scheduler with start time %v, duration %d, command %f, and ADS On flag %v.", startTime, caiso.eventDurationMinutes, command, adsOnFlag)
	_, err = f.SendAndVerify("all", fims.FimsMsg{
		Method:  "post",
		Uri:     fmt.Sprintf("/scheduler/%s", s.id),
		Replyto: fmt.Sprintf("/fleet/features/%s/event_verification", caisoAdsId),
		Body:    msgBody,
	})
	if err != nil {
		return fmt.Errorf("failed sending CAISO event to scheduler: %w", err)
	}
	return nil
}

// handleCaisoAdsPacket parses out commands sent from CAISO and passes them to the CAISO ADS feature for scheduling.
func (caiso *caisoAdsFeature) handleCaisoAdsPacket(msg fims.FimsMsg) error {
	// Replyto verification
	if msg.Replyto != "" {
		if err := f.SendSet(msg.Replyto, "", true); err != nil {
			log.Errorf("Error sending reply-to verification to %s: %v.", msg.Replyto, err)
		}
		return nil
	}

	// verify variables came in map[string]interface{} form
	varMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected FIMS msg body to be type map[string]interface{}. Type of received FIMS msg body is %T", msg.Body)
	}

	// parse out CAISO ADS batch ID
	batchIdInterface, err := fg.ExtractValueWithType(varMap, "id", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract id from CAISO ADS packet: %w", err)
	}
	batchId := batchIdInterface.(string)
	// update batch record
	caiso.lastId = batchId

	// parse out start timestamp
	startTimeInterface, err := fg.ExtractValueWithType(varMap, "start_time", fg.STRING)
	if err != nil {
		return fmt.Errorf("failed to extract start time from CAISO ADS packet: %w", err)
	}
	startTime, err := time.Parse(time.RFC3339, startTimeInterface.(string))
	if err != nil {
		return err
	}
	caiso.lastStartTime = startTime

	// parse out washer data map
	dataMapInterface, err := fg.ExtractValueWithType(varMap, "data", fg.MAP_STRING_INTERFACE)
	if err != nil {
		return fmt.Errorf("failed to extract washer data map from CAISO ADS packet: %w", err)
	}
	dataMap := dataMapInterface.(map[string]interface{})

	// parse out CAISO ADS command
	commandInterface, err := fg.ExtractValueWithType(dataMap, caiso.commandId, fg.FLOAT64)
	if err != nil {
		return fmt.Errorf("failed to extract %s from CAISO ADS packet: %w", caiso.commandId, err)
	}
	command := commandInterface.(float64)

	// pass commands to feature for scheduling
	err = caiso.scheduleAdsEvent(startTime, command)
	if err != nil {
		return fmt.Errorf("failed to schedule ADS event: %w", err)
	}

	// send this latest batch to dbi for storage. used to make subsequent queries to the ISO
	caiso.backupToDbi()
	return nil
}

func (caiso *caisoAdsFeature) backupToDbi() {
	_, err := f.SendAndVerify("latest", fims.FimsMsg{
		Method:  "set",
		Uri:     fmt.Sprintf("/dbi/fleet_manager/features/features/%s", caisoAdsId),
		Replyto: fmt.Sprintf("/fleet/features/%s/latest_batch_verification", caisoAdsId),
		Body:    caiso.buildObj(),
	})
	if err != nil {
		log.Errorf("Error backing up CAISO ADS feature settings to DBI: %v.", err)
	}
}

// First, resets the last_id field to "-1" only if the last dispatch batch is older than 24 hours.
// Then, sends the last_id field to the given URI.
func (caiso *caisoAdsFeature) sendLatestBatchResponse(uri string) error {
	// assumes a 24 hour day
	if time.Since(caiso.lastStartTime) > time.Hour*24 {
		// if the batch is stale, send the reserved -1 which will request all batches in the last 24 hours from the ISO
		caiso.lastId = "-1"
	}
	if err := f.SendSet(uri, "", caiso.lastId); err != nil {
		return fmt.Errorf("failed to send last ID in SET: %w", err)
	}
	log.Infof("Returned dispatch batch %s to %s.", caisoAdsId, uri)
	return nil
}

// buildObj makes a map of the feature for easy JSON sending.
func (caiso *caisoAdsFeature) buildObj() map[string]interface{} {
	return map[string]interface{}{
		"scheduler_mode":         caiso.schedulerMode,
		"ads_kw_cmd_id":          caiso.commandId,
		"ads_on_flag_id":         caiso.adsOnFlagId,
		"event_duration_minutes": caiso.eventDurationMinutes,
		"last_id":                caiso.lastId,
		"last_start_time":        caiso.lastStartTime,
	}
}

// Publishes the CAISO ADS feature's data if the feature has been configured.
func (caiso *caisoAdsFeature) publish() {
	if caiso == nil {
		return
	}
	pubUri := fmt.Sprintf("/fleet/features/%s", caisoAdsId)
	if err := f.SendPub(pubUri, caiso.buildObj()); err != nil {
		log.Errorf("Error publishing to %s: %v.", pubUri, err)
	}
}

// takes CAISO ADS feature configuration settings and parses them into the feature object
func parseCaisoAdsFeature(inputCfg interface{}) (caiso *caisoAdsFeature, err error) {
	caiso = &caisoAdsFeature{}

	// verify type is map[string]interface{}
	varMap, ok := inputCfg.(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("expected map[string]interface{}, got %T", inputCfg)
	}

	// extract ID of Scheduler mode that feature will use
	schedulerModeInterface, err := fg.ExtractValueWithType(varMap, "scheduler_mode", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract scheduler_mode: %w", err)
	}
	caiso.schedulerMode = schedulerModeInterface.(string)

	// extract ID of command that feature will look for and build Scheduler msg from
	commandIdInterface, err := fg.ExtractValueWithType(varMap, "ads_kw_cmd_id", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract ads_kw_cmd_id: %w", err)
	}
	caiso.commandId = commandIdInterface.(string)

	// extract ID of ADS On flag that will be used when building Scheduler event
	adsOnFlagIdInterface, err := fg.ExtractValueWithType(varMap, "ads_on_flag_id", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract ads_on_flag_id: %w", err)
	}
	caiso.adsOnFlagId = adsOnFlagIdInterface.(string)

	// extract duration that will be assigned to CAISO ADS Scheduler events
	caiso.eventDurationMinutes, err = fg.ExtractAsInt(varMap, "event_duration_minutes")
	if err != nil {
		return nil, fmt.Errorf("failed to extract event_duration_minutes as integer: %w", err)
	}

	// parse out CAISO ADS batch ID if present
	batchIdInterface, err := fg.ExtractValueWithType(varMap, "last_id", fg.STRING)
	if err != nil {
		// Use default reserved id
		caiso.lastId = "-1"
		log.MsgDebug("No CAISO latest dispatch batch ID in config.")
	} else {
		caiso.lastId = batchIdInterface.(string)
	}

	// parse out timestamp if present
	timeInterface, err := fg.ExtractValueWithType(varMap, "last_start_time", fg.STRING)
	if err != nil {
		// use empty time as default case
		log.MsgDebug("No CAISO latest timestamp in config.")
	} else {
		caiso.lastStartTime, err = time.Parse(time.RFC3339, timeInterface.(string))
		if err != nil {
			return nil, fmt.Errorf("failed to parse lastest timestamp: %w", err)
		}
	}

	return caiso, nil
}

// Endpoint for any SETs to a URI beginning in /fleet/features/caiso_ads.
func handleCaisoSet(msg fims.FimsMsg) error {
	if msg.Nfrags < 4 {
		newCaisoFeature, err := parseCaisoAdsFeature(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to parse CAISO ADS config object: %w", err)
		}
		features.caisoAds = newCaisoFeature
		newCaisoFeature.backupToDbi()
		return nil
	}

	if features.caisoAds == nil {
		return errors.New("CAISO ADS feature pointer is nil")
	}

	endpointFrag := msg.Frags[3]
	switch endpointFrag {
	case "latest_batch_verification", "event_verification":
		// endpoint for verifying FIMS receipt, which is handled by go_fims so do nothing here
		return nil
	case "dispatch_batch": // expected to contain data from CAISO ADS that must be translated into Scheduler event
		if err := features.caisoAds.handleCaisoAdsPacket(msg); err != nil {
			return fmt.Errorf("failed to handle CAISO ADS packet: %w", err)
		}
		return nil
	case "scheduler_mode": // rest of these endpoints are configuration edits
		newSchedulerMode, ok := msg.Body.(string)
		if !ok {
			return fmt.Errorf("expected string but got %T", msg.Body)
		}
		features.caisoAds.schedulerMode = newSchedulerMode
	case "ads_kw_cmd_id":
		newCommandId, ok := msg.Body.(string)
		if !ok {
			return fmt.Errorf("expected string but got %T", msg.Body)
		}
		features.caisoAds.commandId = newCommandId
	case "ads_on_flag_id":
		newAdsOnFlagId, ok := msg.Body.(string)
		if !ok {
			return fmt.Errorf("expected string but got %T", msg.Body)
		}
		features.caisoAds.adsOnFlagId = newAdsOnFlagId
	case "event_duration_minutes":
		newEventDurationMinutes, err := fg.CastToInt(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to cast body to int: %w", err)
		}
		features.caisoAds.eventDurationMinutes = newEventDurationMinutes
	default:
		return fmt.Errorf("%s in an invalid 4th fragment for a CAISO ADS SET", endpointFrag)
	}

	// cases that did not return upon success are setpoints that must be backed up to DBI when edited
	if err := f.SendSet(fmt.Sprintf("/dbi/fleet/features/features/%s/%s", caisoAdsId, endpointFrag), "", msg.Body); err != nil {
		return fmt.Errorf("failed to send CAISO ADS %s to DBI: %w", endpointFrag, err)
	}
	return nil
}

// Endpoint for any GETs to a URI beginning in /fleet/features/caiso_ads.
func (caiso *caisoAdsFeature) handleGet(msg fims.FimsMsg) error {
	if caiso == nil {
		return errors.New("pointer to CAISO ADS feature is nil")
	}

	if msg.Nfrags < 4 {
		if err := f.SendSet(msg.Replyto, "", caiso.buildObj()); err != nil {
			return fmt.Errorf("failed to send CAISO ADS object to %s: %w", msg.Replyto, err)
		}
		return nil
	}

	endpointFrag := msg.Frags[3]
	switch endpointFrag {
	case "latest_batch": // special GET endpoint that returns last_id but first does an expiration check
		if err := caiso.sendLatestBatchResponse(msg.Replyto); err != nil {
			return fmt.Errorf("failed to send latest batch response: %w", err)
		}
		return nil
	case "scheduler_mode":
		if err := f.SendSet(msg.Replyto, "", caiso.schedulerMode); err != nil {
			return fmt.Errorf("failed to send scheduler_mode: %w", err)
		}
		return nil
	case "ads_kw_cmd_id":
		if err := f.SendSet(msg.Replyto, "", caiso.commandId); err != nil {
			return fmt.Errorf("failed to send ads_kw_cmd_id: %w", err)
		}
		return nil
	case "ads_on_flag_id":
		if err := f.SendSet(msg.Replyto, "", caiso.adsOnFlagId); err != nil {
			return fmt.Errorf("failed to send ads_on_flag_id: %w", err)
		}
		return nil
	case "event_duration_minutes":
		if err := f.SendSet(msg.Replyto, "", caiso.eventDurationMinutes); err != nil {
			return fmt.Errorf("failed to send event_duration_minutes: %w", err)
		}
		return nil
	case "last_id":
		if err := f.SendSet(msg.Replyto, "", caiso.lastId); err != nil {
			return fmt.Errorf("failed to send last_id: %w", err)
		}
		return nil
	case "last_start_time":
		if err := f.SendSet(msg.Replyto, "", caiso.lastStartTime); err != nil {
			return fmt.Errorf("failed to send last_start_time: %w", err)
		}
		return nil
	default:
		return fmt.Errorf("%s is invalid fragment for CAISO ADS GET", endpointFrag)
	}
}
