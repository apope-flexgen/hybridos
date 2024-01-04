package main

import (
	"testing"
	"math"
)

var transfer_terminal terminal
type testEntry struct {
	comment string
	rpu		float64
	xpu		float64
	pnoload	float64
	qnoload	float64
	v1 		float64
	v1_load float64
	v2		float64
	v2_dist	float64
	slopep	float64
	slopes	float64
	xnomp	float64
	xnoms	float64
	f		float64
	ph		float64
	s		float64
	p		float64
	q		float64
}

var entries = []testEntry{}
var xfmrs = []xfmr{}
var up_terms = []terminal{}
var down_terms = []terminal{}


func setupTest() {
	entries = append(entries, testEntry{
		comment: "Default xfmr values",
		rpu: 0.0115,
		xpu: 0.069,
		pnoload: 1.087,
		qnoload: 1.034,
		v1: 10000,
		v1_load: 9680,
		v2: 968.35,
		v2_dist: 970.48,
		slopep:	-0.3,
		slopes: -3,
		xnomp: 10000,
		xnoms: 1000,
		f: 	60,
		ph: 5,
		s: 413.06,
		p: -385.01,
		q: -149.06,
	})
	entries = append(entries, testEntry{
		comment: "User input xfmr values",
		rpu: 0.01,
		xpu: 0.09,
		pnoload: 2.557,
		qnoload: 7.051,
		v1:	10000,
		v1_load: 9680,
		v2: 963.5,
		v2_dist: 962.93,
		slopep:	-0.3,
		slopes: -3,
		xnomp: 10000,
		xnoms: 1000,
		f: 60,
		ph: 5,
		s: 419.04,
		p: -386.00,
		q: -163.07,
	})
	xfmrs = append(xfmrs, xfmr{
		ID: "default_xfmr",
		Vn: 10000,
		Sn: 500,
		N: 10,
		Zpu: -1, // input of -1 sets value to default
		XoR: -1,
		Mag: -1,
		Eff: -1,
		Q: -150,
		P: -475,
	})
	xfmrs = append(xfmrs, xfmr{
		ID: "test_1_xfmr",
		Vn: 10000,
		Sn: 500,
		N: 10,
		Zpu: math.Sqrt(82),
		XoR: 9,
		Mag: 2,
		Eff: 98,
		Q: -150,
		P: -475,
	})
	down_terms = append(down_terms, terminal{
		v: 968,
		p: -380,
		q: -125,
		s: -400,
		dVolts: droop{
			slope: -3,
			XNom: 1000,
		},
	})
	down_terms = append(down_terms, terminal{
		v: 968,
		p: -380,
		q: -125,
		s: -400,
		dVolts: droop{
			slope: -3,
			XNom: 1000,
		},
	})
	up_terms = append(up_terms, terminal{
		v: 10000,
		f: 60,
		ph: 5,
		dVolts: droop{
			slope: -0.3,
			XNom: 10000,
		},
	})
	up_terms = append(up_terms, terminal{
		v: 10000,
		f: 60,
		ph: 5,
		dVolts: droop{
			slope: -0.3,
			XNom: 10000,
		},
	})
}

