package main

import (
	"math"
)

type solar struct {
	ID           string
	Aliases      []string
	V            float64
	VLN          float64
	I            float64
	Di           float64
	Qi           float64
	P            float64
	Q            float64
	S            float64
	Pf           float64
	F            float64
	Ph           float64
	On           bool
	Oncmd        bool
	Offcmd       bool
	Pcmd         float64
	Plim         float64
	Phigh        float64
	Qcmd         float64
	Pramp        float64 // kW/min
	Qramp        float64
	PercentCmd   bool    // impacts Plim, Qcmd
	PfMode       bool    // overrides Qcmd based on PfCmd
	Noise        float64 // level of kW noise on Pcmd and Qcmd following
	PfCmd        float64
	Status       []bitfield
	StatusCfg    []bitfieldcfg
	CtrlWord1    int
	CtrlWord1Cfg []ctrlwordcfg
	CtrlWord2    int
	CtrlWord2Cfg []ctrlwordcfg
	Dactive      droop
	Dreactive    droop
}

func (pv *solar) Init() {
	pv.Status = processBitfieldConfig(pv, pv.StatusCfg)
	return
}

func (pv *solar) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(pv, pv.CtrlWord1Cfg, pv.CtrlWord1)
	processCtrlWordConfig(pv, pv.CtrlWord2Cfg, pv.CtrlWord2)

	if !pv.On && pv.Oncmd {
		//fmt.Println("ONCMD RECEIVED!!!!!")
		pv.On = true
		pv.Oncmd = false
	} else if pv.On && pv.Offcmd {
		pv.On = false
		pv.Offcmd = false
	}

	return
}

func (pv *solar) GetLoadLines(input terminal, dt float64) (output terminal) {
	return output
}

func (pv *solar) DistributeVoltage(input terminal) (output terminal) {
	if input.v <= 0 {
		pv.On, pv.Oncmd, pv.Offcmd = false, false, false
	}
	assetStatus := processBitfieldConfig(pv, pv.StatusCfg)
	for i, v := range assetStatus {
		pv.Status[i] = v
	}
	pv.V, pv.F, pv.Ph = input.v, input.f, input.ph
	return
}

func (pv *solar) CalculateState(input terminal, dt float64) (output terminal) {
	pv.F, pv.Ph = input.f, input.ph
	plim := pv.Plim
	qcmd := pv.Qcmd
	if pv.PercentCmd {
		plim = plim * pv.Phigh / 100
		qcmd = qcmd * pv.Phigh / 100
	}
	if pv.PfMode {
		qcmd = pfToQ(plim, pv.PfCmd)
	}
	plim = math.Min(plim, pv.Phigh)
	plim += powerNoise(pv.Noise)
	qcmd += powerNoise(pv.Noise)
	pcmd := pv.Pcmd + powerNoise(pv.Noise)
	pcmd = math.Max(pcmd, 0)
	pcmd = math.Min(plim, pcmd)
	if pv.On {
		output.p = getSlew(pv.P, pcmd, pv.Pramp, dt/60)
		output.q = getSlew(pv.Q, qcmd, pv.Qramp, dt/60)
	} else {
		output.p, output.q = 0, 0
	}
	pv.P, pv.Q = output.p, output.q
	// fmt.Printf("pv %v is P: %.0fkVAR\tdt: %.3f\n", pv.ID, pv.Q, dt)
	return output
}

func (pv *solar) DistributeLoad(input terminal) (output terminal) {
	return input
}

func (pv *solar) UpdateState(input terminal, dt float64) (output terminal) {
	pv.S = rss(pv.P, pv.Q)
	pv.Pf = pf(pv.P, pv.Q, pv.S)
	pv.I, pv.Di, pv.Qi = sToI(pv.S, pv.V), sToI(pv.P, pv.V), sToI(pv.Q, pv.V)
	pv.VLN = pv.V / sqrt3
	output.p, output.q, output.s = pv.P, pv.Q, pv.S
	return output
}

func (pv *solar) GetID() string {
	return pv.ID
}

func (pv *solar) GetAliases() []string {
	return pv.Aliases
}

func (pv *solar) Term() terminal {
	return terminal{pv.V, 0, pv.P, pv.Q, pv.S, pv.F, pv.Ph, pv.Dactive, pv.Dreactive, droop{}}
}
