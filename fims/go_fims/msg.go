package fims

import (
	"fmt"
	"regexp"
	"strings"
)

// Fims message struct
type FimsMsg struct {
	Method      string
	Uri         string
	Replyto     string
	ProcessName string
	Username    string
	Body        interface{}
	Nfrags      int
	Frags       []string
}

// Fims message raw (byte array) struct
type FimsMsgRaw struct {
	Method      string
	Uri         string
	Replyto     string
	ProcessName string
	Username    string
	Body        []byte
	Nfrags      int
	Frags       []string
}

// A handler interface that responds to incoming FIMS messages (a request).
// Handlers should not modify the msg contents, only read from the FimsMsg contents.
// These type definitions mimic the style of the Go HTTP library under server.go:
// https://go.dev/src/net/http/server.go
type Handler interface {
	ServeFIMS(*FimsMsg) error
}

// Adapter to allow use of ordinary functions as FIMS handlers.
// Given a function f with appropriate signature, HandlerFunc(f)
// is a handler that calls function f.
type HandlerFunc func(*FimsMsg) error

// Method handler for serving (executing) route defined function handlers.
// Satisfies the Handler interface.
// Mimics the style from net/http/server.go ServeHTTP().
func (f HandlerFunc) ServeFIMS(msg *FimsMsg) error {
	return f(msg)
}

// Function that implements splitting a uri into fragments
func GetFrags(uri string) ([]string, error) {
	// Check to make sure our string contains "/"
	if !strings.Contains(uri, "/") {
		return nil, fmt.Errorf("error parsing uri - does not contain slashes")
	}

	// Use regexp package to parse the string
	regx := regexp.MustCompile(`/`)
	s := regx.Split(uri, -1)
	return s[1:], nil
}
