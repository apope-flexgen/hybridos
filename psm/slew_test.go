package main

import (
	"testing"
)

func TestGetSlew(t *testing.T){
	//far under

	type testEntry struct {
		comment string
		currentValue float64
		targetValue float64
		slewRate float64
		deltaT float64
		nextValue float64
	}

	vector := []testEntry {
		testEntry{"far under", 1, 50, 5, 2, 11},
		testEntry{"far over", 101, 50, 5, 2, 91},
		testEntry{"equal", 50, 50, 5, 2, 50},
		testEntry{"near under", 48, 50, 5, 2, 50},
		testEntry{"near over", 52, 50, 5, 2, 50},
		testEntry{"negative far over", -1, -50, 5, 2, -11},
		testEntry{"negative far under", -101, -50, 5, 2, -91},
		testEntry{"negative equal", -50, -50, 5, 2, -50},
		testEntry{"negative near over", -48, -50, 5, 2, -50},
		testEntry{"negative near under", -52, -50, 5, 2, -50},
		testEntry{"negative to positive", -52, 50, 5, 14, 18},
		testEntry{"positive to negative", 52, -50, 5, 14, -18},
		testEntry{"negative near over", 52, 10, 0.3, 35, 41.5},
		testEntry{"negative near under", 52, 10, 5, 0.7, 48.5},
	}
	
	for i, entry := range vector {
		nextValue := getSlew(entry.currentValue, entry.targetValue, entry.slewRate, entry.deltaT)
		if !(nextValue == entry.nextValue) {
			t.Log("getSlew Test", i, entry.comment)
			t.Logf("Got %.2f expected %.2f", nextValue, entry.nextValue)
			t.Fail()
		}
	}	
}