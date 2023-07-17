package main

import (
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/internal/setpoint"
	"github.com/flexgen-power/scheduler/pkg/events"
)

type testValuesStruct struct {
	jan1_20_NY1200 time.Time
	jan1_20_NY1700 time.Time
	jan2_20_NY1200 time.Time
	jan2_20_NY1700 time.Time
}

var tVals testValuesStruct = testValuesStruct{
	time.Date(2020, 1, 1, 12, 0, 0, 0, forceLoadLocation("America/New_York")),
	time.Date(2020, 1, 1, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
	time.Date(2020, 1, 2, 12, 0, 0, 0, forceLoadLocation("America/New_York")),
	time.Date(2020, 1, 2, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
}

// buildTestEvent helps streamline event creation
func buildTestEvent(startTime time.Time, durationMins uint, mode string, value int) *events.Event {
	return &events.Event{
		Id:        0,
		StartTime: startTime,
		Duration:  durationMins,
		Mode:      mode,
		Variables: map[string]interface{}{"kW_cmd": value},
		Repeat:    events.NewNonRepeatingSeries(),
	}
}

// buildTestEventWithId helps streamline event creation
func buildTestEventWithId(id idcache.Id, startTime time.Time, durationMins uint, mode string, value int) *events.Event {
	e := buildTestEvent(startTime, durationMins, mode, value)
	e.Id = id
	return e
}

func TestEventListLess(t *testing.T) {
	type testCase struct {
		inputEventList eventList
		expectedResult bool
	}

	tests := []testCase{
		{ // test first event starts first
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 2, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
				},
			},
			expectedResult: true,
		},
		{ // test second event starts first
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 5, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 2, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
				},
			},
			expectedResult: false,
		},
		{ // test first event has exception
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
					Repeat:    &events.Series{},
				},
			},
			expectedResult: false,
		},
		{ // test second event has exception
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 2, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat:    &events.Series{},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 2, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 2, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: true,
		},
		{ // test first event is shorter
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  30,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: true,
		},
		{ // test second event is shorter
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  30,
					Mode:      "aeolian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: false,
		},
		{ // test first event's mode is first alphabetically
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: true,
		},
		{ // test second event's mode is first alphabetically
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "aeolian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: false,
		},
		{ // test first event's ID is lower
			inputEventList: eventList{
				&events.Event{
					Id:        1,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: true,
		},
		{ // test second event's ID is lower
			inputEventList: eventList{
				&events.Event{
					Id:        3,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
				&events.Event{
					Id:        2,
					StartTime: time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime),
					Duration:  60,
					Mode:      "mixolydian",
					Repeat: &events.Series{
						Exceptions: []time.Time{time.Date(2023, 4, 1, 0, 0, 0, 0, EasternTime)},
					},
				},
			},
			expectedResult: false,
		},
	}

	for i, test := range tests {
		actualResult := test.inputEventList.Less(0, 1)
		if actualResult != test.expectedResult {
			t.Errorf("Test index %d returned %v but expected %v.", i, actualResult, test.expectedResult)
		}
	}
}

