package main

import (
	"fims"
	"io/ioutil"
	"log"
	"os"
	"path"
	"testing"
	"time"
)

// creates a mock FimsMsg object that contains the data COPS would expect from a process replying to a heartbeat GET request
func setupMockHeartbeatReplyMsg(processName string, heartbeat, pid interface{}) (replyMsg fims.FimsMsg) {
	msgBody := make(map[string]interface{})
	msgBody["cops_heartbeat"] = heartbeat
	msgBody["pid"] = pid
	replyMsg.Body = msgBody
	replyMsg.Uri = path.Join("cops", processName)
	return
}

// creates a mock processInfo object to be used in a mock processJurisdiction
func setupMockProcess(name, uri string, killOnHang bool, hangTimeAllowance int) *processInfo {
	var processEntry processInfo
	processEntry.name = name
	processEntry.uri = uri
	processEntry.killOnHang = killOnHang
	processEntry.hangTimeAllowance = time.Duration(hangTimeAllowance) * time.Millisecond
	// Set default initial values
	processEntry.replyToURI = path.Join("/cops", name)
	processEntry.alive = true
	processEntry.healthStats.lastConfirmedAlive = beginningTime
	processEntry.healthStats.lastRestart = beginningTime
	processEntry.healthStats.totalRestarts = -1 // First startup will increment this to zero
	return &processEntry
}

// creates a mock processJurisdiction object to be used for unit tests
func setupMockProcessJurisdiction() map[string]*processInfo {
	beginningTime = time.Now()
	mockProcessJurisdiction := make(map[string]*processInfo)
	mockProcessJurisdiction["site_controller"] = setupMockProcess("site_controller", "/site", true, 3000)
	mockProcessJurisdiction["metrics"] = setupMockProcess("metrics", "/metrics", true, 5000)
	mockProcessJurisdiction["modbus_client"] = setupMockProcess("modbus_client", "/components", true, 2000)
	return mockProcessJurisdiction
}

// parseHeartbeat function unit test
func TestParseHeartbeat(t *testing.T) {
	// test valid message
	input := make(map[string]interface{})
	input["cops_heartbeat"] = 4
	result, errMsg := parseHeartbeat(input)

	if errMsg != "" {
		t.Errorf("parseHeartbeat(%v) failed with error: %v", input, errMsg)
	} else if result != 4 {
		t.Errorf("findSender(%v) failed, expected 4, got", result)
	}

	// test invalid body
	badInput := 25
	_, errMsg = parseHeartbeat(badInput)

	if errMsg == "" {
		t.Errorf("parseHeartbeat(%v) failed, expected parseHeartbeat to return verbose errorMsg, returned errorMsg was empty", badInput)
	}

	// test no heartbeat field
	input = make(map[string]interface{})
	_, errMsg = parseHeartbeat(input)

	if errMsg == "" {
		t.Errorf("parseHeartbeat(%v) failed, expected parseHeartbeat to return verbose errorMsg, returned errorMsg was empty", input)
	}

	// test invalid heartbeat field
	input = make(map[string]interface{})
	input["cops_heartbeat"] = true
	_, errMsg = parseHeartbeat(input)

	if errMsg == "" {
		t.Errorf("parseHeartbeat(%v) failed, expected parseHeartbeat to return verbose errorMsg, returned errorMsg was empty", input)
	}

	// test negative int
	input = make(map[string]interface{})
	input["cops_heartbeat"] = -30
	result, errMsg = parseHeartbeat(input)

	if errMsg != "" {
		t.Errorf("parseHeartbeat(%v) failed with error: %v", input, errMsg)
	} else if result != 18446744073709551586 {
		t.Errorf("findSender(%v) failed, expected 18446744073709551586, got", result)
	}
}

// parsePID function unit test
func TestParsePID(t *testing.T) {
	// test valid message
	input := make(map[string]interface{})
	input["pid"] = 123
	result, errMsg := parsePID(input)

	if errMsg != "" {
		t.Errorf("parsePID(%v) failed with error: %v", input, errMsg)
	} else if result != 123 {
		t.Errorf("findPID(%v) failed, expected 123, got", result)
	}

	// test invalid body
	badInput := 25
	_, errMsg = parsePID(badInput)

	if errMsg == "" {
		t.Errorf("parsePID(%v) failed, expected parsePID to return verbose errorMsg, returned errorMsg was empty", badInput)
	}

	// test no pid field
	input = make(map[string]interface{})
	_, errMsg = parsePID(input)

	if errMsg == "" {
		t.Errorf("parsePID(%v) failed, expected parsePID to return verbose errorMsg, returned errorMsg was empty", input)
	}

	// test invalid pid field
	input = make(map[string]interface{})
	input["pid"] = true
	_, errMsg = parsePID(input)

	if errMsg == "" {
		t.Errorf("parsePID(%v) failed, expected parsePID to return verbose errorMsg, returned errorMsg was empty", input)
	}
}

// isResurrected function unit test
func TestIsResurrected(t *testing.T) {
	// test still alive process
	input := setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now()
	result := input.isResurrected()

	if result != false {
		t.Errorf("isResurrected() failed for case when process was still alive, returned true but expected false")
	}

	// test still dead process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now().Add(-1 * time.Duration(5001) * time.Millisecond)
	input.alive = false
	result = input.isResurrected()

	if result != false {
		t.Errorf("isResurrected() failed for case when process was still dead, returned true but expected false")
	}

	// test newly dead process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now().Add(-1 * time.Duration(5001) * time.Millisecond)
	input.alive = true
	result = input.isResurrected()

	if result != false {
		t.Errorf("isResurrected() failed for case when process was newly dead, returned true but expected false")
	}

	// test newly alive process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now()
	input.alive = false
	result = input.isResurrected()

	if result != true {
		t.Errorf("isResurrected() failed for case when process was newly alive, returned false but expected true")
	}
}

