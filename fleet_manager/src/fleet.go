/*
 * fleet.go
 *
 * Functions and variables for fleet-wide operation.
 */

package main

import (
	"errors"
	"fims"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Map of all sites in fleet.
type siteMap map[string]*site

// The master object for all sites.
var fleet siteMap = make(siteMap)

// Overwrites the existing fleet with the given new sites object.
func replaceFleet(newFleetObj interface{}) error {
	newFleet := make(siteMap)
	err := newFleet.addMultipleSites(newFleetObj)
	if err != nil {
		return fmt.Errorf("failed to add multiple sites to the fleet (none were added): %w", err)
	}
	fleet = newFleet
	backupSites()
	return nil
}

// Endpoint for any SETs to a URI beginning in /fleet/sites.
func handleSitesSet(msg fims.FimsMsg) error {
	// /fleet/sites
	if msg.Nfrags < 3 {
		err := replaceFleet(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to process new sites object: %w", err)
		}
		return nil
	}

	// URI must be /fleet/sites/<site ID>
	targetSite, ok := fleet[msg.Frags[2]]
	if !ok {
		return fmt.Errorf("did not find site %s in fleet", msg.Frags[2])
	}
	err := targetSite.handleSet(msg)
	if err != nil {
		return fmt.Errorf("failed to handle SET to site %s", msg.Frags[2])
	}
	return nil
}

// Handles FIMS GETs to URIs that begin with /fleet/sites.
func handleSitesGet(msg fims.FimsMsg) error {
	// /fleet/sites
	if msg.Nfrags < 3 {
		err := f.SendSet(msg.Replyto, "", fleet.buildObj())
		if err != nil {
			return fmt.Errorf("failed to reply to GET for fleet: %w", err)
		}
		return nil
	}

	// URI starts with /fleet/sites/<site ID> so gather data for the targeted site
	siteStruct, ok := fleet[msg.Frags[2]]
	if !ok {
		return fmt.Errorf("did not find site %s in fleet", msg.Frags[2])
	}
	siteJson := siteStruct.buildObj()

	// URI is /fleet/sites/<site ID>
	if msg.Nfrags < 4 {
		return f.SendSet(msg.Replyto, "", siteJson)
	}

	// URI is targeting a specific variable. get it from the already-built map of site data
	val, ok := siteJson[msg.Frags[3]]
	if !ok {
		return fmt.Errorf("did not find variable %s in data for site %s", msg.Frags[3], msg.Frags[2])
	}
	return f.SendSet(msg.Replyto, "", val)
}

// Endpoint for any POSTs to a URI beginning in /fleet/sites.
func handleSitesPost(msg fims.FimsMsg) error {
	switch msg.Nfrags {
	case 2: // /fleet/sites
		err := fleet.addMultipleSites(msg.Body)
		if err != nil {
			return fmt.Errorf("failed to handle multiple sites POST: %w", err)
		}
	case 3: // /fleet/sites/<site ID>
		err := fleet.addSite(msg.Frags[2], msg.Body)
		if err != nil {
			return fmt.Errorf("failed to add site %s to fleet: %w", msg.Frags[2], err)
		}
	default:
		return errors.New("invalid URI")
	}
	return nil
}

// Takes a map of site configurations and adds them to the site map.
// Each site is parsed and added individually, so if one has a configuration error, the rest are still added.
func (sm siteMap) addMultipleSites(cfgMapInterface interface{}) error {
	cfgMap, ok := cfgMapInterface.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected map[string]interface{} but got %T", cfgMapInterface)
	}
	// optional wrapping in "sites" key must be allowed since configuration may have come from DBI
	if nestedMapInterface, ok := cfgMap["sites"]; ok {
		cfgMap, ok = nestedMapInterface.(map[string]interface{})
		if !ok {
			return fmt.Errorf("expected nested map to be map[string]interface{}, but got %T", nestedMapInterface)
		}
	}

	for id, cfg := range cfgMap {
		err := sm.addSite(id, cfg)
		if err != nil {
			log.Errorf("Error adding site %s to fleet: %v.", id, err)
		}
	}
	return nil
}

// Adds a site with the given ID and configuration to the fleet.
// If a site with the given ID already exists in the fleet, it will be overwritten.
func (sm siteMap) addSite(id string, cfgInterface interface{}) error {
	if _, ok := sm[id]; ok {
		return fmt.Errorf("site %s already exists", id)
	}

	cfg, ok := cfgInterface.(map[string]interface{})
	if !ok {
		return fmt.Errorf("expected map[string]interface{} but got %T", cfgInterface)
	}

	newSite, err := createSite(id, cfg)
	if err != nil {
		return fmt.Errorf("failed to create site %s: %w", id, err)
	}

	sm[id] = newSite
	newSite.backupToDbi()
	log.Infof("Added site %s to the fleet.", id)
	return nil
}

// getRandomSite returns the first site in a range-based iteration across the site map, and range-based iterations do not have guaranteed orders (pseudo-random).
// Returns an error if there are no sites in the map.
func (sm siteMap) getRandomSite() (*site, error) {
	for _, s := range sm {
		return s, nil
	}
	return nil, fmt.Errorf("no sites in the site map")
}

// buildObj builds a map[string]interface{} that represents the receiver siteMap map for FIMS sending.
func (sm siteMap) buildObj() map[string]interface{} {
	siteMapObj := make(map[string]interface{})
	for siteId, site := range sm {
		siteMapObj[siteId] = site.buildObj()
	}
	return siteMapObj
}

// sendControlValues iterates through all sites in the site map and tells them to send their SCADA control values.
func (sm siteMap) sendControlValues() {
	for _, site := range sm {
		site.sendControlValues()
	}
}

// Makes one publish per site.
func publishSites() {
	for id, site := range fleet {
		pubUri := fmt.Sprintf("/fleet/sites/%s", id)
		err := f.SendPub(pubUri, site.buildObj())
		if err != nil {
			log.Errorf("Error publishing to %s: %v", pubUri, err)
		}
	}
}

// update recalculates all standard variables for each site
func (sm siteMap) update() {
	for _, site := range sm {
		site.recalculateSiteVariables()
	}
}
