package main

import (
	"container/heap"
	"io/ioutil"
	"log"
	"os"
	"reflect"
	"testing"
	"time"

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

// minsAfterNewYears20
func minsAfterNewYears20(minutesAfterMidnight int) time.Time {
	return time.Date(2020, 1, 1, 0, 0, 0, 0, forceLoadLocation("America/New_York")).Add(time.Duration(minutesAfterMidnight) * time.Minute)
}

// buildTestEvent helps streamline event creation
func buildTestEvent(startTime time.Time, duration int, mode string, value int) *events.Event {
	return &events.Event{
		StartTime:  startTime,
		Duration:   time.Duration(duration) * time.Minute,
		Mode:       mode,
		Variables:  map[string]interface{}{"kW_cmd": value},
		NonRolling: false,
	}
}

// buildNonRollingTestEvent helps streamline event creation
func buildNonRollingTestEvent(startTime time.Time, duration int, mode string, value int) *events.Event {
	return &events.Event{
		StartTime:  startTime,
		Duration:   time.Duration(duration) * time.Minute,
		Mode:       mode,
		Variables:  map[string]interface{}{"kW_cmd": value},
		NonRolling: true,
	}
}

type peekEventHeapTest struct {
	inputHeap   *eventHeap
	resultEvent *events.Event
}
type peekEventHeapTestCases []*peekEventHeapTest

// Peek() method unit test for eventHeap
func TestPeek(t *testing.T) {
	peekTestCases := peekEventHeapTestCases{
		&peekEventHeapTest{ // Peek from heap with multiple events
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
		},
		&peekEventHeapTest{ // Peek from heap with just one event
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000),
		},
		&peekEventHeapTest{ // Peek from empty heap
			&eventHeap{},
			nil,
		},
	}

	for i, testCase := range peekTestCases {
		h := testCase.inputHeap
		heap.Init(h)
		e, err := h.peek()
		if testCase.resultEvent == nil {
			if e != nil {
				t.Errorf("Peek() test %v failed. Peek() returned a non-nil event but it should have returned nil", i)
			}
		} else if e == nil {
			t.Errorf("Peek() test %v failed. Peek() returned with error %v", i, err.Error())
		} else if !e.Equals(testCase.resultEvent) {
			t.Errorf("Peek() test %v failed. Peeked event did not match expected event", i)
		}
	}
}

type getEventHeapTest struct {
	inputHeap   *eventHeap
	targetEvent time.Time
	resultEvent *events.Event
}
type getEventHeapTestCases []*getEventHeapTest

// Get() method unit test for eventHeap
func TestGet(t *testing.T) {
	getTestCases := getEventHeapTestCases{
		&getEventHeapTest{ // Get top-of-heap event
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1200,
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
		},
		&getEventHeapTest{ // Get event that is not at the top of the heap
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1700,
			buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000),
		},
		&getEventHeapTest{ // Try to get event that is not in heap
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			tVals.jan1_20_NY1200,
			nil,
		},
		&getEventHeapTest{ // Try to get event from empty heap
			&eventHeap{},
			tVals.jan1_20_NY1200,
			nil,
		},
	}

	for i, testCase := range getTestCases {
		h := testCase.inputHeap
		heap.Init(h)
		e, _ := h.get(testCase.targetEvent)
		if testCase.resultEvent == nil {
			if e != nil {
				t.Errorf("Get() test %v failed. Expected nil pointer but got back actual event", i)
			}
		} else if e == nil {
			t.Errorf("Get() test %v failed. Expected event but got nil pointer", i)
		} else if !e.Equals(testCase.resultEvent) {
			t.Errorf("Get() test %v failed. Event returned does not match expected event", i)
		}
	}
}

type setEventHeapTest struct {
	inputHeap   *eventHeap
	setEvent    *events.Event
	resultEvent *events.Event
}
type setEventHeapTestCases []*setEventHeapTest

// Set() method unit test for eventHeap
func TestSet(t *testing.T) {
	setTestCases := setEventHeapTestCases{
		&setEventHeapTest{ // Set top-of-heap event
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
			buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
		},
		&setEventHeapTest{ // Set event that is not at the top of the heap
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1700, 10, "Frequency Response", 1000),
			buildTestEvent(tVals.jan1_20_NY1700, 10, "Frequency Response", 1000),
		},
		&setEventHeapTest{ // Try to set event that is not in heap
			&eventHeap{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)},
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
			nil,
		},
		&setEventHeapTest{ // Try to set event in empty heap
			&eventHeap{},
			buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000),
			nil,
		},
	}

	for i, testCase := range setTestCases {
		h := testCase.inputHeap
		heap.Init(h)
		h.Set(testCase.setEvent)
		e, _ := h.get(testCase.setEvent.StartTime)
		if testCase.resultEvent == nil {
			if e != nil {
				t.Errorf("Set() test %v failed. Expected nil pointer but got back actual event", i)
			}
		} else if e == nil {
			t.Errorf("Set() test %v failed. Expected event but got nil pointer", i)
		} else if !e.Equals(testCase.resultEvent) {
			t.Errorf("Set() test %v failed. Event returned does not match expected event", i)
		}
	}
}

