/**
 *
 * websocket.go
 *
 * All code related to managing the HTTPS connection between Fleet Manager and Site Controller instances of Scheduler.
 *
 * Due to regulations, Fleet Manager will always be the client and initiate the connection to its Site Controllers, which will always be servers.
 *
 * To generate a self-signed security certificate:
 * sudo vi /etc/pki/tls/openssl.cnf
 * Find section [ v3_ca ] and subsection "Extensions for a typical CA"
 *
 * For just a single server...
 * Add line: subjectAltName = IP:<IP address of server box>
 *
 * If you want multiple servers to share a certificate...
 * Add line: subjectAltName = @alt_names
 * Above [ v3_ca ] section, add an [ alt_names ] section
 * 		[ alt_names ]
 *		IP.1 = <IP address of first server box>
 * 		IP.2 = <IP address of second server box>
 *		...
 * 		IP.n = <IP address of nth server box>
 *
 * Esc + wq
 * openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:4096 -keyout /usr/local/etc/config/scheduler/my.key -out /usr/local/etc/config/scheduler/my.crt
 * You will be prompted to enter fields such as Country Name, Organization Name, Common Name, etc. Just hit Enter to skip through them
 *
 */

package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/url"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

// webSocketConfig contains all information needed for a websocket connection to a Site Controller
type webSocketConfig struct {
	address string
	url     string
	endConn chan interface{}
	atomicWebSocketConn
}

// webSocketMsg defines the fields a message should have that is being sent or received through the Fleet-Site interface websocket.
type webSocketMsg struct {
	Method  string      `json:"method"`  // set, get, etc...
	Uri     string      `json:"uri"`     // target recipient
	Body    interface{} `json:"body"`    // data payload
	ReplyTo string      `json:"replyTo"` // endpoint that any reply data should be sent to
}

// data structure for a thread-safe WebSocket connection
type atomicWebSocketConn struct {
	conn *websocket.Conn
	mu   sync.RWMutex
}

// upgrader allows an HTTP connection to be upgraded to be a websocket.
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin:     func(r *http.Request) bool { return true },
}

var webSocketReceive chan *webSocketMsg

// adds a received WebSocket message to webSocketReceive so main thread can process it
func addWebSocketMsgToReadQueue(msg *webSocketMsg) {
	webSocketReceive <- msg
}

// newWsConfig builds a new `webSocketConfig` object to hold websocket connection data.
func newWsConfig(address string) *webSocketConfig {
	var ws webSocketConfig
	ws.address = address
	ws.endConn = make(chan interface{}, 1)
	ws.url = (&url.URL{Scheme: "wss", Host: address, Path: "/ws"}).String()
	return &ws
}

// assignHTTPHandlers declares which functions should be called when an HTTP request is received to a certain endpoint.
func assignHTTPHandlers() {
	http.HandleFunc("/ws", createWebSocket)
}

// startServer configures an HTTPS server for the Site Controller box and starts it listening for a client connection from the Fleet Manager.
func startServer(port string) error {
	// load certificate and private key
	cert, err := tls.LoadX509KeyPair("/usr/local/etc/config/scheduler/my.crt", "/usr/local/etc/config/scheduler/my.key")
	if err != nil {
		return err
	}

	// initialize server
	httpServer = &http.Server{
		Addr: port,
		TLSConfig: &tls.Config{
			Certificates: []tls.Certificate{cert},
		},
	}

	go func() {
		// set a blocker to give server time to properly shutdown when it is closed
		httpServerExitDone = &sync.WaitGroup{}
		httpServerExitDone.Add(1)
		defer httpServerExitDone.Done() // let parent goroutine know when server is done cleaning up (post-shutdown)

		// run the server
		if err := httpServer.ListenAndServeTLS("", ""); err != nil && err != http.ErrServerClosed {
			log.Println("ListenAndServer():", err)
		}
	}()

	log.Println("Waiting for Fleet Manager to connect.")
	log.Println("Could take up to 30 seconds...") // because after 10 seconds of Fleet Manager failing to connect, it will slow down to attempts every 30 seconds

	return nil
}

// createWebSocket upgrades an HTTP connection to a websocket and begins reading from it.
func createWebSocket(w http.ResponseWriter, r *http.Request) {
	// upgrade HTTP connection to be a websocket
	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println(err)
		return
	}

	if ws == nil {
		log.Println("Failed to establish connection to Fleet Manager")
		return
	}

	// save websocket connection to global variable
	fleetManagerRW.setConnection(ws)
	log.Println("Successfully established connection with Fleet Manager")

	// send pub to update about connection status
	f.SendPub("/scheduler/connected", buildConnectionMap())

	// begin reading
	readFromFleetManager()
}