// isStillHungOrDead function unit test
func TestIsStillHungOrDead(t *testing.T) {
	// test still alive process
	input := setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now()
	result := input.isStillHungOrDead()

	if result != false {
		t.Errorf("isStillHungOrDead() failed for case when process was still alive, returned true but expected false")
	}

	// test still dead process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now().Add(-1 * time.Duration(5001) * time.Millisecond)
	input.alive = false
	result = input.isStillHungOrDead()

	if result != true {
		t.Errorf("isStillHungOrDead() failed for case when process was still dead, returned false but expected true")
	}

	// test newly dead process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now().Add(-1 * time.Duration(5001) * time.Millisecond)
	input.alive = true
	result = input.isStillHungOrDead()

	if result != false {
		t.Errorf("isStillHungOrDead() failed for case when process was newly dead, returned true but expected false")
	}

	// test newly alive process
	input = setupMockProcess("site_controller", "/cops/site_controller", true, 5000)
	input.healthStats.lastConfirmedAlive = time.Now()
	input.alive = false
	result = input.isStillHungOrDead()

	if result != false {
		t.Errorf("isStillHungOrDead() failed for case when process was newly alive, returned true but expected false")
	}
}

// handleSet function unit test
func TestHandleSet(t *testing.T) {
	processJurisdiction = setupMockProcessJurisdiction()

	// test new heartbeat and new PID
	processJurisdiction["site_controller"].heartbeat = 4
	mockMsg := setupMockHeartbeatReplyMsg("/cops/site_controller", 4, 123)
	passedTest = false
	helpingTestHandleSet(mockMsg)
	result1 := processJurisdiction["site_controller"].heartbeat
	result2 := processJurisdiction["site_controller"].pid

	if result1 != 5 {
		t.Errorf("handleSet() failed for case with new heartbeat, expected 5 but got %v", result1)
	} else if result2 != 123 {
		t.Errorf("handleSet() failed for case with new PID, expected 123 but got %v", result2)
	}

	// test old heartbeat and new PID
	processJurisdiction["site_controller"].heartbeat = 4
	mockMsg = setupMockHeartbeatReplyMsg("/cops/site_controller", 3, 456)
	passedTest = false
	helpingTestHandleSet(mockMsg)
	result1 = processJurisdiction["site_controller"].heartbeat
	result2 = processJurisdiction["site_controller"].pid

	if result1 != 4 {
		t.Errorf("handleSet() failed for case with old heartbeat, expected 4 but got %v", result1)
	} else if result2 != 456 {
		t.Errorf("handleSet() failed for case with new PID, expected 456 but got %v", result2)
	}

	// test bad URI
	log.SetOutput(ioutil.Discard)
	processJurisdiction["site_controller"].heartbeat = 4
	mockMsg = setupMockHeartbeatReplyMsg("/cops/", 3, 789)
	passedTest = false
	helpingTestHandleSet(mockMsg)
	result1 = processJurisdiction["site_controller"].heartbeat
	result2 = processJurisdiction["site_controller"].pid
	log.SetOutput(os.Stdout)

	if result1 != 4 {
		t.Errorf("handleSet() failed for case with bad message URI, expected 4 but got %v", result1)
	} else if result2 != 456 {
		t.Errorf("handleSet() failed for case with bad message URI, expected 456 but got %v", result2)
	}

	// test bad heartbeat
	log.SetOutput(ioutil.Discard)
	processJurisdiction["site_controller"].heartbeat = 4
	mockMsg = setupMockHeartbeatReplyMsg("/cops/site_controller", "4", 789)
	passedTest = false
	helpingTestHandleSet(mockMsg)
	result1 = processJurisdiction["site_controller"].heartbeat
	result2 = processJurisdiction["site_controller"].pid
	log.SetOutput(os.Stdout)

	if result1 != 4 {
		t.Errorf("handleSet() failed for case with bad heartbeat, expected 4 but got %v", result1)
	} else if result2 != 456 {
		t.Errorf("handleSet() failed for case with bad heartbeat, expected 456 but got %v", result2)
	}

	// test bad PID
	log.SetOutput(ioutil.Discard)
	processJurisdiction["site_controller"].heartbeat = 4
	mockMsg = setupMockHeartbeatReplyMsg("/cops/site_controller", 4, "456")
	passedTest = false
	helpingTestHandleSet(mockMsg)
	result1 = processJurisdiction["site_controller"].heartbeat
	result2 = processJurisdiction["site_controller"].pid
	log.SetOutput(os.Stdout)

	if result1 != 5 {
		t.Errorf("handleSet() failed for case with bad message URI, expected 4 but got %v", result1)
	} else if result2 != 456 {
		t.Errorf("handleSet() failed for case with bad message URI, expected 456 but got %v", result2)
	}
}

// helper function to test handleSet since updatePID() will cause panic
func helpingTestHandleSet(setMsg fims.FimsMsg) {
	defer func() {
		if r := recover(); r != nil {
			passedTest = true
		}
	}()
	handleSet(setMsg)
}
