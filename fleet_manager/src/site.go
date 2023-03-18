/*
 * site.go
 *
 * Functions and variables for keeping track of a single site.
 *
 */

package main

import (
	"errors"
	"fims"
	"fmt"

	fg "github.com/flexgen-power/go_flexgen"
	log "github.com/flexgen-power/go_flexgen/logger"
)

// site is the struct that represents a single site in the fleet
type site struct {
	id                 string                // no spaces or slashes. used for URIs and internal tracking
	name               string                // all kinds of characters allowed. Used for UI display
	activePower        scadapoint            // current active power output of the site (+ for site discharge and - for site charge)
	reactivePower      scadapoint            // current reactive power output of the site (+ for site discharge and - for site charge)
	breakerClosed      scadapoint            // boolean value representing the site's POI breaker being open (false) or closed (true)
	genAvrStatus       scadapoint            // generation voltage regulator status
	dischargeablePower scadapoint            // the overall amount of power that the site could discharge, taking into account POI limits and asset availability
	chargeablePower    scadapoint            // the overall amount of power the the site could charge, taking into account POI limits and asset availability
	ess                essData               // scadapoints for sites that have ESSs
	gridVolts          gridVoltageData       // various grid voltages at the site
	freqResp           frequencyResponseData // control values related to the Frequency Response active power feature
}

// Instantiates a new site object and configures it using the provided config object.
func createSite(id string, cfgInterface interface{}) (*site, error) {
	cfg, ok := cfgInterface.(map[string]interface{})
	if !ok {
		return nil, fmt.Errorf("expected map[string]interface{} but got %T", cfgInterface)
	}

	newSite := site{
		id:                 id,
		activePower:        newFloatStatus("active_power"),
		reactivePower:      newFloatStatus("reactive_power"),
		breakerClosed:      newBoolStatus("breaker_status"),
		genAvrStatus:       newBoolStatus("gen_avr_status"),
		dischargeablePower: newFloatStatus("site_dischargeable_mw"),
		chargeablePower:    newFloatStatus("site_chargeable_mw"),
		ess:                newEssData(),
		gridVolts:          newGridVoltageData(),
		freqResp:           newFrequencyResponseData(),
	}

	// parse site name
	name, err := fg.ExtractValueWithType(cfg, "name", fg.STRING)
	if err != nil {
		return nil, fmt.Errorf("failed to extract name from site with ID %s: %w", id, err)
	}
	newSite.name = name.(string)

	return &newSite, nil
}

// Overwrites the configuration of the existing site with the new configuration.
func (s *site) overwrite(cfg interface{}) error {
	newSite, err := createSite(s.id, cfg)
	if err != nil {
		return fmt.Errorf("failed to parse site config: %w", err)
	}
	*s = *newSite
	s.backupToDbi()
	return nil
}

func (s *site) backupToDbi() {
	err := f.SendSet(fmt.Sprintf("/dbi/fleet_manager/sites/sites/%s", s.id), "", s.buildObj())
	if err != nil {
		log.Errorf("Error backing up site %s to DBI: %v.", s.id, err)
	}
}

// Endpoint for all SETs to URIs beginning with /fleet/sites/<site ID>.
func (s *site) handleSet(msg fims.FimsMsg) error {
	// /fleet/sites/<site ID>
	if msg.Nfrags == 3 {
		err := s.overwrite(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to overwrite site %s: %w", s.id, err)
		}
		return nil
	}
	return errors.New("invalid URI")
}

// buildObj builds a map[string]interface{} that represents the receiver site struct for FIMS sending.
func (s site) buildObj() map[string]interface{} {
	obj := map[string]interface{}{
		"name": s.name,
	}
	s.activePower.addToObject(obj)
	s.reactivePower.addToObject(obj)
	s.dischargeablePower.addToObject(obj)
	s.chargeablePower.addToObject(obj)
	s.breakerClosed.addToObject(obj)
	s.genAvrStatus.addToObject(obj)
	s.ess.addToObject(obj)
	s.gridVolts.addToObject(obj)
	s.freqResp.addToObject(obj)
	return obj
}

// handleClientPub is the handler function with FIMS PUBs that have URIs of the form /sites/<site ID>_event
func (s *site) handleClientPub(msg fims.FimsMsg) error {
	dataMap, ok := msg.Body.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected FIMS msg body to be map[string]interface{} but got %T", msg.Body)
	}
	s.activePower.optionalParse(dataMap, s.id)
	s.reactivePower.optionalParse(dataMap, s.id)
	s.dischargeablePower.optionalParse(dataMap, s.id)
	s.chargeablePower.optionalParse(dataMap, s.id)
	s.breakerClosed.optionalParse(dataMap, s.id)
	s.genAvrStatus.optionalParse(dataMap, s.id)
	s.ess.handleClientPub(dataMap, s.id)
	s.freqResp.handleClientPub(dataMap, s.id)
	s.gridVolts.handleClientPub(dataMap, s.id)
	return nil
}

// recalculateSiteVariables recalculates the site status values necessary for every feature
func (s *site) recalculateSiteVariables() {
	s.gridVolts.calculateAvgVoltage()
}

// sendControlValues iterates through all SCADA data and sends their control values to their SCADA URIs if the status values do not match.
func (s site) sendControlValues() {
	// currently the only control points are the frequency response control points used by the ERCOT AS feature,
	// so only send them if this site is in the ERCOT AS feature configuration. Later, once site-specific features
	// have been refactored to be under the site itself, this can be written cleaner
	if _, ok := features.ercotAs[s.id]; !ok {
		return
	}
	s.freqResp.sendControlValues(fmt.Sprintf("/sites/%s", s.id))
}
