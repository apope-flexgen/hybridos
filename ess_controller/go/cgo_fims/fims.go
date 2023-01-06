package fims

import (
	"encoding/json"
	"fmt"
	"log"
	"net"
)

var socketName = "/tmp/FlexGen_FIMS_Server.socket"
var maxMessageSize = 65536
var maxSubscriptions = 64
var welcomeString = "Connection to FlexGen Internal Messaging Service Server - v1.0.0"
var maxURIDepth = 64

type Fims struct {
	conn      net.Conn
	connected bool
	buff      []byte
}

type FimsMsg struct {
	Method  string      `json:"method"`
	Uri     string      `json:"uri,omitempty"`
	Replyto string      `json:"replyto,omitempty"`
	Body    interface{} `json:"body,omitempty"`
	Nfrags  int         `json:"-"`
	Frags   []string    `json:"-"`
}

func Connect(pName string) (Fims, error) {
	// Set up struct vars
	connected := false
	buff := make([]byte, maxMessageSize)
	// "unixpacket" is the particular socket type that FIMS uses, vs. unix or others
	conn, err := net.Dial("unixpacket", socketName)
	if err != nil {
		fmt.Errorf("%v", err)
	} else {
		connected = true
	}
	f := Fims{conn, connected, buff}
	// Check for welcome string
	s, err := f.ReceiveRaw()
	if s != welcomeString {
		err = fmt.Errorf("Failed to validate the welcome message, got %s", s)
	}
	// Save this for FIMS upgrade
	//connect(pname string)
	if pName == "" {
		pName = "Undefined process"
	}
	f.conn.Write([]byte(pName))
	return f, err
}

func (f *Fims) Subscribe(uris ...string) error {
	// Send a message with no uri, method: sub, body: [{"uri":uris[x],"pub_only":false}]
	// Save for FIMS upgrade
	var body []map[string]interface{}
	for _, u := range uris {
		body = append(body, map[string]interface{}{
			"uri":      u,
			"pub_only": false,
		})
	}
	msg := FimsMsg{
		Method: "sub",
		Body:   body,
	}
	// fmt.Println(uris)
	_, err := f.SendRaw(msg)
	// fmt.Println(n)
	s, err := f.ReceiveRaw()
	if s != "SUCCESS" {
		err = fmt.Errorf("Didn't get the success string got: %s", s)
	}
	return err
}

func (f *Fims) Send(msg FimsMsg) (int, error) {
	// logic to unpack FimsMsg into a buffer to write out
	var err error
	var bb []byte
	bb, err = json.Marshal(msg.Body)
	msg.Body = string(bb)
	b, err := json.Marshal(msg)
	// fmt.Println(string(b))
	n, err := f.conn.Write(b)
	if err != nil {
		f.connected = false
	}
	return n, err
}

func (f *Fims) SendRaw(msg FimsMsg) (int, error) {
	// logic to unpack FimsMsg into a buffer to write out
	var err error
	b, err := json.Marshal(msg)
	// fmt.Println(string(b))
	n, err := f.conn.Write(b)
	if err != nil {
		f.connected = false
	}
	return n, err
}

func (f *Fims) ReceiveRaw() (string, error) {
	n, err := f.conn.Read(f.buff)
	if err != nil {
		f.connected = false
		return "", err
	}
	s := string(f.buff[:n-1])
	return s, err
}

func (f *Fims) Receive() (FimsMsg, error) {
	n, err := f.conn.Read(f.buff)
	var msg FimsMsg
	if err != nil {
		f.connected = false
		return FimsMsg{}, err
	}
	// logic to unpack the buffer into msg
	msg, err = Decode(f.buff[:n])
	if err != nil {
		return FimsMsg{Uri: msg.Uri}, err
	}
	msg.Nfrags, msg.Frags = GetFrags(msg.Uri)
	// fmt.Printf("Got %v bytes %s\n", n, f.buff[:n])
	return msg, nil
}

func (f *Fims) ReceiveChannel(c chan<- FimsMsg) {
	for f.connected == true {
		msg, err := f.Receive()
		if err != nil {
			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
		}
		// fmt.Printf("about to put the msg on the channel %v\n", msg)
		c <- msg
	}
}

func (f *Fims) Close() error {
	err := f.conn.Close()
	f.connected = false
	return err
}

func (f *Fims) Connected() bool {
	return f.connected
}
