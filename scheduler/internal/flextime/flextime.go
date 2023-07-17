// Package flextime provides helper functions dealing with time.
//
// The functions are pure enough to be able to be separated from the application that uses them.
package flextime

import (
	"errors"
	"fmt"
	"os"
	"strings"
	"time"
)

// Func pointer that defaults to time.Now() but can be changed to point to another function for simulating test cases.
var Now func() time.Time = time.Now

// Retrieves the name of the system's configured time zone and loads the corresponding time.Location.
// ex: America/New_York, America/Chicago, America/Los_Angeles, etc.
func GetLocalTimezone() (*time.Location, error) {
	// /etc/localtime is a symlink that points to the file path containing the time zone data that the system is using
	// the file path contains the name of the time zone in the last two fragments, and os.Readlink can get the file path
	localtime, err := os.Readlink("/etc/localtime")
	if err != nil {
		return nil, err
	}

	// get the last two fragments, which should be the time zone name
	pathFrags := strings.Split(localtime, "/")
	if len(pathFrags) < 2 {
		return nil, errors.New("invalid time zone string parsed from output of os.Readlink")
	}

	// format the two fragments together into the time zone name and return
	name := fmt.Sprintf("%v/%v", pathFrags[len(pathFrags)-2], pathFrags[len(pathFrags)-1])

	// load the corresponding time.Location
	loc, err := time.LoadLocation(name)
	if err != nil {
		return nil, fmt.Errorf("failed to load *time.Location from time zone name %s: %w", name, err)
	}
	return loc, nil
}

// Creates a timestamp that is midnight on the given number of days from the reference time provided.
// Positive, negative, and 0 are all allowed for the days argument.
func GetMidnightInNDays(days int, currentTime time.Time) time.Time {
	// Very special case to keep in mind for the logic in this function:
	// Some time zones (Cuba, Lebanon, Paraguay) start Daylight Saving Time at midnight. On a Spring Forward day in a time zone
	// such as this, the midnight hour will not exist. Using time.Date() with a 0 in the hours argument will return either 11pm
	// on the previous day or 1am on the DST day non-deterministically. 1am on the DST day would be the equivalent of midnight,
	// so any time we use time.Date() to get midnight, we check the Day() field of the result and add an hour if it does not
	// match the Day() field of the reference time (input to time.Date()) so that if 11pm is chosen by time.Date(), it will be
	// forwarded to 1am.

	// get timestamp for today's midnight
	midnightToday := time.Date(currentTime.Year(), currentTime.Month(), currentTime.Day(), 0, 0, 0, 0, currentTime.Location())
	if midnightToday.Day() != currentTime.Day() { // special case check
		midnightToday = midnightToday.Add(time.Hour)
	}

	// the following logic will set morningNDaysFromNow to be on the correct day at either midnight, 1am, or 2am (based on Daylight Saving Time).
	// if positive days, move 24*(days-1)+25 hours into the future so that if there is a long day due to DST, we get all the way through it.
	// if negative days, move 24*(days+1)-23 hours into the past so that if there is a short day due to DST, we do not go past it.
	var morningNDaysFromNow time.Time
	if days > 0 {
		morningNDaysFromNow = midnightToday.Add(time.Duration(24*(days-1))*time.Hour + time.Hour*25)
	} else if days < 0 {
		morningNDaysFromNow = midnightToday.Add(time.Duration(24*(days+1))*time.Hour - time.Hour*23)
	} else {
		return midnightToday
	}

	// now that we are definitely in the right day, set hour to 0 to get midnight
	midnightNDaysFromNow := time.Date(morningNDaysFromNow.Year(), morningNDaysFromNow.Month(), morningNDaysFromNow.Day(), 0, 0, 0, 0, morningNDaysFromNow.Location())
	if midnightNDaysFromNow.Day() != morningNDaysFromNow.Day() { // special case check
		midnightNDaysFromNow = midnightNDaysFromNow.Add(time.Hour)
	}
	return midnightNDaysFromNow
}

// Returns the duration of the day in which the reference timestamp falls.
func GetDayLength(ref time.Time) time.Duration {
	return GetMidnightInNDays(1, ref).Sub(GetMidnightInNDays(0, ref))
}

// Returns if the given time is before the current minute. Ignores seconds and nanoseconds.
func TimeIsBeforeCurrentMinute(t time.Time) bool {
	currentMinute := Now().In(t.Location()).Truncate(time.Minute)
	return t.Before(currentMinute)
}

// Returns a timestamp that has the same date as the newDate time.Time but the same clock time as the originalTime time.Time,
// UNLESS the new date is a Daylight Saving Time day that skips an hour and the original time is during that skipped hour.
// In that case, the returned time will still have the same date as the newDate time.Time but has a clock time one hour ahead
// of the originalTime time.Time.
//
// The locations of originalTime and newDate should ideally already be the same. However, if they are not,
// the new time will be given the original time's location.
func ApplyNewDateToTime(newDate, originalTime time.Time) (newTime time.Time) {
	newTime = time.Date(newDate.Year(), newDate.Month(), newDate.Day(), originalTime.Hour(), originalTime.Minute(), originalTime.Second(), originalTime.Nanosecond(), originalTime.Location())
	if newTime.Hour() != originalTime.Hour() {
		springForwardAdjustedTime := originalTime.Add(time.Hour)
		newTime = time.Date(newDate.Year(), newDate.Month(), newDate.Day(), springForwardAdjustedTime.Hour(), springForwardAdjustedTime.Minute(), springForwardAdjustedTime.Second(), springForwardAdjustedTime.Nanosecond(), springForwardAdjustedTime.Location())
	}
	return newTime
}
