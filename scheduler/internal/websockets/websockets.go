// Package websockets provides an abstraction of WebSocket clients and a WebSocket server that can be used to share data.
package websockets

// Msg defines the fields a message should have that is being sent or received through the Fleet-Site interface WebSocket.
type Msg struct {
	Method  string      `json:"method"`   // set, get, etc...
	Uri     string      `json:"uri"`      // target recipient
	Body    interface{} `json:"body"`     // data payload
	ReplyTo string      `json:"reply_to"` // endpoint that any reply data should be sent to
}

// Channel for WebSocket read goroutines to pass messages to main goroutine.
var MsgReceive chan Msg = make(chan Msg)

var (
	pathToCertificateFile = "/usr/local/etc/config/scheduler/my.crt"
	pathToKeyFile         = "/usr/local/etc/config/scheduler/my.key"
)

// Closes any existing WebSocket clients and shuts down the WebSocket server if it exists.
func CloseExistingConnections() {
	disconnectAllClients()
	shutdownServer()
}
