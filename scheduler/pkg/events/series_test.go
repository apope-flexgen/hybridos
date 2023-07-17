package events

import (
	"testing"
	"time"
)

func TestAddException(t *testing.T) {
	type testCase struct {
		inputExceptions    []time.Time
		exceptionToAdd     time.Time
		expectedExceptions []time.Time
	}

	testCases := []testCase{
		{ // add exception to empty array
			inputExceptions:    []time.Time{},
			exceptionToAdd:     time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime)},
		},
		{ // add exception to non-empty array
			inputExceptions:    []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime)},
			exceptionToAdd:     time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
		},
		{ // adding exception that already exists
			inputExceptions:    []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
			exceptionToAdd:     time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
		},
		{ // adding exceptions that are out of order
			inputExceptions:    []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
			exceptionToAdd:     time.Date(2023, 4, 14, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 4, 14, 8, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
		},
	}

	for i, test := range testCases {
		result := Series{Exceptions: test.inputExceptions}
		result.addException(test.exceptionToAdd)

		if len(result.Exceptions) != len(test.expectedExceptions) {
			t.Errorf("Test with index %d failed. Lengths do not match. Expected: %v. Got: %v.", i, test.expectedExceptions, result.Exceptions)
		}
		for j, expectedTime := range test.expectedExceptions {
			if !expectedTime.Equal(result.Exceptions[j]) {
				t.Errorf("Test with index %d failed. Expected exception with index %d to be %s but got %s instead.", i, j, expectedTime.String(), result.Exceptions[j].String())
			}
		}
	}
}

func TestValidateExceptions(t *testing.T) {
	type testCase struct {
		inputExceptions    []time.Time
		startTime          time.Time
		expectedExceptions []time.Time
	}

	// alignment function is thoroughly tested in flextime unit test so this unit test focuses on the higher-level details (slice of times versus single time)
	testCases := []testCase{
		{ // empty exceptions
			inputExceptions:    []time.Time{},
			startTime:          time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{},
		},
		{ // single exception that does not need any edits
			inputExceptions: []time.Time{
				time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			},
			startTime: time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{
				time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			},
		},
		{ // exception in the past should be deleted
			inputExceptions: []time.Time{
				time.Date(2023, 5, 9, 12, 0, 0, 0, EasternTime),
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
			},
			startTime: time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
			},
		},
		{ // making sure align function is actually being called
			inputExceptions: []time.Time{
				time.Date(2023, 5, 11, 14, 30, 0, 0, EasternTime),
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
			},
			startTime: time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{
				time.Date(2023, 5, 11, 12, 0, 0, 0, EasternTime),
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
			},
		},
		{ // sorting out of order exceptions
			inputExceptions: []time.Time{
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
				time.Date(2023, 5, 11, 12, 0, 0, 0, EasternTime),
			},
			startTime: time.Date(2023, 5, 10, 12, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{
				time.Date(2023, 5, 11, 12, 0, 0, 0, EasternTime),
				time.Date(2023, 5, 12, 12, 0, 0, 0, EasternTime),
			},
		},
	}

	for i, test := range testCases {
		result := Series{Exceptions: test.inputExceptions}
		result.validateExceptions(test.startTime)

		if len(result.Exceptions) != len(test.expectedExceptions) {
			t.Errorf("Test with index %d failed. Lengths do not match. Expected: %v. Got: %v.", i, test.expectedExceptions, result.Exceptions)
		}
		for j, expectedTime := range test.expectedExceptions {
			if !expectedTime.Equal(result.Exceptions[j]) {
				t.Errorf("Test with index %d failed. Expected exception with index %d to be %s but got %s instead.", i, j, expectedTime.String(), result.Exceptions[j].String())
			}
		}
	}
}

