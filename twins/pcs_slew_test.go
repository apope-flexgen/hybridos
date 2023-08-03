package main

import (
	"testing"
	// "fmt"
)

func TestPcsSlew(t *testing.T) {
	//far under

	type testEntry struct {
		AbsRampRate		  bool
		powerInitialState bool
		oncmd			  bool
		offcmd			  bool

		Pdc 			  float64
		Q				  float64
		targetPcmd  	  float64
		targetQcmd		  float64

		Pramprise     	  float64
		Prampdrop	 	  float64
		Prampstart	 	  float64
		Prampstop	 	  float64
		deltaT       	  float64

		PRampDropEnable   bool
		PRampRiseEnable   bool
		PRampStartEnable  bool
		PRampStopEnable   bool
		RampEnable 		  bool

		QRampStartEnable  bool
		QRampRiseEnable   bool
		QRampDropEnable   bool
		QRampStopEnable   bool

		Qrampstart		  float64
		Qramprise		  float64
		Qrampdrop		  float64
		Qrampstop 		  float64

		PexpectedResult	  float64
		QexpectedResult	  float64
		comment      	  string
	}

	var p pcs // instance of the pcs class
	var input terminal // dummy input terminal
	vector := []testEntry{
		// test cases for Active power Ramp Rate testing
		testEntry{ true, true, true, false, 2000, 0, 1000, 0, 2.0, 1.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1966.67, 0.00, "Ramp Drop Test"},
		testEntry{ true, true, true, false, 2000, 0, 1000, 0, 1.0, 3.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1000.00, 0.00, "Ramp Drop Test using Ramp Rise Rate"},
		testEntry{ true, true, true, false, 2000, 0, 1000, 0, 1.0, 1.0, 0.0, 0.0, 3.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1950.00, 0.00, "Ramp Drop Test with different deltaT"},
		testEntry{ true, true, true, false, 2000, 0, 1000, 0, 2.0, 1.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1966.67, 0.00, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 10000, 0, 1000, 0, 5.0, 5.0, 0.0, 0.0, 4.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 5000.00, 0.00, "Ramp Drop Test with a large value"},
		testEntry{ true, true, true, false, 1000, 0, 2000, 0, 1.0, 5.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1033.33, 0.00, "Ramp Rise Test"},
		testEntry{ true, true, true, false, 1000, 0, 2000, 0, 1.0, 5.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 2000.00, 0.00, "Ramp Rise Test using Ramp Drop Rate"},
		testEntry{ true, true, true, false, 1000, 0, 2000, 0, 1.0, 1.0, 0.0, 0.0, 3.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1050.00, 0.00, "Ramp Rise Test with different deltaT"},
		testEntry{ true, true, true, false, 1000, 0, 2000, 0, 1.0, 2.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1033.33, 0.00, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 1000, 0, 10000, 0, 5.0, 5.0, 0.0, 0.0, 30.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 3500, 0.00, "Ramp Rise Test with a large value"}, 
		testEntry{ true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, false, false, true, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test"},
		testEntry{ true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 66.67, 0.00, "Ramp Start Test using Ramp Rise Rate"},
		testEntry{ true, false, true, false, 0, 0, 1000, 0, 2.0, 2.0, 5.0, 0.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1000.00, 0.00, "Ramp Start Test using Ramp Drop Rate"},
		testEntry{ true, false, true, false, 0, 0, 2000, 0, 3.0, 3.0, 5.0, 0.0, 2.0, true, true, true, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test with both Ramp Rise and Ramp Drop Rates active",},
		testEntry{ true, false, true, false, 0, 0, 2000, 0, 3.0, 3.0, 5.0, 1.0, 2.0, true, true, true, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 166.67, 0.00, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active"},
		testEntry{ true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, false, false, false, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test"},
		testEntry{ true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, false, true, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.00, 0.00, "Ramp Stop Test using Ramp Rise Rate"},
		testEntry{ true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, false, false, false, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 0.00, 0.00, "Ramp Stop Test using Ramp Drop Rate"}, //investigate this
		testEntry{ true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, true, false, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, false, true, 2000, 0, 0, 0, 3.0, 3.0, 1.0, 5.0, 2.0, true, true, true, true, false, false, false, false, false, 0.0, 0.0, 0.0, 0.0, 1833.33, 0.00, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates"},
		// test cases for Reactive power Ramp Rate testing
		testEntry{ true, true, true, false, 0, 2000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 0.00, 1833.33, "Ramp Drop Test"},
		testEntry{ true, true, true, false, 0, 2000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, true, true, false, 0.0, 3.0, 5.0, 0.0, 0.00, 1833.33, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 0, 10000, 0, 1000, 0.0, 0.0, 0.0, 0.0, 4.0, false, false, false, false, false, false, false, true, false, 0.0, 0.0, 10.0, 0.0, 0.00, 5000.00, "Ramp Drop Test with a large value"},
		testEntry{ true, true, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 0.00, 1066.67, "Ramp Rise Test"},
		testEntry{ true, true, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, false, false, 0.0, 2.0, 3.0, 0.0, 0.00, 1066.67, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 0, 1000, 0, 10000, 0.0, 0.0, 0.0, 0.0, 30.0, false, false, false, false, false, false, true, false, false, 0.0, 3.0, 0.0, 0.0, 0.00, 2500.00, "Ramp Rise Test with a large value"}, 
		testEntry{ true, false, true, false, 0, 1000, 0, 2000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, false, false, false, 5.0, 2.0, 0.0, 0.0, 0.00, 1166.67, "Ramp Start Test"},
		testEntry{ true, false, true, false, 0, 0, 0, 1000, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 0.00, 166.67, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active"},
		testEntry{ true, true, false, true, 0, 1000, 0, 0, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 0.00, 933.33, "Ramp Stop Test"},
		testEntry{ true, true, false, true, 0, 2000, 0, 0, 0.0, 0.0, 0.0, 0.0, 2.0, false, false, false, false, false, true, true, true, true, 2.0, 3.0, 3.0, 2.0, 0.00, 1933.33, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates"},
		// test cases with both Reactive power ramping active and Active power ramping active
		testEntry{ true, true, true, false, 2000, 2000, 1000, 1000, 0.0, 2.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 1933.33, 1833.33, "Ramp Drop Test"},
		testEntry{ true, true, true, false, 2000, 2000, 1000, 1000, 5.0, 2.0, 0.0, 0.0, 2.0, true, true, false, false, false, false, true, true, false, 0.0, 3.0, 5.0, 0.0, 1933.33, 1833.33, "Ramp Drop Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 10000, 10000, 1000, 1000, 0.0, 15.0, 0.0, 0.0, 4.0, true, false, false, false, false, false, false, true, false, 0.0, 0.0, 10.0, 0.0, 5000.00, 5000.00, "Ramp Drop Test with a large value"},
		testEntry{ true, true, true, false, 1000, 1000, 2000, 2000, 5.0, 0.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 1166.67, 1066.67, "Ramp Rise Test"},
		testEntry{ true, true, true, false, 1000, 1000, 2000, 2000, 5.0, 3.0, 0.0, 0.0, 2.0, true, true, false, false, false, true, true, false, false, 0.0, 2.0, 3.0, 0.0, 1166.67, 1066.67, "Ramp Rise Test with both Ramp Rise and Ramp Drop Rates active"},
		testEntry{ true, true, true, false, 1000, 1000, 10000, 10000, 10.0, 0.0, 0.0, 0.0, 30.0, false, true, false, false, false, false, true, false, false, 0.0, 3.0, 0.0, 0.0, 5000.00, 2500.00, "Ramp Rise Test with a large value"}, 
		testEntry{ true, false, true, false, 1000, 1000, 2000, 2000, 0.0, 0.0, 3.0, 0.0, 2.0, false, false, true, false, false, true, false, false, false, 5.0, 2.0, 0.0, 0.0, 1100.00, 1166.67, "Ramp Start Test"},
		testEntry{ true, false, true, false, 0, 0, 1000, 1000, 2.0, 3.0, 4.0, 5.0, 2.0, true, true, true, true, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 133.33, 166.67, "Ramp Start Test with Ramp Stop, Ramp Start, and Ramp Drop Rates active"},
		testEntry{ true, true, false, true, 1000, 1000, 0, 0, 0.0, 0.0, 0.0, 5.0, 2.0, false, false, false, true, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 833.33, 933.33, "Ramp Stop Test"},
		testEntry{ true, true, false, true, 2000, 2000, 0, 0, 2.0, 3.0, 2.0, 2.0, 2.0, true, true, true, true, false, true, true, true, true, 2.0, 3.0, 3.0, 2.0, 1933.33, 1933.33, "Ramp Stop Test with Ramp Start, Ramp Rise, and Ramp Drop Rates"},

		// test cases for percentage maximum power per second conversion
		testEntry{ false, false, true, false, 0, 0, 1000, 1000, 2.0, 3.0, 4.0, 5.0, 2.0, true, true, true, true, false, true, true, true, true, 5.0, 2.0, 3.0, 8.0, 400.00, 500.00, "Ramp Start Test"},
		testEntry{ false, true, true, false, 1000, 1000, 2000, 2000, 5.0, 0.0, 0.0, 0.0, 2.0, false, true, false, false, false, false, true, false, false, 0.0, 2.0, 0.0, 0.0, 1500.00, 1200.00, "Ramp Rise Test"},
		testEntry{ false, true, true, false, 2000, 2000, 1000, 1000, 0.0, 2.0, 0.0, 0.0, 2.0, true, false, false, false, false, false, false, true, false, 0.0, 0.0, 5.0, 0.0, 1800.00, 1500.00, "Ramp Drop Test"},
		testEntry{ false, true, false, true, 1000, 1000, 0, 0, 0.0, 0.0, 0.0, 5.0, 2.0, false, false, false, true, false, false, false, false, true, 0.0, 0.0, 0.0, 2.0, 500.00, 800.00, "Ramp Stop Test"},
	}
	
	// initialize values for testing on pcs
	for _,entry := range vector {
		p.AbsRampRate = entry.AbsRampRate	
		p.On = entry.powerInitialState
		p.Oncmd = entry.oncmd
		p.Offcmd = entry.offcmd
		p.Plim = 5000
		p.Qlim = 5000
		p.AcContactorCloseCmd = true
		p.DcContactorCloseCmd = true
		p.GridForming = false
		p.Pdc = entry.Pdc
		p.Q = entry.Q
		p.Pcmd = entry.targetPcmd
		p.Qcmd = entry.targetQcmd
		p.PRampRise = entry.Pramprise
		p.PRampDrop = entry.Prampdrop
		p.PRampStart = entry.Prampstart
		p.PRampStop = entry.Prampstop
		p.PRampDropEnable = entry.PRampDropEnable
		p.PRampRiseEnable = entry.PRampRiseEnable
		p.PRampStartEnable = entry.PRampStartEnable
		p.PRampStopEnable = entry.PRampStopEnable
		p.RampEnable = entry.RampEnable
		p.QRampStart = entry.Qrampstart
		p.QRampRise = entry.Qramprise
		p.QRampDrop = entry.Qrampdrop
		p.QRampStop = entry.Qrampstop
		p.QRampStartEnable = entry.QRampStartEnable
		p.QRampRiseEnable = entry.QRampRiseEnable
		p.QRampDropEnable = entry.QRampDropEnable
		p.QRampStopEnable = entry.QRampStopEnable
		
		(&p).Init()
		if(entry.Pdc > entry.targetPcmd){
			input.p = entry.Pdc
		} else {
			input.p = entry.targetPcmd
		}
		
		_ = (&p).UpdateMode(input)
		// fmt.Println("[After UpdateMode]","p.on", p.On, "p.AcContactor", p.AcContactor, "P.DcContactor", p.DcContactor)		
		_ = (&p).CalculateState(input, entry.deltaT)
		_ = (&p).UpdateState(input, entry.deltaT)
		if !(entry.PexpectedResult * 1.01 >= p.Pdc && entry.PexpectedResult * 0.99 <= p.Pdc){
			t.Log("Incorrect Ramprate used")
			t.Logf("Got %.2f expected %.2f", p.Pdc, entry.PexpectedResult)
			t.Fail()
		}

		if !(entry.QexpectedResult * 1.01 >= p.Q && entry.QexpectedResult * 0.99 <= p.Q){
			t.Log("Incorrect Ramprate used")
			t.Logf("Got %.2f expected %.2f", p.Q, entry.QexpectedResult)
			t.Fail()
		}
	}
}