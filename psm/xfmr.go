package main

import (
	"log"
	"math"
)

type xfmr struct {
	ID        string
	Aliases   []string
	I         float64
	Di        float64
	Qi        float64
	P         float64
	Q         float64
	S         float64
	F         float64
	Ph        float64
	Sn        float64
	Vn        float64
	V1        float64
	V1LN      float64
	V2        float64
	V2LN      float64
	V         float64
	N         float64 // Ratio of V1/V2
	Zpu       float64
	XoR       float64 // Ratio of series reactance over series resistance
	Mag       float64 // Magnetizing current, as a % of rated current
	Eff       float64
	pnoload   float64
	qnoload   float64 // reactive power from magnetization of the transformer
	xpu       float64 // Series reactance
	rpu       float64 // Series resistance
	calcdrop  bool
	calcloss  bool
	Status    []bitfield
	StatusCfg []bitfieldcfg
	Dactive   droop
	Dreactive droop
}

func (x *xfmr) Init() {
	x.calcdrop = false
	x.calcloss = false

	// crash if N not set
	if x.N <= 0 {
		log.Fatal(x.ID, ": Transformer must have positive turns ratio")
	}
	// If nominal apparent power (Sn) is omitted or negative, do not calculate
	// voltage drop or power loss
	if x.Sn <= 0 {
		return
	}

	x.calcdrop = true
	x.calcloss = true

	// If per unit impedance (Zpu, in %) is omitted, set to 7% (mid range for single stage transformer)
	if x.Zpu <= 0 {
		x.Zpu = 7.0
	}
	// If XoR is omitted, assume XoR = 6 (conservatively lossy for >1MVA transformers)
	if x.XoR <= 0 {
		x.XoR = 6
	}
	x.rpu = (x.Zpu / math.Sqrt(1+math.Pow(x.XoR, 2))) / 100.0
	x.xpu = x.rpu * x.XoR
	// Work out no load losses
	// If efficiency (Eff) is omitted, assume 99%
	// Based on an interpretation of DOE 2016 efficiency standards, assume this is measured at 50% load
	if x.Eff <= 0 {
		x.Eff = 99.0
	} else if x.Eff >= 100 {
		x.Eff = 99.9999
	}
	loss := (x.Sn * 0.50) * (100/x.Eff - 1)
	loadloss := x.rpu * math.Pow(0.50, 2) * x.Sn
	if loadloss > loss {
		log.Fatal(x.ID, ": Transformer losses calculated from Zpu and XoR greater than loss calculated from efficiency")
	}
	x.pnoload = loss - loadloss
	// Work out no load reactive power (e.g., magnetizing reactive power)
	// If magnetizing current is omitted, assume it is 0.3% of the rated current of the transformer
	if x.Mag <= 0 {
		x.Mag = 0.3
	}
	snoload := (x.Mag * x.Sn) / 100 // Apparent power at no load, being a combination of magnetizing reactive power and core losses
	if snoload < x.pnoload {
		log.Printf("%v: Transformer magnetizing reactive power calculated from x.Mag, x.Eff, and X.Zpu is zero. Increase x.Mag.\n", x.ID)
		x.qnoload = 0
	} else {
		x.qnoload = math.Sqrt(math.Pow(snoload, 2) - math.Pow(x.pnoload, 2))
	}
	x.Status = processBitfieldConfig(x, x.StatusCfg)
	return
}

// Transformer UpdateMode() transforms V1 to V2 via the turns ratio N,
// less voltage drop due to impedance and last tick's P and Q
func (x *xfmr) UpdateMode(input terminal) (output terminal) {
	if input.v > 0 {
		x.V1 = input.v
		vWithDrop := x.V1 + (x.Q*x.xpu+x.P*x.rpu)*(x.Vn/x.Sn)
		x.V2 = (vWithDrop / x.N)
		x.F, x.Ph = input.f, input.ph
	} else {
		x.V1, x.V2 = 0, 0
	}
	assetStatus := processBitfieldConfig(x, x.StatusCfg)
	for i, v := range assetStatus {
		x.Status[i] = v
	}
	output.v = x.V2
	return output
}

