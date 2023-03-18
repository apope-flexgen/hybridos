package main

import (
	"testing"
)

type withinToleranceTest struct {
	inputScadapoint *scadapoint
	expectedBool    bool
}

type withinToleranceTestCases []*withinToleranceTest

func TestWithinToleranceTestCases(t *testing.T) {
	withinTestCases := withinToleranceTestCases{
		//Test 0 Default values - zeroes
		{
			//input scadapoint
			&scadapoint{
				id:      "test 0",
				control: float64(0),
				status:  float64(0),
			},
			//expected boolean return
			true,
		},
		//Test 1 Same value, different sign
		{
			&scadapoint{
				id:      "test 1",
				control: float64(-5.764),
				status:  float64(5.764),
			},
			false,
		},
		//Test 2 Exact same value
		{
			&scadapoint{
				id:      "test 2",
				control: float64(1.234567),
				status:  float64(1.234567),
			},
			true,
		},
		//Test 3 Values just within 10% tolerance
		{
			&scadapoint{
				id:      "test 3",
				control: float64(1.001),
				status:  float64(1.0),
			},
			true,
		},
		//Test 4 Values just outside 10% tolerance
		{
			&scadapoint{
				id:      "test 4",
				control: float64(1.11),
				status:  float64(1.0),
			},
			false,
		},
		//Test 5 Control value is zero
		{
			&scadapoint{
				id:      "test 5",
				control: float64(0),
				status:  float64(5),
			},
			false,
		},
		//Test 6 Status value is zero
		{
			&scadapoint{
				id:      "test 6",
				control: float64(6),
				status:  float64(0),
			},
			false,
		},
		//Test 7 Within 10%, but still large difference
		{
			&scadapoint{
				id:      "test 7",
				control: float64(101.25),
				status:  float64(95.25),
			},
			false,
		},
	}
	for i, testCase := range withinTestCases {
		output := testCase.inputScadapoint.withinTolerance(.1)
		if output != testCase.expectedBool {
			t.Errorf("withinTolerance() test %d failed. Expected '%v' and got '%v'", i, testCase.expectedBool, output)
		}
	}
}
