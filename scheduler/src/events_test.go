package main

import (
	"testing"
	"time"

	events "github.com/flexgen-power/scheduler/pkg/events"
)

var currentTime time.Time
var inOneMin time.Time
var inTwoMins time.Time
var inOneHour time.Time
var inOneHourOneMin time.Time

func init() {
	currentTime = time.Date(time.Now().Year(), time.Now().Month(), time.Now().Day(), time.Now().Hour(), time.Now().Minute(), 0, 0, time.Now().Location())
	inOneMin = currentTime.Add(time.Minute)
	inTwoMins = currentTime.Add(2 * time.Minute)
	inOneHour = currentTime.Add(time.Hour)
	inOneHourOneMin = currentTime.Add(time.Hour + time.Minute)
}

// hasContinuousTransition() unit test
func TestHasContinuousTransition(t *testing.T) {
	frEvent := events.CreateEvent(currentTime, time.Minute, "Frequency Response")
	setEvent := events.CreateEvent(currentTime, time.Minute, "Site Export Target")

	// Each event in this list represents a test case against both of the events given above
	followingEvents := []*events.Event{
		events.CreateEvent(inOneMin, time.Minute, "Frequency Response"),
		events.CreateEvent(inOneMin, time.Minute, "Site Export Target"),
		events.CreateEvent(inTwoMins, time.Minute, "Frequency Response"),
		events.CreateEvent(inTwoMins, time.Minute, "Site Export Target"),
	}

	frHourEvent := events.CreateEvent(currentTime, time.Hour, "Frequency Response")
	setHourEvent := events.CreateEvent(currentTime, time.Hour, "Site Export Target")

	// Each event in this list represents a test case against both of the hour-long events given above
	followingHourEvents := []*events.Event{
		events.CreateEvent(inOneHour, time.Hour, "Frequency Response"),
		events.CreateEvent(inOneHour, time.Hour, "Site Export Target"),
		events.CreateEvent(inOneHourOneMin, time.Hour, "Frequency Response"),
		events.CreateEvent(inOneHourOneMin, time.Hour, "Site Export Target"),
	}

	// Same expected results across minute-long and hour-long events
	// Expected results of the above cases against the FR Event. True means a continuous transition
	frExpectedResults := []bool{true, false, false, false}
	// Expected results of the above cases against the Site Export Target Event. True means a continuous transition
	setExpectedResults := []bool{false, true, false, false}

	// FR test
	for i, followingEvent := range followingEvents {
		if frExpectedResults[i] != frEvent.HasContinuousTransition(followingEvent) {
			t.Errorf("Expected continuous transition between FR event and Event %d to be %t, but it was %t", i, frExpectedResults[i], frEvent.HasContinuousTransition(followingEvent))
		}
	}
	// FR hour test
	for i, followingEvent := range followingHourEvents {
		if frExpectedResults[i] != frHourEvent.HasContinuousTransition(followingEvent) {
			t.Errorf("Expected continuous transition between FR hour event and Event %d to be %t, but it was %t", i, frExpectedResults[i], frHourEvent.HasContinuousTransition(followingEvent))
		}
	}

	// Site Export Target test
	for i, followingEvent := range followingEvents {
		if setExpectedResults[i] != setEvent.HasContinuousTransition(followingEvent) {
			t.Errorf("Expected continuous transition between Site Export Target event and Event %d to be %t, but it was %t", i, setExpectedResults[i], setEvent.HasContinuousTransition(followingEvent))
		}
	}
	// Site Export Target hour test
	for i, followingEvent := range followingHourEvents {
		if setExpectedResults[i] != setHourEvent.HasContinuousTransition(followingEvent) {
			t.Errorf("Expected continuous transition between Site Export Target hour event and Event %d to be %t, but it was %t", i, setExpectedResults[i], setHourEvent.HasContinuousTransition(followingEvent))
		}
	}

}
