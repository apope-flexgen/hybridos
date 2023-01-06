package fims

import (
	"encoding/json"
	"strings"
)

func Decode(b []byte) (FimsMsg, error) {
	var msg FimsMsg
	err := json.Unmarshal(b, &msg)
	if err != nil {
		return FimsMsg{}, err
	}
	var body interface{}
	if msg.Body != nil {
		err = json.Unmarshal([]byte(msg.Body.(string)), &body)
		if err == nil {
			msg.Body = body
		} else {
			return FimsMsg{Uri: msg.Uri}, err
		}
	}
	return msg, nil
}

func GetFrags(s string) (int, []string) {
	// Expecting something like /components/ess1 or /components/ess1/voltage_ac
	// String split will have empty strings before a leading / or after a trailing /
	frags := strings.Split(s, "/")
	start, end, n := 0, len(frags), len(frags)
	if end == 0 {
		return 0, []string{}
	}
	if frags[0] == "" {
		if end == 1 {
			return 0, []string{}
		}
		start = 1
		n--
	}
	if frags[end-1] == "" {
		end--
		n--
	}
	return n, frags[start:end]
}
