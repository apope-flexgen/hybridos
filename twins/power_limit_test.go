package main

import (
	"testing"
)

func TestGetPowerLimit(t *testing.T){

	type testEntry struct {
		comment string
		apparentPower float64
		domPower float64
		subPower float64
		domOut float64
		subOut float64
	}

	vector := []testEntry {
		testEntry{"positive, positive no limit", 0, 0, 0, 0, 0},
		testEntry{"negative, negative no limit", 6, 3, 4, 3, 4},
		testEntry{"negative, positive no limit", 6, -3, 4, -3, 4},
		testEntry{"positive, negative no limit", 6, 3, -4, 3, -4},
		testEntry{"positive dom limit", 5, 8, 4, 5, 0},
		testEntry{"negative dom limit", 5, -8, 4, -5, 0},
		testEntry{"positive sub limit", 5, 4, 4, 4, 3},
		testEntry{"negative sub limit", 5, 4, -4, 4, -3},
		testEntry{"positive high sub limit", 5, 4, 8, 4, 3},
		testEntry{"negative high sub limit", 5, 4, -8, 4, -3},
	}
	
	for i, entry := range vector {
		dom, sub := getPowerLimit(entry.apparentPower, entry.domPower, entry.subPower)
		if !(dom == entry.domOut && sub == entry.subOut) {
			t.Log("getPowerLimit Test", i, entry.comment)
			t.Logf("Got %.2f dom, %.2f sub, expected %2f dom %2f sub", dom, sub, entry.domOut, entry.subOut)
			t.Fail()
		}
	}	
}