package main

import (
	"reflect"
	"testing"

	"github.com/stretchr/testify/assert"
)

// isIPLessThan function unit test
var passedTest bool = false

func TestIsIPLessThan(t *testing.T) {
	var err error
	// test same IP address, first port is lower
	input1 := "127.0.0.1:8000"
	input2 := "127.0.0.1:8001"
	result, _ := isIPLessThan(input1, input2)
	if result != true {
		t.Errorf("compareIPAddrs(%v,%v) failed, expected true, got %v", input1, input2, result)
	}

	// test same IP address, first port is higher
	input1 = "127.0.0.1:8001"
	input2 = "127.0.0.1:8000"
	result, _ = isIPLessThan(input1, input2)
	if result != false {
		t.Errorf("compareIPAddrs(%v,%v) failed, expected false, got %v", input1, input2, result)
	}

	// test first IP address is lower
	input1 = "127.0.0.0:8001"
	input2 = "127.0.0.1:8000"
	result, _ = isIPLessThan(input1, input2)
	if result != true {
		t.Errorf("compareIPAddrs(%v,%v) failed, expected true, got %v", input1, input2, result)
	}

	// test first IP address is higher
	input1 = "127.0.0.1:8000"
	input2 = "127.0.0.0:8001"
	result, _ = isIPLessThan(input1, input2)
	if result != false {
		t.Errorf("compareIPAddrs(%v,%v) failed, expected false, got %v", input1, input2, result)
	}

	// test invalid IP address
	input1 = "127.0.1:8001"
	input2 = "127.0.0.1:8000"
	expectedError := "formatting is invalid"
	result, err = isIPLessThan(input1, input2)
	assert.Containsf(t, err.Error(), expectedError,
		"Expected error: %q. Actual error: %s,", expectedError, err)

	// test no port given
	input1 = "127.0.0.1:8000"
	input2 = "127.0.0.1"
	expectedError = "formatting is invalid"
	result, err = isIPLessThan(input1, input2)
	assert.Containsf(t, err.Error(), expectedError,
		"Expected error: %q. Actual error: %s,", expectedError, err)

	// test identical IP addresses and ports
	input1 = "127.0.0.1:8000"
	input2 = "127.0.0.1:8000"
	expectedError = "identical"
	result, err = isIPLessThan(input1, input2)
	assert.Containsf(t, err.Error(), expectedError,
		"Expected error: %q. Actual error: %s,", expectedError, err)
}

// fimsToC2c unit test
func TestFimsToC2C(t *testing.T) {
	keyword := "Setpoints"
	uri := "/cops/site_controller/setpoints/assets/ess/ess_1/maint_mode"
	body := make(map[string]interface{})
	body["maint_mode"] = true
	actual := fimsToC2C(keyword, uri, body)
	expected := "Setpoints$$/cops/site_controller/setpoints/assets/ess/ess_1/maint_mode$${\"maint_mode\":true}"
	if expected != actual {
		t.Errorf("TestFimsToC2C failed, expected: %v, actual %v", expected, actual)
	}
}

// parseSetpointsMsg unit test
func TestParseSetpointsMsg(t *testing.T) {
	prefixURI := "/cops/site_controller/setpoints"
	msg := "Setpoints$$/cops/site_controller/setpoints/assets/ess/ess_1/maint_mode$${\"maint_mode\":true}"
	expectedURI := "/assets/ess/ess_1/maint_mode"
	expectedBody := make(map[string]interface{})
	expectedBody["maint_mode"] = true
	actualURI, actualBody := parseDBIMsg(true, prefixURI, msg)
	if expectedURI != actualURI {
		t.Errorf("TestFimsToC2C failed, uri mismatch, expected: %v, actual %v", expectedURI, actualURI)
	} else if !reflect.DeepEqual(expectedBody, actualBody) {
		t.Errorf("TestFimsToC2C failed, body mismatch, expected: %v, actual %v", expectedBody, actualBody)
	}

	prefixURI = "/assets/ess/ess_1/maint_mode"
	msg = "Setpoints$$/cops/site_controller/setpoints/assets/ess/ess_1/maint_mode$${\"maint_mode\":true}"
	expectedURI = "/assets/ess/ess_1/maint_mode"
	expectedBody = make(map[string]interface{})
	expectedBody["maint_mode"] = true
	actualURI, actualBody = parseDBIMsg(false, prefixURI, msg)
	if expectedURI != actualURI {
		t.Errorf("TestFimsToC2C failed, uri mismatch, expected: %v, actual %v", expectedURI, actualURI)
	} else if !reflect.DeepEqual(expectedBody, actualBody) {
		t.Errorf("TestFimsToC2C failed, body mismatch, expected: %v, actual %v", expectedBody, actualBody)
	}
}