type copyEventHeapTest struct {
	inputHeap *eventHeap
}
type copyEventHeapTestCases []*copyEventHeapTest

// Copy() method unit test for eventHeap
func TestCopy(t *testing.T) {
	copyTestCases := copyEventHeapTestCases{
		{&eventHeap{}}, // Empty heap
		{&eventHeap{buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)}},                                                                                                                                           // 1 event
		{&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000)}},                                                                     // 2 events
		{&eventHeap{buildTestEvent(tVals.jan1_20_NY1200, 10, "Frequency Response", 1000), buildTestEvent(tVals.jan1_20_NY1700, 20, "Energy Arbitrage", 2000), buildTestEvent(tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000)}}, // 3 events
	}

	for i, testCase := range copyTestCases {
		copyHeap := testCase.inputHeap.copy()
		if !reflect.DeepEqual(copyHeap, testCase.inputHeap) {
			t.Errorf("Copy() test %v failed. Event heap copy was not equal to original", i)
		}
		if top, err := testCase.inputHeap.peek(); err == nil {
			top.Duration++
			if reflect.DeepEqual(&copyHeap, testCase.inputHeap) {
				t.Errorf("Copy() test %v failed. Event heap copy is still same as original after original had an event change", i)
			}
			testCase.inputHeap.Pop()
			if reflect.DeepEqual(&copyHeap, testCase.inputHeap) {
				t.Errorf("Copy() test %v failed. Event heap copy is still same as original after original had an event removed", i)
			}
		}
	}
}

type deleteEventsWithModeTest struct {
	inputSchedule  schedule
	mode           string
	resultSchedule schedule
}
type deleteEventsWithModeTestCases []*deleteEventsWithModeTest

// deleteEventsWithMode() method unit test for masterSchedule
func TestDeleteEventsWithMode(t *testing.T) {
	delEventsWithModeTestCases := deleteEventsWithModeTestCases{
		&deleteEventsWithModeTest{ // basic deletion of a single mode with multiple modes existing, single site
			schedule{
				"raleigh": &siteController{
					"raleigh",
					"Raleigh",
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
						buildTestEvent(tVals.jan1_20_NY1700, 20, "Frequency Response", 2000),
						buildTestEvent(tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000),
						buildTestEvent(tVals.jan2_20_NY1700, 20, "Frequency Response", 2000),
					},
					newEventHeap(),
					nil,
					nil,
				},
			},
			"Frequency Response",
			schedule{
				"raleigh": &siteController{
					"raleigh",
					"Raleigh",
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(tVals.jan1_20_NY1200, 20, "Energy Arbitrage", 2000),
						buildTestEvent(tVals.jan2_20_NY1200, 20, "Energy Arbitrage", 2000),
					},
					newEventHeap(),
					nil,
					nil,
				},
			},
		},
		&deleteEventsWithModeTest{ // two sites, one empty, one with only the mode to be deleted and an active event
			schedule{
				"raleigh": &siteController{
					"raleigh",
					"Raleigh",
					forceLoadLocation("America/New_York"),
					&eventHeap{},
					newEventHeap(),
					nil,
					nil,
				},
				"durham": &siteController{
					"durham",
					"Durham",
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(tVals.jan1_20_NY1700, 20, "Frequency Response", 2000),
						buildTestEvent(tVals.jan2_20_NY1200, 20, "Frequency Response", 2000),
					},
					newEventHeap(),
					buildTestEvent(tVals.jan1_20_NY1200, 20, "Frequency Response", 2000),
					nil,
				},
			},
			"Frequency Response",
			schedule{
				"raleigh": &siteController{
					"raleigh",
					"Raleigh",
					forceLoadLocation("America/New_York"),
					&eventHeap{},
					newEventHeap(),
					nil,
					nil,
				},
				"durham": &siteController{
					"durham",
					"Durham",
					forceLoadLocation("America/New_York"),
					&eventHeap{},
					newEventHeap(),
					nil,
					nil,
				},
			},
		},
	}

	// run tests
	for i, testCase := range delEventsWithModeTestCases {
		masterSchedule.data = testCase.inputSchedule
		stdoutput := os.Stdout
		log.SetOutput(ioutil.Discard) // suppress log output (may want to remove this if working on this test)
		deleteEventsWithMode(testCase.mode)
		log.SetOutput(stdoutput) // restore log output
		if !masterSchedule.data.equals(testCase.resultSchedule) {
			log.Println("Output schedule failed:")
			masterSchedule.data.print()
			t.Errorf("deleteEventsWithMode() test %v failed. Output schedule did not match expected schedule", i)
		}
	}
}

