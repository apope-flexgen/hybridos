package main

import (
	"testing"
)

func TestEssSlew(t *testing.T) {
	//far under

	type testEntry struct {
		AbsRampRate       bool
		powerInitialState bool
		oncmd             bool
		offcmd            bool

		p          float64
		Q          float64
		targetPcmd float64
		targetQcmd float64

		Pramprise  float64
		Prampdrop  float64
		Prampstart float64
		Prampstop  float64
		deltaT     float64

		PRampDropEnable  bool
		PRampRiseEnable  bool
		PRampStartEnable bool
		PRampStopEnable  bool
		RampEnable       bool

		QRampStartEnable bool
		QRampRiseEnable  bool
		QRampDropEnable  bool
		QRampStopEnable  bool

		Qrampstart float64
		Qramprise  float64
		Qrampdrop  float64
		Qrampstop  float64

		PexpectedResult float64
		QexpectedResult float64
		comment         string
	}

	var e ess          // instance of the ess class
	var input terminal // dummy input terminal
	vector := []testEntry{
		// test cases for Active power Ramp Rate testing
		testEntry{true, true, true, false, 2000, 0, 1000, 0, 2.0, 1.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1966.67, 0.00, "Ramp Drop Test		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 2000, 0, 1000, 0, 1.0, 3.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1000.00, 0.00, "Ramp Drop Test using Ramp Rise Rate		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 2000, 0, 1000, 0, 1.0, 1.0, 0.0, 0.0, 3.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1950.00, 0.00, "Ramp Drop Test with different deltaT		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 2000, 0, 1000, 0, 2.0, 1.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1966.67, 0.00, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 10000, 0, 1000, 0, 5.0, 5.0, 0.0, 0.0, 4.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 5000.00, 0.00, "Ramp Drop Test with a large value		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 1000, 0, 2000, 0, 1.0, 5.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1033.33, 0.00, "Ramp Rise Test		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 1000, 0, 2000, 0, 1.0, 5.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 2000.00, 0.00, "Ramp Rise Test using Ramp Drop Rate		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 1000, 0, 2000, 0, 1.0, 1.0, 0.0, 0.0, 3.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1050.00, 0.00, "Ramp Rise Test with different deltaT		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 1000, 0, 2000, 0, 1.0, 2.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1033.33, 0.00, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active		Active power Ramp Rate testing"},
		testEntry{true, true, true, false, 1000, 0, 10000, 0, 5.0, 5.0, 0.0, 0.0, 30.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 3500, 0.00, "Ramp Rise Test with a large value		Active power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, false, false, true, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test		Active power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 66.67, 0.00, "Ramp Start Test using Ramp Rise Rate		Active power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1000.00, 0.00, "Ramp Start Test using Ramp Drop Rate		Active power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 2000, 0, 3.0, 3.0, 5.0, 0.0, 2.0, true, true, true, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test with both Ramp Rise and Ramp Drop Rates active		Active power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 2000, 0, 3.0, 3.0, 5.0, 1.0, 2.0, true, true, true, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active		Active power Ramp Rate testing"},
		testEntry{true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, false, false, false, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test		Active power Ramp Rate testing"},
		testEntry{true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.00, 0.00, "Ramp Stop Test using Ramp Rise Rate		Active power Ramp Rate testing"},
		testEntry{true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.00, 0.00, "Ramp Stop Test using Ramp Drop Rate		Active power Ramp Rate testing"}, //investigate this
		testEntry{true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, true, false, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test with both Ramp Rise and Ramp Drop Rates active		Active power Ramp Rate testing"},
		testEntry{true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, true, true, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates		Active power Ramp Rate testing"},
		// test cases for Reactive power Ramp Rate testing
		testEntry{true, true, true, false, 0, 2000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 0.00, 1833.33, "Ramp Drop Test		Reactive power Ramp Rate testing"},
		testEntry{true, true, true, false, 0, 2000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, true, true, false, 0.0, 3.0, 5.0, 0.0, 0.00, 1833.33, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active		Reactive power Ramp Rate testing"},
		testEntry{true, true, true, false, 0, 10000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 4.0, false, false, false, false, false, false, false, true, false, 0.0, 0.0, 10.0, 0.0, 0.00, 5000.00, "Ramp Drop Test with a large value		Reactive power Ramp Rate testing"},
		testEntry{true, true, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 0.00, 1066.67, "Ramp Rise Test		Reactive power Ramp Rate testing"},
		testEntry{true, true, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, false, false, 0.0, 2.0, 3.0, 0.0, 0.00, 1066.67, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active		Reactive power Ramp Rate testing"},
		testEntry{true, true, true, false, 0, 1000, 0, 10000, 0.0, 0.0, 0.0, 0.0, 30.0, false, false, false, false, false, false, true, false, false, 0.0, 3.0, 0.0, 0.0, 0.00, 2500.00, "Ramp Rise Test with a large value		Reactive power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, false, false, false, 5.0, 2.0, 0.0, 0.0, 0.00, 1166.67, "Ramp Start Test		Reactive power Ramp Rate testing"},
		testEntry{true, false, true, false, 0, 0, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 0.00, 166.67, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active		Reactive power Ramp Rate testing"},
		testEntry{true, true, false, true, 0, 1000, 0, 0, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 0.00, 933.33, "Ramp Stop Test		Reactive power Ramp Rate testing"},
		testEntry{true, true, false, true, 0, 2000, 0, 0, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, true, true, 2.0, 3.0, 3.0, 2.0, 0.00, 1933.33, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates		Reactive power Ramp Rate testing"},
		// test cases with both Reactive power ramping active and Active power ramping active
		testEntry{true, true, true, false, 2000, 2000, 1000, 1000, 0.0, 2.0, 0.0, 0.0, 2.0, true, false, false, false, true, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 1933.33, 1833.33, "Ramp Drop Test		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, true, false, 2000, 2000, 1000, 1000, 5.0, 2.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, true, true, false, 0.0, 3.0, 5.0, 0.0, 1933.33, 1833.33, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, true, false, 10000, 10000, 1000, 1000, 0.0, 15.0, 0.0, 0.0, 4.0, true, false, false, false, false, false, false, true, false, 0.0, 0.0, 10.0, 0.0, 5000.00, 5000.00, "Ramp Drop Test with a large value		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, true, false, 1000, 1000, 2000, 2000, 5.0, 0.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 1166.67, 1066.67, "Ramp Rise Test		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, true, false, 1000, 1000, 2000, 2000, 5.0, 3.0, 0.0, 0.0, 2.0, true, true, false, false, false, true, true, false, false, 0.0, 2.0, 3.0, 0.0, 1166.67, 1066.67, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, true, false, 1000, 1000, 10000, 10000, 10.0, 0.0, 0.0, 0.0, 30.0, false, true, false, false, false, false, true, false, false, 0.0, 3.0, 0.0, 0.0, 5000.00, 2500.00, "Ramp Rise Test with a large value		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, false, true, false, 1000, 1000, 2000, 2000, 0.0, 0.0, 3.0, 0.0, 2.0, false, false, true, false, false, true, false, false, false, 5.0, 2.0, 0.0, 0.0, 1100.00, 1166.67, "Ramp Start Test		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, false, true, false, 0, 0, 1000, 1000, 2.0, 3.0, 4.0, 5.0, 2.0, true, true, true, true, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 133.33, 166.67, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, false, true, 1000, 1000, 0, 0, 0.0, 0.0, 0.0, 5.0, 2.0, false, false, false, true, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 833.33, 933.33, "Ramp Stop Test		both Reactive power ramping active and Active power ramping active"},
		testEntry{true, true, false, true, 2000, 2000, 0, 0, 2.0, 3.0, 2.0, 2.0, 2.0, true, true, true, true, false, true, true, true, true, 2.0, 3.0, 3.0, 2.0, 1933.33, 1933.33, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates		both Reactive power ramping active and Active power ramping active"},

		// test cases for percentage maximum power per second conversion
		testEntry{false, false, true, false, 0, 0, 1000, 1000, 2.0, 3.0, 4.0, 5.0, 2.0, true, true, true, true, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 400.00, 500.00, "Ramp Start Test		percentage maximum power per second conversion"},
		testEntry{false, true, true, false, 1000, 1000, 2000, 2000, 5.0, 0.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 1500.00, 1200.00, "Ramp Rise Test		percentage maximum power per second conversion"},
		testEntry{false, true, true, false, 2000, 2000, 1000, 1000, 0.0, 2.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 1800.00, 1500.00, "Ramp Drop Test		percentage maximum power per second conversion"},
		testEntry{false, true, false, true, 1000, 1000, 0, 0, 0.0, 0.0, 0.0, 5.0, 2.0, false, false, false, true, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 500.00, 800.00, "Ramp Stop Test		percentage maximum power per second conversion"},
	}

	// initialize values for testing on pcs
	for _, entry := range vector {
		e.AbsRampRate = entry.AbsRampRate
		e.On = entry.powerInitialState
		e.Oncmd = entry.oncmd
		e.Offcmd = entry.offcmd
		e.Soc = 50
		e.Phigh = 5000
		e.Qhigh = 5000
		e.Pcharge = e.Phigh
		e.Pdischarge = e.Phigh
		e.Cap = 2000
		e.AcContactorCloseCmd = true
		e.DcContactorCloseCmd = true
		e.AcContactor = entry.powerInitialState
		e.DcContactor = entry.powerInitialState
		e.GridForming = false
		e.P = entry.p
		e.Q = entry.Q
		e.Pcmd = entry.targetPcmd
		e.Qcmd = entry.targetQcmd

		// these guys
		e.PRampRise = entry.Pramprise
		e.PRampDrop = entry.Prampdrop
		e.PRampStart = entry.Prampstart
		e.PRampStop = entry.Prampstop

		e.PRampDropEnable = entry.PRampDropEnable
		e.PRampRiseEnable = entry.PRampRiseEnable
		e.PRampStartEnable = entry.PRampStartEnable
		e.PRampStopEnable = entry.PRampStopEnable

		// us too
		e.RampEnable = entry.RampEnable // !!
		e.QRampStart = entry.Qrampstart
		e.QRampRise = entry.Qramprise
		e.QRampDrop = entry.Qrampdrop
		e.QRampStop = entry.Qrampstop

		e.QRampStartEnable = entry.QRampStartEnable
		e.QRampRiseEnable = entry.QRampRiseEnable
		e.QRampDropEnable = entry.QRampDropEnable
		e.QRampStopEnable = entry.QRampStopEnable
		e.QstopActive = false
		e.QstartActive = false

		// fmt.Println("\n Test: ", entry.comment)
		(&e).Init()
		if entry.p > entry.targetPcmd {
			input.p = entry.p
		} else {
			input.p = entry.targetPcmd
		}

		_ = (&e).UpdateMode(input)
		// fmt.Println("[After UpdateMode]", "e.on", e.On, "e.AcContactor", e.AcContactor, "e.DcContactor", e.DcContactor)
		_ = (&e).CalculateState(input, entry.deltaT)
		_ = (&e).UpdateState(input, entry.deltaT)
		if !(entry.PexpectedResult*1.01 >= e.P && entry.PexpectedResult*0.99 <= e.P) {
			t.Log("Incorrect Ramprate used")
			t.Logf("Got %.2f expected %.2f", e.P, entry.PexpectedResult)
			t.Fail()
		}

		if !(entry.QexpectedResult*1.01 >= e.Q && entry.QexpectedResult*0.99 <= e.Q) {
			t.Log("Incorrect Ramprate used")
			t.Logf("Got %.2f expected %.2f", e.Q, entry.QexpectedResult)
			t.Fail()
		}
	}
}
