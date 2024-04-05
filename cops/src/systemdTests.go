// systemd-tests.go implements framework for tests against
// starting and stopping systemctl services.
package main

import (
	"fims"
	"fmt"
	"log"
	"strings"
	"sync"
	"time"

	"github.com/coreos/go-systemd/dbus"
)

// Set margin of duration expected between the current time and time of service start
var margin = 10 * time.Second

// Global variable to interchange out which service to run tests
// against - providing a single static process.
// Tests are ran under the assumption this unit exists in systemd.
var testProcess = "fims"

// Initialize a FIMS test message struct.
var testMsg = fims.FimsMsg{
	Uri:     fmt.Sprintf("/cops/stats/%s", testProcess),
	Frags:   []string{"cops", "stats", testProcess},
	Replyto: "cops",
}

// Catch all function to execute all internal tests involving systemd.
// To execute test:
// $ go build -o cops ./src/
// $ sudo ./cops -test
func runTests() {
	// Opted for Go's standard logger to avoid bulky printouts.
	log.Printf("Running Tests...\n")

	// Setup fake process list for testing enabled statuses.
	generateFakeProcessJurisdiction()

	// Tests are executed consecutively and independent of each other.
	// Helps isolate each test so as not to interfere with one another.
	var wg sync.WaitGroup
	result := make(chan string)

	// Wait on total number of tests being performed.
	wg.Add(3)

	// Verify restart actually restarts a process and timestamps are updated.
	log.Printf("TEST: restart timestamp verification.\n")
	go testTimestamps(&wg, result)
	log.Printf("Timestamps result: %s", <-result)

	// Test start and stop actions.
	log.Printf("TEST: start/stop actions.\n")
	go testActions(&wg, result)
	log.Printf("Actions result: %s", <-result)

	// Start actions restrictions test.
	log.Printf("TEST: restrict actions.\n")
	go testActionRestrictions(&wg, result)
	log.Printf("Restrictions result: %s", <-result)

	// Wait for each test to finish.
	wg.Wait()

	// Close result channel.
	close(result)
}

// Test timestamp verification is within defined margin.
// Tests getUnitTime() and restarting a stopped service.
// Tests our API effectively restarts a process by comparing the current
// time and the time of the service after sending a restart command.
func testTimestamps(wg *sync.WaitGroup, result chan string) {
	defer wg.Done()

	// Generate service string for sending to dbus.
	service := fmt.Sprintf("%s.service", testProcess)

	// Establish connection.
	conn, err := dbus.New()
	if err != nil {
		log.Fatalf("Failed to connect to D-Bus: %v", err)
	}
	defer conn.Close()

	// Restart test service.
	if err := takeAction("restart", testMsg); err != nil {
		log.Fatalf("performing restart on %s: %v", testProcess, err)
	}

	// Get timestamp using
	timeUint64, err := getUnitTime(conn, service)
	if err != nil {
		log.Fatalf("getting timestamp for service: %s: %v", service, err)
	}

	// Retrieve service time of restart from dbus.
	serviceTime := convertMicrosecondsToUnixTime(timeUint64)

	// Retrieve current time from Unix.
	expectedTime := time.Now()

	// Check expected timestamp vs the service timestamp.
	// Verify difference between the two is less than a defined margin (10 secs).
	if expectedTime.Sub(serviceTime) < margin {
		log.Printf("PASS: timestamps verified.\n")
	} else {
		log.Printf("FAIL: timestamps are not within margin.\n")
	}

	// Stop the unit.
	time.Sleep(1 * time.Second)
	resultChan := make(chan string)
	_, err = conn.StopUnit(service, "replace", resultChan)
	if err != nil {
		log.Fatalf("error stopping unit: %v", err)
	}

	// Report service job status.
	job := <-resultChan
	log.Printf("Stopping %s result: %v\n", service, job)

	// Indicate our test finished.
	result <- "done"
}

