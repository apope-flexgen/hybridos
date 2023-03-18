/**
 * wsdl_interface
 * wsdl_interface.go
 *
 * Created September 2021
 *
 * The wsdl_interface is used for communicated with ISO, receiving and requesting XML power commands over ISO's WSDL (SOAP) API. This machine will act as the client, connecting to the ISO server in order to facilitate communication
 *
 */

package main

import (
	"bufio"
	"bytes"
	"crypto/tls"
	"fims"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"os/signal"
	"strings"
	"time"
)

var iso string                // Name of the ISO
var serverIP string           // The ISO server's IP
var serverPort string         // Server port used to service requests
var apiEndpoint string        // String appended to the server IP to give the full URL to which API requests are sent
var serverQueryRateSecs int   // How often to resend the request for the latest instructions from the ISO
var interrupt chan os.Signal  // Channel used to communicate interrupts
var done chan struct{}        // Channel indicating connection closure
var tlsConfig *tls.Config     // Tls security configuration
var conn *tls.Conn            // Websocket connection to ISO
var soapClient *http.Client   // Client for reading/writing soap requests to the connection
var maxConnectionAttempts int // Number of unsuccessful connection attempts before shutdown and restart of the program
var queryDocument string      // WSDL document envelope for request to ISO, simply stored as a string

// TODO: replace with sequences file that indicates the steps of the handshake with ISO
var requests = []string{"getDispatchBatchesSinceUID", "getDispatchBatch", "validateDispatchBatch(#)"}

// Configure connection to ISO, including loading in certificates and keys to support TLS
func configureInterface() error {
	// Support system interrupt for graceful shutdown of connection
	interrupt = make(chan os.Signal, 1)
	// Channel indicating connection closure
	done = make(chan struct{})
	signal.Notify(interrupt, os.Interrupt)

	// load key and certificate as part of TLS connection
	cert, err := tls.LoadX509KeyPair("/usr/local/etc/config/washer/caiso.crt", "/usr/local/etc/config/washer/caiso.key")
	if err != nil {
		log.Println(err)
		return err
	}

	tlsConfig = &tls.Config{
		Certificates: []tls.Certificate{cert},
	}
	return nil
}

// Main WSDL connection loop, handling various interactions with the ISO server
func handleConnection() {
	// Block until configuration is received and read successfully from DBI
	<-configurationReceived

	err := connectToISO()
	// Connection failure, indicate restart to main loop
	if err != nil {
		log.Println(err)
		close(done)
		return
	}

	// Ticker to query ISO at configured rate
	if serverQueryRateSecs == 0 {
		log.Println("no server query rate received, defaulting to 30 seconds")
		serverQueryRateSecs = 30
	}
	serverQueryTicker := time.NewTicker(time.Duration(serverQueryRateSecs) * time.Second)
	defer serverQueryTicker.Stop()

	for {
		select {
		// Default ticker will restart ISO query every serverQueryRate seconds
		case <-serverQueryTicker.C:
			// Clear the list of sets being tracked for verification
			// If a set was not received, it will be implicitly reissued due to an outdated response for getLatestBatchToQuery()
			fims.VerificationRecords.ResetRecords()
			// A response from fleet manager for this query will be received in washer's fims handler, triggering the ISO query
			getLatestBatchToQuery()
		// System interrupt received, exit gracefully
		case <-interrupt:
			log.Println("program terminated, closing connection")
			conn.Close()
			select {
			case <-done:
			case <-time.After(time.Second):
			}
			return
		case <-done:
			return
		}
	}
}

// Connect to the ISO server
// Will shutdown the program if connection attempts fail five times in a row
func connectToISO() error {
	// Verify TLS configuration before connecting
	if serverIP == "" {
		return fmt.Errorf("connection received nil server IP")
	}
	if tlsConfig == nil {
		return fmt.Errorf("connection received invalid TLS configuration")
	}
	// Begin connection attempts
	err := attemptToConnect(0)
	if err != nil {
		log.Println(err, " restarting WASHER")
		return err
	}
	return err
}

