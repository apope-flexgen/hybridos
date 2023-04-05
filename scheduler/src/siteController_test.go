package main

import (
	"encoding/json"
	"testing"
	"time"

	"github.com/flexgen-power/scheduler/pkg/events"
)

var EasternTime *time.Location = forceLoadLocation("America/New_York")
var EmptyMapStringInterface map[string]interface{} = make(map[string]interface{})

type buildLegacyObjectTest struct {
	inputSiteController siteController
	inputCurrentTime    time.Time
	expectedJson        string
}
type buildLegacyObjectTestCases []buildLegacyObjectTest

func TestBuildLegacyObject(t *testing.T) {
	tests := buildLegacyObjectTestCases{
		{	// "yesterday" event crossing midnight
			inputSiteController: siteController{
				timezone:        EasternTime,
				scheduledEvents: &eventHeap{},
				expiredEvents:   &eventHeap{},
				activeEvent: &events.Event{
					StartTime: time.Date(2023, 2, 22, 23, 0, 0, 0, EasternTime),
					Duration:  time.Hour * 2,
					Variables: EmptyMapStringInterface,
				},
			},
			inputCurrentTime: time.Date(2023, 2, 23, 17, 0, 0, 0, EasternTime),
			expectedJson:     "[[{\"duration\":120,\"mode\":\"\",\"nonRolling\":false,\"start_time\":-60,\"variables\":{}}],[]]",
		},
		{	// last day of month
			inputSiteController: siteController{
				timezone: EasternTime,
				scheduledEvents: &eventHeap{
					{
						StartTime: time.Date(2023, 3, 1, 0, 0, 0, 0, EasternTime),
						Duration:  time.Hour,
						Variables: EmptyMapStringInterface,
					},
				},
				expiredEvents: &eventHeap{},
				activeEvent:   nil,
			},
			inputCurrentTime: time.Date(2023, 2, 28, 12, 0, 0, 0, EasternTime),
			expectedJson:     "[[],[{\"duration\":60,\"mode\":\"\",\"nonRolling\":false,\"start_time\":0,\"variables\":{}}]]",
		},
		{	// last day of year
			inputSiteController: siteController{
				timezone: EasternTime,
				scheduledEvents: &eventHeap{
					{
						StartTime: time.Date(2024, 1, 1, 0, 0, 0, 0, EasternTime),
						Duration:  time.Hour,
						Variables: EmptyMapStringInterface,
					},
				},
				expiredEvents: &eventHeap{},
				activeEvent:   nil,
			},
			inputCurrentTime: time.Date(2023, 12, 31, 12, 0, 0, 0, EasternTime),
			expectedJson:     "[[],[{\"duration\":60,\"mode\":\"\",\"nonRolling\":false,\"start_time\":0,\"variables\":{}}]]",
		},
		{	// February 28th on leap year
			inputSiteController: siteController{
				timezone: EasternTime,
				scheduledEvents: &eventHeap{
					{
						StartTime: time.Date(2024, 2, 29, 0, 0, 0, 0, EasternTime),
						Duration:  time.Hour,
						Variables: EmptyMapStringInterface,
					},
				},
				expiredEvents: &eventHeap{},
				activeEvent:   nil,
			},
			inputCurrentTime: time.Date(2024, 2, 28, 12, 0, 0, 0, EasternTime),
			expectedJson:     "[[],[{\"duration\":60,\"mode\":\"\",\"nonRolling\":false,\"start_time\":0,\"variables\":{}}]]",
		},
	}

	for i, test := range tests {
		// override the getCurrentTime function to maintain unit test conditions
		getCurrentTime = func() time.Time {
			return test.inputCurrentTime
		}
		result := test.inputSiteController.buildLegacyObject()
		marshaledResult, _ := json.Marshal(result)
		stringifiedResult := string(marshaledResult)
		if stringifiedResult != test.expectedJson {
			t.Errorf("test index %d failed: expected %s; got %s", i, test.expectedJson, stringifiedResult)
		}
	}
}

type getOtherDayEventsTest struct {
	inputSiteController siteController
	inputDay            int
	expectedEventHeap   eventHeap
}
type getOtherDayEventsTestCases []getOtherDayEventsTest

func TestGetOtherDayEvents(t *testing.T) {
	tests := getOtherDayEventsTestCases{
		{
			inputSiteController: siteController{
				timezone:        EasternTime,
				scheduledEvents: &eventHeap{},
				expiredEvents:   &eventHeap{},
				activeEvent: &events.Event{
					StartTime: time.Date(2023, 2, 22, 23, 0, 0, 0, EasternTime),
					Duration:  time.Hour * 2,
					Variables: EmptyMapStringInterface,
				},
			},
			inputDay:          0,
			expectedEventHeap: eventHeap{},
		},
		{
			inputSiteController: siteController{
				timezone:        EasternTime,
				scheduledEvents: &eventHeap{},
				expiredEvents:   &eventHeap{},
				activeEvent: &events.Event{
					StartTime: time.Date(2023, 2, 22, 23, 0, 0, 0, EasternTime),
					Duration:  time.Hour * 2,
					Variables: EmptyMapStringInterface,
				},
			},
			inputDay: 1,
			expectedEventHeap: eventHeap{
				&events.Event{
					StartTime: time.Date(2023, 2, 22, 23, 0, 0, 0, EasternTime),
					Duration:  time.Hour * 2,
					Variables: EmptyMapStringInterface,
				},
			},
		},
	}

	// override the getCurrentTime function to maintain unit test conditions
	getCurrentTime = func() time.Time {
		return time.Date(2023, 2, 23, 17, 0, 0, 0, EasternTime)
	}

	for i, test := range tests {
		result := test.inputSiteController.getOtherDayEvents(test.inputDay)
		if !result.equals(&test.expectedEventHeap) {
			t.Errorf("test index %d failed: expected %v; got %v", i, test.expectedEventHeap, *result)
		}
	}
}
