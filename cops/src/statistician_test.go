package main

import (
	"testing"
	"time"
)

// createListOfPIDs function unit test
func TestCreateListOfPIDs(t *testing.T) {
	processJurisdiction = setupMockProcessJurisdiction()

	// test creating a list when all processes are healthy
	for _, process := range processJurisdiction {
		process.alive = true
		process.healthStats.lastConfirmedAlive = time.Now()
	}
	processJurisdiction["site_controller"].pid = 123
	processJurisdiction["metrics"].pid = 456
	processJurisdiction["modbus_client"].pid = 789
	result := createListOfPIDs()

	no123 := true
	no456 := true
	no789 := true
	for _, pid := range result {
		if pid == 123 && no123 {
			no123 = false
		} else if pid == 456 && no456 {
			no456 = false
		} else if pid == 789 && no789 {
			no789 = false
		} else {
			t.Errorf("createListOfPIDs() failed for case when all processes are healthy. Found unexpected or duplicated PID number")
		}
	}
	if no123 {
		t.Errorf("createListOfPIDs() failed for case when all processes are healthy. Did not find PID 123")
	}
	if no456 {
		t.Errorf("createListOfPIDs() failed for case when all processes are healthy. Did not find PID 456")
	}
	if no789 {
		t.Errorf("createListOfPIDs() failed for case when all processes are healthy. Did not find PID 789")
	}

	// test creating a list when one process is dead
	for _, process := range processJurisdiction {
		process.alive = true
		process.healthStats.lastConfirmedAlive = time.Now()
	}
	processJurisdiction["site_controller"].alive = false
	processJurisdiction["site_controller"].pid = 123
	processJurisdiction["metrics"].pid = 456
	processJurisdiction["modbus_client"].pid = 789
	result = createListOfPIDs()

	no123 = true
	no456 = true
	no789 = true
	for _, pid := range result {
		if pid == 123 && no123 {
			no123 = false
		} else if pid == 456 && no456 {
			no456 = false
		} else if pid == 789 && no789 {
			no789 = false
		} else {
			t.Errorf("createListOfPIDs() failed for case when one process is dead. Found unexpected or duplicated PID number")
		}
	}
	if !no123 {
		t.Errorf("createListOfPIDs() failed for case when one process is dead. Found PID 123 but that process is dead")
	}
	if no456 {
		t.Errorf("createListOfPIDs() failed for case when one process is dead. Did not find PID 456")
	}
	if no789 {
		t.Errorf("createListOfPIDs() failed for case when one process is dead. Did not find PID 789")
	}
}