// Recursive connection attempt, will continue trying to connect until the number of attempts exceeds the maximum configured
// A maximum connection attempts of -1 indicates that the program should continue to attempt to connect indefinitely
func attemptToConnect(attempts int) error {
	if attempts >= maxConnectionAttempts && maxConnectionAttempts != -1 {
		return fmt.Errorf("failed to connect to ISO server %d times", attempts)
	}

	// Manually configure dialer with a timeout
	var dialer net.Dialer
	dialer.Timeout = 5 * time.Second
	// Connection attempt
	var err error
	conn, err = tls.DialWithDialer(&dialer, "tcp", serverIP+":"+serverPort, tlsConfig)
	if err != nil || conn == nil {
		log.Println("failed to connect: ", err)
		log.Println("trying again in 3 seconds...")
		time.Sleep(3 * time.Second)
		if maxConnectionAttempts != -1 {
			// Increment connection attempts
			return attemptToConnect(attempts + 1)
		} else {
			// Attempting indefinitely, reset attempts and try again
			return attemptToConnect(0)
		}
	}
	soapClient = &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: tlsConfig,
		},
		Timeout: 3 * time.Second,
	}
	log.Println("Successfully connected to ISO server")
	return nil
}

// Reads from the underlying buffer until the end of the xml is detected as determined by delimiter passed
func readUntilDelim(fullReader *bufio.Reader, delim string) (buffer []byte, err error) {
	buffer = make([]byte, 0)
	// Read from connection
	for {
		// Read up to last byte of delimiter
		currentBytes, err := fullReader.ReadBytes([]byte(delim)[len([]byte(delim))-1])
		if err != nil {
			return nil, err
		}
		// Add the full read to the buffer
		buffer = append(buffer, currentBytes...)
		// If the buffer ends with the full delimiter string, exit and return result
		if bytes.HasSuffix(currentBytes, []byte("</SOAP-ENV:Envelope>")) {
			return buffer, err
		}
	}
}

// Calls parser to parse the message received from ISO and will request further instructions from ISO
// or dispatch commands over fims as appropriate based on result
func handleISOMessage(done chan struct{}, msg *http.Response) {
	// Create a reader for the msg and read all bytes up to and including the closing xml tag
	fullReader := bufio.NewReader(msg.Body)
	responseBuffer, err := readUntilDelim(fullReader, "</SOAP-ENV:Envelope>")
	if err != nil {
		log.Println("Failed to read complete WSDL response from ISO")
		return
	}
	// Parse the response depending on the type of message, into either a list of batchUIDs or a list of FimsObjects
	parsedBatchList, parsedFimsObjs, responseType, err := parseSOAPEnvelope(responseBuffer)
	if err != nil {
		log.Println("Parsing error: ", err)
		return
	}
	if parsedBatchList == nil && parsedFimsObjs == nil {
		log.Println("Parsing failed to extract any messages from ISO response")
		return
	}
	if responseType == "getDispatchBatchesSinceUIDResponse" {
		// This ISO response contains a list of instructions, follow up for more details for each
		for _, parsedBatch := range parsedBatchList {
			// Request full dispatch instructions for each id (batchUID)
			sendISORequest(done, requests[1], parsedBatch)
		}
	} else if responseType == "getDispatchBatchResponse" {
		// This ISO response contains a list of dispatched batches, parsed as FimsObjs
		// Send each fims object parsed to its associated URI
		sendFimsObjs(parsedFimsObjs)
	}
}

// Send the given request to the ISO server over the TLS connection
// Will manually notify to close the connection on failure to send
// Will send the specified command, replacing the special character with the dispatch batch specified
func sendISORequest(done chan struct{}, query string, batchUID string) {
	if conn == nil {
		log.Println("Socket writer received invalid connection")
		close(done)
		return
	}
	if queryDocument == "" {
		log.Println("ISO query document unconfigured")
		close(done)
		return
	}
	if query == "" || batchUID == "" {
		log.Println("Received invalid arguments for ISO query")
		// Connection not closed in this case
		return
	}
	// Replace the special keywords #query and #batchUID with the type of query and parsed ID received
	msg := strings.Replace(queryDocument, "#query", query, -1)
	msg = strings.Replace(msg, "#batchUID", batchUID, -1)
	// API requests do not use the server port, just the IP and endpoint
	response, err := soapClient.Post("https://"+serverIP+apiEndpoint, "text/xml", bytes.NewBufferString(msg))
	if err != nil || response == nil {
		log.Println("Failed to write: ", err)
		// TODO: should this crash the system? The map server seems to frequently time out
		// close(done)
		return
	}
	defer response.Body.Close()
	if response.StatusCode != 200 {
		log.Println("Sent message to ISO but received back unsuccessful status code: ", response.StatusCode)
		// TODO: should this crash the system? The map server seems to frequently time out
		// close(done)
		return
	}
	handleISOMessage(done, response)
}
