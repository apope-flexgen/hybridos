package main

import (
	"testing"
)

func TestGetIntegral(t *testing.T) {

	type testEntry struct {
		comment       string
		accumulateIn  float64
		rateIn        float64
		deltaT        float64
		minimum       float64
		maximum       float64
		accumulateOut float64
		rateOut       float64
		under         bool
		over          bool
	}

	vector := []testEntry{
		testEntry{"zero test", 0, 0, 0, 0, 0, 0, 0, false, false},
		testEntry{"normal, no limit", 1, 4, .5, 0, 5, 3, 4, false, false},
		testEntry{"negative, no limit", 1, -4, .5, -5, 5, -1, -4, false, false},
		testEntry{"normal, max limit", 1, 4, 3, 0, 7, 7, 2, false, true},
		testEntry{"normal, min limit", 1, -4, 4, 0, 7, 0, -0.25, true, false},
	}

	for i, entry := range vector {
		accumulateOut, rateOut, under, over := getIntegral(entry.accumulateIn, entry.rateIn, entry.deltaT, entry.minimum, entry.maximum)
		if !(accumulateOut == entry.accumulateOut && rateOut == entry.rateOut && under == entry.under && over == entry.over) {
			t.Log("getIntegral Test", i, entry.comment)
			t.Logf("Got %.2f accumulateOut %.2f rateOut, expected %.2f accumulateOut %.2f rateOut", accumulateOut, rateOut, entry.accumulateOut, entry.rateOut)
			t.Logf("Got %t under %t over,  expected %t under %t over", under, over, entry.under, entry.over)
			t.Fail()
		}
	}
}
