package main

import (
	"testing"

	dbus "github.com/coreos/go-systemd/dbus"
	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/stretchr/testify/assert"
)

// Test errors on retrieval of a .service file state.
func TestGetUnitState(t *testing.T) {

	var tests = []struct {
		name           string
		service        string
		expectedError  string
		expectedStatus string
		expectedState  string
	}{
		{"no unit file ext",
			"fims",
			"not valid",
			"",
			""},
		{"invalid delimiter in path",
			"fi/ms.service",
			"not valid",
			"",
			""},
		{"validate output with fake service",
			"IamNotAService.service",
			"",
			"inactive",
			""},
		{"validate sub state with fake service",
			"IamNotAService.service",
			"",
			"inactive (dead)",
			""},
		{"validate nil unit file state",
			"fake.service",
			"",
			"",
			""},
		{"validate expected unit file state",
			"fims.service",
			"",
			"",
			"disabled"},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			conn, err := dbus.New()
			if err != nil {
				log.Fatalf("Failed to connect to D-Bus: %v", err)
			}
			defer conn.Close()

			status, unitFileState, err := getUnitState(conn, test.service)
			if err != nil {
				// Check if error contains expectedError.
				assert.Containsf(t, err.Error(), test.expectedError,
					"Expected error: %q. Actual error: %s", test.expectedError, err)
			} else {
				// Verify expected output is correct.
				assert.Containsf(t, status, test.expectedStatus,
					"Expected output: %q. Actual output: %s", test.expectedError, err)
				assert.Containsf(t, unitFileState, test.expectedState,
					"Expected error: %q. Actual error: %s", test.expectedError, err)
			}

		})
	}
}

// Test errors on retrieval of a timestamp from dbus.
func TestGetUnitTime(t *testing.T) {

	var tests = []struct {
		name           string
		service        string
		expectedError  string
		expectedOutput string
	}{
		{"no unit file ext",
			"fims",
			"not valid",
			""},
		{"invalid delimiter in path",
			"fi/ms.service",
			"not valid",
			""},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			conn, err := dbus.New()
			if err != nil {
				log.Fatalf("Failed to connect to D-Bus: %v", err)
			}
			defer conn.Close()

			_, err = getUnitTime(conn, test.service)
			if err != nil {
				// Check if error contains expectedError.
				assert.Containsf(t, err.Error(), test.expectedError,
					"Expected error: %q. Actual error: %s", test.expectedError, err)
			}

		})
	}
}
