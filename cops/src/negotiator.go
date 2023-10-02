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
	"crypto/rsa"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"os"
	"os/exec"
	"path"
	"strconv"
	"strings"
	"time"

	fp "path/filepath"

	log "github.com/flexgen-power/go_flexgen/logger"

	"github.com/gosnmp/gosnmp"
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
var pduUser = "hybridos"  // SNMP User and Community names used by the PDU
var enableRedundantFailover bool
var controllerName string // Name of the controller machine, used to distinguish the controllers when running failover
var primaryIP []string
var primaryNetworkInterface []string // Virtual interface used to hold the primary IP
var pduIP string
var pduHashedAuthPath = fp.Join(cfgDir, "encrypted_auth.enc") // Location of hashed password used for encrypting authentication with the PDU
var pduHashedPrivPath = fp.Join(cfgDir, "encrypted_priv.enc") // Location of hashed password used for encrypting privacy with the PDU
var pduDecryptionKeyPath = fp.Join(cfgDir, "decrypt.key")     // Location of private key used to decrypt hashed passwords
var pduAuth string                                            // Decrypted password used for encrypting authentication with the PDU
var pduPriv string                                            // Decrypted password used for encrypting privacy with the PDU
var pduOutletEndpoint string                                  // SNMP endpoint of the PDU. Should always be the same but made (optionally) configurable in case another model is used
var otherCtrlrOutlet string                                   // Outlet that the other controller is plugged in to
var pduResetCmd = 3                                           // Supported PDU command values are 1 (disable), 2 (enable), 3 (reboot)
const checkModeFreqMS = 1000                                  // Rate in ms at which to poll between client and server
const replyFromOtherAllowableDelay = 3000 * time.Millisecond  // Communication delays longer than this are considered to have failed
const firstTCPConnectionAttemptTimeAllowanceMS = 10000        // How long to wait for c2c connection to be established before erroring and assuming primary

// Execute mode negotiation actions
func checkControllerMode() {
	// Only take over if we are not currently updating
	if controllerMode != Update && otherControllerIsUnresponsive() {
		takeOverAsPrimary()
	} else {
		sendC2CMsg("ModeQuery")
	}
}

// If in Secondary mode, every second check if This controller should take over as Primary
func negotiateControllerMode() {
	checkModeTicker := startTicker(checkModeFreqMS)
	defer checkModeTicker.Stop()
	for {
		if controllerMode != Primary {
			checkControllerMode()
		}
		<-checkModeTicker.C
	}
}

// Checks if TCP connection has been lost or if last mode confirmation from Other was too long ago
func otherControllerIsUnresponsive() bool {
	if !c2c.connected || time.Since(c2c.lastModeConfirmationFromOther) > replyFromOtherAllowableDelay {
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
			sendC2CMsg("GetDBIUpdate")
		}
	}
	c2c.lastModeConfirmationFromOther = time.Now()
}

// If the Other controller asks what mode it should be in, reply telling it what it should be and give health status
func replyToModeQueryMsg() {
	reply := "ModeCommand "
	if controllerMode == Primary || (c2c.amServerNotClient && controllerMode != Update) {
		reply += "Secondary"
	} else {
		reply += "Primary"
	}
	totalHealthScore := dr.healthCheckup()
	reply += fmt.Sprintf(" %f", totalHealthScore)
	sendC2CMsg(reply)
}

// Attempt to connect to other controller then start negotiation
func startControllerModeNegotiation() {
	waitForTCPConnectionAttempt()
	negotiateControllerMode()
}

