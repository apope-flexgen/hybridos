package events

import (
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/flextime"
)

// simplified struct to avoid having to make a bunch of Event copies when only StartTime/Duration is needed
type timeWindow struct {
	start time.Time
	dur   time.Duration
}

// returns if two events are overlapping
func (e1 *Event) Overlaps(e2 *Event) bool {
	if e1 == nil || e2 == nil {
		return false
	}

	// finite series x finite series
	if e1.Repeat.isFiniteSeries() && e2.Repeat.isFiniteSeries() {
		return listOverlapsList(expandFiniteSeries(e1), expandFiniteSeries(e2))
	}

	// infinite series x finite series
	if e1.Repeat.isInfiniteSeries() && e2.Repeat.isFiniteSeries() {
		return finiteSeriesOverlapsInfiniteSeries(e2, e1)
	}

	// finite series x infinite series
	if e1.Repeat.isFiniteSeries() && e2.Repeat.isInfiniteSeries() {
		return finiteSeriesOverlapsInfiniteSeries(e1, e2)
	}

	// infinite daily x infinite daily
	if e1.Repeat.Cycle == DailyRepeat && e2.Repeat.Cycle == DailyRepeat {
		return infDailyOverlapsInfDaily(e1, e2)
	}

	// infinite daily x infinite weekly
	if e1.Repeat.Cycle == DailyRepeat && e2.Repeat.Cycle == WeeklyRepeat {
		return infDailyOverlapsInfWeekly(e1, e2)
	}

	// infinite weekly x infinite daily
	if e1.Repeat.Cycle == WeeklyRepeat && e2.Repeat.Cycle == DailyRepeat {
		return infDailyOverlapsInfWeekly(e2, e1)
	}

	// infinite weekly x infinite weekly
	if e1.Repeat.Cycle == WeeklyRepeat && e2.Repeat.Cycle == WeeklyRepeat {
		dailySplits1 := splitWeeklyByDays(e1)
		dailySplits2 := splitWeeklyByDays(e2)
		for _, d1 := range dailySplits1 {
			for _, d2 := range dailySplits2 {
				if infDailyOverlapsInfDaily(d1, d2) {
					return true
				}
			}
		}
		return false
	}

	// at this point all possible combinations have been covered.
	// if this part of the code is reached, then there is a logic
	// error somewhere in this func or another part of scheduler.
	// print a warning and return true to prevent undefined behavior
	log.Errorf("During event overlap check, found unexpected settings indicating error. Please report this to Software team. Event 1: %+v. Event 2: %+v.", e1, e2)
	return true
}

// extracts StartTime/duration into timeWindow struct
func (e *Event) toTimeWindow() timeWindow {
	return timeWindow{e.StartTime, e.TimeDuration()}
}

// takes a finite series and expands it to a list of timeWindows so each can be independently verified for overlap
func expandFiniteSeries(e *Event) []timeWindow {
	e = e.Copy() // need to deep copy so event's Repeat object does not get modified during this function's algorithm
	expansion := make([]timeWindow, 0)
	for {
		if !e.HasException(e.StartTime) {
			expansion = append(expansion, e.toTimeWindow())
		}
		springForwardEvent, seriesOver := e.ShiftSeries()
		if springForwardEvent != nil && !e.HasException(springForwardEvent.StartTime) {
			expansion = append(expansion, springForwardEvent.toTimeWindow())
		}
		if seriesOver {
			break
		}
	}
	return expansion
}

// Splits a weekly series that repeats every N weeks into separate daily series that repeat every 7*N days.
func splitWeeklyByDays(weeklySer *Event) []*Event {
	weeklySer = weeklySer.Copy()
	if weeklySer.Repeat.Cycle != WeeklyRepeat {
		return []*Event{weeklySer}
	}

	dayMask := weeklySer.Repeat.DayMask
	days := make([]*Event, 0)
	for dayMask > 0 {
		daySplit := weeklySer.Copy()
		daySplit.Repeat.Cycle = DailyRepeat
		daySplit.Repeat.Frequency *= 7
		days = append(days, daySplit)
		dayMask &= ^(weekdayEnumToMask(weeklySer.StartTime.Weekday())) // turn off the bit associated with this day
		weeklySer.ShiftSeries()
	}
	return days
}

// compares the clock times of two Events' StartTimes regardless of their days
// returns true if the clock times would hypothetically overlap if they were on the same or adjacent days
func clockTimesOverlap(e1 *Event, e2 *Event) bool {
	minsSinceMidnight1 := e1.StartTime.Hour()*60 + e1.StartTime.Minute()
	minsSinceMidnight2 := e2.StartTime.Hour()*60 + e2.StartTime.Minute()
	dur1 := int(e1.Duration)
	dur2 := int(e2.Duration)
	if minsSinceMidnight2 >= minsSinceMidnight1 {
		minsSinceMidnight1, minsSinceMidnight2 = minsSinceMidnight2, minsSinceMidnight1
		dur1, dur2 = dur2, dur1
	}
	return (minsSinceMidnight1+dur1 > minsSinceMidnight2) || (minsSinceMidnight2+dur2 > minsSinceMidnight1+24*60)
}

