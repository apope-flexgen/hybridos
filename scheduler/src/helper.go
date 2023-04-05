/**
 *
 * helper.go
 *
 * Miscellaneous helper functions for the rest of Scheduler to use.
 *
 */

package main

import (
	"errors"
	"fmt"
	"log"
	"os"
	"reflect"
	"strconv"
	"strings"
	"time"
)

// getCurrentTime is a func pointer that defaults to time.Now() but can be changed to point to another value for simulating test cases.
var getCurrentTime func() time.Time = time.Now

// Print a message if there is a fatal error and panic
func fatalErrorCheck(err error, message string) {
	if err != nil {
		log.Println("Fatal Error:", err)
		panic(message)
	}
}

// Verifies an IP address has 4 fields and ends with a port number
func checkIPAddrFormat(ip string) error {
	if strings.Count(ip, ":") != 1 {
		return fmt.Errorf("port formatting is invalid. Expected format: #.#.#.#:Port#")
	}
	if strings.Count(ip, ".") != 3 {
		return fmt.Errorf("ip address formatting is invalid. Expected format: #.#.#.#:Port#")
	}
	return nil
}

// extractIntThatMightBeFloat64 extracts a key's value from a map[string]interface{}
// and verifies it is an int or float64, then type casts it as an int.
// Returns error if either fails
func extractIntThatMightBeFloat64(m *map[string]interface{}, k string) (int, error) {
	// check that key is in map
	v, exists := (*m)[k]
	if !exists {
		return 0, fmt.Errorf("key %v not found in map", k)
	}

	// return typecasted int if value exists as float64
	valAsFloat, ok := v.(float64)
	if ok {
		return int(valAsFloat), nil
	}

	// return int if value exists as int
	valAsInt, ok := v.(int)
	if ok {
		return valAsInt, nil
	}

	// return error if value is neither float64 nor int
	return 0, fmt.Errorf("value is neither float64 nor int. Actual type is %v", reflect.TypeOf(v))
}

// extractKeys takes a map and returns a slice of strings matching the keys that the map has
func extractKeys(m map[string]interface{}) []string {
	keys := make([]string, 0)
	for k := range m {
		keys = append(keys, k)
	}
	return keys
}

// unwrapVariable asserts the passed-in interface as a map[string]interface{} then returns the value of the key 'value'.
// If the type assertion fails or the key 'value' is not found, the original interface{} is returned.
// Clothed or naked values can be passed to this func and the appropriate value will be returned.
func unwrapVariable(obj interface{}) interface{} {
	// check if value is clothed or not. either way, load payload into valueInterface for further processing
	m, ok := obj.(map[string]interface{})
	if ok {
		valueInterface, ok := m["value"]
		if ok {
			return valueInterface
		}
		return obj
	}
	return obj
}

// getLocalTimezone retrieves the name of the system's configured time zone and loads the corresponding time.Location
// ex: America/New_York, America/Chicago, America/Los_Angeles, etc.
func getLocalTimezone() (*time.Location, error) {
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
func getMidnightInNDays(days int, currentTime time.Time) time.Time {
	// Very special case to keep in mind for the logic in this function:
	// Some time zones, such as Cuba, start Daylight Saving Time at midnight. On a Spring Forward day in a time zone such as this,
	// the midnight hour will not exist. Using time.Date() with a 0 in the hours argument will return either 11pm on the previous
	// day or 1am on the DST day non-deterministically. 1am on the DST day would be the equivalent of midnight, so any time we use
	// time.Date() to get midnight, we check the Day() field of the result and add an hour if it does not match the Day() field of
	// the reference time (input to time.Date()) so that if 11pm is chosen by time.Date(), it will be forwarded to 1am.

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

// Converts minutes since midnight start_time to a timestamp
func convertIntToTimestamp(minsSinceMidnight int, dayIndex int, timeZone *time.Location) time.Time {
	// Get midnight on the date daysIndex days from now
	midnightOnDate := getMidnightInNDays(dayIndex, getCurrentTime().In(timeZone))
	// Add the minutes since midnight of the event and return
	return time.Date(midnightOnDate.Year(), midnightOnDate.Month(), midnightOnDate.Day(), 0, minsSinceMidnight, 0, 0, midnightOnDate.Location())
}

// Casts interface 2 as type of interface 1, then compares them. Compares primitive types only
// TODO: move to go_flexgen for use by other modules
func interfaceEquals(i1 interface{}, i2 interface{}) bool {
	switch i1 := i1.(type) {
	case bool:
		return compareBoolToInterface(i1, i2)
	case int:
		return compareIntToInterface(i1, i2)
	case float32:
		return compareFloat32ToInterface(i1, i2)
	case float64:
		return compareFloat64ToInterface(i1, i2)
	case string:
		return compareStringToInterface(i1, i2)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given bool
func compareBoolToInterface(b bool, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return b == i
	case int:
		return b == (i != 0)
	case float32:
		return b == (i != 0.0)
	case float64:
		return b == (i != 0.0)
	case string:
		return b == (i != "")
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given int
func compareIntToInterface(n int, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (n == 0 && !i) || (n != 0 && i)
	case int:
		return n == i
	case float32:
		return n == int(i)
	case float64:
		return n == int(i)
	case string:
		iVal, err := strconv.Atoi(i)
		return (err == nil) && (n == iVal)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float32
func compareFloat32ToInterface(f float32, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (f == 0.0 && !i) || (f != 0.0 && i)
	case int:
		return f == float32(i)
	case float32:
		return f == i
	case float64:
		return f == float32(i)
	case string:
		iVal, err := strconv.ParseFloat(i, 32)
		return (err == nil) && (f == float32(iVal))
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float64
func compareFloat64ToInterface(f float64, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (f == 0.0 && !i) || (f != 0.0 && i)
	case int:
		return f == float64(i)
	case float32:
		return f == float64(i)
	case float64:
		return f == i
	case string:
		iVal, err := strconv.ParseFloat(i, 64)
		return (err == nil) && (f == iVal)
	default:
		return false
	}
}

// Static type must be known at compile time, so dynamic type comparison is not possible
// Cast interface based on its type and compare to the given float64
func compareStringToInterface(s string, i interface{}) bool {
	switch i := i.(type) {
	case bool:
		return (s == "" && !i) || (s != "" && i)
	case int:
		return s == strconv.Itoa(i)
	case float32:
		// Could use sprintf to convert the interface to string, but then precision leaves ambiguity in string to string comparison
		iVal, err := strconv.ParseFloat(s, 32)
		return (err == nil) && (i == float32(iVal))
	case float64:
		// Could use sprintf to convert the interface to string, but then precision leaves ambiguity in string to string comparison
		iVal, err := strconv.ParseFloat(s, 64)
		return (err == nil) && (i == float64(iVal))
	case string:
		return s == i
	default:
		return false
	}
}