// Test our 3 actions: start, stop, restart.
// Setup our test service appropriately beforehand for each measure.
// Assumption: test service is dead prior to execution.
func testActions(wg *sync.WaitGroup, result chan string) {
	defer wg.Done()

	// Test start.
	// Intentionally leave service running to test following commands.
	if err := testAction("start", testProcess, "dead", "running"); err != nil {
		result <- fmt.Sprintf("testing start: %v", err)
	}

	// Test restart.
	// Doesn't effectively test restart here, however, restart action is verified with timestamps test.
	if err := testAction("restart", testProcess, "running", "running"); err != nil {
		result <- fmt.Sprintf("testing restart: %v", err)
	}

	// Test stop.
	if err := testAction("stop", testProcess, "running", "dead"); err != nil {
		result <- fmt.Sprintf("testing stop: %v", err)
	}

	result <- "done"
}

// Verify actions are restricted based upon runtime status.
// Restart is always allowed.
// handleSystemdCmd is our unit to test here
func testActionRestrictions(wg *sync.WaitGroup, result chan string) {
	defer wg.Done()

	// Verify start is not allowed on a running process.
	// Start our service first to enter a running state.
	if err := takeAction("start", testMsg); err != nil {
		result <- fmt.Sprintf("FAIL: taking action: %s: %s", "start", err)
	}
	// Update internal process status.
	updateTestStatuses()

	// Verify we get expected error when attempting to start again during a run state.
	err := handleSystemdCmd("start", testMsg)
	expectedErr := "not enabled"

	// Validate expected error.
	if err == nil {
		log.Println("FAIL: did not receive error when attempting start on a running service.")
	} else if strings.Contains(fmt.Sprintf("%v", err), expectedErr) {
		log.Println("PASS: start restrictions.")
	}

	// Stop our started unit to prepare for next test.
	if err := takeAction("stop", testMsg); err != nil {
		result <- fmt.Sprintf("FAIL: taking action: %s: %s", "start", err)
	}

	// Update internal process status.
	updateTestStatuses()

	// Verify we get expected error when attempting to stop during a stopped state.
	err = handleSystemdCmd("stop", testMsg)
	expectedErr = "not enabled"

	// Validate expected error.
	if err == nil {
		log.Println("FAIL: did not receive error when attempting start on a running service.")
	} else if strings.Contains(fmt.Sprintf("%v", err), expectedErr) {
		log.Println("PASS: stop restrictions.")
	}

	result <- "done"
}

// Test our takeAction() function for a given action.
func testAction(action string, process string, beforeState, afterState string) error {
	service := fmt.Sprintf("%s.service", process)

	// Open dbus connection.
	conn, err := dbus.New()
	if err != nil {
		return fmt.Errorf("failed to connect to D-Bus: %w", err)
	}
	defer conn.Close()

	// Verify status of service is dead first, to start.
	state, _, err := getUnitState(conn, service)
	if err != nil {
		return fmt.Errorf("getting unit state: %w", err)
	}

	// Verify dead status.
	if strings.Contains(state, beforeState) {
		// Test action
		if err := takeAction(action, testMsg); err != nil {
			return fmt.Errorf("taking action: %s: %w", action, err)
		}
	} else {
		return fmt.Errorf("test service %s must be stopped to test start", service)
	}

	// Verify status is running.
	state, _, err = getUnitState(conn, service)
	if err != nil {
		return fmt.Errorf("getting unit state: %w", err)
	}

	if strings.Contains(state, afterState) {
		log.Printf("PASS: %s on %s.", action, service)
	} else {
		log.Printf("FAIL: %s on %s: expected state %s but received %s.", action, service, afterState, state)
	}

	return nil
}

// Generate fake processes to test restrictions against.
func generateFakeProcessJurisdiction() {
	processes := []Process{
		{Name: "fims", AllowActions: true},
	}

	// Set process variables
	processJurisdiction = make(map[string]*processInfo)
	for _, process := range processes {
		var processEntry processInfo
		processEntry.name = process.Name
		processEntry.allowActions = process.AllowActions
		processJurisdiction[process.Name] = &processEntry
	}

	// Update process
	updateTestStatuses()

}

// Helper function to force updates to our enabled statuses per process.
// Helps mimic period process updates from the original runtime loop.
func updateTestStatuses() {
	// Update process
	for _, process := range processJurisdiction {
		// Update the service status for a given process
		if err := process.updateStatus(); err != nil {
			log.Fatalf("updating service %v status: %v", process.name, err)
		}
	}
}
