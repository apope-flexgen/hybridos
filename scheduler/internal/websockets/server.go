package websockets

import (
	"context"
	"crypto/tls"
	"fmt"
	"net/http"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/gorilla/websocket"
)

// Represents a server's connection to a client.
var ServerConn atomicConn

// Server for hosting a WebSocket connection.
var httpsServer *http.Server

// Blocks the server shutdown thread from exiting until the server has had time to cleanup.
var httpsServerExitDone sync.WaitGroup

// Allows an HTTP connection to be upgraded to a WebSocket.
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
	CheckOrigin:     func(r *http.Request) bool { return true },
}

// Channel for WebSocket server thread to update main thread about connection status.
// No data is needed other than empty struct signal since as of right now only action is a publish of the connection map.
var SocketServerConnUpdates chan struct{} = make(chan struct{}, 10)

func init() {
	http.HandleFunc("/ws", createWebSocket)
	ServerConn.isConnected.Store(false)
}

// Returns a list of cipher suites that are safe to allow clients to use to connect to the WebSocket server.
func getAllowedCipherSuites() []uint16 {
	return []uint16{
		// AEADs w/ ECDHE
		tls.TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, tls.TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
		tls.TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384, tls.TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384,
		tls.TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305, tls.TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305,

		// AEADs w/o ECDHE
		tls.TLS_RSA_WITH_AES_128_GCM_SHA256,
		tls.TLS_RSA_WITH_AES_256_GCM_SHA384,
	}
}

// Configures an HTTPS server and starts listening for a client connection.
func StartServer(port int) error {
	// load certificate and private key
	cert, err := tls.LoadX509KeyPair(pathToCertificateFile, pathToKeyFile)
	if err != nil {
		return fmt.Errorf("failed to load certificate/key pair: %w", err)
	}

	// initialize server
	httpsServer = &http.Server{
		Addr: fmt.Sprintf(":%d", port),
		TLSConfig: &tls.Config{
			Certificates: []tls.Certificate{cert},
			CipherSuites: getAllowedCipherSuites(),
			MinVersion:   tls.VersionTLS12,
		},
	}

	// set a blocker to give server time to properly shutdown when it is closed
	httpsServerExitDone.Add(1)
	go func() {
		defer httpsServerExitDone.Done() // let parent goroutine know when server is done cleaning up (post-shutdown)

		// run the server
		if err := httpsServer.ListenAndServeTLS("", ""); err != nil && err != http.ErrServerClosed {
			log.Errorf("HTTPS server error: %v.", err)
		}
	}()

	log.Infof("WebSocket server is launched. Waiting for client to connect.")
	return nil
}

// Upgrades an HTTPS connection to a WebSocket and begins reading from it.
func createWebSocket(w http.ResponseWriter, r *http.Request) {
	// upgrade HTTPS connection to be a WebSocket
	ws, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Errorf("Error upgrading HTTPS connection to be WebSocket: %v.", err)
		return
	}

	// save WebSocket connection to global variable
	ServerConn.setNewConnection(ws)
	log.Infof("Successfully established WebSocket connection with client.")

	// update main thread
	SocketServerConnUpdates <- struct{}{}

	// begin reading
	for {
		if err := ServerConn.read(); err != nil {
			log.Errorf("Error reading from server's WebSocket: %v. Closing connection to reset.", err)
			// cannot know if error was due to lost connection or bad individual read, so treat it as lost connection
			// and call close() in case it was a bad individual read (so that conn does not leak)
			ServerConn.close()
			// update main thread
			SocketServerConnUpdates <- struct{}{}
			return
		}
	}
}

// Closes the WebSocket connection to the client and shuts down the HTTPS server.
func shutdownServer() {
	// close the WebSocket
	if ServerConn.IsConnected() {
		ServerConn.close()
		log.Infof("Closed WebSocket connection with client.")
		// update main thread
		SocketServerConnUpdates <- struct{}{}
	}

	if httpsServer == nil {
		return
	}

	// context given to shutdown allows 5 seconds for graceful shutdown before force stop.
	// graceful shutdown = stop listening for new requests and finish all existing requests
	ctxShutDown, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	// shutdown the server gracefully
	if err := httpsServer.Shutdown(ctxShutDown); err != nil {
		log.Errorf("Failure/timeout shutting down the server gracefully: %v.", err)
	}

	// wait for goroutine started in startServer() to stop
	httpsServerExitDone.Wait()
	log.Infof("Finished shutting down the HTTPS server.")
}
