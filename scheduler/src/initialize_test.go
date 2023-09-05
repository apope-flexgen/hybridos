package main

import (
	"encoding/json"
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/internal/setpoint"
	"github.com/flexgen-power/scheduler/pkg/events"
)

var testCfg = schedulerConfig{
	SchedulerType: "FM",
	WebSockets: webSocketsConfig{
		ClientConfigs: &webSocketClientConfigList{
			webSocketClientConfig{
				Id:   "raleigh",
				Name: "Raleigh",
				Ip:   "172.16.2.82",
				Port: 9000,
			},
			webSocketClientConfig{
				Id:   "durham",
				Name: "Durham",
				Ip:   "172.16.2.83",
				Port: 9000,
			},
		},
	},
	Scada: scadaConfiguration{
		StageSize:    1,
		MaxNumEvents: 20,
		NumFloats:    1,
		NumInts:      0,
		NumBools:     1,
		NumStrings:   0,
	},
}
var testModes = modeMap{
	"default": &mode{
		Name:      "Default",
		Icon:      "Build",
		ColorCode: "gray",
		Constants: setpoint.List{
			setpoint.Setpoint{
				Id:      "charge_flag",
				Name:    "Charge Flag",
				VarType: "Bool",
				Unit:    "",
				Uri:     "/features/active_power/absolute_ess_direction_flag",
				Value:   false,
			},
			setpoint.Setpoint{
				Id:      "kw_cmd",
				Name:    "Absolute ESS Setpoint",
				VarType: "Float",
				Unit:    "kW",
				Uri:     "/features/active_power/absolute_ess_kW_cmd",
				Value:   0.0,
			},
			setpoint.Setpoint{
				Id:      "soc_target",
				Name:    "Target SoC Setpoint",
				VarType: "Float",
				Unit:    "%",
				Uri:     "/features/active_power/ess_charge_control_target_soc",
				Value:   0,
			},
		},
		Variables: nil,
	},
	"target_soc": &mode{
		Name:      "Target SOC",
		Icon:      "BatteryCharging",
		ColorCode: "lightGreen",
		Constants: setpoint.List{
			setpoint.Setpoint{
				Id:      "runmode1_kW_mode_cmd",
				Name:    "Active Power Feature",
				VarType: "Int",
				Unit:    "",
				Uri:     "/features/active_power/runmode1_kW_mode_cmd",
				Value:   1,
			},
		},
		Variables: setpoint.List{
			setpoint.Setpoint{
				Id:      "soc_target",
				Name:    "Target SoC Setpoint",
				VarType: "Float",
				Unit:    "%",
				Uri:     "/features/active_power/ess_charge_control_target_soc",
				Value:   100,
			},
		},
	},
}
var testTimezones = map[string]string{
	"raleigh": "America/New_York",
	"durham":  "America/Chicago",
}
var testEvents = map[string]eventList{
	"durham": {
		&events.Event{
			Id:        0,
			StartTime: time.Now(),
			Duration:  90,
			Mode:      "target_soc",
			Variables: map[string]interface{}{"soc_target": 100},
			Repeat:    events.NewNonRepeatingSeries(),
		},
	},
	"raleigh": {},
}
var testLastMod = map[string]string{
	"timestamp": "2023-02-03T04:56:00Z",
}

// marshallTestCase helps transform a test case into a valid input for buildInitialStateFromDocs
func marshalTestCase(inputCfg map[string]interface{}) map[string]json.RawMessage {
	marshalledCfg := make(map[string]json.RawMessage)
	for key, inputData := range inputCfg {
		// only add to return value if non-nil for that type
		if inputData != nil {
			marshalledData, _ := json.Marshal(inputData)
			marshalledCfg[key] = json.RawMessage(marshalledData)
		}
	}
	return marshalledCfg
}