func TestEventListGet(t *testing.T) {
	type testCase struct {
		inputList     *eventList
		targetEvent   time.Time
		expectedEvent *events.Event
	}

	tests := []testCase{
		{ // Get top-of-list event
			&eventList{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1200,
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
		},
		{ // Get event that is not at the top of the list
			&eventList{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1700,
			buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000),
		},
		{ // Try to get event that is not in list
			&eventList{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1200,
			nil,
		},
		{ // Try to get event from empty list
			&eventList{},
			tVals.jan1_20_NY1200,
			nil,
		},
	}

	for i, test := range tests {
		resultEvent, _ := test.inputList.get(test.targetEvent)
		if areEqual, reasonNotEqual := resultEvent.Equals(test.expectedEvent); !areEqual {
			t.Errorf("Test with index %d failed. Result event %+v does not equal expected event %+v for reason: %s.", i, resultEvent, test.expectedEvent, reasonNotEqual)
		}
	}
}

func TestEventListSet(t *testing.T) {
	type testCase struct {
		inputList     *eventList
		setEvent      *events.Event
		expectedEvent *events.Event
	}

	tests := []testCase{
		{ // Set top-of-list event
			&eventList{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
			buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
		},
		{ // Set event that is not at the top of the list
			&eventList{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1700, 10, "Frequency Response", 1000),
			buildTestEvent(tVals.jan1_20_NY1700, 10, "Frequency Response", 1000),
		},
		{ // Try to set event that is not in list
			&eventList{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
			nil,
		},
		{ // Try to set event in empty list
			&eventList{},
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
			nil,
		},
	}

	for i, test := range tests {
		test.inputList.Set(test.setEvent)
		resultEvent, _ := test.inputList.get(test.setEvent.StartTime)
		if areEqual, reasonNotEqual := resultEvent.Equals(test.expectedEvent); !areEqual {
			t.Errorf("Test with index %d failed. Result event %+v does not equal expected event %+v for reason: %s.", i, resultEvent, test.expectedEvent, reasonNotEqual)
		}
	}
}

func TestEventListRemove(t *testing.T) {
	type testCase struct {
		inputList     eventList
		indexToRemove int
		expectedList  eventList
	}

	tests := []testCase{
		{ // basic removal of only event
			inputList: eventList{
				&events.Event{},
			},
			indexToRemove: 0,
			expectedList:  eventList{},
		},
		{ // negative indices ignored
			inputList: eventList{
				&events.Event{},
			},
			indexToRemove: -1,
			expectedList: eventList{
				&events.Event{},
			},
		},
		{ // out-of-bound indices ignored
			inputList: eventList{
				&events.Event{},
			},
			indexToRemove: 1,
			expectedList: eventList{
				&events.Event{},
			},
		},
	}

	for i, test := range tests {
		test.inputList.remove(test.indexToRemove)
		if areEqual, reasonNotEqual := test.inputList.equals(test.expectedList); !areEqual {
			t.Errorf("Test index %d's output did not match expected list for reason: %s.", i, reasonNotEqual)
		}
	}
}

func TestEventListCopy(t *testing.T) {
	type testCase struct {
		inputList eventList
	}

	tests := []testCase{
		{ // empty list
			eventList{},
		},
		{ // 1 event
			eventList{
				buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000),
			},
		},
		{ // 2 events
			eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000),
			},
		},
		{ // 3 events
			eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000), buildTestEvent(tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000),
			},
		},
	}

	for i, test := range tests {
		copyList := test.inputList.copy()
		if areEqual, reasonNotEqual := copyList.equals(test.inputList); !areEqual {
			t.Errorf("Test index %d failed. Copy list not equal to source list for reason %s.", i, reasonNotEqual)
		}
	}
}

func TestEventListEquals(t *testing.T) {
	type testCase struct {
		list1          eventList
		list2          eventList
		expectedResult bool
	}

	tests := []testCase{
		{ // empty list == empty list
			list1:          eventList{},
			list2:          eventList{},
			expectedResult: true,
		},
		// eventList should never be allowed to be nil, but cover it just in case
		{ // nil == nil
			list1:          nil,
			list2:          nil,
			expectedResult: true,
		},
		{ // nil != empty list
			list1:          nil,
			list2:          eventList{},
			expectedResult: false,
		},
		{ // empty list != nil
			list1:          eventList{},
			list2:          nil,
			expectedResult: false,
		},
		{ // len(0) != len(1)
			list1: eventList{},
			list2: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			expectedResult: false,
		},
		{ // len(1) != len(0)
			list1: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			list2:          eventList{},
			expectedResult: false,
		},
		{ // len(1) != len(2)
			list1: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			list2: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
				buildTestEvent(tVals.jan1_20_NY1700, 60, "dorian", 10),
			},
			expectedResult: false,
		},
		{ // len(2) != len(1)
			list1: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
				buildTestEvent(tVals.jan1_20_NY1700, 60, "dorian", 10),
			},
			list2: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			expectedResult: false,
		},
		{ // event diff
			list1: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			list2: eventList{
				buildTestEvent(tVals.jan1_20_NY1700, 60, "dorian", 10),
			},
			expectedResult: false,
		},
		{ // same events
			list1: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			list2: eventList{
				buildTestEvent(tVals.jan1_20_NY1200, 60, "dorian", 10),
			},
			expectedResult: true,
		},
	}

	for i, test := range tests {
		areEqual, reasonNotEqual := test.list1.equals(test.list2)
		if areEqual != test.expectedResult {
			if test.expectedResult {
				t.Errorf("Test index %d failed. Expected lists to be equal but they were not equal for reason: %s.", i, reasonNotEqual)
			} else {
				t.Errorf("Test index %d failed. Expected lists to not be equal but they were equal.", i)
			}
		}
	}
}