// readFromFleetManager uses the websocket connection to listen for messages from Fleet Manager and process them.
func readFromFleetManager() {
	// cannot put mutex on conn usage since Read JSON will block,
	// so include recover function in case conn gets set to nil between check and usage.
	// recover function will stop the seg fault panic and simply return the function
	defer recover()

	for {
		if !fleetManagerRW.isConnected() {
			log.Println("Lost connection to Fleet Manager. Waiting for reconnection...")
			markFleetManConnectionClosed()
			return
		}

		var msg webSocketMsg
		err := fleetManagerRW.conn.ReadJSON(&msg)
		if err != nil {
			log.Println("Error reading:", err)
			log.Println("Lost connection to Fleet Manager. Waiting for reconnection...")
			markFleetManConnectionClosed()
			return
		}

		// add received message to processing queue for main thread to handle
		addWebSocketMsgToReadQueue(&msg)
	}
}

// writeToFleetManager puts a websocket message onto the websocket for sending to the Fleet Manager box.
// Only one write allowed at a time, so mutex employed to make this operation atomic.
func writeToFleetManager(msg webSocketMsg) error {
	fleetManagerRW.mu.Lock()
	defer fleetManagerRW.mu.Unlock()
	if fleetManagerRW.conn == nil {
		return fmt.Errorf("cannot write to Fleet Manager - no connection")
	}
	return fleetManagerRW.conn.WriteJSON(msg)
}

// connect reads the security certificate it should trust and attempts to connect to a Site Controller.
// If successful, syncing messages are sent such as sending modes and a timestamp of the last time the Fleet Manager schedule was edited.
func (site *siteController) connect(firstAttempt bool) error {
	if firstAttempt {
		log.Println("Connecting to", site.id, "Site Controller at:", site.ws.url)
	}

	// add the server certificate to the list of certificate authorities trusted by the client
	caCert, err := ioutil.ReadFile("/usr/local/etc/config/scheduler/my.crt")
	if err != nil {
		return err
	}
	caCertPool := x509.NewCertPool()
	caCertPool.AppendCertsFromPEM(caCert)

	// configure dialer with certificate to allow it through the HTTPS server's encryption
	dialer := websocket.Dialer{
		TLSClientConfig: &tls.Config{
			RootCAs: caCertPool,
		},
	}

	// try to dial the server
	conn, resp, err := dialer.Dial(site.ws.url, nil)
	if err != nil {
		site.ws.setConnection(nil)
		if err == websocket.ErrBadHandshake {
			log.Printf("handshake failed with status %d", resp.StatusCode)
		}
		return err
	}

	// save successful connection
	site.ws.setConnection(conn)
	log.Println("Successfully connected to", site.id, "Site Controller")

	// pub with update about connection status
	f.SendPub("/scheduler/connected", buildConnectionMap())

	// update the Site Controller with Fleet Manager's modes
	sendModesThroughWebSocket(fmt.Sprintf("/siteController/%v/modes", site.id))

	// send the lastScheduleModification time so that the Site Controller can either request a schedule sync or send a schedule sync
	// Site Controller will also send its timezone when it sees this
	var msg webSocketMsg
	msg.Method = "set"
	msg.Body = lastScheduleModification
	msg.Uri = "/siteController/updateSync"
	msg.ReplyTo = fmt.Sprintf("/masterSchedule/%s/events", site.id)
	site.write(msg)
	return nil
}

// readWithConnection reads from the websocket connection to a Site Controller if that connection is still open.
// If the connection fails, Fleet Manager will begin polling the Site Controller in an attempt to reconnect.
// If the connection is purposefully closed (the Scheduler is being reconfigured), abandon polling and the connection.
func (site *siteController) readWithConnection() {
	// try to reconnect if disconnected. if connection purposefully closed, leave thread without starting new routine
	if !site.isConnected() {
		connTerminated := site.pollToReconnect()
		if connTerminated {
			return
		}
	}

	// read from connection
	for {
		select {
		case <-site.ws.endConn:
			// if the connection is purposefully closed, leave thread without starting new routine
			return
		default:
		}
		// if the connection breaks erroneously, try to restart connection with new goroutine
		if !site.read() {
			go site.readWithConnection()
			break
		}
	}
}

// pollToReconnect tries to reconnect to a failed Site Controller connection.
// It polls every second for ten seconds, then slows down to one poll every 30 seconds.
func (site *siteController) pollToReconnect() bool {
	// poll every second for ten seconds
	for i := 0; i < 10; i++ {
		connTerminated := site.connectionAttemptWithLag(1)
		if connTerminated {
			return true
		}
		if site.ws.isConnected() {
			return false
		}
	}

	// slow down to one poll every 30 seconds indefinitely
	for {
		connTerminated := site.connectionAttemptWithLag(30)
		if connTerminated {
			return true
		}
		if site.ws.isConnected() {
			return false
		}
	}
}

