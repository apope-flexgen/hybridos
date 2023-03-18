package main

import (
	"fmt"
	"math"
	"testing"
	"time"
)

func TestParseUnixMicro(t *testing.T) {
	// map from filename and extension to expected parsed time
	testTimes := map[[2]string]time.Time{
		{"", ""}:                           {},
		{"test_archive.tar.gz", ".tar.gz"}: {},
		{"test_archive-1673465027557700.tar.gz", ".tar.gz"}:                  time.UnixMicro(1673465027557700),
		{"test_archive-0.tar.gz", ".tar.gz"}:                                 time.UnixMicro(0),
		{"test_archive-" + fmt.Sprint(math.MaxInt64) + ".tar.gz", ".tar.gz"}: time.UnixMicro(math.MaxInt64),
		{"test_1673465027557700_archive-1734981827779410.tar.gz", ".tar.gz"}: time.UnixMicro(1734981827779410),
		{"test_1" + fmt.Sprint(math.MaxInt64) + ".tar.gz", ".tar.gz"}:        {},
		{"test_archive-1257894000000000.other", ".other"}:                    time.UnixMicro(1257894000000000),
	}

	for params, expectedTime := range testTimes {
		// we only expect the time to be an empty struct upon not finding a timestamp
		expectedFound := expectedTime != time.Time{}
		timeParsed, found := parseUnixMicro(params[0], params[1])
		if found != expectedFound {
			t.Errorf("Parse result found was %v but expected found was %v for test case params %v", found, expectedFound, params)
		}
		if timeParsed != expectedTime {
			t.Errorf("Parsed time was %v but expected time was %v for test case params %v", timeParsed, expectedTime, params)
		}
	}
}
