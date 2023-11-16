// systemd-tests.go implements framework for tests against
// starting and stopping systemctl services.
package main

import (
	"fmt"
	"log"
	"time"

	"github.com/coreos/go-systemd/dbus"
)

// Set margin of duration expected between the current time and time of service start
var margin = 10 * time.Second

// To execute test:
// $ go build -o test systemd-tests.go
// $ sudo ./test
func main() {

	var tests = []struct {
		name           string
		service        string
		expectedError  string
		expectedOutput string
	}{
		{"Verify timestamps",
			"fims.service",
			"not valid",
			""},
	}

	for _, test := range tests {
		log.Printf("TEST: %v \n", test.name)

		// Establish connection.
		conn, err := dbus.New()

		if err != nil {
			log.Fatalf("Failed to connect to D-Bus: %v", err)
		}
		defer conn.Close()

		// Start test service.
		resultChan := make(chan string)
		_, err = conn.RestartUnit(test.service, "replace", resultChan)
		if err != nil {
			log.Fatalf("error starting unit: %v", err)
		}

		// Report service job status.
		job := <-resultChan
		log.Printf("Starting %s result: %v\n", test.service, job)

		// Get timestamp.
		timeUint64, err := getUnitTime(conn, test.service)
		if err != nil {
			log.Fatalf("getting timestamp for service: %s: %v", test.service, err)
		}

		// Collect time values.
		serviceTime := convertMicrosecondsToUnixTime(timeUint64)
		expectedTime := time.Now()

		// Check expected timestamp vs the service timestamp.
		// Verify difference between the two is less than a defined margin (10 secs).
		if expectedTime.Sub(serviceTime) < margin {
			log.Printf("TEST PASS: starting timestamps are within margin.\n")
		} else {
			log.Fatalf("TEST FAIL: timestamps are not within margin.\n")
		}

		// Stop the unit.
		time.Sleep(1 * time.Second)
		_, err = conn.StopUnit(test.service, "replace", resultChan)
		if err != nil {
			log.Fatalf("error stopping unit: %v", err)
		}

		// Report service job status.
		job = <-resultChan
		log.Printf("Stopping %s result: %v\n", test.service, job)

	}
}

// Get the time integer value for a given service since it was started.
func getUnitTime(conn *dbus.Conn, service string) (uint64, error) {
	var TimeUint64 uint64

	// Retrieve service properties.
	// If the .service file does not exist, it will return "inactive" for its state.
	stats, err := conn.GetUnitProperties(service)
	if err != nil {
		return 0, fmt.Errorf("Failed to get service status: %v", err)
	}

	if stats["ActiveEnterTimestamp"] != nil {
		// Retrieve Time since active.
		// Requires some calculations to convert from integer to proper format.
		TimeUint64 = stats["ActiveEnterTimestamp"].(uint64)
	} else {
		return 0, fmt.Errorf("service [time since last restart] property for %v is not available", service)
	}

	return TimeUint64, nil
}

// Helper function to convert a 64 bit uint to a Unix timestamp.
// Unix timestamps are represented in seconds since the Unix epoch -
// dividing the 64 bit integer by 10^6 provides seconds with a remainder of nanoseconds.
func convertMicrosecondsToUnixTime(timeUint64 uint64) time.Time {
	return time.Unix(int64(timeUint64)/1e6, int64(timeUint64)%1e6)
}
