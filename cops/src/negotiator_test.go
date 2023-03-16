package main

import (
	"testing"
	"time"
)

// otherControllerIsUnresponsive unit test
func TestOtherControllerIsUnresponsive(t *testing.T) {
	// test unconnected controller
	c2c.connected = false
	result := otherControllerIsUnresponsive()

	if result != true {
		t.Errorf("otherControllerIsUnresponsive() failed, expected true, got false")
	}

	// test timed out connection
	c2c.connected = true
	c2c.lastModeConfirmationFromOther = time.Now().Add(-1 * time.Duration(3001) * time.Millisecond)
	result = otherControllerIsUnresponsive()

	if result != true {
		t.Errorf("otherControllerIsUnresponsive() failed, expected true, got false")
	}

	// test ok connection
	c2c.lastModeConfirmationFromOther = time.Now()
	result = otherControllerIsUnresponsive()

	if result != false {
		t.Errorf("otherControllerIsUnresponsive() failed, expected false, got true")
	}
}
