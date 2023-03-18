package main

import (
	"fmt"
	"testing"
)

/*	to run tests:
	go test -cover -coverprofile=c.out -run bms_test.go

	to get coverage info:
	go tool cover -html=c.out -o coverage.html
*/

func TestBMSOnOff(t *testing.T) {
	bms := bms{
		ID:   "bms",
		Cap:  1000,
		Vnom: 990,
		Soc:  50,
	}
	input := terminal{
		v: 990,
	}

	bms.Init()

	bms.Vdc = input.v
	bms.Oncmd = true

	bms.UpdateMode(input)
	ess1.DistributeVoltage(input)
	ess1.UpdateState(input, 1)

	if bms.Vdc != bms.Vnom {
		t.Errorf("BMS voltage incorrect, got %f but expected %f", bms.Vdc, bms.Vnom)
	}
	fmt.Printf("bms.soc %f\n", bms.Soc)
}