func TestXfmr(t *testing.T) {
	setupTest()
	for i, answers := range entries {
		// Run the Initialization method and test if it is valid
		xfmrs[i].Init()
		if !(floatEq(xfmrs[i].rpu, answers.rpu, .01) && floatEq(xfmrs[i].xpu, answers.xpu, .01) && floatEq(xfmrs[i].pnoload, answers.pnoload, 10) && floatEq(xfmrs[i].qnoload, answers.qnoload, 10)) {
			t.Log("Transformer Initialization Test for", answers.comment)
			t.Logf("Got %.3f expected %.3f resistance", xfmrs[i].rpu, answers.rpu)
			t.Logf("Got %.3f expected %.3f reactance", xfmrs[i].xpu, answers.xpu)
			t.Logf("Got %.2f expected %.2f kW", xfmrs[i].pnoload, answers.pnoload)
			t.Logf("Got %.2f expected %.2f kVAR", xfmrs[i].qnoload, answers.qnoload)
			t.Error("Transformer Initialization Test failed.")
		}
		// Run the UpdateMode method and test its validity
		transfer_terminal = xfmrs[i].UpdateMode(up_terms[i])
		if !(floatEq(transfer_terminal.v, answers.v2, 10)) {
			t.Log("Transformer UpdateMode Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.v, answers.v2)
			t.Error("Transformer UpdateMode Test failed.")
		}
		// Run the GetLoadLines method and test its validity
		transfer_terminal = xfmrs[i].GetLoadLines(down_terms[i], 600)
		if !(floatEq(transfer_terminal.dVolts.slope, answers.slopep, 0.1) && floatEq(transfer_terminal.dVolts.XNom, answers.xnomp, 10) && floatEq(transfer_terminal.v, answers.v1_load, 10)) {
			t.Log("Transformer GetLoadLines Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f VAR/V", transfer_terminal.dVolts.slope, answers.slopep)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.dVolts.XNom, answers.xnomp)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.v, answers.v1_load)
			t.Error("Transformer GetLoadLines Test failed.")
		}
		// Run the DistributeVoltage method and test its validity
		transfer_terminal = xfmrs[i].DistributeVoltage(up_terms[i])
		if !(floatEq(transfer_terminal.v, answers.v2, 10)) {
			t.Log("Transformer DistributeVoltage Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.v, answers.v2)
			t.Error("Transformer DistributeVoltage Test failed.")
		}
		// Run the CalculateState method and test its validity
		transfer_terminal = xfmrs[i].CalculateState(down_terms[i], 600)
		if !(floatEq(transfer_terminal.p, answers.p, 10) && floatEq(transfer_terminal.q, answers.q, 10) && floatEq(transfer_terminal.s, answers.s, 10)) {
			t.Log("Transformer CalculateState Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f kW", transfer_terminal.p, answers.p)
			t.Logf("Got %.2f expected %.2f kVAR", transfer_terminal.q, answers.q)
			t.Logf("Got %.2f expected %.2f kVA", transfer_terminal.s, answers.s)
			t.Error("Transformer CalculateState Test failed.")
		}
		// Run the DistributeLoad method and test its validity
		transfer_terminal = xfmrs[i].DistributeLoad(up_terms[i])
		if !(floatEq(transfer_terminal.v, answers.v2_dist, 10) && floatEq(transfer_terminal.f, answers.f, 1) && floatEq(transfer_terminal.ph, answers.ph, 1) && floatEq(transfer_terminal.dVolts.slope, answers.slopes, 0.1) && floatEq(transfer_terminal.dVolts.XNom, answers.xnoms, 10)) {
			t.Log("Transformer DistributeLoad Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.v, answers.v2_dist)
			t.Logf("Got %.2f expected %.2f Hz", transfer_terminal.f, answers.f)
			t.Logf("Got %.2f expected %.2f degrees", transfer_terminal.ph, answers.ph)
			t.Logf("Got %.2f expected %.2f VAR/V", transfer_terminal.dVolts.slope, answers.slopes)
			t.Logf("Got %.2f expected %.2f Volts", transfer_terminal.dVolts.XNom, answers.xnoms)
			t.Error("Transformer DistributeLoad Test failed.")
		}
		// Run the UpdateState method and test its validity
		transfer_terminal = xfmrs[i].UpdateState(down_terms[i], 600)
		if !(floatEq(transfer_terminal.p, answers.p, 10) && floatEq(transfer_terminal.q, answers.q, 10) && floatEq(transfer_terminal.s, answers.s, 10)) {
			t.Log("Transformer UpdateState Test for", answers.comment)
			t.Logf("Got %.2f expected %.2f kW", transfer_terminal.p, answers.p)
			t.Logf("Got %.2f expected %.2f kVAR", transfer_terminal.q, answers.q)
			t.Logf("Got %.2f expected %.2f kVA", transfer_terminal.s, answers.s)
			t.Error("Transformer UpdateState Test failed.")
		}
	}
}