// There is a separate unit test to handle thorough testing of the Event::Overlaps method.
// This unit test strictly checks that eventList::hasOverlaps is using the returned value
// from Event::Overlaps correctly.
func TestEventListHasOverlaps(t *testing.T) {
	type testCase struct {
		list           eventList
		expectedResult bool
	}

	tests := []testCase{
		{ // no overlap
			list: eventList{
				&events.Event{
					StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
				&events.Event{
					StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			expectedResult: false,
		},
		{ // overlap
			list: eventList{
				&events.Event{
					StartTime: time.Date(2022, 5, 16, 14, 30, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
				&events.Event{
					StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			expectedResult: true,
		},
	}

	for i, test := range tests {
		actualResult, _, _ := test.list.hasOverlaps()
		if actualResult != test.expectedResult {
			t.Errorf("Test index %d failed. Expected %v but got %v.", i, test.expectedResult, actualResult)
		}
	}
}

func TestEventListDeleteVarOfMode(t *testing.T) {
	type testCase struct {
		list           eventList
		varId          string
		modeId         string
		expectedResult eventList
	}

	tests := []testCase{
		{ // standard deletion of var from mode
			list: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
			},
			varId:  "power",
			modeId: "lydian",
			expectedResult: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
			},
		},
		{ // mode mismatch means variable not deleted
			list: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
			},
			varId:  "power",
			modeId: "phrygian",
			expectedResult: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
			},
		},
		{ // if varId not in variables map, no changes
			list: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
			},
			varId:  "power",
			modeId: "lydian",
			expectedResult: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
			},
		},
		{ // two events - delete happens in both
			list: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
			},
			varId:  "power",
			modeId: "lydian",
			expectedResult: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
			},
		},
		{ // two events where both have the varId but only one has the modeId
			list: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
				{
					Mode: "phrygian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
			},
			varId:  "power",
			modeId: "phrygian",
			expectedResult: eventList{
				{
					Mode: "lydian",
					Variables: map[string]interface{}{
						"power":   10,
						"voltage": 20,
					},
				},
				{
					Mode: "phrygian",
					Variables: map[string]interface{}{
						"voltage": 20,
					},
				},
			},
		},
	}

	for i, test := range tests {
		test.list.deleteVarOfMode(test.varId, test.modeId)
		if areEqual, reasonNotEqual := test.list.equals(test.expectedResult); !areEqual {
			t.Errorf("Test index %d expected list did not match actual list for reason %s.", i, reasonNotEqual)
		}
	}
}