// Transformer GetLoadLines() passes droop parameters up, modifying
// voltage droop according to the turns ratio
func (x *xfmr) GetLoadLines(input terminal, dt float64) (output terminal) {
	input.dVolts.slope = input.dVolts.slope / x.N
	input.dVolts.XNom = input.dVolts.XNom * x.N
	input.v = input.v * x.N
	return input
}

// Tranfsormer DistributeVoltage() transforms V1 to V2 via the turns ratio N,
// less voltage drop due to impedance and last tick's P and Q. Also transforms
// voltage droop according to the turns ratio
func (x *xfmr) DistributeVoltage(input terminal) (output terminal) {
	if input.v > 0 {
		x.V1 = input.v
		vint := x.V1 + (x.Q*x.xpu+x.P*x.rpu)*(x.Vn/x.Sn)
		x.V2 = (vint / x.N)
		x.F, x.Ph = input.f, input.ph
		output.dVolts.slope = input.dVolts.slope * x.N
		output.dVolts.XNom = input.dVolts.XNom / x.N
	} else {
		x.V1, x.V2 = 0, 0
	}
	output.v = x.V2
	output.f, output.ph = x.F, x.Ph
	return output
}

func (x *xfmr) CalculateState(input terminal, dt float64) (output terminal) {
	// Subtract losses on the way up
	if x.V2 > 0 {
		loadloss := x.rpu * math.Pow((input.s*(x.Vn/x.N))/x.V2, 2) / x.Sn
		loadq := x.xpu * math.Pow((input.s*(x.Vn/x.N))/x.V2, 2) / x.Sn
		x.P = input.p - loadloss - x.pnoload
		x.Q = input.q - loadq - x.qnoload
	} else {
		x.P = 0
		x.Q = 0
	}
	x.S = rss(x.P, x.Q)
	x.I, x.Di, x.Qi = sToI(x.S, x.V2), sToI(x.P, x.V2), sToI(x.Q, x.V2)
	output.p, output.q, output.s = x.P, x.Q, x.S
	return output
}

// Transformer DistributeLoad() transforms sharing v and f based on collected droop
// to pass down, but does not store it
func (x *xfmr) DistributeLoad(input terminal) (output terminal) {
	if input.v > 0 {
		vint := input.v + (x.Q*x.xpu+x.P*x.rpu)*(x.Vn/x.Sn)
		output.v = (vint / x.N)
		output.f, output.ph = input.f, input.ph
		output.dVolts.slope = input.dVolts.slope * x.N
		output.dVolts.XNom = input.dVolts.XNom / x.N
	}
	return output
}

func (x *xfmr) UpdateState(input terminal, dt float64) (output terminal) {
	// TODO: figure out how to save old losses, and not recalculate with new input
	if x.V2 > 0 {
		loadloss := x.rpu * math.Pow((input.s*(x.Vn/x.N))/x.V2, 2) / x.Sn
		loadq := x.xpu * math.Pow((input.s*(x.Vn/x.N))/x.V2, 2) / x.Sn
		x.P = input.p - loadloss - x.pnoload
		x.Q = input.q - loadq - x.qnoload
	} else {
		x.P = 0
		x.Q = 0
	}
	x.S = rss(x.P, x.Q)
	x.I, x.Di, x.Qi = sToI(x.S, x.V2), sToI(x.P, x.V2), sToI(x.Q, x.V2)
	x.V1LN = x.V1 / sqrt3
	x.V2LN = x.V2 / sqrt3
	output.p, output.q, output.s = x.P, x.Q, x.S
	return output
}

func (x *xfmr) GetID() string {
	return x.ID
}

func (x *xfmr) GetAliases() []string {
	return x.Aliases
}

func (x *xfmr) Term() terminal {
	return terminal{x.V, 0, x.P, x.Q, x.S, x.F, x.Ph, x.Dactive, x.Dreactive, droop{}}
}
