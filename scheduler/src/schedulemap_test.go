package main

import (
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/internal/flextime"
)

// Allows unit tests to fill in their test parameters without error checking.
//
// Should only be used in testing! Production code needs to handle all errors.
func forceLoadLocation(name string) *time.Location {
	loc, _ := time.LoadLocation(name)
	return loc
}

var CentralTime *time.Location = forceLoadLocation("America/Chicago")
var EasternTime *time.Location = forceLoadLocation("America/New_York")

func TestScheduleCheck(t *testing.T) {
	type testCase struct {
		currentTime time.Time
		inputMap    scheduleMap
		expectedMap scheduleMap
	}

	const odessa string = "odessa"
	// Set the dummy mode map
	modes = make(modeMap)
	modes[defaultModeId] = &mode{}
	modes["Frequency Response"] = &mode{}

	tests := []testCase{
		// Test 0: Eastern Daylight Savings 2021 March 14th Shift Forward; before shift
		{
			currentTime: time.Date(2021, time.March, 14, 1, 0, 0, 0, EasternTime),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 14, 1, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Today; 1:00 am EST
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.March, 14, 1, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Active; 1:00 am EST
				),
			},
		},
		// Test 1: Eastern Daylight Savings 2021 March 14th Shift Forward; at shift
		{
			currentTime: time.Date(2021, time.March, 14, 1, 59, 59, 0, EasternTime).Add(time.Second),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Today; 3:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Active; 3:00 am
				),
			},
		},
		// Test 2: Eastern Daylight Savings 2021 March 14th Shift Forward; after shift
		{
			currentTime: time.Date(2021, time.March, 14, 4, 0, 0, 0, EasternTime),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 14, 4, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Today; 4:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.March, 14, 4, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Active; 4:00 am
				),
			},
		},
		// Test 3: Eastern Daylight Savings 2021 Nov 7th Shift Backward; before shift
		{
			currentTime: time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Second),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Second), 60, "Frequency Response", 2000), // Today; 1:00 am EDT
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000),                    // Tomorrow; 6:00 am EST
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Second), 60, "Frequency Response", 2000), // Active; 1:00 am EDT
				),
			},
		},
		// Test 4: Eastern Daylight Savings 2021 Nov 7th Shift Backward; at shift
		{
			currentTime: time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Hour).Add(time.Second),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Hour).Add(time.Second), 60, "Frequency Response", 2000), // Today; 1:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000),                                   // Tomorrow; 6:00 am EST
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, EasternTime).Add(time.Hour).Add(time.Second), 60, "Frequency Response", 2000), // Active; 1:00 am EST
				),
			},
		},
		// Test 5: Eastern Daylight Savings 2021 Nov 7th Shift Backward; after shift
		{
			currentTime: time.Date(2021, time.November, 7, 3, 0, 0, 0, EasternTime),
			inputMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 7, 3, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Today; 3:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					&eventList{},
					nil,
				),
			},
			expectedMap: scheduleMap{
				odessa: buildTestSchedule(odessa, EasternTime,
					&eventList{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					&eventList{},
					buildTestEvent(time.Date(2021, time.November, 7, 3, 0, 0, 0, EasternTime), 60, "Frequency Response", 2000), // Active; 3:00 am EST
				),
			},
		},
	}

	for i, test := range tests {
		// Instead of calling actual time functions, use the test case provided time
		flextime.Now = func() time.Time {
			return test.currentTime
		}
		masterSchedule = test.inputMap
		reconfigureScadaInterface(schedCfg.Scada)
		checkMasterSchedule()
		if areEqual, reasonNotEqual := test.inputMap.equals(test.expectedMap); !areEqual {
			t.Errorf("Test with index %d failed for reason: %s.", i, reasonNotEqual)
		}
	}
}

func TestDeleteEventsWithMode(t *testing.T) {
	type testCase struct {
		inputSchedule  scheduleMap
		mode           string
		resultSchedule scheduleMap
	}

	tests := []testCase{
		{ // basic deletion of a single mode with multiple modes existing, single schedule
			inputSchedule: scheduleMap{
				"raleigh": buildTestSchedule("raleigh", forceLoadLocation("America/New_York"), &eventList{
					buildTestEventWithId(1, tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
					buildTestEventWithId(2, tVals.jan1_20_NY1700, 20, "Frequency Response", 2000),
					buildTestEventWithId(3, tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000),
					buildTestEventWithId(4, tVals.jan2_20_NY1700, 20, "Frequency Response", 2000),
				},
					&eventList{}, nil),
			},
			mode: "Frequency Response",
			resultSchedule: scheduleMap{
				"raleigh": buildTestSchedule("raleigh", forceLoadLocation("America/New_York"), &eventList{
					buildTestEventWithId(1, tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
					buildTestEventWithId(3, tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000),
				},
					&eventList{}, nil),
			},
		},
		{ // two schedules, one empty, one with only the mode to be deleted and an active event
			inputSchedule: scheduleMap{
				"raleigh": buildTestSchedule("raleigh", forceLoadLocation("America/New_York"), &eventList{}, &eventList{}, nil),
				"durham": buildTestSchedule("durham", forceLoadLocation("America/New_York"), &eventList{
					buildTestEventWithId(1, tVals.jan1_20_NY1700, 20, "Frequency Response", 2000),
					buildTestEventWithId(2, tVals.jan2_20_NY1200, 20, "Frequency Response", 2000),
				}, &eventList{}, buildTestEventWithId(3, tVals.jan1_20_NY1200, 20, "Frequency Response", 2000)),
			},
			mode: "Frequency Response",
			resultSchedule: scheduleMap{
				"raleigh": buildTestSchedule("raleigh", forceLoadLocation("America/New_York"), &eventList{}, &eventList{}, nil),
				"durham":  buildTestSchedule("durham", forceLoadLocation("America/New_York"), &eventList{}, &eventList{}, nil),
			},
		},
	}

	for i, test := range tests {
		test.inputSchedule.deleteEventsWithMode(test.mode)
		if areEqual, reasonNotEqual := test.inputSchedule.equals(test.resultSchedule); !areEqual {
			t.Errorf("Test with index %d failed for reason: %s.", i, reasonNotEqual)
		}
	}
}
