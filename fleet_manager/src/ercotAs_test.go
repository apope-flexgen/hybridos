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

// Check that editSetting successfully edits variables
func TestEditSetting(t *testing.T) {
	// Currently only tests the calculated_r endpoints
	type testCaseStruct struct {
		variableId string
		pVariable  *float64 // pointer to variable being modified
		newValue1  float64  // first value to set variable to
		newValue2  float64  // second value to set variable to
	}
	siteId := "test_site"

	fp := &fleetPoints{}
	// set id roots for all necessary variables
	var cfg map[string]interface{} // nil config
	fp.emergencyDownRamp = parseGLMuxFloat(cfg, "emergency_down_ramp_rate")
	fp.emergencyUpRamp = parseGLMuxFloat(cfg, "emergency_up_ramp_rate")
	fp.normalDownRamp = parseGLMuxFloat(cfg, "normal_down_ramp_rate")
	fp.normalUpRamp = parseGLMuxFloat(cfg, "normal_up_ramp_rate")
	fp.scedBasepoint = parseGLMuxFloat(cfg, "basepoint")
	fp.responsiveReserveFfr = parseGLSNoMux(cfg, "ffr")
	fp.frrsDown = parseGLPService(cfg, "frrs_down")
	fp.frrsUp = parseGLPService(cfg, "frrs_up")
	fp.regulationDown = parseGLPService(cfg, "regulation_down")
	fp.regulationUp = parseGLPService(cfg, "regulation_up")
	fp.resourceStatus = parseGLMuxInt(cfg, "resource_status")
	fp.responsiveReservePfr = parseGLSNoMux(cfg, "responsive_reserve")
	fp.nonSpin = parseGLSService(cfg, "non_spin")
	fp.updatedBasepoint = parseGLMuxFloat(cfg, "updated_basepoint")

	testCases := []testCaseStruct{
		{
			"calculated_rurs_gen",
			&fp.calcRursGen,
			0.0,
			1.5,
		},
		{
			"calculated_rupf_gen",
			&fp.calcRupfGen,
			0.0,
			2.7,
		},
		{
			"calculated_rdrs_gen",
			&fp.calcRdrsGen,
			0.0,
			-3.1,
		},
		{
			"calculated_rdpf_gen",
			&fp.calcRdpfGen,
			0.0,
			-4.6,
		},
		{
			"calculated_rurs_load",
			&fp.calcRursLoad,
			0.0,
			-1.5,
		},
		{
			"calculated_rupf_load",
			&fp.calcRupfLoad,
			0.0,
			-2.7,
		},
		{
			"calculated_rdrs_load",
			&fp.calcRdrsLoad,
			0.0,
			3.1,
		},
		{
			"calculated_rdpf_load",
			&fp.calcRdpfLoad,
			0.0,
			4.6,
		},
	}

	for _, testCase := range testCases {
		err := fp.editSetting(testCase.variableId, siteId, testCase.newValue1)
		if err != nil {
			t.Errorf("Error editing setting for variable %s and value %v: %v", testCase.variableId, testCase.newValue1, err)
			continue
		}
		if *testCase.pVariable != testCase.newValue1 {
			t.Errorf("Error editing setting for variable %s and value %v: value should now be %v but is instead %v", testCase.variableId, testCase.newValue1, testCase.newValue1, *testCase.pVariable)
			continue
		}
		err = fp.editSetting(testCase.variableId, siteId, testCase.newValue2)
		if err != nil {
			t.Errorf("Error editing setting for variable %s and value %v: %v", testCase.variableId, testCase.newValue2, err)
			continue
		}
		if *testCase.pVariable != testCase.newValue2 {
			t.Errorf("Error editing setting for variable %s and value %v: value should now be %v but is instead %v", testCase.variableId, testCase.newValue2, testCase.newValue2, *testCase.pVariable)
			continue
		}
	}
}
