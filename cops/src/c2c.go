/**
 * COPS-to-COPS (C2C) Messaging Interface
 *
 * Created February 2021
 *
 * C2C is a messaging interface used by COPS that utilizes a TCP client-server
 * scheme to send data between separate Site Controllers. This is a required
 * tool for the redundant failover feature.
 *
 */

package main

import (
	"bufio"
	"crypto/tls"
	"crypto/x509"
	_ "embed"
	"encoding/json"
	"fims"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"path"
	"strconv"
	"strings"
	"sync"
	"time"
	"unicode"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// C2C holds variables necessary for COPS-to-COPS communication
type C2C struct {
	tcpConnection                 net.Conn
	tcpListener                   net.Listener
	amServerNotClient             bool
	connected                     bool
	lastModeConfirmationFromOther time.Time
	connectionLock                sync.RWMutex // Lock on establishing connection
	resolvingConnection           bool         // Whether a connection error is currently being handled
}

// Global variables
var c2c C2C
var startWord = "C2C" // indicates beginning of TCP message in serial stream
var thisCtrlrStaticIP string
var otherCtrlrStaticIP string
var bufferSize = 65536 // Size of the TCP buffer, default 4096, set manually to accommodate larger config files

// Constants
const waitUntilNextRead = time.Second // Wait this long before the next attempt to read if disconnected
const serverKeyPairPath = "/usr/local/etc/config/cops/"
const serverCrtPath = serverKeyPairPath + "c2c_server.crt"
const serverKeyPath = serverKeyPairPath + "c2c_server.key"
const C2CPort = ":8000"

// Set to one second to generate inital event if connection error occurs
var eventTimer = time.NewTimer(1 * time.Second)
var eventConnectionCheck bool

// Establish a connection wait time for events to generate after the initial event occurs
var connectionEventWaitTime = 1 * time.Second

// Error wrapper to distinguish connection errors
type connectionError struct {
	err error
}

func (m *connectionError) Error() string {
	return m.err.Error()
}

// Error wrapper to distinguish c2c parsing errors
type parseError struct {
	err error
}

func (m *parseError) Error() string {
	return m.err.Error()
}

func (c2c *C2C) close() {
	c2c.connectionLock.Lock()
	defer c2c.connectionLock.Unlock()
	if c2c.tcpConnection != nil {
		c2c.tcpConnection.Close()
	}
}

func (c2c *C2C) setConnection(newConn net.Conn) {
	c2c.connectionLock.Lock()
	defer c2c.connectionLock.Unlock()

	// Send event status when a valid connection has been made
	sendEvent(Status, "Controller has made a connection via c2c.")
	eventConnectionCheck = false

	// Reset event timer that generates an event when c2c.connected is true
	// and we receive a connection error during retrieving socket data
	// Otherwise, this event is generated one time for a connected instance
	eventTimer.Reset(connectionEventWaitTime)

	c2c.tcpConnection = newConn
	c2c.connected = true
	c2c.resolvingConnection = false
	c2c.lastModeConfirmationFromOther = time.Now()
}

// Handle a C2C connection error
func (c2c *C2C) handleConnectionError() {
	// Make sure a new connection attempt is not already ongoing
	c2c.connectionLock.Lock()
	// Do not reconnect is connection is already being resolved
	if c2c.resolvingConnection {
		c2c.connectionLock.Unlock()
		return
	}
	c2c.connected = false
	c2c.resolvingConnection = true
	c2c.connectionLock.Unlock()
	connectOverTCP()
}

// Verifies an IP address has 4 fields and ends with a port number
func checkIPAddrFormat(ip string) error {
	if strings.Count(ip, ":") != 1 {
		return fmt.Errorf("port formatting is invalid. Expected format: #.#.#.#:Port#")
	}
	if net.ParseIP(ip[:strings.Index(ip, ":")]) == nil {
		return fmt.Errorf("IP address formatting is invalid. Expected format: #.#.#.#:Port#")
	}
	return nil
}

// Returns true if the first IP address & port is less than the second IP address & port.
func isIPLessThan(thisCtrlrIP, otherCtrlrIP string) (bool, error) {
	ip1, err := numberfyIP(thisCtrlrIP)
	if err != nil {
		return false, fmt.Errorf("converting thisCtrlrStaticIP to a number: %w", err)
	}

	ip2, err := numberfyIP(otherCtrlrIP)
	if err != nil {
		return false, fmt.Errorf("converting otherCtrlrStaticIP to a number: %w", err)
	}

	if ip1 == ip2 {
		return false, fmt.Errorf("IP addresses are identical.")
	}

	return ip1 < ip2, nil
}

// Called at the beginning of COPS to configure important C2C variables and begin listening for new connection requests
func configureC2C(config Config) error {
	var err error
	c2c.connected = false
	thisCtrlrStaticIP = config.ThisCtrlrStaticIP + C2CPort
	otherCtrlrStaticIP = config.OtherCtrlrStaticIP + C2CPort
	c2c.amServerNotClient, err = isIPLessThan(thisCtrlrStaticIP, otherCtrlrStaticIP)
	if err != nil {
		return fmt.Errorf("comparing thisCtrlrStaticIP, otherCtrlrStaticIP: %w", err)
	}

	if c2c.amServerNotClient {
		log.Infof("Running as TCP server")
		c2c.tcpListener, err = listenToLocalPort(thisCtrlrStaticIP)
		if err != nil {
			return fmt.Errorf("error listening to local port: %w", err)
		}
	} else {
		log.Infof("Running as TCP client")
	}
	go connectOverTCP()
	go processC2C()
	return nil
}

// Connect to either TCP client or TCP server then start processing any messages that appear on connection
func connectOverTCP() {
	if c2c.amServerNotClient {
		connectToTCPClient()
	} else {
		connectToTCPServer(otherCtrlrStaticIP)
	}

	// Report on server/client connection status
	if c2c.amServerNotClient {
		log.Infof("Server - TCP connection established - status: %v", c2c.connected)
	} else {
		log.Infof("Client - TCP connection established - status: %v", c2c.connected)
	}
}

// Closes the existing connection and listens for a new TCP client connection request
func connectToTCPClient() {
	c2c.close()
	for {
		newClientConn, err := c2c.tcpListener.Accept()
		if err != nil {
			log.Errorf("Error: Received TCP connection request that could not be accepted")
		} else {
			c2c.setConnection(newClientConn)
			return
		}
	}
}

// Attempts to connect to the server every 1 second and returns TCP connection when successful
func connectToTCPServer(serverAddr string) {
	for {
		// add the server certificate to the list of certificate authorities trusted by the client
		caCert, err := ioutil.ReadFile(serverCrtPath)
		if err != nil {
			log.Errorf("Error trying to load trusted server certificate: %s", err)
			time.Sleep(time.Second) // Wait a second before trying again
			continue
		}
		caCertPool := x509.NewCertPool()
		caCertPool.AppendCertsFromPEM(caCert)

		// configure dialer with certificate to allow it through the HTTPS server's encryption
		tlsConfig := &tls.Config{RootCAs: caCertPool}
		tcpServerConnection, err := tls.Dial("tcp", serverAddr, tlsConfig)
		if err == nil {
			c2c.setConnection(tcpServerConnection)
			return
		}
		log.Errorf("error dialing tcp: %v", err)
		time.Sleep(time.Second) // Wait a second before trying again
	}
}

// Establishes a TCP server by creating a listener pointed at a local port
func listenToLocalPort(localAddr string) (net.Listener, error) {
	// load server certificate and private key
	tlsCert, err := tls.LoadX509KeyPair(serverCrtPath, serverKeyPath)
	tlsConfig := &tls.Config{Certificates: []tls.Certificate{tlsCert}}
	if err != nil {
		return nil, fmt.Errorf("error trying to load server key pair: %w", err)
	}
	ln, err := tls.Listen("tcp", localAddr, tlsConfig)
	if err != nil {
		return nil, fmt.Errorf("error trying to establish listen to local TCP port: %w", err)
	}
	return ln, err
}

// Converts an IP address with the format #.#.#.#:Port# to a concatenated 18-digit number for comparison with another IP address
func numberfyIP(ip string) (uint64, error) {
	if err := checkIPAddrFormat(ip); err != nil {
		return 0, fmt.Errorf("checking IP address format: %w", err)
	}

	ipFrags, err := splitIPAddr(ip)
	if err != nil {
		return 0, fmt.Errorf("splitting IP address: %w", err)
	}

	// Concatenate the IP string
	ipConcatenatedString := fmt.Sprintf("%03d%03d%03d%03d%06d", ipFrags[0], ipFrags[1], ipFrags[2], ipFrags[3], ipFrags[4])
	ipConcatenatedNumber, err := strconv.ParseUint(ipConcatenatedString, 10, 64)
	if err != nil {
		return 0, fmt.Errorf("error converting IP address to number")
	}

	return ipConcatenatedNumber, nil

}

// Read a c2c message
func getC2CMessage() (string, error) {
	c2c.connectionLock.RLock()
	defer c2c.connectionLock.RUnlock()

	// Check status and generate event if we never initially connected
	if !eventConnectionCheck {
		if !c2c.connected {
			// We currently never made an initial connection and have not generated an initial fault
			sendEvent(Fault, "C2C is not currently connected. Potentially have two controllers in primary mode.")
			eventConnectionCheck = true
		}
	}

	// Only read if connected
	if !c2c.connected {
		time.Sleep(waitUntilNextRead)
		return "", nil
	}

	// Try reading for the configured period
	c2c.tcpConnection.SetReadDeadline(time.Now().Add(replyFromOtherAllowableDelay))
	reader := bufio.NewReaderSize(c2c.tcpConnection, bufferSize)
	startOfMsg, err := reader.Peek(len(startWord))
	if err != nil {
		return "", &connectionError{err}
	} else if string(startOfMsg) != startWord {
		reader.Discard(1)
		return "", &parseError{fmt.Errorf("message started with %s, not %s", string(startOfMsg), startWord)}
	}

	// If no errors have occurred, now read the entire message
	var msg string
	msg, err = parseC2CMsg(reader)
	if err != nil {
		return "", &parseError{err}
	}

	return msg, nil
}

// Parses a C2C message and returns either the message contents or an error message, indicated by error flag
func parseC2CMsg(reader *bufio.Reader) (string, error) {
	_, err := reader.Discard(len(startWord))
	if err != nil {
		return "", fmt.Errorf("Error trying to discard message start word")
	}
	msgSizeString, err := reader.ReadString(' ')
	if err != nil {
		return "", fmt.Errorf("Error parsing received message size")
	}
	msgSize, err := strconv.Atoi(strings.Trim(msgSizeString, " "))
	if err != nil {
		return "", fmt.Errorf("Error parsing received message size")
	}
	msgBuffer := make([]byte, msgSize)
	c2c.tcpConnection.SetReadDeadline(time.Now().Add(replyFromOtherAllowableDelay))
	// Need ReadFull to guarantee a full read of msgSize bytes
	_, err = io.ReadFull(reader, msgBuffer)
	if err != nil {
		return "", fmt.Errorf("Error reading from message buffer")
	}
	msg := string(msgBuffer)
	return msg, nil
}

// Waits for a new C2C message to appear on a TCP connection and processes it
func processC2C() {
	for {
		msg, err := getC2CMessage()
		if err != nil {
			switch err.(type) {
			case *connectionError:
				// Select on timer only if timer is reset by a reset connection status
				select {
				case <-eventTimer.C:
					// Generate event
					sendEvent(Fault, "TCP connection error receiving C2C message. Potentially have two controllers in primary mode.")

				default:
					// Do nothing, continue with the rest of the logic
				}

				// Log error more often than event
				log.Errorf("TCP Connection error: %s", err.Error())
				go c2c.handleConnectionError()
				continue
			case *parseError:
				log.Errorf("C2C Parsing error: %s", err.Error())
				continue
			}
		}

		if strings.HasPrefix(msg, "ModeQuery") {
			// Mode Queries, controllers negotiate who is primary
			replyToModeQueryMsg()
		} else if strings.HasPrefix(msg, "ModeCommand") {
			// Mode Commands, other controller has commanded this one to run in a particular mode
			processModeCommand(msg)
		} else if strings.HasPrefix(msg, "Setpoints") {
			handleSetpoints(msg)
		} else if strings.HasPrefix(msg, "GetDBIUpdate") {
			// The other controller (secondary) was offline and missed setpoints and configuration data
			// Get all data this primary has in DBI and send it to the other controller through c2c (handled elsewhere)
			getDBIUpdate()
		} else if strings.HasPrefix(msg, "SetDBIUpdate") {
			handleDbiUpdateSet(msg)
		}

	}
}

// Convert the message to the required C2C format and send it
func sendC2CMsg(msgBody string) {
	c2c.connectionLock.RLock()
	if !c2c.connected {
		c2c.connectionLock.RUnlock()
		return
	}

	msgHeader := startWord + strconv.Itoa(len(msgBody)) + " "

	c2c.tcpConnection.SetWriteDeadline(time.Now().Add(replyFromOtherAllowableDelay))
	_, err := c2c.tcpConnection.Write([]byte(msgHeader + string(msgBody)))
	c2c.connectionLock.RUnlock()
	if err != nil {
		select {
		case <-eventTimer.C:
			// Generate event
			sendEvent(Fault, "TCP connection error sending C2C message. Are two controllers in primary mode?")

			// Don't generate another event until defined wait time
			eventTimer.Reset(connectionEventWaitTime)
		default: // Do nothing, continue with the rest of the logic
		}
		c2c.handleConnectionError()
	}
}

// Splits an IP address string into five integer parts: 4 IP address fields and its port number
func splitIPAddr(ip string) (ipFrags [5]int, err error) {

	f := func(c rune) bool {
		return !unicode.IsNumber(c)
	}

	for i, ipFragStr := range strings.FieldsFunc(ip, f) {
		ipFrag, err := strconv.Atoi(ipFragStr)
		if err != nil {
			return ipFrags, fmt.Errorf("parsing number from IP address: %w", err)
		}

		ipFrags[i] = ipFrag
	}
	return ipFrags, nil
}

// Convert a fims message (uri, body) to a single string that can be sent via C2C
// Use '$$' as a delimiter
//
//	the JSON may contain spaces, commas, etc
func fimsToC2C(keyword string, uri string, body interface{}) (message string) {
	bodyJSON, err := json.Marshal(body)
	if err != nil {
		log.MsgError("Error parsing message JSON")
		return
	}
	return keyword + "$$" + uri + "$$" + string(bodyJSON)
}

// Parse out keyword and COPS prefix, fix JSON
func parseDBIMsg(uriParse bool, prefixURI string, msg string) (uri string, body interface{}) {
	fullUri, err := extractUriFromC2CMsg(msg)
	if err != nil {
		log.Errorf("Error: failed to extract URI from C2C msg: %s", err.Error())
	}

	c2cBody, err := extractBodyFromC2CMsg(msg)
	if err != nil {
		log.Errorf("Error: failed to extract body from C2C msg: %s", err.Error())
	}

	// Indicating whether to parse out the base URI or use the URI provided, Examples:
	// uriParse == true,  prefixURI:     /cops/site_controller/setpoints,
	//					  fragments[1]:  /cops/site_controller/setpoints/assets/ess/ess_1/maint_mode
	//					  uri: 			 /assets/ess/ess_1/maint_mode
	// uriParse == false, prefixURI:     /assets/ess/ess_1/maint_mode
	//					  uri:			 /assets/ess/ess_1/maint_mode
	if uriParse {
		uri = fullUri[len(prefixURI):]
		// First try to parse URIs with an explicit destination i.e. site setpoints
		// If the destination is undefined, use the source as the destination as well i.e. scheduler
		if uri == "" {
			uri = prefixURI[len("/cops"):]
		}
	} else {
		uri = prefixURI
	}
	var jsonBody map[string]interface{}
	json.Unmarshal([]byte(c2cBody), &jsonBody)
	body = jsonBody
	return
}

// Parse out the keyword and COPS prefix and send the set containing setpoint(s)
func sendDBIUpdate(uriParse bool, uri string, msg string) {
	uri, body := parseDBIMsg(uriParse, uri, msg)
	// Send the parsed setpoint(s)
	f.SendSet(uri, "", body)
}

// Handles the response back from dbi with the latest configuration data
func handleDBIUpdate(msg fims.FimsMsg) {
	// As the primary, the other controller (secondary) has requested our latest config on its startup
	// This controller sent the request to DBI, and has now received the response for each uri (document) requested

	// Send the document we got from DBI for this uri
	// Sets will be received and written through C2C one-by-one, with the last set containing the uri keyword /dbi_update_complete
	sendC2CMsg(fimsToC2C("SetDBIUpdate", msg.Uri, msg.Body))
}

// `handleSetpoints` takes setpoint updates received from the Primary controller and passes them
// along to the appropriate DBI entry.
func handleSetpoints(msg string) error {
	// extract the target URI and target process from the msg
	targetUri, err := extractUriFromC2CMsg(msg)
	if err != nil {
		return fmt.Errorf("failed to extract target URI from C2C msg: %v", err.Error())
	}

	// extract the target process from the URI
	targetProcess, err := extractTargetProcessFromC2CUri(msg)
	if err != nil {
		return fmt.Errorf("failed to extract target process from C2C URI: %v", err.Error())
	}

	// iterate through each of the process's writeout URIs to find one that matches the target URI
	for _, writeOut := range targetProcess.writeOutC2C {
		// if match found, pass update along to DBI entry
		if strings.Contains(targetUri, writeOut) {
			sendDBIUpdate(true, writeOut, msg)
			return nil
		}
	}
	return fmt.Errorf("target URI %v does not match any writeout URIs in target process %v", targetUri, targetProcess.name)
}

// Handles writing out setpoint data. If in Update mode, also tracks any processes that have been updated in order to restart them as needed
func handleSetpointWriteout(msg fims.FimsMsg, process *processInfo) {
	// Writeout our setpoint sent from our process to the other process
	if controllerMode == Primary {
		// Convert the setpoint message to a C2C message and send to the secondary process
		sendC2CMsg(fimsToC2C("Setpoints", msg.Uri, msg.Body))
		return
	} else if controllerMode == Update {
		// If updating, record all processes that have received updates so they can be restarted on exiting the mode
		// This will include both the configuration changes uploaded to this Update controller,
		// and any setpoints that may be received from the primary while updating
		updatedProcesses = append(updatedProcesses, process)
	}
}

// `handleDbiUpdateSet` processes a C2C message from the Primary controller that contains the
// most up-to-date setpoints. The message is expected to come after this controller starts
// up as Secondary and sends a GET request to the Primary controller over C2C asking for its
// setpoints. The updated setpoints will be sent to this controller's DBI so that the updated
// process can get them from there.
//
// A process can have multiple groups of setpoints, so multiple C2C messages will be received
// (triggering this function once per setpoint group). A message that does not contain any
// setpoints and instead only contains dbi_update_complete will signal that no more setpoints
// are coming for that process.
//
// When the all setpoint groups are received, processes that must be restarted in order to GET
// new setpoints from DBI (i.e. site_controller) will be killed and processes that can accept new
// setpoints without a restart (i.e. scheduler) will have a dbi_update flag sent to them, telling
// them to GET the setpoints from DBI.
func handleDbiUpdateSet(msg string) {
	// extract the target URI and target process from the msg
	targetUri, err := extractUriFromC2CMsg(msg)
	if err != nil {
		log.Errorf("failed to extract target URI from C2C msg: %v", err.Error())
	}

	// extract the target process from the URI
	targetProcess, err := extractTargetProcessFromC2CUri(msg)
	if err != nil {
		log.Errorf("failed to extract target process from C2C URI: %v", err.Error())
	}

	// check the msg to see if it is a dbi_update_complete, indicating there are no more setpoint groups for which to wait
	if strings.Contains(targetUri, path.Join(targetProcess.name, "/dbi_update_complete")) {
		// if the process requires a restart in order to GET updated setpoints from DBI (i.e. site_controller), then kill it so it can be restarted.
		// else, send dbi_update to the process so it knows it needs to GET updated setpoints from DBI (process such as scheduler can do this).
		if targetProcess.configRestart {
			log.Warnf("Starting a delayed kill for %s", targetProcess.name)
			go delayedkill(targetProcess, 1000) // delay the kill in case the process has not yet fully started up and been assigned a PID
		} else {
			log.Infof("Sending dbi_update flag to %s", path.Join(targetProcess.uri, "operation/dbi_update"))
			f.SendSet(path.Join(targetProcess.uri, "/operation/dbi_update"), "", true)
		}
		return
	}

	// check the target URI against each of the target process's writeout URIs.
	// if a match is found, send the received data along to that writeout URI's DBI entry.
	for _, writeOutUri := range targetProcess.writeOutC2C {
		if strings.Contains(targetUri, path.Join(writeOutUri, "/dbi_update")) {
			_, setpointGroupName := path.Split(writeOutUri)
			sendDBIUpdate(false, path.Join("/dbi", targetProcess.name, setpointGroupName), msg)
			return
		}
	}
	log.Errorf("target URI %v did not match any writeout URIs for target process %v", targetUri, targetProcess)
}

// `extractUriFromC2CMsg` extracts only the URI from a C2C msg.
func extractUriFromC2CMsg(msg string) (string, error) {
	c2cFrags := strings.Split(msg, "$$")
	if len(c2cFrags) != 3 {
		return "", fmt.Errorf("C2C message has %v fragments but should have 3", len(c2cFrags))
	}
	return c2cFrags[1], nil
}

// `extractTargetProcessFromC2CUri` parses out a process name from a C2C URI and searches the process jurisdiction for it.
// If found, returns pointer. Else, returns nil
func extractTargetProcessFromC2CUri(uri string) (*processInfo, error) {
	// extract the target process name from the URI
	uriFrags := strings.Split(uri, "/")
	if len(uriFrags) < 3 { // len == 3 would mean 2 frags since leading slash creates an empty frag
		return nil, fmt.Errorf("URI has %v fragments but expected at least 2. Received URI: %v", len(uriFrags), uri)
	}
	targetProcessName := uriFrags[2]

	// search for the target process in the process jurisdiction
	targetProcess := getProcess(targetProcessName)
	if targetProcess == nil {
		return nil, fmt.Errorf("could not find process %v in the process jurisdiction", targetProcessName)
	}
	return targetProcess, nil
}

// `extractBodyFromC2CMsg` extracts only the body from a C2C msg.
func extractBodyFromC2CMsg(msg string) (string, error) {
	c2cFrags := strings.Split(msg, "$$")
	if len(c2cFrags) != 3 {
		return "", fmt.Errorf("C2C message has %v fragments but should have 3", len(c2cFrags))
	}
	return c2cFrags[2], nil
}
