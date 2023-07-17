package main

import "testing"

// Test auto PFR down cmd calculation
func TestCalculateAutoPfrDownCmd(t *testing.T) {
	type testCaseStruct struct {
		siteChargeable float64
		expectedCmd    float64
	}

	testCases := []testCaseStruct{
		{
			siteChargeable: 0,
			expectedCmd:    0,
		},
		{
			siteChargeable: 2000,
			expectedCmd:    2000,
		},
		{
			siteChargeable: -2000,
			expectedCmd:    0,
		},
	}

	for _, testCase := range testCases {
		fPoints := fleetPoints{
			genMaxCharge: muxFloat{actual: testCase.siteChargeable},
		}
		s := &site{}

		cmd := fPoints.calculateAutoPfrDownCmd(s)
		if cmd != testCase.expectedCmd {
			t.Errorf("Expected cmd was %f but calculated was %f.", testCase.expectedCmd, cmd)
		}
	}
}

// Test auto PFR up cmd calculation
func TestCalculateAutoPfrUpCmd(t *testing.T) {
	type testCaseStruct struct {
		siteDischargeable float64
		ffrActiveCmd      float64
		expectedCmd       float64
	}

	testCases := []testCaseStruct{
		{
			siteDischargeable: 0,
			ffrActiveCmd:      0,
			expectedCmd:       0,
		},
		{
			siteDischargeable: 2000,
			ffrActiveCmd:      0,
			expectedCmd:       2000,
		},
		{
			siteDischargeable: -2000,
			ffrActiveCmd:      0,
			expectedCmd:       0,
		},
		{
			siteDischargeable: 2000,
			ffrActiveCmd:      1000,
			expectedCmd:       1000,
		},
		{
			siteDischargeable: 2000,
			ffrActiveCmd:      2000,
			expectedCmd:       0,
		},
		{
			siteDischargeable: 2000,
			ffrActiveCmd:      3000,
			expectedCmd:       0,
		},
	}

	for _, testCase := range testCases {
		fPoints := fleetPoints{
			genMaxDischarge: muxFloat{actual: testCase.siteDischargeable},
		}
		s := &site{
			freqResp: frequencyResponseData{
				rrsFfrCmd: scadapoint{control: testCase.ffrActiveCmd},
			},
		}

		cmd := fPoints.calculateAutoPfrUpCmd(s)
		if cmd != testCase.expectedCmd {
			t.Errorf("Expected cmd was %f but calculated was %f.", testCase.expectedCmd, cmd)
		}
	}
}
