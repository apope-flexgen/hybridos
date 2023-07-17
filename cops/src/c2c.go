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
func checkIPAddrFormat(ip string) {
	if strings.Count(ip, ":") != 1 {
		panic("Port formatting is invalid. Expected format: #.#.#.#:Port#")
	}
	if net.ParseIP(ip[:strings.Index(ip, ":")]) == nil {
		panic("IP address formatting is invalid. Expected format: #.#.#.#:Port#")
	}
}

// Returns true if the first IP address & port is less than the second IP address & port. Panics if identical addresses/ports
func compareIPAddrs(ip1Str, ip2Str string) bool {
	ip1 := numberfyIP(ip1Str)
	ip2 := numberfyIP(ip2Str)
	if ip1 == ip2 {
		panic("Identical IP addresses given. Primary and secondary controllers must be separate instances")
	}
	return ip1 < ip2
}

// Concatenates an array of IP address fields into a standardized format for comparison with another IP address
func concatIPFrags(ipFrags [5]int) uint64 {
	ipConcatenatedString := fmt.Sprintf("%03d%03d%03d%03d%06d", ipFrags[0], ipFrags[1], ipFrags[2], ipFrags[3], ipFrags[4])
	ipConcatenatedNumber, err := strconv.ParseUint(ipConcatenatedString, 10, 64)
	fatalErrorCheck(err, "Error converting IP address to number")
	return ipConcatenatedNumber
}

// Called at the beginning of COPS to configure important C2C variables and begin listening for new connection requests
func configureC2C(config cfg) error {
	c2c.connected = false
	thisCtrlrStaticIP = config.thisCtrlrStaticIP + ":8000"
	otherCtrlrStaticIP = config.otherCtrlrStaticIP + ":8000"
	c2c.amServerNotClient = compareIPAddrs(thisCtrlrStaticIP, otherCtrlrStaticIP)
	if c2c.amServerNotClient {
		log.Infof("Running as TCP server")
		var err error
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
	log.Infof("TCP connection established")
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
func numberfyIP(ip string) uint64 {
	checkIPAddrFormat(ip)
	ipFrags := splitIPAddr(ip)
	return concatIPFrags(ipFrags)
}

// Read a c2c message
func getC2CMessage() (string, error) {
	c2c.connectionLock.RLock()
	defer c2c.connectionLock.RUnlock()
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
		log.Errorf("TCP Connection error: %v", err)
		c2c.handleConnectionError()
	}
}

// Splits an IP address string into five integer parts: 4 IP address fields and its port number
func splitIPAddr(ip string) (ipFrags [5]int) {
	f := func(c rune) bool {
		return !unicode.IsNumber(c)
	}
	for i, ipFragStr := range strings.FieldsFunc(ip, f) {
		ipFrag, err := strconv.Atoi(ipFragStr)
		fatalErrorCheck(err, "Error parsing number from IP address")
		ipFrags[i] = ipFrag
	}
	return
}

// Convert a fims message (uri, body) to a single string that can be sent via C2C
// Use '$$' as a delimiter
//		the JSON may contain spaces, commas, etc
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
