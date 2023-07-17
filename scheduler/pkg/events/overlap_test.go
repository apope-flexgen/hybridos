package events

import (
	"testing"
	"time"
)

// forceLoadLocation allows the unit tests to fill in their test parameters without error checking.
//
// Should only be used in testing! Production code needs to handle all errors.
func forceLoadLocation(name string) *time.Location {
	loc, _ := time.LoadLocation(name)
	return loc
}

var EasternTime *time.Location = forceLoadLocation("America/New_York")
var CentralTime *time.Location = forceLoadLocation("America/Chicago")

func TestOverlaps(t *testing.T) {
	type overlapsTestCase struct {
		e1             *Event
		e2             *Event
		expectedResult bool
	}

	testCases := []overlapsTestCase{
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// nil pointer handling
			e1:             nil,
			e2:             nil,
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// event x event - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// event x event - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// event x fin daily - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 22, 14, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 5,
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// fin weekly x event - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 15, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x41, // Sunday, Saturday
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 16, 14, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// fin weekly x fin weekly - no overlap. includes Spring Forward event.
			e1: &Event{
				StartTime: time.Date(2022, 3, 12, 2, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x41, // Sunday, Saturday
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 11, 2, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x22, // Monday, Friday
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// fin weekly x fin weekly - overlap
			e1: &Event{
				StartTime: time.Date(2022, 4, 10, 12, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x49, // Sunday, Wednesday, Saturday
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 4, 12, 22, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  1380,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x22, // Monday, Tuesday, Friday
					EndTime:   time.Date(2022, 5, 27, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// event x inf daily - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 22, 14, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 5,
					EndCount:  -1,
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// event x inf daily - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 22, 14, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 5,
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x event - no overlap. Includes Spring Forward DST corner case
			e1: &Event{
				StartTime: time.Date(2022, 3, 3, 2, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 5,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 12, 14, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat:    NewNonRepeatingSeries(),
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf weekly x fin weekly - overlap
			e1: &Event{
				StartTime: time.Date(2022, 2, 23, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 6, 15, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x6A, // Sunday, Monday, Wednesday, Friday
					EndCount:  5,
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// fin weekly x inf weekly with aligned frequencies - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 3, 6, 2, 30, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x6A, // Sunday, Monday, Wednesday, Friday
					EndCount:  5,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 23, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x inf daily with aligned frequencies - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x inf daily with aligned frequencies - no overlap. includes Spring Forward DST corner case
			e1: &Event{
				StartTime: time.Date(2022, 3, 12, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 4,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 11, 2, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x inf daily with aligned frequencies - no overlap. includes Spring Forward DST corner case
			e1: &Event{
				StartTime: time.Date(2022, 3, 12, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 4,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 11, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x inf daily with misaligned frequencies - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 3,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf weekly x inf daily - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf weekly x inf daily - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 13, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 17, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf daily x inf weekly - no overlap. includes Spring Forward DST corner case
			e1: &Event{
				StartTime: time.Date(2022, 3, 13, 10, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 1,
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 3, 6, 13, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x6A, // Sunday, Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf weekly x inf weekly with aligned frequencies - no overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 16, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 23, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			expectedResult: false,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// inf weekly x inf weekly - overlap
			e1: &Event{
				StartTime: time.Date(2022, 5, 9, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			e2: &Event{
				StartTime: time.Date(2022, 5, 23, 15, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  60,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 2,
					DayMask:   0x2A, // Monday, Wednesday, Friday
					EndCount:  -1,
				},
			},
			expectedResult: true,
		},
		{ //////////////////////////////////////////////////////////////////////////////////////////
			// infinite series with exception x finite series that only overlaps exception
			e1: &Event{
				StartTime: time.Date(2023, 3, 6, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  90,
				Repeat: &Series{
					Cycle:     WeeklyRepeat,
					Frequency: 1,
					DayMask:   0x3C, // Monday-Thursday
					EndCount:  -1,
					Exceptions: []time.Time{
						time.Date(2023, 3, 6, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
						time.Date(2023, 3, 7, 17, 0, 0, 0, forceLoadLocation("America/New_York")),
					},
				},
			},
			e2: &Event{
				StartTime: time.Date(2023, 3, 2, 23, 0, 0, 0, forceLoadLocation("America/New_York")),
				Duration:  1140,
				Repeat: &Series{
					Cycle:     DailyRepeat,
					Frequency: 2,
					EndTime:   time.Date(2023, 3, 6, 23, 0, 0, 0, forceLoadLocation("America/New_York")),
				},
			},
			expectedResult: false,
		},
	}
	for i, testCase := range testCases {
		if testCase.e1.Overlaps(testCase.e2) != testCase.expectedResult || testCase.e2.Overlaps(testCase.e1) != testCase.expectedResult {
			t.Errorf("Test case %d failed. Expected %t but got %t", i, testCase.expectedResult, !testCase.expectedResult)
		}
	}
}
