package websockets

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io/ioutil"
	"net/url"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/gorilla/websocket"
)

type client struct {
	id         string           // used to identify client in URIs/JSON. no spaces or slashes allowed
	address    string           // IP address and port (separated by a colon ':') of the server this client is connecting to. example: "172.16.1.80:9000"
	url        string           // URL that the server is hosting its WebSocket on
	endConn    chan interface{} // when the client is asked to disconnect, a signal is put on this channel and received by the read thread so it knows to exit
	atomicConn                  // WebSocket connection to the server
}

type ClientConnectionUpdate struct {
	ClientId  string
	Connected bool // true if client made connection; false if client lost connection
}

// Channel for WebSocket client threads to update main thread about connection status.
var ClientConnUpdates chan ClientConnectionUpdate = make(chan ClientConnectionUpdate, 10)

// Creates a new client.
func NewClient(id, ip string, port int) *client {
	address := fmt.Sprintf("%s:%d", ip, port)
	c := &client{
		id:      id,
		address: address,
		url:     (&url.URL{Scheme: "wss", Host: address, Path: "/ws"}).String(),
		endConn: make(chan interface{}, 1),
	}
	c.isConnected.Store(false)
	return c
}

func (cl *client) launch() {
	// connect to the server via HTTPS
	if err := cl.connect(true); err != nil {
		log.Errorf("Error connecting to %s's WebSocket server: %v. Will try to reattempt connection once per second then slow down to once per 30 seconds.", cl.id, err)
	}

	cl.readWithConnection()
}

func (cl *client) connect(firstAttempt bool) error {
	if firstAttempt {
		log.Infof("Connecting to %s WebSocket server at: %s.", cl.id, cl.url)
	}

	// add the server certificate to the list of certificate authorities trusted by the client
	caCert, err := ioutil.ReadFile(pathToCertificateFile)
	if err != nil {
		return fmt.Errorf("failed to read certificate file: %w", err)
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
	conn, resp, err := dialer.Dial(cl.url, nil)
	if err != nil {
		if err == websocket.ErrBadHandshake {
			return fmt.Errorf("handshake with URL %s failed with status %d", cl.url, resp.StatusCode)
		}
		return fmt.Errorf("failed to dial server with URL %s: %w", cl.url, err)
	}

	// save successful connection
	cl.setNewConnection(conn)
	log.Infof("Successfully connected to %s WebSocket server.", cl.id)

	// update main thread
	ClientConnUpdates <- ClientConnectionUpdate{
		ClientId:  cl.id,
		Connected: true,
	}
	return nil
}

// Tries to reconnect a failed client connection.
// It tries to connect every second for ten seconds, then slows down to one attempt every 30 seconds.
func (cl *client) pollToReconnect() (connTerminated bool) {
	// attempt connection every second for ten seconds
	for i := 0; i < 10; i++ {
		connTerminated := cl.connectionAttemptWithLag(time.Second * 1)
		if connTerminated {
			// failed because client is being terminated
			return true
		}
		if cl.IsConnected() {
			// success !
			return false
		}
		// failure - retry
	}

	// slow down to one attempt every 30 seconds indefinitely
	for {
		connTerminated := cl.connectionAttemptWithLag(time.Second * 30)
		if connTerminated {
			// failed because client is being terminated
			return true
		}
		if cl.IsConnected() {
			// success !
			return false
		}
		// failure - retry
	}
}

func (cl *client) connectionAttemptWithLag(lagDuration time.Duration) (connTerminated bool) {
	lag := time.NewTimer(lagDuration)
	select {
	case <-cl.endConn:
		if !lag.Stop() {
			<-lag.C
		}
		return true
	case <-lag.C:
		err := cl.connect(false)
		if err == nil {
			return false
		}
		// do not print error, because first connection attempt already printed it and do not want to spam
		return false
	}
}

func (cl *client) disconnect() {
	cl.endConn <- struct{}{}
	if !cl.IsConnected() {
		return
	}
	log.Infof("Disconnecting %s WebSocket client.", cl.id)
	cl.close()
	ClientConnUpdates <- ClientConnectionUpdate{
		ClientId:  cl.id,
		Connected: false,
	}
}

// Reads from the WebSocket connection to a client if that connection is still open.
// If the connection fails, will begin polling the client in an attempt to reconnect.
// If the connection is purposefully closed, abandon polling and the connection.
func (cl *client) readWithConnection() {
	// try to reconnect if disconnected. if connection purposefully closed, leave thread without starting new routine
	if !cl.IsConnected() {
		connTerminated := cl.pollToReconnect()
		if connTerminated {
			// connection was purposefully closed, so leave thread without starting new routine
			return
		}
		// update main thread
		ClientConnUpdates <- ClientConnectionUpdate{
			ClientId:  cl.id,
			Connected: true,
		}
	}

	// read from connection
	for {
		select {
		case <-cl.endConn:
			// connection was purposefully closed, so leave thread without starting new routine
			return
		default:
		}
		if err := cl.read(); err != nil {
			// cannot know if error was due to lost connection or bad individual read, so treat it as lost connection
			// and call Close() in case it was a bad individual read (so that conn does not leak)
			log.Errorf("Error reading from %s's WebSocket connection: %v. Closing connection to reset.", cl.id, err)
			cl.close()
			ClientConnUpdates <- ClientConnectionUpdate{
				ClientId:  cl.id,
				Connected: false,
			}
			go cl.readWithConnection()
			break
		}
	}
}
