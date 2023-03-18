/*
 * fims.go
 *
 * Handles the interface with the FlexGen Internal Messages Service (FIMS).
 *
 */
package main

import (
	"fims"
	"fmt"
	"strings"
)

// f points to the FIMS connection struct.
var f *fims.Fims

// configureFims configures a connection to the FIMS server
func configureFims() (chan fims.FimsMsg, error) {
	// Connect to FIMS
	fimsObj, err := fims.Connect("fleet_manager")
	if err != nil {
		return nil, fmt.Errorf("unable to connect to FIMS server: %w", err)
	}
	f = &fimsObj

	// Subscribe to messages targeted at Fleet Manager and messages from sites about statuses
	err = f.Subscribe("/fleet", "/sites")
	if err != nil {
		return nil, fmt.Errorf("failed to subscribe to /fleet and/or /components URIs on FIMS: %w", err)
	}

	fims.ConfigureVerification(5, nil)

	// Start a FIMS Receive channel that will be used to hold incoming FIMS messages
	fimsReceive := make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
	return fimsReceive, nil
}

// handleFimsMsg is the starting point for handling any and all incoming FIMS messages.
func handleFimsMsg(msg fims.FimsMsg) error {
	// need to subscribe to /sites to get PUBs, but do not want to process any other messages sent to /sites
	if msg.Frags[0] == "sites" && msg.Method != "pub" {
		return nil
	}
	switch msg.Method {
	case "set":
		return handleSet(msg)
	case "get":
		return handleGet(msg)
	case "post":
		return handlePost(msg)
	case "del":
		return handleDel(msg)
	case "pub":
		return handlePub(msg)
	default:
		return fmt.Errorf("invalid FIMS method")
	}
}

// handleSet routes all received FIMS messages that have the method "set" to their appropriate handler function.
func handleSet(msg fims.FimsMsg) error {
	switch {
	case msg.Uri == "/fleet/sites":
		return handleSitesSet(msg)
	case strings.HasPrefix(msg.Uri, "/fleet/features"):
		return handleFeaturesSet(msg)
	case msg.Uri == "/fleet/operation/dbi_update":
		return handleDbiUpdateFlagSet(msg)
	case msg.Uri == "/fleet/operation/primary_controller":
		return handlePrimaryFlagSet(msg)
	case msg.Uri == "/fleet/cops/cops_heartbeat":
		return handleCopsHeartbeatSet(msg)
	default:
		return fmt.Errorf("URI is not a valid SET endpoint")
	}
}

// handleGet checks the endpoint of the GET message and replies with the appropriate data.
func handleGet(msg fims.FimsMsg) error {
	switch {
	case strings.HasPrefix(msg.Uri, "/fleet/sites"):
		return handleSitesGet(msg)
	case strings.HasPrefix(msg.Uri, "/fleet/features"):
		return handleFeaturesGet(msg)
	case strings.HasPrefix(msg.Uri, "/fleet/configuration"):
		return handleConfigurationGet(msg)
	case msg.Uri == "/fleet/cops":
		return sendHeartBeat(msg.Replyto)
	default:
		return fmt.Errorf("URI is not a valid GET endpoint")
	}
}

// handlePost routes all received FIMS messages that have the method "post" to their appropriate handler function.
func handlePost(msg fims.FimsMsg) error {
	if msg.Nfrags < 2 {
		return fmt.Errorf("URI has %d fragments but at least 2 are required", msg.Nfrags)
	}
	switch msg.Frags[1] {
	case "features":
		err := handleFeaturesPost(msg)
		if err != nil {
			return fmt.Errorf("failed to handle features POST: %w", err)
		}
	case "sites":
		err := handleSitesPost(msg)
		if err != nil {
			return fmt.Errorf("failed to handle sites POST: %w", err)
		}
	default:
		return fmt.Errorf("URI with second fragment %s is not a valid POST endpoint", msg.Frags[0])
	}
	return nil
}

// handlePub routes all received FIMS messages that have the method "pub" to their appropriate handler function.
func handlePub(msg fims.FimsMsg) error {
	switch {
	case msg.Frags[0] == "sites":
		return handleSitesPub(msg)
	default:
		return fmt.Errorf("URI is not a valid PUB endpoint")
	}
}

// handleDel routes all received FIMS messages that have the method "del" to their appropriate handler function.
func handleDel(msg fims.FimsMsg) error {
	switch {
	default:
		return fmt.Errorf("URI is not a valid DEL endpoint")
	}
}

// handleSitesPub is the handler function for FIMS PUBs that have URIs beginning with /sites.
func handleSitesPub(msg fims.FimsMsg) error {
	// should have URI in the format: /sites/<site-id>_events
	if msg.Nfrags > 1 {
		eventName := msg.Frags[msg.Nfrags-1]
		siteId := strings.TrimSuffix(eventName, "_event") //gets the whole event id without the appended "_event"
		site, ok := fleet[siteId]
		if !ok {
			// if site is not one that fleet_manager cares about, ignore it
			return nil
		}
		return site.handleClientPub(msg)
	}

	// otherwise, there are no PUB endpoints for this URI yet
	return fmt.Errorf("no PUB endpoint for URI: %s", msg.Uri)
}
