package flextime

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

var CentralTime *time.Location = forceLoadLocation("America/Chicago")
var EasternTime *time.Location = forceLoadLocation("America/New_York")

type getMidnightInNDaysTest struct {
	inputNow         time.Time
	numDays          int
	expectedMidnight time.Time
}
type getMidnightInNDaysTestCases []getMidnightInNDaysTest

func TestGetMidnightInNDays(t *testing.T) {
	tests := getMidnightInNDaysTestCases{
		// non-DST tests, reference time in morning
		{
			inputNow:         time.Date(2023, 2, 23, 9, 30, 00, 00, EasternTime),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 2, 21, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 9, 30, 00, 00, EasternTime),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 2, 22, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 9, 30, 00, 00, EasternTime),
			numDays:          0,
			expectedMidnight: time.Date(2023, 2, 23, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 9, 30, 00, 00, EasternTime),
			numDays:          1,
			expectedMidnight: time.Date(2023, 2, 24, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 9, 30, 00, 00, EasternTime),
			numDays:          2,
			expectedMidnight: time.Date(2023, 2, 25, 0, 0, 0, 0, EasternTime),
		},
		// non-DST tests, reference time at midnight
		{
			inputNow:         time.Date(2023, 2, 23, 0, 0, 00, 00, EasternTime),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 2, 21, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 0, 0, 00, 00, EasternTime),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 2, 22, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 0, 0, 00, 00, EasternTime),
			numDays:          0,
			expectedMidnight: time.Date(2023, 2, 23, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 0, 0, 00, 00, EasternTime),
			numDays:          1,
			expectedMidnight: time.Date(2023, 2, 24, 0, 0, 0, 0, EasternTime),
		},
		{
			inputNow:         time.Date(2023, 2, 23, 0, 0, 00, 00, EasternTime),
			numDays:          2,
			expectedMidnight: time.Date(2023, 2, 25, 0, 0, 0, 0, EasternTime),
		},
		//
		// Spring Forward DST tests - normal time zone
		//
		{ // day after, going back 2 days
			inputNow:         time.Date(2023, 3, 13, 9, 30, 00, 00, EasternTime),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 3, 11, 0, 0, 0, 0, EasternTime),
		},
		{ // day after, going back 1 day
			inputNow:         time.Date(2023, 3, 13, 9, 30, 00, 00, EasternTime),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 3, 12, 0, 0, 0, 0, EasternTime),
		},
		{ // day of, getting midnight of same day
			inputNow:         time.Date(2023, 3, 12, 9, 30, 00, 00, EasternTime),
			numDays:          0,
			expectedMidnight: time.Date(2023, 3, 12, 0, 0, 0, 0, EasternTime),
		},
		{ // day before, going forward 1 day
			inputNow:         time.Date(2023, 3, 11, 9, 30, 00, 00, EasternTime),
			numDays:          1,
			expectedMidnight: time.Date(2023, 3, 12, 0, 0, 0, 0, EasternTime),
		},
		{ // day before, going forward 2 days
			inputNow:         time.Date(2023, 3, 11, 9, 30, 00, 00, EasternTime),
			numDays:          2,
			expectedMidnight: time.Date(2023, 3, 13, 0, 0, 0, 0, EasternTime),
		},
		//
		// Fall Back DST tests - normal time zone
		//
		{ // day after, going back 2 days
			inputNow:         time.Date(2023, 11, 6, 9, 30, 00, 00, EasternTime),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 11, 4, 0, 0, 0, 0, EasternTime),
		},
		{ // day after, going back 1 day
			inputNow:         time.Date(2023, 11, 6, 9, 30, 00, 00, EasternTime),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, EasternTime),
		},
		{ // morning, day of, getting midnight of same day
			inputNow:         time.Date(2023, 11, 5, 9, 30, 00, 00, EasternTime),
			numDays:          0,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, EasternTime),
		},
		{ // inside extra hour, day of, getting midnight of same day
			inputNow:         time.Date(2023, 11, 5, 1, 30, 00, 00, EasternTime),
			numDays:          0,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, EasternTime),
		},
		{ // day before, going forward 1 day
			inputNow:         time.Date(2023, 11, 4, 9, 30, 00, 00, EasternTime),
			numDays:          1,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, EasternTime),
		},
		{ // day before, going forward 2 days
			inputNow:         time.Date(2023, 11, 4, 9, 30, 00, 00, EasternTime),
			numDays:          2,
			expectedMidnight: time.Date(2023, 11, 6, 0, 0, 0, 0, EasternTime),
		},
		//
		// Spring Forward DST tests - Cuba
		//
		{ // day after, going back 2 days
			inputNow:         time.Date(2023, 3, 13, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 3, 11, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day after, going back 1 day
			inputNow:         time.Date(2023, 3, 13, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 3, 12, 1, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day of, getting midnight of same day
			inputNow:         time.Date(2023, 3, 12, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          0,
			expectedMidnight: time.Date(2023, 3, 12, 1, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day before, going forward 1 day
			inputNow:         time.Date(2023, 3, 11, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          1,
			expectedMidnight: time.Date(2023, 3, 12, 1, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day before, going forward 2 days
			inputNow:         time.Date(2023, 3, 11, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          2,
			expectedMidnight: time.Date(2023, 3, 13, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		//
		// Fall Back DST tests - Cuba
		//
		{ // day after, going back 2 days
			inputNow:         time.Date(2023, 11, 6, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          -2,
			expectedMidnight: time.Date(2023, 11, 4, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day after, going back 1 day
			inputNow:         time.Date(2023, 11, 6, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          -1,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day of, getting midnight of same day
			inputNow:         time.Date(2023, 11, 5, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          0,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day before, going forward 1 day
			inputNow:         time.Date(2023, 11, 4, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          1,
			expectedMidnight: time.Date(2023, 11, 5, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
		{ // day before, going forward 2 days
			inputNow:         time.Date(2023, 11, 4, 9, 30, 00, 00, forceLoadLocation("Cuba")),
			numDays:          2,
			expectedMidnight: time.Date(2023, 11, 6, 0, 0, 0, 0, forceLoadLocation("Cuba")),
		},
	}

	for i, test := range tests {
		if result := GetMidnightInNDays(test.numDays, test.inputNow); !result.Equal(test.expectedMidnight) {
			t.Errorf("test index %d: %v does not equal %v", i, result, test.expectedMidnight)
		}
	}
}

func TestTimeIsBeforeCurrentMinute(t *testing.T) {
	type testCase struct {
		inputTime      time.Time
		currentTime    time.Time
		expectedResult bool
	}

	tests := []testCase{
		{ // equal times
			inputTime:      time.Date(2023, 4, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input year before current year
			inputTime:      time.Date(2022, 4, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: true,
		},
		{ // input year after current year
			inputTime:      time.Date(2024, 4, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input month before current month
			inputTime:      time.Date(2023, 3, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: true,
		},
		{ // input month after current month
			inputTime:      time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input day before current day
			inputTime:      time.Date(2023, 4, 9, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: true,
		},
		{ // input day after current day
			inputTime:      time.Date(2023, 4, 11, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input hour before current hour
			inputTime:      time.Date(2023, 4, 10, 11, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: true,
		},
		{ // input hour after current hour
			inputTime:      time.Date(2023, 4, 10, 13, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input minute before current minute
			inputTime:      time.Date(2023, 4, 10, 12, 0, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 1, 20, 10, EasternTime),
			expectedResult: true,
		},
		{ // input minute after current minute
			inputTime:      time.Date(2023, 4, 10, 12, 1, 0, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input second before current second
			inputTime:      time.Date(2023, 4, 10, 12, 0, 19, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input second after current second
			inputTime:      time.Date(2023, 4, 10, 12, 0, 21, 0, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input nanosecond before current nanosecond
			inputTime:      time.Date(2023, 4, 10, 12, 0, 20, 9, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // input nanosecond after current nanosecond
			inputTime:      time.Date(2023, 4, 10, 12, 0, 20, 11, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, EasternTime),
			expectedResult: false,
		},
		{ // time zones are different - input is before current time
			inputTime:      time.Date(2023, 4, 10, 12, 30, 20, 10, EasternTime),
			currentTime:    time.Date(2023, 4, 10, 12, 0, 20, 10, CentralTime),
			expectedResult: true,
		},
		{ // time zones are different - input is after current time
			inputTime:      time.Date(2023, 4, 10, 12, 0, 20, 10, CentralTime),
			currentTime:    time.Date(2023, 4, 10, 12, 30, 20, 10, EasternTime),
			expectedResult: false,
		},
	}

	for i, test := range tests {
		Now = func() time.Time { return test.currentTime }
		actualResult := TimeIsBeforeCurrentMinute(test.inputTime)
		if actualResult != test.expectedResult {
			t.Errorf("Test index %d expected %v but got %v.", i, test.expectedResult, actualResult)
		}
	}
}

func TestApplyNewDateToTime(t *testing.T) {
	type testCase struct {
		newDate        time.Time
		originalTime   time.Time
		expectedResult time.Time
	}

	tests := []testCase{
		{ // newDate exactly equals originalTime
			newDate:        time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedResult: time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
		},
		{ // newDate is in the future with the same clock time
			newDate:        time.Date(2023, 5, 11, 12, 0, 0, 0, CentralTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, CentralTime),
			expectedResult: time.Date(2023, 5, 11, 12, 0, 0, 0, CentralTime),
		},
		{ // newDate is in the past with the same clock time
			newDate:        time.Date(2023, 5, 9, 12, 0, 0, 0, EasternTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedResult: time.Date(2023, 5, 9, 12, 0, 0, 0, EasternTime),
		},
		{ // newDate is in the future with a different clock time
			newDate:        time.Date(2023, 5, 11, 19, 30, 20, 10, CentralTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, CentralTime),
			expectedResult: time.Date(2023, 5, 11, 12, 0, 0, 0, CentralTime),
		},
		{ // newDate is in the past with a different clock time
			newDate:        time.Date(2023, 5, 9, 5, 4, 3, 2, EasternTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedResult: time.Date(2023, 5, 9, 12, 0, 0, 0, EasternTime),
		},
		{ // newDate is on Spring Forward DST day and originalTime is at 2AM so result should be hour-shifted to 3AM
			newDate:        time.Date(2024, 3, 10, 5, 0, 0, 0, CentralTime),
			originalTime:   time.Date(2024, 3, 9, 2, 0, 0, 0, CentralTime),
			expectedResult: time.Date(2024, 3, 10, 3, 0, 0, 0, CentralTime),
		},
		{ // newDate is on Spring Forward DST day and original time is at 2:30AM so result should be hour-shifted to 3:30AM
			newDate:        time.Date(2024, 3, 10, 5, 0, 0, 0, EasternTime),
			originalTime:   time.Date(2024, 3, 9, 2, 30, 0, 0, EasternTime),
			expectedResult: time.Date(2024, 3, 10, 3, 30, 0, 0, EasternTime),
		},
		{ // newDate is on a Spring Forward DST day and originalTime is after 3AM
			newDate:      time.Date(2024, 3, 10, 8, 0, 0, 0, EasternTime),
			originalTime: time.Date(2024, 3, 9, 4, 30, 30, 30, EasternTime),
			expectedResult: time.Date(2024, 3, 10, 4, 30, 30, 30, EasternTime),
		},
		{ // newDate is on a Spring Forward DST day and originalTime is before 2AM
			newDate:      time.Date(2024, 3, 10, 8, 0, 0, 0, EasternTime),
			originalTime: time.Date(2024, 3, 9, 1, 30, 30, 30, EasternTime),
			expectedResult: time.Date(2024, 3, 10, 1, 30, 30, 30, EasternTime),
		},
		{ // newDate has different location than originalTime so result takes newDate's date but originalTime's time and location
			newDate:        time.Date(2023, 5, 11, 19, 30, 20, 10, EasternTime),
			originalTime:   time.Date(2023, 5, 10, 12, 0, 0, 0, CentralTime),
			expectedResult: time.Date(2023, 5, 11, 12, 0, 0, 0, CentralTime),
		},
		{ // originalTime is on a Spring Forward DST day AFTER the skipped hour. result should still have the same clock time as the originalTime
			newDate:      time.Date(2024, 3, 11, 1, 0, 0, 0, EasternTime),
			originalTime: time.Date(2024, 3, 10, 4, 0, 0, 0, EasternTime),
			expectedResult: time.Date(2024, 3, 11, 4, 0, 0, 0, EasternTime),
		},
	}

	for i, test := range tests {
		actualResult := ApplyNewDateToTime(test.newDate, test.originalTime)
		if !actualResult.Equal(test.expectedResult) {
			t.Errorf("Test case %d expected %v but got %v.", i, test.expectedResult, actualResult)
		}
	}
}