// Handle configuration for SNMP encryption
func (config *Config) handleEncryptionConfig() error {
	// Configure hashed passwords and decryption key from their hard-coded locations as well
	pduHashedAuth, err := os.ReadFile(pduHashedAuthPath)
	if err != nil {
		return fmt.Errorf("failed to read hashed authentication password: %w", err)
	}
	pduHashedPriv, err := os.ReadFile(pduHashedPrivPath)
	if err != nil {
		return fmt.Errorf("failed to read hashed privacy password: %w", err)
	}

	decryptKeyBytes, err := os.ReadFile(pduDecryptionKeyPath)
	if err != nil {
		return fmt.Errorf("failed to read decryption key: %w", err)
	}
	block, _ := pem.Decode(decryptKeyBytes)
	pduDecryptionKey, err := x509.ParsePKCS1PrivateKey(block.Bytes)
	if err != nil {
		return fmt.Errorf("failed to parse decryption key interface: %w", err)
	}

	decryptedPduAuth, err := rsa.DecryptPKCS1v15(nil, pduDecryptionKey, pduHashedAuth)
	if err != nil {
		return fmt.Errorf("failed to decrypt hashed pdu Authentication password: %w", err)
	}
	pduAuth = strings.TrimSpace(string(decryptedPduAuth))
	decryptedPduPriv, err := rsa.DecryptPKCS1v15(nil, pduDecryptionKey, pduHashedPriv)
	if err != nil {
		return fmt.Errorf("failed to decrypt hashed pdu Privacy password: %w", err)
	}
	pduPriv = strings.TrimSpace(string(decryptedPduPriv))

	return nil
}

// Claim the virtual interface and add the Primary IP to it
func setupPrimaryIP() error {
	if len(primaryIP) < 1 || len(primaryNetworkInterface) != len(primaryIP) {
		return fmt.Errorf("failed to takeover: there must be one network interface for each ip, received %d ips and %d interfaces.", len(primaryIP), len(primaryNetworkInterface))
	}

	for i, ip := range primaryIP {
		// Take the primary IP on this machine
		// Set up virtual device
		ipCmd := exec.Command("ip", "link", "add", primaryNetworkInterface[i], "type", "dummy")
		if ipErr := ipCmd.Run(); ipErr != nil {
			return fmt.Errorf("error adding virtual interface: %w", ipErr)
		}
		// Create IP alias
		ipAliasCmd := exec.Command("ip", "addr", "add", ip, "dev", primaryNetworkInterface[i], "label", primaryNetworkInterface[i])
		if ipAliasErr := ipAliasCmd.Run(); ipAliasErr != nil {
			return fmt.Errorf("error setting IP: %w", ipAliasErr)
		}
		log.Infof("Claimed %s on %s", ip, primaryNetworkInterface[i])
	}
	return nil
}

// Configure and return PDU SNMP connection
func configureSNMP() {
	gosnmp.Default.Target = pduIP
	gosnmp.Default.Version = gosnmp.Version3
	gosnmp.Default.Transport = "udp"
	gosnmp.Default.MsgFlags = gosnmp.AuthPriv
	gosnmp.Default.Community = pduUser
	gosnmp.Default.UseUnconnectedUDPSocket = true
	gosnmp.Default.SecurityModel = gosnmp.UserSecurityModel
	gosnmp.Default.SecurityParameters = &gosnmp.UsmSecurityParameters{
		UserName:                 pduUser,
		AuthenticationProtocol:   gosnmp.SHA,
		PrivacyProtocol:          gosnmp.AES,
		AuthenticationPassphrase: pduAuth,
		PrivacyPassphrase:        pduPriv,
	}
}

// Sets controller mode to Primary, takes over primary IP and restarts the other controller
func takeOverAsPrimary() {
	controllerMode = Primary
	log.Infof("Taking over as Primary controller")
	for _, process := range processJurisdiction {
		process.sendPrimaryFlag(true)
	}

	if enableRedundantFailover {
		// Setup Primary IP for this controller
		if err := setupPrimaryIP(); err != nil {
			log.Errorf("Failover takeover error: %v\n", err)
		}

		// Configure and negotiate SNMP connection
		configureSNMP()
		if err := gosnmp.Default.Connect(); err != nil {
			log.Errorf("Failed to connect to pdu: %v\n", err)
		}

		// Restart the PDU outlet
		result, err := gosnmp.Default.Set([]gosnmp.SnmpPDU{
			{
				Name:  pduOutletEndpoint + otherCtrlrOutlet,
				Type:  gosnmp.Integer,
				Value: pduResetCmd,
			},
		})
		if err != nil {
			log.Errorf("Failed to restart other outlet: %v\n", err)
		}
		if result != nil && result.Error != 0 {
			log.Errorf("Failed to restart other outlet: %v\n", result.Error)
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