// connectionAttemptWithLag waits for a lag, then tries to reconnect to a Site Controller.
// If the connection is declared formally closed during the lag, then the connection attempt is abandoned.
func (site *siteController) connectionAttemptWithLag(lag time.Duration) (connTerminated bool) {

	ticker := time.NewTicker(lag * time.Second)

	select {
	case <-site.ws.endConn:
		return true
	case <-ticker.C:
		err := site.connect(false)
		if err == nil {
			return false
		}
		// do not print error, because first connection attempt already printed it and do not want to spam
	}
	return false
}

// read reads from a connected Site Controller's websocket connection.
func (site *siteController) read() bool {
	// cannot put mutex on conn usage since Read JSON will block,
	// so include recover function in case conn gets set to nil between check and usage.
	// recover function will stop the seg fault panic and simply return the function
	// with a return value that is the default value of the return types of the panicking function,
	// in this case false for bool
	defer recover()

	if !site.ws.isConnected() {
		return false
	}

	var msg webSocketMsg
	err := site.ws.conn.ReadJSON(&msg)
	if err != nil {
		log.Println("Error reading:", err)
		log.Println("Lost connection to", site.id, "Site Controller")
		site.markConnectionClosed()
		return false
	}

	// add received message to processing queue for main thread to handle
	addWebSocketMsgToReadQueue(&msg)
	return true
}

// write puts a websocket message on a Site Controller connection's websocket for sending.
func (site *siteController) write(msg webSocketMsg) error {
	site.ws.mu.Lock()
	defer site.ws.mu.Unlock()
	if site.ws.conn == nil {
		return fmt.Errorf("not connected to %v Site Controller", site.id)
	}
	return site.ws.conn.WriteJSON(msg)
}

// closeExistingConnections is called when (re)configuring Scheduler connection(s).
// It closes any existing HTTP connections so that new ones can be established, or so that a Site Controller can become disconnected.
func closeExistingConnections() {
	// a secondary mode controller should not have any actual connections to close
	if isPrimaryScheduler {
		log.Println("New configuration received, so closing any existing HTTPS connections ")
		switch schedulerInstanceType {
		case FLEET_MANAGER:
			for _, site := range masterSchedule.getSites() {
				site.closeConnection()
			}
		case SITE_WITH_SERVER:
			shutdownHTTPServer()
		default:
			break
		}
	}
}

// shutdownHTTPServer shuts down the HTTP server a Site Controller is hosting.
func shutdownHTTPServer() {
	// close the websocket
	if fleetManagerRW.isConnected() {
		fleetManagerRW.close()
		markFleetManConnectionClosed()
		log.Println("Closed websocket connection with Fleet Manager")
	}

	if httpServer == nil {
		log.Println("No HTTPS server to shutdown so skipping this step")
		return
	}

	// context given to shutdown allows 5 seconds for graceful shutdown before force stop
	// graceful shutdown = stop listening for new requests and finish all existing requests
	ctxShutDown, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer func() {
		cancel()
	}()

	// close the server gracefully ("shutdown")
	if err := httpServer.Shutdown(ctxShutDown); err != nil {
		log.Println("Failure/timeout shutting down the server gracefully:", err)
	}

	// wait for goroutine started in startServer() to stop
	httpServerExitDone.Wait()
	log.Println("Finished closing the HTTP server")
}

// sends a channel signal that WebSocket connection is being purposefully closed, then closes the WebSocket connection
func (site *siteController) closeConnection() {
	site.ws.endConn <- struct{}{} // sending empty struct as signal since size of empty struct is zero
	if !site.ws.isConnected() {
		log.Println(site.id, "Site Controller has no connection to close")
		return
	}
	log.Println("Closing connection to", site.id, "Site Controller")
	site.ws.close()
	site.markConnectionClosed()
	log.Println("Closed connection to", site.id, "Site Controller")
}

// markConnectionClosed sets conn to nil so no one tries to use the invalid connection, and pubs an update about the connection status
func (site *siteController) markConnectionClosed() {
	site.ws.setConnection(nil)
	f.SendPub("/scheduler/connected", buildConnectionMap())
}

// markFleetManConnectionClosed sets the Fleet Manager connection to nil so no one tries to use it, and pubs an update about the connection status
func markFleetManConnectionClosed() {
	fleetManagerRW.setConnection(nil)
	f.SendPub("/scheduler/connected", buildConnectionMap())
}

func (aws *atomicWebSocketConn) isConnected() bool {
	aws.mu.RLock()
	defer aws.mu.RUnlock()
	return aws.conn != nil
}

func (aws *atomicWebSocketConn) setConnection(newConn *websocket.Conn) {
	aws.mu.Lock()
	defer aws.mu.Unlock()
	aws.conn = newConn
}

func (aws *atomicWebSocketConn) close() error {
	aws.mu.Lock()
	defer aws.mu.Unlock()
	if aws.conn != nil {
		err := aws.conn.Close()
		if err != nil {
			return err
		}
	}
	return fmt.Errorf("conn is nil")
}
