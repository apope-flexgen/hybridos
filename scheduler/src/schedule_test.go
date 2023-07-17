package main

import (
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/internal/flextime"
	"github.com/flexgen-power/scheduler/internal/idcache"
	"github.com/flexgen-power/scheduler/pkg/events"
)

func buildTestSchedule(id string, timezone *time.Location, scheduledEvents, expiredEvents *eventList, activeEvent *events.Event) *schedule {
	s := newSchedule(id, timezone)
	s.expiredEvents = expiredEvents
	s.scheduledEvents = scheduledEvents
	s.activeEvent = activeEvent
	return s
}

func TestGetEventWithId(t *testing.T) {
	type testCase struct {
		inputSchedule schedule
		inputId       idcache.Id
		expectedEvent *events.Event
	}

	testCases := []testCase{
		{ // no events in schedule, matching ID not found
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{},
				&eventList{},
				nil),
			inputId:       123,
			expectedEvent: nil,
		},
		{ // events in schedule, matching ID not found
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{buildTestEventWithId(321, tVals.jan1_20_NY1200, 60, "t", 1)},
				&eventList{},
				nil),
			inputId:       123,
			expectedEvent: nil,
		},
		{ // matching ID in scheduled events, expired/active empty
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)},
				&eventList{},
				nil),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
		{ // matching ID in expired events, scheduled/active empty
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{},
				&eventList{buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)},
				nil),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
		{ // matching ID in active event, scheduled/expired empty
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{},
				&eventList{},
				buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
		{ // matching ID in active event, scheduled/expired filled
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{buildTestEventWithId(321, tVals.jan1_20_NY1200, 60, "t", 1)},
				&eventList{buildTestEventWithId(1, tVals.jan1_20_NY1200, 60, "t", 1)},
				buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
		{ // matching ID in expired events, scheduled/active filled
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{buildTestEventWithId(321, tVals.jan1_20_NY1200, 60, "t", 1)},
				&eventList{buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)},
				buildTestEventWithId(1, tVals.jan1_20_NY1200, 60, "t", 1)),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
		{ // matching ID in scheduled events, expired/active filled
			inputSchedule: *buildTestSchedule("test_schedule", EasternTime,
				&eventList{buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1)},
				&eventList{buildTestEventWithId(1, tVals.jan1_20_NY1200, 60, "t", 1)},
				buildTestEventWithId(321, tVals.jan1_20_NY1200, 60, "t", 1)),
			inputId:       123,
			expectedEvent: buildTestEventWithId(123, tVals.jan1_20_NY1200, 60, "t", 1),
		},
	}

	for i, test := range testCases {
		result, _ := test.inputSchedule.getEventWithId(test.inputId)
		if areEqual, reasonNotEqual := result.Equals(test.expectedEvent); !areEqual {
			t.Errorf("Test with index %d failed. Result event %+v does not equal expected event %+v for reason: %s.", i, result, test.expectedEvent, reasonNotEqual)
		}
	}
}

