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