func TestEventListHandleSpringForwardEvent(t *testing.T) {
	type testCase struct {
		list                    eventList
		inputSpringForwardEvent events.Event // should always have non-repeating series setting
		expectedList            eventList
		expectedFlag            bool
	}

	tests := []testCase{
		{ // trim shifted Spring Forward event that starts before next event to fit into schedule
			list: eventList{
				&events.Event{
					StartTime: time.Date(2023, 3, 12, 2, 30, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			inputSpringForwardEvent: events.Event{
				StartTime: time.Date(2023, 3, 12, 2, 15, 0, 0, EasternTime),
				Duration:  30,
				Repeat:    events.NewNonRepeatingSeries(),
			},
			expectedList: eventList{
				&events.Event{
					StartTime: time.Date(2023, 3, 12, 2, 15, 0, 0, EasternTime),
					Duration:  15,
					Repeat:    events.NewNonRepeatingSeries(),
				},
				&events.Event{
					StartTime: time.Date(2023, 3, 12, 2, 30, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			expectedFlag: true,
		},
		{ // discard shifted Spring Forward event that starts in the middle of the event that was already in the 2am-3am hour
			list: eventList{
				&events.Event{
					StartTime: time.Date(2023, 3, 12, 2, 30, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			inputSpringForwardEvent: events.Event{
				StartTime: time.Date(2023, 3, 12, 2, 45, 0, 0, EasternTime),
				Duration:  30,
				Repeat:    events.NewNonRepeatingSeries(),
			},
			expectedList: eventList{
				&events.Event{
					StartTime: time.Date(2023, 3, 12, 2, 30, 0, 0, EasternTime),
					Duration:  60,
					Repeat:    events.NewNonRepeatingSeries(),
				},
			},
			expectedFlag: false,
		},
	}

	for i, test := range tests {
		resultFlag := test.list.handleSpringForwardEvent(&test.inputSpringForwardEvent)
		if resultFlag != test.expectedFlag {
			t.Errorf("Test index %d expected %v flag but got %v.", i, test.expectedFlag, resultFlag)
			continue
		}

		if isEqual, reasonNotEqual := test.list.equals(test.expectedList); !isEqual {
			t.Errorf("Test index %d output list did not equal expected list for reason: %s.", i, reasonNotEqual)
		}
	}
}

func TestEventListGetStartTimes(t *testing.T) {
	type testCase struct {
		list          eventList
		expectedTimes []time.Time
	}

	timeSlicesAreEqual := func(s1, s2 []time.Time) bool {
		if (s1 == nil && s2 != nil) || (s1 != nil && s2 == nil) {
			return false
		}
		if len(s1) != len(s2) {
			return false
		}
		for i, t1 := range s1 {
			if !t1.Equal(s2[i]) {
				return false
			}
		}
		return true
	}

	tests := []testCase{
		{
			list:          eventList{},
			expectedTimes: []time.Time{},
		},
		{
			list: eventList{
				&events.Event{StartTime: time.Date(2023, 3, 20, 12, 0, 0, 0, EasternTime)},
			},
			expectedTimes: []time.Time{
				time.Date(2023, 3, 20, 12, 0, 0, 0, EasternTime),
			},
		},
		{
			list: eventList{
				&events.Event{StartTime: time.Date(2023, 3, 20, 12, 0, 0, 0, EasternTime)},
				&events.Event{StartTime: time.Date(2023, 3, 21, 12, 0, 0, 0, EasternTime)},
			},
			expectedTimes: []time.Time{
				time.Date(2023, 3, 20, 12, 0, 0, 0, EasternTime),
				time.Date(2023, 3, 21, 12, 0, 0, 0, EasternTime),
			},
		},
	}

	for i, test := range tests {
		if !timeSlicesAreEqual(test.list.getStartTimes(), test.expectedTimes) {
			t.Errorf("Test index %d failed.", i)
		}
	}
}

func TestEventListValidate(t *testing.T) {
	type testCase struct {
		inputList    eventList
		expectErr    bool
		expectedList eventList
	}

	tests := []testCase{
		{ // empty list is OK
			inputList:    eventList{},
			expectedList: eventList{},
		},
		{ // nil list is invalid
			inputList: nil,
			expectErr: true,
		},
		{ // list with invalid event inside it gets flagged as invalid
			inputList: eventList{&events.Event{}},
			expectErr: true,
		},
		{ // list with valid events inside it gets sorted
			inputList: eventList{
				buildTestEventWithId(1, tVals.jan1_20_NY1700, 60, "ionian", 10),
				buildTestEventWithId(2, tVals.jan1_20_NY1200, 60, "ionian", 10),
			},
			expectedList: eventList{
				buildTestEventWithId(2, tVals.jan1_20_NY1200, 60, "ionian", 10),
				buildTestEventWithId(1, tVals.jan1_20_NY1700, 60, "ionian", 10),
			},
		},
		{ // list with overlapping events gets flagged as invalid
			inputList: eventList{
				buildTestEventWithId(1, tVals.jan1_20_NY1700, 60, "ionian", 10),
				buildTestEventWithId(2, tVals.jan1_20_NY1200, 360, "ionian", 10),
			},
			expectErr: true,
		},
	}

	// cannot fit nil ptr test case into parameterized tests so test it independently
	var nilPtr *eventList
	if err := nilPtr.validate(EasternTime, map[string]setpoint.List{}); err == nil {
		t.Errorf("Nil pointer did not return error.")
	}

	// mode variables need to match in order to events to be considered valid.
	modeVars := map[string]setpoint.List{
		"ionian": {{Id: "kW_cmd", VarType: "Int"}},
	}

	for i, test := range tests {
		// event::Validate unit test should cover corner cases involving time zone so do not need to make this varying here
		err := test.inputList.validate(EasternTime, modeVars)
		if err != nil {
			if !test.expectErr {
				t.Errorf("Test index %d did not expect error but got %v.", i, err)
			}
			continue
		}

		if test.expectErr {
			t.Errorf("Test index %d expected error but did not get one.", i)
			continue
		}

		if isEqual, reasonNotEqual := test.inputList.equals(test.expectedList); !isEqual {
			t.Errorf("Test index %d failed due to unequal lists for reason: %s.", i, reasonNotEqual)
		}
	}
}

func TestEventListBuildIdCache(t *testing.T) {
	type testCase struct {
		eList         eventList
		expectedCache idcache.IdCache
	}

	tests := []testCase{
		{
			eList:         eventList{},
			expectedCache: idcache.IdCache{},
		},
		{
			eList: eventList{
				{Id: 123},
			},
			expectedCache: idcache.IdCache{
				123: struct{}{},
			},
		},
		{
			eList: eventList{
				{Id: 123},
				{Id: 456},
			},
			expectedCache: idcache.IdCache{
				123: struct{}{},
				456: struct{}{},
			},
		},
	}

	for i, test := range tests {
		actualResult := test.eList.buildIdCache()
		if isEqual, reasonNotEqual := test.expectedCache.Equals(actualResult); !isEqual {
			t.Errorf("Test case %d's generated cache did not equal expected cache for reason: %s.", i, reasonNotEqual)
		}
	}
}
