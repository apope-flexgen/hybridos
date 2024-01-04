package main

import (
	"testing"
)

func TestEssGridFollowing(t *testing.T) {
	ess1 := ess{
		ID:    "ess_1",
		Cap:   1000,
		Soc:   50,
		Phigh: 500,
		Plow:  500,
		Oncmd: true,
	}
	input := terminal{
		v: 480,
	}
	ess1.UpdateMode(input)
	ess1.DistributeVoltage(input)
	ess1.UpdateState(input, 1)
	type testEntry struct {
		comment    string
		currentSoc float64
		powerCmd   float64
		deltaT     float64
		finalSoc   float64
		powerOut   float64
	}

	vector := []testEntry{
		testEntry{"one hour discharge, 1/2C", 100, 500, 3600, 50, 500},
		testEntry{"one hour discharge, 1/4C", 100, 250, 3600, 75, 250},
		testEntry{"one hour charge, 1/2C", 0, -500, 3600, 50, -500},
		testEntry{"one hour charge, 1/4C", 0, -250, 3600, 25, -250},
		testEntry{"empty discharge, 1/2C", 25, 500, 3600, 0, 250},
		testEntry{"fill up charge, 1/2C", 75, -500, 3600, 100, -250},
	}

	for i, test := range vector {
		ess1.Soc = test.currentSoc
		ess1.Pcmd = test.powerCmd
		ess1.CalculateState(input, test.deltaT)
		ess1.UpdateState(input, test.deltaT)
		if !(floatEq(ess1.Soc, test.finalSoc, 1) && floatEq(ess1.P, test.powerOut, 1)) {
			t.Log("ESS Grid Forming Test", i, test.comment)
			t.Logf("Got %.2f%% expected %.2f%%", ess1.Soc, test.finalSoc)
			t.Logf("Got %.2fkW expected %.2fkW", ess1.P, test.powerOut)
			t.Logf("%+v\n", ess1)
			t.Fail()
		}
	}
}