type peekLastEventHeapTest struct {
	inputHeap   *eventHeap
	resultEvent *events.Event
}
type peekLastEventHeapTestCases []*peekLastEventHeapTest

// peekLast() method unit test for eventHeap
func TestPeekLast(t *testing.T) {
	peekLastTestCases := peekLastEventHeapTestCases{
		&peekLastEventHeapTest{ // Peek from heap with 8 events
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(100), 10, "Frequency Response", 1000),
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
				buildTestEvent(minsAfterNewYears20(300), 30, "Frequency Response", 3000),
				buildTestEvent(minsAfterNewYears20(400), 40, "Energy Arbitrage", 4000),
				buildTestEvent(minsAfterNewYears20(500), 50, "Frequency Response", 5000),
				buildTestEvent(minsAfterNewYears20(600), 60, "Energy Arbitrage", 6000),
				buildTestEvent(minsAfterNewYears20(700), 70, "Frequency Response", 7000),
				buildTestEvent(minsAfterNewYears20(800), 80, "Energy Arbitrage", 8000),
			},
			buildTestEvent(minsAfterNewYears20(800), 80, "Energy Arbitrage", 8000),
		},
		&peekLastEventHeapTest{ // Peek from heap with 8 events that start in a different order before Init() is called
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
				buildTestEvent(minsAfterNewYears20(300), 30, "Frequency Response", 3000),
				buildTestEvent(minsAfterNewYears20(400), 40, "Energy Arbitrage", 4000),
				buildTestEvent(minsAfterNewYears20(100), 10, "Frequency Response", 1000),
				buildTestEvent(minsAfterNewYears20(500), 50, "Frequency Response", 5000),
				buildTestEvent(minsAfterNewYears20(800), 80, "Energy Arbitrage", 8000),
				buildTestEvent(minsAfterNewYears20(600), 60, "Energy Arbitrage", 6000),
				buildTestEvent(minsAfterNewYears20(700), 70, "Frequency Response", 7000),
			},
			buildTestEvent(minsAfterNewYears20(800), 80, "Energy Arbitrage", 8000),
		},
		&peekLastEventHeapTest{ // Peek from heap with 4 events
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(100), 10, "Frequency Response", 1000),
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
				buildTestEvent(minsAfterNewYears20(300), 30, "Frequency Response", 3000),
				buildTestEvent(minsAfterNewYears20(400), 40, "Energy Arbitrage", 4000),
			},
			buildTestEvent(minsAfterNewYears20(400), 40, "Energy Arbitrage", 4000),
		},
		&peekLastEventHeapTest{ // Peek from heap with 3 events
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(100), 10, "Frequency Response", 1000),
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
				buildTestEvent(minsAfterNewYears20(300), 30, "Frequency Response", 3000),
			},
			buildTestEvent(minsAfterNewYears20(300), 30, "Frequency Response", 3000),
		},
		&peekLastEventHeapTest{ // Peek from heap with 2 events
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(100), 10, "Frequency Response", 1000),
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
			},
			buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
		},
		&peekLastEventHeapTest{ // Peek from heap with just one event
			&eventHeap{
				buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
			},
			buildTestEvent(minsAfterNewYears20(200), 20, "Energy Arbitrage", 2000),
		},
		&peekLastEventHeapTest{ // Peek from empty heap
			&eventHeap{},
			nil,
		},
	}

	for i, testCase := range peekLastTestCases {
		h := testCase.inputHeap
		heap.Init(h)
		e, err := h.peekLast()
		if testCase.resultEvent == nil {
			if e != nil {
				t.Errorf("peekLast() test %v failed. peekLast() returned a non-nil event but it should have returned nil", i)
			}
		} else if e == nil {
			t.Errorf("peekLast() test %v failed. peekLast() returned with error %v", i, err.Error())
		} else if !e.Equals(testCase.resultEvent) {
			t.Errorf("peekLast() test %v failed. Peeked event did not match expected event", i)
		}
	}
}
