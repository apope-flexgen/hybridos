package main

import (
	"testing"
)

func TestPCSOnOff(t *testing.T) {
	pcs := pcs{
		ID:   "pcs",
	}
	input := terminal{
		v: 480,
	}

	pcs.Init()

	pcs.Oncmd = true

	pcs.UpdateMode(input)
	ess1.DistributeVoltage(input)
	ess1.UpdateState(input, 1)

	if pcs.Vac != input.v {
		t.Errorf("pcs voltage incorrect, got %f but expected %f", pcs.Vac, input.v)
	}
}
