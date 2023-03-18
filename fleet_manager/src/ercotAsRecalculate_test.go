package main

// TO DO: revisit these unit tests.
// move ercotAs into its own package.
// make them smaller (skip the parsing / site creation and just instantiate fleetPoints and site as input)
// only focus on ercotAs feature, not generic site calculations

// import (
// 	"fmt"
// 	"math"
// 	"os"
// 	"testing"

// 	log "github.com/flexgen-power/go_flexgen/logger"
// )

// func initializeLogger() {
// 	err := log.InitConfig("fleet_man").Init("fleet_man")
// 	if err != nil {
// 		fmt.Fprintf(os.Stderr, "Logger failed to initialize: %v\n", err)
// 		os.Exit(-1)
// 	}
// }

// type recalculateSitePointsTest struct {
// 	inputFleetPoints    map[string]interface{}
// 	inputScadapoints    *site
// 	expectedScadapoints *site
// }

// type recalculateSitePointsTestCases []*recalculateSitePointsTest

// func TestRecalculateSitePointsTestCases(t *testing.T) {
// 	initializeLogger()
// 	features.ercotAs = &ercotAsFeature{}
// 	features.caisoAds = &caisoAdsFeature{}
// 	// Precision variable
// 	var precision float64 = 1000000.0
// 	// Create the test cases
// 	recalculateSpTestCases := recalculateSitePointsTestCases{
// 		// Test 0 Enable RRS-FF w/ out of range baseload MW cmd
// 		{
// 			//input FM points
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     true,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			//input site
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 24,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			//output site
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -9.9,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 28,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -9.9,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 1 Enable RRS-FF w/ in-range baseload MW cmd
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       2.7,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -2.7,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 28,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -2.7,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 2 Enable FRRS Up w/ reg manual input to baseload cmd
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   32.5,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      2.7,
// 					"load_requirement_override":    true,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  true,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -2.7,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 25,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -29.8,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -2.7,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 32.5,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 3 Enable FRRS Up w/o reg input to baseload cmd
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   32.5,
// 					"load_responsibility_manual":   22.6,
// 					"load_responsibility_override": true,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  true,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 25,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -22.6,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 22.6,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 4 Enable FRRS Down w/ updated basepoint input
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   1.3,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   22.6,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     true,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"updated_basepoint": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  false,
// 					"load_actual":   12.3,
// 					"load_manual":   0.0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -9.9,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 26,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -11.2,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 1.3,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 5 Enable RRS-PF Up w/ neg baseload
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"regulation_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       2.5,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  12.5,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 2.5,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 24,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -2.5,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 6 Enable RRS-PF Down w/ load rrs responsibility
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"regulation_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       2.5,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 12.5,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 2.5,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 24,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -2.5,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 7 Enable RRS-FF w/ negative inactive MW cmd limit
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      12.9,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    21,
// 					"gen_override":  true,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  2.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 12.5,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 28,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -14.5,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 14.5,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 8 Enable RRS-FF & FRRS Down
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  12.5,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"frrs_down": map[string]interface{}{
// 					"gen_responsibility_actual":    3.4,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     false,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       2.0,
// 					"gen_requirement_manual":       1.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       2.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    21,
// 					"gen_override":  true,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -2.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 30,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -10.5,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -5.4,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 12.5,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 3.4,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 9 Enable FRRS Up
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    12.3,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  2.7,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 25,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -12.3,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 12.3,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 10 Enable FRRS Up w/ non-zero gen FFR requirement
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    12.3,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    0,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 2.7,
// 					"load_requirement":    2.7,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 25,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -15.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 2.7,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 12.3,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 11 Enable RRS-FF & FRRS Up w/ non-zero gen FFR requirement
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     20.7,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    12.3,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       20.6,
// 					"gen_requirement_override":     false,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     20.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 10.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: -9.9,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 29,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -2.4,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: -9.9,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 12.3,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 		},
// 		// Test 12 gen voltage
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     1.2,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    2.0,
// 					"gen_responsibility_manual":    0.3,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"frrs_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   3.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       1.2,
// 					"gen_requirement_manual":       1.3,
// 					"gen_requirement_override":     true,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      1.2,
// 					"load_requirement_manual":      1.3,
// 					"load_requirement_override":    true,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"updated_basepoint": map[string]interface{}{
// 					"gen_actual":    5.8,
// 					"gen_manual":    0.0,
// 					"gen_override":  false,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 754.2,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 1123.4,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 1000.0,
// 					},
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 2.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 29,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -2.3,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.3,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 754.2,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 1123.4,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 1000.0,
// 					},
// 					avgLineVoltage: .9592,
// 				},
// 			},
// 		},
// 		// Test 13 All random numbers
// 		{
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"ffr": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     1.2,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"frrs_up": map[string]interface{}{
// 					"gen_responsibility_actual":    2.0,
// 					"gen_responsibility_manual":    0.3,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"frrs_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  true,
// 					"gen_requirement_actual":       0.0,
// 					"gen_requirement_manual":       0.0,
// 					"gen_requirement_override":     true,
// 					"gen_participation":            0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   3.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      0.0,
// 					"load_requirement_manual":      0.0,
// 					"load_requirement_override":    false,
// 					"load_participation":           0.0,
// 				},
// 				"regulation_down": map[string]interface{}{
// 					"gen_responsibility_actual":    0.0,
// 					"gen_responsibility_manual":    0.0,
// 					"gen_responsibility_override":  false,
// 					"gen_requirement_actual":       1.2,
// 					"gen_requirement_manual":       1.3,
// 					"gen_requirement_override":     true,
// 					"gen_scheduled":                0.0,
// 					"load_responsibility_actual":   0.0,
// 					"load_responsibility_manual":   0.0,
// 					"load_responsibility_override": false,
// 					"load_requirement_actual":      1.2,
// 					"load_requirement_manual":      1.3,
// 					"load_requirement_override":    true,
// 					"load_scheduled":               0.0,
// 				},
// 				"resource_status": map[string]interface{}{
// 					"gen_actual":    21,
// 					"gen_manual":    19,
// 					"gen_override":  false,
// 					"load_actual":   0,
// 					"load_manual":   0,
// 					"load_override": false,
// 				},
// 				"responsive_reserve": map[string]interface{}{
// 					"gen_responsibility":  0.0,
// 					"gen_requirement":     0.0,
// 					"gen_scheduled":       0.0,
// 					"load_responsibility": 0.0,
// 					"load_requirement":    0.0,
// 					"load_scheduled":      0.0,
// 				},
// 				"updated_basepoint": map[string]interface{}{
// 					"gen_actual":    5.8,
// 					"gen_manual":    0.0,
// 					"gen_override":  false,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": false,
// 				},
// 				"inactive_cmd_min_limit_mw": -9.9,
// 				"inactive_cmd_max_limit_mw": 9.9,
// 				"baseload_cmd_min_limit_mw": -9.9,
// 				"baseload_cmd_max_limit_mw": 9.9,
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.0,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 10234.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 234.00,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 1234.5,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 2.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 29,
// 					},
// 					autoPfrUpCmd: scadapoint{
// 						control: -2.3,
// 					},
// 					autoPfrDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					rrsFfrCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsDownCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					frrsUpCmd: scadapoint{
// 						control: 0.3,
// 					},
// 				},
// 				ess: essData{},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 10234.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 234.00,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 1234.5,
// 					},
// 					avgLineVoltage: 3.900833333333,
// 				},
// 			},
// 		},
// 	}
// 	for i, testCase := range recalculateSpTestCases {
// 		fmt.Printf("-------TEST %d--------\n", i)
// 		outputSite, err := createSite(fmt.Sprintf("test_%v", i), map[string]interface{}{"name": fmt.Sprintf("Test %v", i), "feature": ercotAsId})
// 		if outputSite == nil || err != nil {
// 			t.Errorf(err.Error())
// 			return
// 		}
// 		var expectedControl float64
// 		var calculatedControl float64
// 		inputFp, err := parseFleetPoints(testCase.inputFleetPoints)
// 		if err != nil {
// 			t.Errorf(err.Error())
// 			return
// 		}

// 		// FFRA
// 		outputSite.recalculateErcotFeatureVariables(inputFp)
// 		expectedFreqResp := testCase.expectedScadapoints.freqResp

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.baseloadCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.baseloadCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_baseload_power_command", i, expectedFreqResp.baseloadCmd.control.(float64), outputSite.freqResp.baseloadCmd.control.(float64))
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.inactiveFfrCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.inactiveFfrCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_inactive_power_command", i, expectedFreqResp.inactiveFfrCmd.control.(float64), outputSite.freqResp.inactiveFfrCmd.control.(float64))
// 		}

// 		if expectedFreqResp.responseEnableBitField.control.(int) != outputSite.freqResp.responseEnableBitField.control.(int) {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for response_enable_mask", i, expectedFreqResp.responseEnableBitField.control.(int), outputSite.freqResp.responseEnableBitField.control.(int))
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.autoPfrDownCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.autoPfrDownCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_auto_pfr_down_cmd", i, expectedFreqResp.autoPfrDownCmd.control.(float64), outputSite.freqResp.autoPfrDownCmd.control.(float64))
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.autoPfrUpCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.autoPfrUpCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_auto_pfr_up_cmd", i, expectedControl, calculatedControl)
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.frrsDownCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.frrsDownCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_frrs_down_cmd", i, expectedControl, calculatedControl)
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.frrsUpCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.frrsUpCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_frrs_up_cmd", i, expectedControl, calculatedControl)
// 		}

// 		expectedControl = math.Round(testCase.expectedScadapoints.freqResp.rrsFfrCmd.control.(float64) * precision)
// 		calculatedControl = math.Round(outputSite.freqResp.rrsFfrCmd.control.(float64) * precision)
// 		if expectedControl != calculatedControl {
// 			t.Errorf("recalculateErcotFeatureVariables() test %d failed. Expected: %v  Got: %v for fr_rrs_ffr_cmd", i, expectedControl, calculatedControl)
// 		}

// 		// Site
// 		testCase.inputScadapoints.recalculateSiteVariables()
// 		calculatedControl = math.Round(testCase.inputScadapoints.gridVolts.avgLineVoltage * precision)
// 		expectedControl = math.Round(testCase.expectedScadapoints.gridVolts.avgLineVoltage * precision)
// 		if calculatedControl != expectedControl {
// 			t.Errorf("recalculate() test %d failed. Expected: %v  Got: %v for gen_voltage", i, expectedControl, calculatedControl)
// 		}
// 	}
// }

// type recalculateFleetPointsTest struct {
// 	inputScadapoints *site
// 	expectedFm       map[string]interface{}
// }

// type recalculateFleetPointsTestCases []*recalculateFleetPointsTest

// func TestRecalculateFleetPointsTestCases(t *testing.T) {
// 	features.caisoAds = &caisoAdsFeature{}
// 	features.ercotAs = &ercotAsFeature{}
// 	// Precision variable
// 	var precision float64 = 1000000.0
// 	initializeLogger()
// 	// Create the test cases
// 	recalculateFpTestCases := recalculateFleetPointsTestCases{
// 		// Test 0 load pseudo switch status
// 		{
// 			//input scadapoints
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 						status:  0,
// 					},
// 				},
// 				ess: essData{
// 					chargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 					dischargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 				},
// 				gridVolts: gridVoltageData{
// 					line1ToLine2: scadapoint{
// 						status: 0.0,
// 					},
// 					line2ToLine3: scadapoint{
// 						status: 0.0,
// 					},
// 					line3ToLine1: scadapoint{
// 						status: 0.0,
// 					},
// 					avgLineVoltage: 0.0,
// 				},
// 			},
// 			//expected fleet points
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"load_pseudo_switch_status_actual":   true,
// 				"load_pseudo_switch_status_manual":   false,
// 				"load_pseudo_switch_status_override": false,
// 				"gen_max_discharge_actual":           0.0,
// 				"gen_max_discharge_manual":           0.0,
// 				"gen_max_discharge_override":         false,
// 				"gen_max_charge_actual":              0.0,
// 				"gen_max_charge_manual":              0.0,
// 				"gen_max_charge_override":            false,
// 				"baseload_cmd_min_limit_mw":          -9.9,
// 				"inactive_cmd_min_limit_mw":          -9.9,
// 				"baseload_cmd_max_limit_mw":          9.9,
// 				"inactive_cmd_max_limit_mw":          9.9,
// 			},
// 		},
// 		// Test 1 max charge
// 		{
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 						status:  0,
// 					},
// 				},
// 				ess: essData{
// 					chargeablePower: scadapoint{
// 						status: 1234.5,
// 					},
// 					dischargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 				},
// 				gridVolts: gridVoltageData{},
// 			},
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"load_pseudo_switch_status_actual":   false,
// 				"load_pseudo_switch_status_manual":   false,
// 				"load_pseudo_switch_status_override": false,
// 				"gen_max_discharge_actual":           0.0,
// 				"gen_max_discharge_manual":           0.0,
// 				"gen_max_discharge_override":         false,
// 				"gen_max_charge_actual":              1.2345,
// 				"gen_max_charge_manual":              0.0,
// 				"gen_max_charge_override":            false,
// 				"baseload_cmd_min_limit_mw":          -9.9,
// 				"inactive_cmd_min_limit_mw":          -9.9,
// 				"baseload_cmd_max_limit_mw":          9.9,
// 				"inactive_cmd_max_limit_mw":          9.9,
// 			},
// 		},
// 		// Test 2 max discharge
// 		{
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 						status:  0,
// 					},
// 				},
// 				ess: essData{
// 					chargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 					dischargeablePower: scadapoint{
// 						status: 735.123,
// 					},
// 				},
// 				gridVolts: gridVoltageData{},
// 			},
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"load_pseudo_switch_status_actual":   false,
// 				"load_pseudo_switch_status_manual":   false,
// 				"load_pseudo_switch_status_override": false,
// 				"gen_max_discharge_actual":           0.735123,
// 				"gen_max_discharge_manual":           0.0,
// 				"gen_max_discharge_override":         false,
// 				"gen_max_charge_actual":              0.0,
// 				"gen_max_charge_manual":              0.0,
// 				"gen_max_charge_override":            false,
// 				"baseload_cmd_min_limit_mw":          -9.9,
// 				"inactive_cmd_min_limit_mw":          -9.9,
// 				"baseload_cmd_max_limit_mw":          9.9,
// 				"inactive_cmd_max_limit_mw":          9.9,
// 			},
// 		},
// 		// Test 3 average grid voltage
// 		{
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 						status:  0,
// 					},
// 				},
// 				ess: essData{
// 					chargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 					dischargeablePower: scadapoint{
// 						status: 0.0,
// 					},
// 				},
// 				gridVolts: gridVoltageData{},
// 			},
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"load_pseudo_switch_status_actual":   false,
// 				"load_pseudo_switch_status_manual":   false,
// 				"load_pseudo_switch_status_override": false,
// 				"gen_max_discharge_actual":           0.0,
// 				"gen_max_discharge_manual":           0.0,
// 				"gen_max_discharge_override":         false,
// 				"gen_max_charge_actual":              0.0,
// 				"gen_max_charge_manual":              0.0,
// 				"gen_max_charge_override":            false,
// 				"baseload_cmd_min_limit_mw":          -9.9,
// 				"inactive_cmd_min_limit_mw":          -9.9,
// 				"baseload_cmd_max_limit_mw":          9.9,
// 				"inactive_cmd_max_limit_mw":          9.9,
// 			},
// 		},
// 		// Test 4 random values
// 		{
// 			&site{
// 				activePower:   scadapoint{},
// 				reactivePower: scadapoint{},
// 				genAvrStatus:  scadapoint{},
// 				freqResp: frequencyResponseData{
// 					baseloadCmd: scadapoint{
// 						control: 0.0,
// 					},
// 					responseEnableBitField: scadapoint{
// 						control: 0,
// 						status:  0,
// 					},
// 				},
// 				ess: essData{
// 					chargeablePower: scadapoint{
// 						status: 0.2,
// 					},
// 					dischargeablePower: scadapoint{
// 						status: 10304.0,
// 					},
// 				},
// 				gridVolts: gridVoltageData{},
// 			},
// 			map[string]interface{}{
// 				"emergency_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"emergency_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_down_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"normal_up_ramp_rate": map[string]interface{}{
// 					"gen_actual":    0.0,
// 					"gen_manual":    0.0,
// 					"gen_override":  0.0,
// 					"load_actual":   0.0,
// 					"load_manual":   0.0,
// 					"load_override": 0.0,
// 				},
// 				"load_pseudo_switch_status_actual":   false,
// 				"load_pseudo_switch_status_manual":   false,
// 				"load_pseudo_switch_status_override": false,
// 				"gen_max_discharge_actual":           10.304,
// 				"gen_max_discharge_manual":           0.0,
// 				"gen_max_discharge_override":         false,
// 				"gen_max_charge_actual":              0.0002,
// 				"gen_max_charge_manual":              0.0,
// 				"gen_max_charge_override":            false,
// 				"baseload_cmd_min_limit_mw":          -9.9,
// 				"inactive_cmd_min_limit_mw":          -9.9,
// 				"baseload_cmd_max_limit_mw":          9.9,
// 				"inactive_cmd_max_limit_mw":          9.9,
// 			},
// 		},
// 	}
// 	for i, testCase := range recalculateFpTestCases {
// 		inputFm := &fleetPoints{}
// 		testCase.inputScadapoints.recalculateErcotFeatureVariables(inputFm)
// 		calculatedFm := inputFm
// 		expectedFm, err := parseFleetPoints(testCase.expectedFm)
// 		if err != nil {
// 			t.Errorf(err.Error())
// 			return
// 		}
// 		var actualFloat float64
// 		var expectedFloat float64

// 		fmt.Printf("---recalculateSitePoints() Test %d---\n", i)
// 		if calculatedFm.loadPseudoSwitchStatus.getSelect() != expectedFm.loadPseudoSwitchStatus.getSelect() {
// 			t.Errorf("recalculate() test %d failed. Calculated loadPseudoSwitchStatus is incorrect", i)
// 			t.Errorf("Expected: %v  Got: %v", expectedFm.loadPseudoSwitchStatus.getSelect(), calculatedFm.loadPseudoSwitchStatus.getSelect())
// 		}

// 		actualFloat = math.Round(calculatedFm.genMaxCharge.getSelect() * precision)
// 		expectedFloat = math.Round(expectedFm.genMaxCharge.getSelect() * precision)
// 		if actualFloat != expectedFloat {
// 			t.Errorf("recalculate() test %d failed. Calculated genMaxCharge is incorrect", i)
// 			t.Errorf("Expected: %v  Got: %v", expectedFloat, actualFloat)
// 		}

// 		actualFloat = math.Round(calculatedFm.genMaxDischarge.getSelect() * precision)
// 		expectedFloat = math.Round(expectedFm.genMaxDischarge.getSelect() * precision)
// 		if actualFloat != expectedFloat {
// 			t.Errorf("recalculate() test %d failed. Calculated genMaxDischarge is incorrect", i)
// 			t.Errorf("Expected: %v  Got: %v", expectedFloat, actualFloat)
// 		}
// 	}
// }
