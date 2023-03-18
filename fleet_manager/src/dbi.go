/*
 * dbi.go
 *
 * Functions and variables related to the database interface (DBI).
 *
 */

package main

import (
	"fims"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// getDbiData sends GET messages to the DBI module for all URIs that contain Fleet Manager data.
func getDbiData() {
	f.SendGet("/dbi/fleet_manager/features/features", "/fleet/features")
	f.SendGet("/dbi/fleet_manager/sites/sites", "/fleet/sites")
}

// Sends current state of features to DBI. Must be wrapped so data is not lost in DBI meta-data.
// Currently logging error instead of returning it since no caller of this function will behave
// differently when there is an error. If ever changed to return error instead of log error, make
// sure to update all callers to include a log.
func backupFeatures() {
	err := f.SendSet("/dbi/fleet_manager/features/features", "", features.buildObj())
	if err != nil {
		log.Errorf("Error backing up features to DBI: %v.", err)
	}
}

// Sends current state of sites to DBI. Must be wrapped so data is not lost in DBI meta-data.
// Currently logging error instead of returning it since no caller of this function will behave
// differently when there is an error. If ever changed to return error instead of log error, make
// sure to update all callers to include a log.
func backupSites() {
	err := f.SendSet("/dbi/fleet_manager/sites/sites", "", fleet.buildObj())
	if err != nil {
		log.Errorf("Error backing up sites to DBI: %v.", err)
	}
}

// handleDbiUpdateSet is the handler function for a FIMS SET to the URI /fleet_manager/operation/dbi_update.
// COPS signaled that Fleet Manager's data should be updated from DBI. This only needs to be done if Fleet Manager is in secondary mode.
func handleDbiUpdateFlagSet(msg fims.FimsMsg) error {
	// verify type of FIMS msg body
	body, ok := msg.Body.(bool)
	if !ok {
		return fmt.Errorf("type assertion of bool on /fleet/operation/dbi_update message body failed")
	}

	// get send update requests to DBI if flag is true and in secondary controller mode
	if body && !inPrimaryMode {
		getDbiData()
	}
	return nil
}
