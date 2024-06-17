// netinfo.go implements collection and storage of network information.
package netinfo

import (
	log "github.com/flexgen-power/go_flexgen/logger"
	fims "github.com/flexgen-power/hybridos/fims/go_fims"
	fimsapi "github.com/flexgen-power/hybridos/pluto/fimsapi"
	dummy "github.com/flexgen-power/hybridos/pluto/internal/dummies"
)

// Handler function for setting up a route to geting network information.
// Handlers are expected to return information to FIMS directly.
func GetNetworkInfoHandler(msg *fims.FimsMsg) error {
	log.Infof("Getting network information...")

	// Send payload to FIMS.
	fimsapi.SendMessage("pluto", dummy.GetDummyNetworkInterface(), msg)
	return nil
}
