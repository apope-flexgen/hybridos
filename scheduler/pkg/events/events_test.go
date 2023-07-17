package events

import (
	"testing"
	"time"
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

func TestEventHasContinuousTransition(t *testing.T) {
	type testCase struct {
		firstEvent     Event
		followingEvent Event
		expectedResult bool
	}

	tests := []testCase{
		{ // continuous times, continuous modes
			firstEvent: Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  1,
				Mode:      "mixolydian",
			},
			followingEvent: Event{
				StartTime: tVals.jan1_20_NY1200.Add(time.Minute),
				Duration:  1,
				Mode:      "mixolydian",
			},
			expectedResult: true,
		},
		{ // continuous times, discontinuous modes
			firstEvent: Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  60,
				Mode:      "mixolydian",
			},
			followingEvent: Event{
				StartTime: tVals.jan1_20_NY1200.Add(time.Hour),
				Duration:  30,
				Mode:      "ionian",
			},
			expectedResult: false,
		},
		{ // discontinuous times, continuous modes
			firstEvent: Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  1,
				Mode:      "mixolydian",
			},
			followingEvent: Event{
				StartTime: tVals.jan1_20_NY1200.Add(time.Hour),
				Duration:  1,
				Mode:      "mixolydian",
			},
			expectedResult: false,
		},
		{ // discontinuous times, discontinuous modes
			firstEvent: Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  30,
				Mode:      "mixolydian",
			},
			followingEvent: Event{
				StartTime: tVals.jan1_20_NY1200.Add(time.Hour),
				Duration:  1,
				Mode:      "ionian",
			},
			expectedResult: false,
		},
	}

	for i, test := range tests {
		actualResult := test.firstEvent.HasContinuousTransition(&test.followingEvent)
		if actualResult != test.expectedResult {
			t.Errorf("Test index %d expected %v but got %v.", i, test.expectedResult, actualResult)
		}
	}
}

func TestOnlyDurationIsDifferent(t *testing.T) {
	type testCase struct {
		e1             *Event
		e2             *Event
		expectedResult bool
	}

	tests := []testCase{
		{ // everything the same
			e1: &Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  5,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  5,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: true,
		},
		{ // only durations different
			e1: &Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  5,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  25,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: true,
		},
		{ // durations different; start times different
			e1: &Event{
				StartTime: tVals.jan1_20_NY1700,
				Duration:  5,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: tVals.jan1_20_NY1200,
				Duration:  25,
				Mode:      "mixolydian",
				Variables: map[string]interface{}{"var1": "val1"},
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: false,
		},
	}

	for i, test := range tests {
		e1Duration := test.e1.Duration
		actualResult, reasonNotEqual := test.e1.onlyDurationCanBeDifferent(test.e2)
		if actualResult == test.expectedResult {
			continue
		}
		// this check is included just because if the function is messed with, there could easily be a bug that introduces a corruption to e1
		if e1Duration != test.e1.Duration {
			t.Errorf("Test index %d changed e1's duration from %d to %d.", i, e1Duration, test.e1.Duration)
		}
		if test.expectedResult {
			t.Errorf("Test index %d expected e1 (%+v with repeat settings %+v) to be equal to e2 (%+v with repeat settings %+v) for all fields except duration, but inequality was found: %s.", i, *test.e1, *test.e1.Repeat, *test.e2, *test.e2.Repeat, reasonNotEqual)
		} else {
			t.Errorf("Test index %d expected to find inequality between e1 (%+v with repeat settings %+v) and e2 (%+v with repeat settings %+v) in a non-duration field, but none was found.", i, *test.e1, *test.e1.Repeat, *test.e2, *test.e2.Repeat)
		}
	}
}
