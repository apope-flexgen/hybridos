package main

import (
	"io/ioutil"
	"log"
	"os"
	"reflect"
	"testing"
	"time"
)

// forceLoadLocation allows the TimestampToDay unit test to fill in its test parameters without error checking.
//
// Should only be used in testing! Production code needs to handle all errors.
func forceLoadLocation(name string) *time.Location {
	loc, _ := time.LoadLocation(name)
	return loc
}

type handleDayTransitionsTest struct {
	lastCheck   time.Time
	currentTime time.Time
	ms          schedule
	resultMS    schedule
}
type handleDayTransitionsTestCases []*handleDayTransitionsTest

// handleDayTransitions() method unit test for scheduler
func TestHandleDayTransitions(t *testing.T) {
	SITE := "odessa"
	// Set the dummy mode map
	modes.data = make(modeMap)
	modes.data["default"] = &mode{}
	modes.data["Frequency Response"] = &mode{}
	// Create the test cases
	dayTransitionTestCases := handleDayTransitionsTestCases{
		// Test 0: EDT 2021 June 8th Noon; expect no transition
		{
			time.Date(2021, time.June, 8, 12, 0, 0, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 8, 12, 0, 1, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 8, 13, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 1:00 pm
						buildTestEvent(time.Date(2021, time.June, 9, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 8, 11, 30, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 11:30 am
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 8, 13, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 1:00 pm
						buildTestEvent(time.Date(2021, time.June, 9, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 8, 11, 30, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 11:30 am
					nil,
				},
			},
		},
		// Test 1: EDT 2021 June 10th-11th Midnight transition; no event crossing days
		{
			time.Date(2021, time.June, 10, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 11, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 11, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 11, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 12, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 2: EDT 2021 June 11th-12th Midnight transition; today has an active event crossing days
		{
			time.Date(2021, time.June, 11, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 12, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 12, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 11, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Active; 11:00 pm
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 12, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 13, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 11, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Active; crossing from 11:00 pm
					nil,
				},
			},
		},
		// Test 3: EDT 2021 June 12th-13th Midnight transition; active event today, none tomorrow
		{
			time.Date(2021, time.June, 12, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 13, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 13, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 12, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 11:00 pm
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 13, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 14, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.June, 12, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 11:00 pm previous night, will be ended in next site check
					nil,
				},
			},
		},
		// Test 4: EDT 2021 June 13th-14th Midnight transition; no active event today, tomorrow starts with an event at midnight
		{
			time.Date(2021, time.June, 13, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 14, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 14, 0, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 12:00 am (day-start)
						buildTestEvent(time.Date(2021, time.June, 14, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 14, 0, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today, starting at next site check
						buildTestEvent(time.Date(2021, time.June, 14, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 15, 0, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 12:00 am (day-start)
						buildTestEvent(time.Date(2021, time.June, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am
					},
					newEventHeap(),
					nil, // midnight-starting event will begin at next site check
					nil,
				},
			},
		},
		// Test 5: EDT 2021 June 14th-15th Midnight transition; tomorrow has an event that crosses days
		{
			time.Date(2021, time.June, 14, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.June, 15, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),   // Tomorrow; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 15, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Tomorrow; 11:00 pm
					},
					newEventHeap(),
					nil, //Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.June, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),   // Today; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 15, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Today; 11:00 pm
						buildTestEvent(time.Date(2021, time.June, 16, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),   // Tomorrow; 6:00 am
						buildTestEvent(time.Date(2021, time.June, 16, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), //Tomorrow; 11:00 pm
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 6: Eastern Daylight Savings 2021 March 14th Shift Forward; expect no transition
		{
			time.Date(2021, time.March, 14, 1, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 14, 1, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 7, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Today; 7:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 14, 9, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Today; 9:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 7, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Today; 7:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 14, 9, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Today; 9:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),  // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 7: Eastern Daylight Savings 2021 Nov 7th Shift Backward; expect no transition
		{
			time.Date(2021, time.November, 7, 1, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 7, 1, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 5, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 5:00 am EST
						buildTestEvent(time.Date(2021, time.November, 7, 7, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 7:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 5, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 5:00 am EST
						buildTestEvent(time.Date(2021, time.November, 7, 7, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 7:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 8: EST 2021 March 13th-14th Midnight Transition; Tomorrow is short and has an event that crosses days
		{
			time.Date(2021, time.March, 13, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 14, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 11:00 pm EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Today; 11:00 pm EDT
						buildTestEvent(time.Date(2021, time.March, 15, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 11:00 pm EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 9: EDT 2021 Nov 6th-7th Midnight Transition; Tomorrow is long and has an event that crosses days
		{
			time.Date(2021, time.November, 6, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 7, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Today; 10:00 pm EST
						buildTestEvent(time.Date(2021, time.November, 8, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 10: EDT 2021 March 14th-15th Midnight Transition; Today is short and has an active event crossing days
		{
			time.Date(2021, time.March, 14, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 15, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 15, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EDT
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.March, 14, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Active; 11:00 pm EDT
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 15, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Today; 10:00 pm EDT
						buildTestEvent(time.Date(2021, time.March, 16, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EDT
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.March, 14, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Active; crossing from 11:00 pm EDT
					nil,
				},
			},
		},
		// Test 11: EST 2021 Nov 7th-8th Midnight Transition; Today is long and has an active event crossing days
		{
			time.Date(2021, time.November, 7, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 8, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 8, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 11:00 pm EST
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.November, 7, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Active; 11:00 pm EST
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 8, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Today; 11:00 pm EST
						buildTestEvent(time.Date(2021, time.November, 9, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 11:00 pm EST
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.November, 7, 23, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Active; 11:00 pm EST
					nil,
				},
			},
		},
		// Test 12: EDT 2021 August 4th-5th Midnight Transition; Tomorrow's schedule would overlap the day after tomorrow's schedule
		{
			time.Date(2021, time.August, 4, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.August, 5, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.August, 5, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Tomorrow; 11:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.August, 5, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 840, "Frequency Response", 2000), // Tomorrow; 10:00 pm EDT 14 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.August, 5, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Today; 11:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.August, 5, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 780, "Frequency Response", 2000), // Today; 10:00 pm EDT 13 hr
						buildTestEvent(time.Date(2021, time.August, 6, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Tomorrow; 11:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.August, 6, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 840, "Frequency Response", 2000), // Tomorrow; 10:00 pm EDT 14 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 13: EST 2021 March 12th-13th Midnight Transition; Day after tomorrow is short, causing inter-day event overlap
		{
			time.Date(2021, time.March, 12, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 13, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 13, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Tomorrow; 11:00 am EST 2 hr
						buildTestEvent(time.Date(2021, time.March, 13, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 780, "Frequency Response", 2000), // Tomorrow; 10:00 pm EST 13 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 13, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Today; 11:00 am EST 2 hr
						buildTestEvent(time.Date(2021, time.March, 13, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 720, "Frequency Response", 2000), // Today; 10:00 pm EST 12 hr
						buildTestEvent(time.Date(2021, time.March, 14, 11, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000), // Tomorrow; 11:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.March, 14, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 780, "Frequency Response", 2000), // Tomorrow; 10:00 pm EDT 13 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 14: EDT 2021 Nov 6th-7th Midnight Transition; Tomorrow is long, does not cause inter-day event overlap
		{
			time.Date(2021, time.November, 6, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 7, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 1, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000),  // Tomorrow; 1:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.November, 7, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EST 3 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 1, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000),  // Today; 1:00 am EDT 2 hr
						buildTestEvent(time.Date(2021, time.November, 7, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Today; 10:00 pm EST 3 hr
						buildTestEvent(time.Date(2021, time.November, 8, 1, 0, 0, 0, forceLoadLocation("America/New_York")), 120, "Frequency Response", 2000),  // Tomorrow; 1:00 am EST 2 hr
						buildTestEvent(time.Date(2021, time.November, 8, 22, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 2000), // Tomorrow; 10:00 pm EST 3 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 15: 2021 March 12th-13th Midnight Transition; Events during skipped hour should be moved to the 15th and the last of the skipped events should be moved to 3 o'clock
		{
			time.Date(2021, time.March, 12, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 13, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 13, 2, 0, 0, 0, forceLoadLocation("America/New_York")), 30, "Frequency Response", 2000),   // Tomorrow; 2:00 am EST 30 min
						buildTestEvent(time.Date(2021, time.March, 13, 2, 30, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000), // Tomorrow; 2:30 am EST 3 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 13, 2, 0, 0, 0, forceLoadLocation("America/New_York")), 30, "Frequency Response", 2000),            // Today; 2:00 am EST 30 min
						buildTestEvent(time.Date(2021, time.March, 13, 2, 30, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000),          // Today; 2:30 am EST 3 hr
						buildNonRollingTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000), // Tomorrow; 3:00 am EDT 3 hr
						// following two out of order bc min heap does not store in exact order by index
						buildTestEvent(time.Date(2021, time.March, 15, 2, 30, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000), // March 15th; 2:30 am EDT 3 hr
						buildTestEvent(time.Date(2021, time.March, 15, 2, 0, 0, 0, forceLoadLocation("America/New_York")), 30, "Frequency Response", 2000),   // March 15th; 2:00 am EDT 30 min
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
		// Test 16: 2021 March 13th-14th Midnight Transition; During the 12-13th midnight transition there were events moved to the 15th. Make sure those do not cause problems now
		{
			time.Date(2021, time.March, 12, 23, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 13, 0, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildNonRollingTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000), // Tomorrow; 3:00 am EDT 3 hr
						buildTestEvent(time.Date(2021, time.March, 15, 2, 0, 0, 0, forceLoadLocation("America/New_York")), 30, "Frequency Response", 2000),            // March 15th; 2:00 am EDT 30 min
						buildTestEvent(time.Date(2021, time.March, 15, 2, 30, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000),          // March 15th; 2:30 am EDT 3 hr
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildNonRollingTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000), // Today; 3:00 am EDT 3 hr
						buildTestEvent(time.Date(2021, time.March, 15, 2, 30, 0, 0, forceLoadLocation("America/New_York")), 180, "Frequency Response", 1000),          // Tomorrow; 2:30 am EDT 3 hr
						buildTestEvent(time.Date(2021, time.March, 15, 2, 0, 0, 0, forceLoadLocation("America/New_York")), 30, "Frequency Response", 2000),            // Tomorrow; 2:00 am EDT 30 min
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
		},
	}

	for i, testCase := range dayTransitionTestCases {
		masterSchedule.data = testCase.ms
		handleDayTransitions(testCase.currentTime, testCase.lastCheck)
		if !reflect.DeepEqual(masterSchedule.data, testCase.resultMS) {
			t.Errorf("dayTransition() test %d failed. Schedule had an unexpected value", i)
			log.Printf("---handleDayTransitions() Test %d---\n", i)
			log.Println("Expected:")
			testCase.resultMS.print()
			log.Println("Got:")
			testCase.ms.print()
		}
	}
}

type scheduleCheckTest struct {
	lastCheck   time.Time
	currentTime time.Time
	ms          schedule
	resultMS    schedule
}
type scheduleCheckTestCases []*scheduleCheckTest

// checkMasterSchedule method unit test for scheduler
func TestScheduleCheck(t *testing.T) {
	SITE := "odessa"
	// Set the dummy mode map
	modes.data = make(modeMap)
	modes.data["default"] = &mode{}
	modes.data["Frequency Response"] = &mode{}
	// Create the test cases
	scheduleCheckTestCases := scheduleCheckTestCases{
		// Test 0: Eastern Daylight Savings 2021 March 14th Shift Forward; before shift
		{time.Date(2021, time.March, 14, 0, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 14, 1, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 1, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 1:00 am EST
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.March, 14, 1, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 1:00 am EST
					nil,
				},
			},
		},
		// Test 1: Eastern Daylight Savings 2021 March 14th Shift Forward; at shift
		{time.Date(2021, time.March, 14, 1, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 14, 1, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 3:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.March, 14, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 3:00 am
					nil,
				},
			},
		},
		// Test 2: Eastern Daylight Savings 2021 March 14th Shift Forward; after shift
		{time.Date(2021, time.March, 14, 3, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.March, 14, 4, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 14, 4, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 4:00 am EDT
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.March, 15, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EDT
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.March, 14, 4, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 4:00 am
					nil,
				},
			},
		},
		// Test 3: Eastern Daylight Savings 2021 Nov 7th Shift Backward; before shift
		{time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second), 60, "Frequency Response", 2000), // Today; 1:00 am EDT
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),                    // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Second), 60, "Frequency Response", 2000), // Active; 1:00 am EDT
					nil,
				},
			},
		},
		// Test 4: Eastern Daylight Savings 2021 Nov 7th Shift Backward; at shift
		{time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Hour), time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Hour).Add(time.Second),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Hour).Add(time.Second), 60, "Frequency Response", 2000), // Today; 1:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000),                                   // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.November, 7, 0, 59, 59, 0, forceLoadLocation("America/New_York")).Add(time.Hour).Add(time.Second), 60, "Frequency Response", 2000), // Active; 1:00 am EST
					nil,
				},
			},
		},
		// Test 5: Eastern Daylight Savings 2021 Nov 7th Shift Backward; after shift
		{time.Date(2021, time.November, 7, 2, 59, 59, 0, forceLoadLocation("America/New_York")), time.Date(2021, time.November, 7, 3, 0, 0, 0, forceLoadLocation("America/New_York")),
			schedule{ // Input
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 7, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Today; 3:00 am EST
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					nil, // Active; nothing
					nil,
				},
			},
			schedule{ // Result
				SITE: {SITE, SITE,
					forceLoadLocation("America/New_York"),
					&eventHeap{
						buildTestEvent(time.Date(2021, time.November, 8, 6, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Tomorrow; 6:00 am EST
					},
					newEventHeap(),
					buildTestEvent(time.Date(2021, time.November, 7, 3, 0, 0, 0, forceLoadLocation("America/New_York")), 60, "Frequency Response", 2000), // Active; 3:00 am EST
					nil,
				},
			},
		},
	}

	for i, testCase := range scheduleCheckTestCases {
		// Instead of calling actual time functions, use the test case provided time
		getCurrentTime = func() time.Time {
			return testCase.currentTime
		}
		lastCheck = testCase.lastCheck
		log.SetOutput(ioutil.Discard)
		masterSchedule.data = testCase.ms
		configureScadaInterface()
		checkMasterSchedule()
		log.SetOutput(os.Stdout)
		if !reflect.DeepEqual(testCase.ms, testCase.resultMS) {
			t.Errorf("(*mSchedule) check() test %d failed.", i)
			log.Printf("---(*mSchedule) check() Test %v---\n", i)
			log.Println("Expected:")
			testCase.resultMS.print()
			log.Println("Got:")
			testCase.ms.print()
		}
	}
}