// most basic check if two timeWindows overlap
func windowsOverlap(w1, w2 timeWindow) bool {
	return w1.start.Equal(w2.start) || (w1.start.Before(w2.start) && w1.start.Add(w1.dur).After(w2.start)) || (w2.start.Before(w1.start) && w2.start.Add(w2.dur).After(w1.start))
}

// verifies a single timeWindow against a list of timeWindows
func windowOverlapsList(singleWindow timeWindow, windowList []timeWindow) bool {
	for _, w := range windowList {
		if windowsOverlap(singleWindow, w) {
			return true
		}
	}
	return false
}

// verifies a list of timeWindows against another list of timeWindows
func listOverlapsList(wList1, wList2 []timeWindow) bool {
	for _, w := range wList1 {
		if windowOverlapsList(w, wList2) {
			return true
		}
	}
	return false
}

// checks if a timeWindow is overlapped by the section of an infinite series that surrounds it
func singleWindowOverlapsInfiniteSeries(loneDuration timeWindow, infSeries *Event) bool {
	infSeries = infSeries.Copy() // need to deep copy so event's Repeat object does not get modified during this function's algorithm
	for {
		if infSeries.StartTime.After(loneDuration.start.Add(loneDuration.dur)) {
			return false
		}

		if !infSeries.HasException(infSeries.StartTime) {
			if windowsOverlap(loneDuration, infSeries.toTimeWindow()) {
				return true
			}
		}

		springForwardEvent, _ := infSeries.ShiftSeries()
		if springForwardEvent != nil {
			if springForwardEvent.StartTime.After(loneDuration.start.Add(loneDuration.dur)) {
				return false
			}

			if !infSeries.HasException(springForwardEvent.StartTime) {
				if windowsOverlap(loneDuration, infSeries.toTimeWindow()) {
					return true
				}
			}
		}
	}
}

// expands a finite series into individual timeWindows and checks each one against an infinite series
func finiteSeriesOverlapsInfiniteSeries(finSeries, infSeries *Event) bool {
	for _, e := range expandFiniteSeries(finSeries) {
		if singleWindowOverlapsInfiniteSeries(e, infSeries) {
			return true
		}
	}
	return false
}

// checks if an infinite daily series would overlap with another infinite daily series
func infDailyOverlapsInfDaily(ser1, ser2 *Event) bool {
	// Ns are different and neither is a multiple of the other, so the two series will appear on the same day and on adjacent days at some point
	areMultiples := (ser1.Repeat.Frequency%ser2.Repeat.Frequency == 0) || (ser2.Repeat.Frequency%ser1.Repeat.Frequency == 0)
	if ser1.Repeat.Frequency != ser2.Repeat.Frequency && !areMultiples {
		return clockTimesOverlap(ser1, ser2)
	}

	// else, Ns are same or one is multiple of other

	bigN := ser1.Copy()
	smallN := ser2.Copy()
	if smallN.Repeat.Frequency > bigN.Repeat.Frequency {
		bigN, smallN = smallN, bigN
	}

	for bigN.StartTime.Before(smallN.StartTime) {
		bigN.ShiftSeries()
	}
	for flextime.GetDayLength(bigN.StartTime).Hours() != 24 {
		bigN.ShiftSeries()
	}

	var springForwardEvent *Event
	for {
		if windowsOverlap(smallN.toTimeWindow(), bigN.toTimeWindow()) {
			return true
		}
		if smallN.StartTime.After(bigN.StartTime) {
			if springForwardEvent != nil {
				bigN.ShiftSeries()
				for flextime.GetDayLength(bigN.StartTime).Hours() != 24 {
					bigN.ShiftSeries()
				}
			} else {
				return false
			}
		}
		springForwardEvent, _ = smallN.ShiftSeries()
		for flextime.GetDayLength(smallN.StartTime).Hours() != 24 {
			springForwardEvent, _ = smallN.ShiftSeries()
			if smallN.StartTime.After(bigN.StartTime) {
				bigN.ShiftSeries()
				for flextime.GetDayLength(bigN.StartTime).Hours() != 24 {
					bigN.ShiftSeries()
				}
			}
		}
	}
}

// checks if an infinite daily series would overlap with an infinite weekly series
func infDailyOverlapsInfWeekly(dailySer, weeklySer *Event) bool {
	dailySplits := splitWeeklyByDays(weeklySer)
	for _, dailySplit := range dailySplits {
		if infDailyOverlapsInfDaily(dailySer, dailySplit) {
			return true
		}
	}
	return false
}
