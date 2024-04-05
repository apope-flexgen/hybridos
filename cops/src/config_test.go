package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

// Path to config test file

var testConfigPath = "../test/config/testConfig.json"

func TestParse(t *testing.T) {
	// Table of tests to run
	var tests = []struct {
		name          string
		fileInput     string
		expectedError string
	}{
		{"no forced errors",
			testConfigPath,
			""},
		{"filepath error",
			"incorrect/path",
			"reading config file:"},
	}

	// Execute tests
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := parse(tt.fileInput)
			if err != nil {
				// Check if error contains expectedError
				assert.Containsf(t, err.Error(), tt.expectedError,
					"Expected error: %q. Actual error: %s", tt.expectedError, err)
			}
		})
	}
}

func BenchmarkParse(b *testing.B) {
	for i := 0; i < b.N; i++ {
		parse(testConfigPath)
	}
}

func TestConfigValidate(t *testing.T) {
	var tests = []struct {
		CompareIO      bool   // check equality between input to output
		name           string // name of the test
		input          *Config
		expectedOutput *Config
		expectedError  string
	}{
		{true,
			"Test default values",
			&Config{},
			&Config{Name: "localhost.localdomain",
				HeartbeatFrequencyMS:        1000,
				PatrolFrequencyMS:           1000,
				BriefingFrequencyMS:         5000,
				C2cMsgFrequencyMS:           50,
				ConnectionHangtimeAllowance: 3500},
			""},
		{false,
			"Test process names not provided",
			&Config{ProcessList: []Process{{Name: ""}}},
			&Config{ProcessList: []Process{{Name: ""}}},
			"process name not provided."},
	}

	// Execute tests
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := tt.input.validate()
			if err == nil && tt.expectedError != "" {
				t.Errorf("Expected %s error, recieved nil err", tt.expectedError)
			} else if err != nil && tt.expectedError == "" {
				t.Errorf("Expected no error, recieved %s err", err)
			} else if tt.CompareIO {
				// Compare expected input vs output if applicable
				assert.Equal(t, tt.expectedOutput, tt.input, "The two structs should be the same.")
			}

		})
	}

}

func TestHandleConfigBody(t *testing.T) {
	// Table of tests to run
	var tests = []struct {
		name          string
		input         map[string]interface{}
		expectedError string
	}{
		{"no error",
			map[string]interface{}{"name": "cops1"},
			""},
		{"no process name",
			map[string]interface{}{"processList": []Process{{Name: ""}}},
			"process name not provided"},
	}

	// Execute tests
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := handleConfigBody(tt.input)
			if err != nil {
				// Check if error contains expectedError
				assert.Containsf(t, err.Error(), tt.expectedError,
					"Expected error: %q. Actual error: %s", tt.expectedError, err)
			}
		})
	}
}

func BenchmarkHandleConfigBody(b *testing.B) {
	for i := 0; i < b.N; i++ {
		handleConfigBody(map[string]interface{}{})
	}
}