func TestDeleteException(t *testing.T) {
	type testCase struct {
		inputExceptions    []time.Time
		exceptionToDelete  time.Time
		expectedExceptions []time.Time
	}

	testCases := []testCase{
		{ // deleting only exception in array
			inputExceptions:    []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
			exceptionToDelete:  time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{},
		},
		{ // deleting one of multiple exceptions in array
			inputExceptions:    []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
			exceptionToDelete:  time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime)},
		},
		{ // empty exceptions array so nothing to delete
			inputExceptions:    []time.Time{},
			exceptionToDelete:  time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{},
		},
		{ // filled exceptions array but targeted exception is not one of them
			inputExceptions:    []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
			exceptionToDelete:  time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
		},
		{ // deleting middle exception when there are 3+ exceptions
			inputExceptions:    []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 9, 0, 0, 0, EasternTime)},
			exceptionToDelete:  time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime),
			expectedExceptions: []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 9, 0, 0, 0, EasternTime)},
		},
		{ // deleting exception using equivalent time in different time zone
			inputExceptions:    []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 9, 0, 0, 0, EasternTime)},
			exceptionToDelete:  time.Date(2023, 4, 15, 13, 0, 0, 0, time.UTC),
			expectedExceptions: []time.Time{time.Date(2023, 3, 13, 10, 0, 0, 0, EasternTime), time.Date(2023, 4, 15, 8, 0, 0, 0, EasternTime)},
		},
	}

	for i, test := range testCases {
		result := Series{Exceptions: test.inputExceptions}
		result.deleteException(test.exceptionToDelete)

		if len(result.Exceptions) != len(test.expectedExceptions) {
			t.Errorf("Test with index %d failed. Lengths do not match. Expected: %v. Got: %v.", i, test.expectedExceptions, result.Exceptions)
		}
		for j, expectedTime := range test.expectedExceptions {
			if !expectedTime.Equal(result.Exceptions[j]) {
				t.Errorf("Test with index %d failed. Expected exception with index %d to be %s but got %s instead.", i, j, expectedTime.String(), result.Exceptions[j].String())
			}
		}
	}
}

func TestSeriesDeleteExceptionsBefore(t *testing.T) {
	type testCase struct {
		inputSeries        Series
		referenceTime      time.Time
		expectedSeries     Series
		expectedResultFlag bool
	}

	tests := []testCase{
		{ // delete only exception before reference time
			inputSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
				},
			},
			referenceTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSeries: Series{
				Exceptions: []time.Time{},
			},
			expectedResultFlag: true,
		},
		{ // delete exception before reference time, leaving exception that falls after reference time
			inputSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
					time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
				},
			},
			referenceTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
				},
			},
			expectedResultFlag: true,
		},
		{ // only exception falls after reference time
			inputSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
				},
			},
			referenceTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 16, 12, 0, 0, 0, EasternTime),
				},
			},
			expectedResultFlag: false,
		},
		{ // empty exceptions array
			inputSeries: Series{
				Exceptions: []time.Time{},
			},
			referenceTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSeries: Series{
				Exceptions: []time.Time{},
			},
			expectedResultFlag: false,
		},
		{ // multiple exceptions before reference time
			inputSeries: Series{
				Exceptions: []time.Time{
					time.Date(2023, 4, 14, 12, 0, 0, 0, EasternTime),
					time.Date(2023, 4, 15, 12, 0, 0, 0, EasternTime),
				},
			},
			referenceTime: time.Date(2023, 4, 16, 0, 0, 0, 0, EasternTime),
			expectedSeries: Series{
				Exceptions: []time.Time{},
			},
			expectedResultFlag: true,
		},
	}

	for i, test := range tests {
		resultFlag := test.inputSeries.DeleteExceptionsBefore(test.referenceTime)
		if isEqual, reasonNotEqual := test.inputSeries.equals(&test.expectedSeries); !isEqual {
			t.Errorf("Test index %d result did not equal expected series for reason: %s.", i, reasonNotEqual)
		}
		if resultFlag != test.expectedResultFlag {
			t.Errorf("Test index %d expected %v result flag but got %v.", i, test.expectedResultFlag, resultFlag)
		}
	}
}

func TestSeriesEqual(t *testing.T) {
	type testCase struct {
		ser1           *Series
		ser2           *Series
		expectedResult bool
	}

	tests := []testCase{
		{
			ser1:           nil,
			ser2:           nil,
			expectedResult: true,
		},
		{
			ser1:           nil,
			ser2:           NewNonRepeatingSeries(),
			expectedResult: false,
		},
		{
			ser1:           NewNonRepeatingSeries(),
			ser2:           nil,
			expectedResult: false,
		},
		{
			ser1:           NewNonRepeatingSeries(),
			ser2:           NewNonRepeatingSeries(),
			expectedResult: true,
		},
	}

	for i, test := range tests {
		actualResult, reasonNotEqual := test.ser1.equals(test.ser2)

		if actualResult != test.expectedResult {
			if test.expectedResult {
				t.Errorf("Test index %d expected true but got false for reason: %s.", i, reasonNotEqual)
			} else {
				t.Errorf("Test index %d expected false but got true.", i)
			}
		}
	}
}
