// Handle logic pertaining to monitoring process heartbeats
package main

import (
	"fims"
	"fmt"
	"path"
	"reflect"
)

// Sends latest heartbeats to processes then requests the processes for their recorded heartbeats
func manageHeartbeats() {
	sendHeartbeats()
	getHeartbeats()
}

// Sends SETs to all processes with their heartbeat values
func sendHeartbeats() {
	for _, process := range processJurisdiction {
		targetURI := path.Join(process.uri, "cops/cops_heartbeat")
		f.SendSet(targetURI, "", process.heartbeat)
	}
}

// Sends GETs to all processes to request heartbeat values and PIDs
func getHeartbeats() {
	for _, process := range processJurisdiction {
		targetURI := path.Join(process.uri, "cops")
		f.SendGet(targetURI, process.replyToURI)
	}
}

// Parse the heartbeat out of a SET message
func parseHeartbeat(body interface{}) (receivedHeartbeat uint, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, "cops_heartbeat", reflect.TypeOf(receivedHeartbeat))
	if errMsg != "" {
		return 0, errMsg
	}
	return uint(extractedVal.Uint()), ""
}

// Update the COPS process heartbeat value if a received heartbeat value matches it
func (process *processInfo) updateHeartbeat(receivedHeartbeat uint) {
	if process.heartbeat == receivedHeartbeat {
		process.heartbeat++
		process.recordResponse()
	}
}

// Handles the given heartbeat reply message and associated process
func handleHeartbeatReply(msg fims.FimsMsg, process *processInfo) error {
	if process == nil {
		return fmt.Errorf("error: process is nil when calling function handleHeartbeatReply")
	}
	receivedHeartbeat, errMsg := parseHeartbeat(msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseHeartbeat: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.updateHeartbeat(receivedHeartbeat)
	readPID, errMsg := parsePID(msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parsePID: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.updatePID(readPID)
	readVersionTag, errMsg := parseStringFromMap("version_tag", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.tag = readVersionTag
	readVersionCommit, errMsg := parseStringFromMap("version_commit", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.commit = readVersionCommit
	readVersionBuild, errMsg := parseStringFromMap("version_build", msg.Body)
	if errMsg != "" {
		return fmt.Errorf("error in parseStringFromMap: when calling function handleHeartbeatReply. %s", errMsg)
	}
	process.version.build = readVersionBuild
	return nil
}
