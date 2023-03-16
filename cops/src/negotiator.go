/**
 * The FlexGen External Device Sensor (The FEDS)
 *
 * Created February 2021
 *
 * The FEDS is used in conjunction with C2C to monitor the presence
 * and health of the "Other" HybridOS machine and determine if "This"
 * HybridOS machine should be in Primary controller mode or Secondary
 * controller mode.
 *
 */

package main

import (
	"fmt"
	"os/exec"
	"path"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Three possible controller modes: Primary, Secondary, and Update
const (
	Primary = iota
	Secondary
	Update
)

var statusNames = [3]string{"Primary", "Secondary", "Update"}

// Global variables
var controllerMode = Secondary
var firstIteration = true // Used to determine additional, one-time startup behavior if Secondary

// Execute mode negotiation actions
func checkControllerMode() {
	// Only take over if we are not currently updating
	if controllerMode != Update && otherControllerIsUnresponsive() {
		takeOverAsPrimary()
	} else {
		queueC2CMsg("ModeQuery")
	}
}

// If in Secondary mode, every second check if This controller should take over as Primary
func negotiateControllerMode() {
	checkModeFreqMS := 1000
	checkModeTicker := startTicker(checkModeFreqMS)
	defer checkModeTicker.Stop()
	for {
		if controllerMode == Secondary {
			checkControllerMode()
		}
		<-checkModeTicker.C
	}
}

// Checks if TCP connection has been lost or if last mode confirmation from Other was too long ago
func otherControllerIsUnresponsive() bool {
	replyFromOtherAllowableDelayMS := 3000
	if !c2c.connected || time.Since(c2c.lastModeConfirmationFromOther).Milliseconds() > int64(replyFromOtherAllowableDelayMS) {
		return true
	}
	return false
}

// call takeOverAsPrimary if ModeCommand tells us to become Primary or if Other controller's Total Health Score is too low
func processModeCommand(msg string) {
	msgFrags := strings.Split(msg, " ")
	modeCommand := msgFrags[1]
	otherControllerTotalHealthScore, err := strconv.ParseFloat(msgFrags[2], 64)
	// Currently secondary and negotiated as primary from other controller
	if (modeCommand == "Primary" && controllerMode == Secondary) ||
		// Only take over if we are not currently updating and we are more healthy
		(controllerMode != Update && err == nil && otherControllerTotalHealthScore < dr.totalHealthThreshold) {
		takeOverAsPrimary()
	} else if modeCommand == "Secondary" && controllerMode == Secondary {
		// Have determined this controller is secondary, try to get any new setpoints and configuration that may have been missed
		// Only run this check once on startup
		if firstIteration {
			firstIteration = false
			// Wait for The primary to complete it's configuration first
			time.Sleep(time.Millisecond * 500)
			// Request the latest configuration data from the primary
			// TODO: send version id of each document instead, primary returns any newer versions it has
			queueC2CMsg("GetDBIUpdate")
		}
	}
	c2c.lastModeConfirmationFromOther = time.Now()
}

// If the Other controller asks what mode it should be in, reply telling it what it should be and give health status
func replyToModeQueryMsg() {
	reply := "ModeCommand "
	if controllerMode == Primary || c2c.amServerNotClient {
		reply += "Secondary"
	} else {
		reply += "Primary"
	}
	totalHealthScore := dr.healthCheckup()
	reply += fmt.Sprintf(" %f", totalHealthScore)
	queueC2CMsg(reply)
}

// Attempt to connect to other controller then start negotiation
func startControllerModeNegotiation() {
	waitForTCPConnectionAttempt()
	negotiateControllerMode()
}

// Sets controller mode to Primary, takes over primary IP and restarts the other controller
func takeOverAsPrimary() {
	controllerMode = Primary
	log.Infof("Taking over as Primary controller")
	for _, process := range processJurisdiction {
		process.sendPrimaryFlag(true)
	}

	if enableRedundantFailover {
		if primaryIP == "" || primaryNetworkInterface == "" {
			log.Errorf("Failed to takeover: primary IP or network interface undefined")
			return
		}
		// Take the primary IP on this machine
		// Set up virtual device
		ipCmd := exec.Command("ip", "link", "add", primaryNetworkInterface, "type", "dummy")
		if ipErr := ipCmd.Run(); ipErr != nil {
			log.Errorf("Error setting IP: %s", ipErr)
		}
		// Create IP alias
		ipAliasCmd := exec.Command("ip", "addr", "add", primaryIP, "dev", primaryNetworkInterface, "label", primaryNetworkInterface)
		if ipAliasErr := ipAliasCmd.Run(); ipAliasErr != nil {
			log.Errorf("Error setting IP: %s", ipAliasErr)
		}

		// Reset the pdu
		restartCmd := exec.Command("/bin/sh", "/usr/local/bin/send_pdu_restart.sh", OtherCtrlrOutlet, pduIP)
		// Shows the execution of the restart script including ssh connection - uncomment for debugging
		// restartCmd.Stdout = os.Stdout
		// restartCmd.Stderr = os.Stderr
		if restartErr := restartCmd.Start(); restartErr != nil {
			log.Errorf("Error restarting PDU: %s", restartErr)
		}

		// This controller started up as secondary when it should have been primary, re-read latest setpoints from DBI
		// Some data was not written out due to secondary status, and missed by components as a result
		// Tell Site_Controller to read its latest settings from DBI again
		if firstIteration {
			firstIteration = false
			for _, process := range processJurisdiction {
				f.SendSet(path.Join(process.uri, "/operation/dbi_update"), "", true)
			}
		}
	}
}

// sendPrimaryFlag sends the passed-in boolean value to the URI of the handler process's primary mode endpoint
func (p *processInfo) sendPrimaryFlag(v bool) {
	f.SendSet(path.Join(p.uri, "/operation/primary_controller"), "", v)
}

// If C2C connection does not succeed after a set amount of time, assume Primary. Otherwise, let negotiation decide
func waitForTCPConnectionAttempt() {
	firstTCPConnectionAttemptTimeAllowanceMS := 10000
	deadlineToConnectToClientIfServer := startTicker(firstTCPConnectionAttemptTimeAllowanceMS)
	defer deadlineToConnectToClientIfServer.Stop()
	for {
		if c2c.connected {
			return
		}
		select {
		case <-deadlineToConnectToClientIfServer.C:
			// Only take over if we are not currently updating
			if controllerMode != Update {
				takeOverAsPrimary()
			}
			return
		default:
		}
	}
}