func TestScheduleOrganizeEvent(t *testing.T) {
	type testCase struct {
		inputSchedule    schedule
		inputEvent       *events.Event
		currentTime      time.Time
		expectedSchedule schedule
	}

	tests := []testCase{
		{
			inputSchedule: schedule{
				timezone:        EasternTime,
				scheduledEvents: &eventList{},
				expiredEvents:   &eventList{},
				activeEvent:     nil,
				cache:           idcache.New(),
			},
			inputEvent: &events.Event{
				StartTime: time.Date(2023, 4, 1, 12, 30, 0, 0, EasternTime),
				Duration:  60,
				Mode:      "ionian",
				Repeat:    events.NewNonRepeatingSeries(),
			},
			currentTime: time.Date(2023, 4, 1, 14, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone:        EasternTime,
				scheduledEvents: &eventList{},
				expiredEvents: &eventList{
					&events.Event{
						StartTime: time.Date(2023, 4, 1, 12, 30, 0, 0, EasternTime),
						Duration:  60,
						Mode:      "ionian",
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
			},
		},
	}

	for i, test := range tests {
		flextime.Now = func() time.Time {
			return test.currentTime
		}
		test.inputSchedule.organizeEvent(test.inputEvent)
		if isEqual, reasonNotEqual := test.inputSchedule.equals(&test.expectedSchedule); !isEqual {
			t.Errorf("Test index %d has unequal schedules for reason: %s.", i, reasonNotEqual)
		}
	}
}

func TestSchedulePruneOldEventsAndExceptions(t *testing.T) {
	type testCase struct {
		inputSchedule      schedule
		currentTime        time.Time
		expectedSchedule   schedule
		expectedChangeFlag bool
	}

	// small isolated test to make sure function is ignoring schedules with nil time zone
	testSchedule := schedule{
		timezone: nil,
	}
	if testSchedule.pruneOldEventsAndExceptions() != false {
		t.Errorf("Nil time zone was not ignored.")
	}

	tests := []testCase{
		{ // basic deletion of yesterday event
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone:        EasternTime,
				expiredEvents:   &eventList{},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: true,
		},
		{ // deleting events when schedule is in different time zone than Scheduler's local time zone (Fleet Manager use case)
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 15, 23, 0, 0, 0, CentralTime), // midnight Eastern Time
			expectedSchedule: schedule{
				timezone:        EasternTime,
				expiredEvents:   &eventList{},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: true,
		},
		{ // Scheduler has been down since yesterday and gets restarted in middle of today
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 10, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 16, 13, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 10, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: true,
		},
		{ // pruning occurs on every check, so test that prune in middle of the day does not delete today events
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 16, 8, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 10, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 16, 13, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 16, 8, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 10, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: false,
		},
		{ // there is an event that started yesterday but crossed midnight into today so do not want to delete it. would be active event at midnight so just need to test middle-of-day prune
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 23, 0, 0, 0, EasternTime),
						Duration:  120,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 8, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 16, 10, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 23, 0, 0, 0, EasternTime),
						Duration:  120,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 8, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: false,
		},
		{ // a midnight-crossing event was not deleted after midnight, but now it is two days after so it should be deleted
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 23, 0, 0, 0, EasternTime),
						Duration:  120,
						Repeat:    events.NewNonRepeatingSeries(),
					},
					{
						Id:        2,
						StartTime: time.Date(2023, 4, 16, 8, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{},
			},
			currentTime: time.Date(2023, 4, 17, 0, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone:        EasternTime,
				expiredEvents:   &eventList{},
				scheduledEvents: &eventList{},
			},
			expectedChangeFlag: true,
		},
		{ // simple test to ensure old events and old exceptions can both be pruned in one pass. thorough testing of exception deletion done in separate unit test
			inputSchedule: schedule{
				timezone: EasternTime,
				expiredEvents: &eventList{
					{
						Id:        1,
						StartTime: time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    events.NewNonRepeatingSeries(),
					},
				},
				scheduledEvents: &eventList{
					{
						Id:        543,
						StartTime: time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat: &events.Series{
							Exceptions: []time.Time{time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime)},
						},
					},
				},
			},
			currentTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSchedule: schedule{
				timezone:      EasternTime,
				expiredEvents: &eventList{},
				scheduledEvents: &eventList{
					{
						Id:        543,
						StartTime: time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
						Duration:  60,
						Repeat:    &events.Series{},
					},
				},
			},
			expectedChangeFlag: true,
		},
	}

	for i, test := range tests {
		flextime.Now = func() time.Time { return test.currentTime }
		actualChangeFlag := test.inputSchedule.pruneOldEventsAndExceptions()
		if isEqual, reasonNotEqual := test.inputSchedule.equals(&test.expectedSchedule); !isEqual {
			t.Errorf("Test index %d schedule result did not match expected schedule for reason: %s.", i, reasonNotEqual)
		} else if test.expectedChangeFlag != actualChangeFlag {
			t.Errorf("Test index %d schedule-changed flag result is %v but expected %v.", i, actualChangeFlag, test.expectedChangeFlag)
		}
	}
}