func TestBuildInitialStateFromDocs(t *testing.T) {
	type testCase struct {
		// interfaces are used so invalid or empty data types can be passed
		configDoc     interface{}
		modesDoc      interface{}
		timeZoneDoc   interface{}
		eventsDoc     interface{}
		lastModDoc    interface{}
		errorExpected bool
	}
	tests := []testCase{
		// Case 0: configuration.json ONLY
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: nil, errorExpected: false},

		// Case 1: configuration document and modes document ONLY
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: nil, errorExpected: false},

		// Case 2: configuration document and timezones document ONLY
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: testTimezones, eventsDoc: nil, lastModDoc: nil, errorExpected: false},

		// Case 3: configuration document, modes document, and timezones document
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: testTimezones, eventsDoc: nil, lastModDoc: nil, errorExpected: false},

		// Case 4: all necessary documents (configuration document, modes document, timezones document,
		//         events document, and last_schedule_modification document)
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: testTimezones, eventsDoc: testEvents, lastModDoc: testLastMod, errorExpected: false},

		// Case 5: modes document ONLY
		{configDoc: nil, modesDoc: testModes, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: nil, errorExpected: true},

		// Case 6: timezones document ONLY
		{configDoc: nil, modesDoc: nil, timeZoneDoc: testTimezones, eventsDoc: nil, lastModDoc: nil, errorExpected: true},

		// Case 7: events document ONLY
		{configDoc: nil, modesDoc: nil, timeZoneDoc: nil, eventsDoc: testEvents, lastModDoc: nil, errorExpected: true},

		// Case 8: last_schedule_modification document ONLY
		{configDoc: nil, modesDoc: nil, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: testLastMod, errorExpected: true},

		// Case 9: configuration document and events document ONLY
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: nil, eventsDoc: testEvents, lastModDoc: nil, errorExpected: true},

		// Case 10: configuration document and last_schedule_modification document ONLY
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: testLastMod, errorExpected: true},

		// Case 11: valid configuration document and malformed modes document
		{configDoc: testCfg, modesDoc: testCfg, timeZoneDoc: nil, eventsDoc: nil, lastModDoc: nil, errorExpected: true},

		// Case 12: valid configuration document and malformed timezones document
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: testEvents, eventsDoc: nil, lastModDoc: nil, errorExpected: true},

		// Case 13: configuration document, events document, and last_schedule_modification document
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: nil, eventsDoc: testEvents, lastModDoc: testLastMod, errorExpected: true},

		// Case 14: modes document, timezones document, events document, and last_schedule_modification document (no config)
		{configDoc: nil, modesDoc: testModes, timeZoneDoc: testTimezones, eventsDoc: testEvents, lastModDoc: testLastMod, errorExpected: true},

		// Case 15: configuration document, timezones document, events document, and last_schedule_modification document (no modes)
		{configDoc: testCfg, modesDoc: nil, timeZoneDoc: testTimezones, eventsDoc: testEvents, lastModDoc: testLastMod, errorExpected: true},

		// Case 16: configuration document, modes document, events document, and last_schedule_modification document (no timezones)
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: nil, eventsDoc: testEvents, lastModDoc: testLastMod, errorExpected: true},

		// Case 17: configuration document, modes document, timezones document, and last_schedule_modification document (no events)
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: testTimezones, eventsDoc: nil, lastModDoc: testLastMod, errorExpected: true},

		// Case 18: configuration document, modes document, timezones document, and events document (no last modification)
		{configDoc: testCfg, modesDoc: testModes, timeZoneDoc: testTimezones, eventsDoc: testEvents, lastModDoc: nil, errorExpected: true},

		// Case 19: configuration document, time zone document, and malformed modes document
		{configDoc: testCfg, modesDoc: testLastMod, timeZoneDoc: testTimezones, eventsDoc: nil, lastModDoc: nil, errorExpected: true},
	}

	for testNum, test := range tests {
		testCaseMap := map[string]interface{}{
			"configuration":              test.configDoc,
			"modes":                      test.modesDoc,
			"timezones":                  test.timeZoneDoc,
			"events":                     test.eventsDoc,
			"last_schedule_modification": test.lastModDoc,
		}
		testDocMap := marshalTestCase(testCaseMap)

		_, err := buildInitialStateFromDocs(testDocMap)
		// Couldn't quantify specific error, so check if the actual error is non-nil
		if err == nil && test.errorExpected {
			t.Errorf("Test Case %d did not return an error when expected to throw one", testNum)
		} else if err != nil && !test.errorExpected {
			t.Errorf("Test Case %d did return an error '%v' when no error was expected", testNum, err)
		}
	}
}
