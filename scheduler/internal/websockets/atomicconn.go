package websockets

import (
	"fmt"
	"sync/atomic"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/gorilla/websocket"
)

// Thread-safe WebSocket connection.
// Only one read and one write is allowed on a WebSocket at a time (one read + one write okay, two reads or two writes not okay).
type atomicConn struct {
	websocket.Conn
	isConnected atomic.Value // must Store with bools and type assert bool on Load. when we get to Go 1.19, replace with atomic.Bool
}

func (ac *atomicConn) IsConnected() bool {
	return ac.isConnected.Load().(bool)
}

// Do not call this while an existing connection is still open. First, call close().
func (ac *atomicConn) setNewConnection(newConn *websocket.Conn) {
	ac.Conn = *newConn
	ac.isConnected.Store(true)
}

func (ac *atomicConn) read() error {
	var msg Msg
	err := ac.ReadJSON(&msg)
	if err != nil {
		return fmt.Errorf("failed to read from WebSocket connection: %w", err)
	}

	// add received message to processing queue for main thread to handle
	MsgReceive <- msg
	return nil
}

func (ac *atomicConn) close() {
	if err := ac.Close(); err != nil {
		log.Errorf("Error while closing connection: %v.", err)
	}
	ac.isConnected.Store(false)
}
