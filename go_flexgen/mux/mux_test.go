package mux

import (
	"fims"
	"testing"

	"github.com/stretchr/testify/assert"
)

type routeTest struct {
	name          string        // name of test
	route         *Route        // route to test
	msg           *fims.FimsMsg // FIMS test message
	expectedError string        // expected error string (if applicable)
}

// Test handle errors when serving a FIMS message.
func TestServe(t *testing.T) {
	r := NewRouter()

	tests := []routeTest{
		{
			name:          "Test not found URI",
			route:         r.NewRoute("/test/serve"),
			msg:           &fims.FimsMsg{Uri: "/test/serve//"},
			expectedError: "not found",
		},
		{
			name:  "Test serving a fcn handler",
			route: r.NewRoute("/test/handler"),
			msg:   &fims.FimsMsg{Uri: "/test/handler"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			err := r.Serve(test.msg)
			if err != nil {
				testError(t, test, err)
			}
		})
	}

}

// Helper function for testing expected errors.
func testError(t *testing.T, test routeTest, err error) {
	// Check if error contains expectedError.
	assert.Containsf(t, err.Error(), test.expectedError,
		"Expected error: %q. Actual error: %s", test.expectedError, err)
}
