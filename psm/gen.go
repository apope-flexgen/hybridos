package main

type gen struct {
	ID               string
	Aliases          []string
	V                float64
	VLN              float64
	I                float64
	Di               float64
	Qi               float64
	P                float64
	Q                float64
	S                float64
	Pf               float64
	F                float64
	Ph               float64
	On               bool
	Oncmd            bool
	Offcmd           bool
	Pcmd             float64
	Qcmd             float64
	Pramp            float64 // kW/min
	Qramp            float64
	PfMode           bool // overrides Qcmd based on PfCmd
	PfCmd            float64
	Noise            float64 // level of kW noise on Pcmd and Qcmd following
	Vcmd             float64 // Voltage setpoint in grid forming
	Fcmd             float64 // Freqency setpoint in grid forming
	GridForming      bool    // Grid forming status
	GridFormingCmd   bool    // Grid forming command
	GridFollowingCmd bool    // Grid following command
	Status           []bitfield
	StatusCfg        []bitfieldcfg
	CtrlWord1Cfg     []ctrlwordcfg
	CtrlWord1        int
	CtrlWord2Cfg     []ctrlwordcfg
	CtrlWord2        int
	Dactive          droop
	Dreactive        droop
}

func (g *gen) Init() {
	g.Status = processBitfieldConfig(g, g.StatusCfg)
	g.Dactive.slope, g.Dactive.offset = getSlope(g.Dactive.Percent, g.Dactive.YNom, g.Dactive.XNom)
	g.Dreactive.slope, g.Dreactive.offset = getSlope(g.Dreactive.Percent, g.Dreactive.YNom, g.Dreactive.XNom)
	// fmt.Println(g.ID, g.Dactive, g.Dreactive)
}

// Generator UpdateMode() processes commands either through control words or direct
// commands.
func (g *gen) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(g, g.CtrlWord1Cfg, g.CtrlWord1)
	processCtrlWordConfig(g, g.CtrlWord2Cfg, g.CtrlWord2)

	if g.GridFormingCmd {
		g.GridForming, g.GridFormingCmd = true, false
	} else if g.GridFollowingCmd && g.GridForming {
		g.GridForming, g.GridFollowingCmd = false, false
	}

	if !g.On && g.Oncmd {
		g.On = true
		g.Oncmd = false
	} else if g.On && g.Offcmd {
		g.On = false
		g.Offcmd = false
	}

	return output // returning zero terminal, since Generator has no assets below it
}

// Generator GetLoadLines() returns droop parameters up the tree if grid forming
func (g *gen) GetLoadLines(input terminal, dt float64) (output terminal) {
	if g.GridForming && g.On {
		output.dHertz = g.Dactive   // Nominal rating is saved in dHertz, but
		output.dHertz.XNom = g.Fcmd // commands can override
		output.dVolts = g.Dreactive
		output.dVolts.XNom = g.Vcmd
		output.p = g.P // g.P and g.Q contain the last iteration's output at this
		output.q = g.Q // point in the solver, so droop has a one tick delay
	}
	return output
}

// Generator DistributeVoltage() accepts the voltage set from upstream, either
// by a grid directly or through droop. The Generator turns off if no voltage
// is present and updates its status output.
func (g *gen) DistributeVoltage(input terminal) (output terminal) {
	if input.v <= 0 { // Turn off if input voltage is 0 after grid forming phase
		g.On, g.Oncmd, g.Offcmd = false, false, false
	}
	assetStatus := processBitfieldConfig(g, g.StatusCfg)
	for i, v := range assetStatus {
		g.Status[i] = v
	}
	g.V, g.F, g.Ph = input.v, input.f, input.ph
	return output // returning zero terminal, since Generator has no assets below it
}

// Generator CalculateState() calculates and returns grid following output P and Q,
// taking into considering power factor mode and ramp rate
func (g *gen) CalculateState(input terminal, dt float64) (output terminal) {
	if g.GridForming {
		return output
	}
	pcmd := g.Pcmd
	qcmd := g.Qcmd
	if g.PfMode {
		qcmd = pfToQ(pcmd, g.PfCmd)
	}
	pcmd += powerNoise(g.Noise)
	qcmd += powerNoise(g.Noise)
	if g.On {
		output.p = getSlew(g.P, pcmd, g.Pramp, dt/60) // slew rates are given in kW/min
		output.q = getSlew(g.Q, qcmd, g.Qramp, dt/60) // slew rates are given in kVAR/min
	} else {
		output.p, output.q = 0, 0
	}

	g.P, g.Q = output.p, output.q
	// fmt.Printf("Gen %v is P: %.0fkW\tdt: %.3f\n", g.ID, g.P, dt)
	return output
}

// Generator DistributeLoad() returns the input to it since it has no assets below it in the tree
func (g *gen) DistributeLoad(input terminal) (output terminal) {
	return input
}

// Genrator UpdateState() calculates grid forming power output P and Q, considering
// the P and Q demand of the tree and its droop parameters
func (g *gen) UpdateState(input terminal, dt float64) (output terminal) {
	var p, q float64
	if !g.GridForming {
		p, q = g.P, g.Q // this is currently redundant, but may not be later if we add fuel
	} else if g.On {
		p = getY(input.f, g.Dactive.slope, g.Dactive.offset)
		q = getY(input.v, g.Dreactive.slope, g.Dreactive.offset)
		// fmt.Println("gen getY", g.V, input.v, g.Dreactive.slope, g.Dreactive.offset, q)
	}
	g.P, g.Q = p, q
	g.S = rss(g.P, g.Q)
	g.Pf = pf(g.P, g.Q, g.S)
	g.I, g.Di, g.Qi = sToI(g.S, g.V), sToI(g.P, g.V), sToI(g.Q, g.V)
	g.VLN = g.V / sqrt3
	output.p, output.q, output.s = g.P, g.Q, g.S
	// fmt.Printf("Generator %s: P: %.1f\tQ: %.1f\tS: %.1f\tV: %.1f\tF: %.1f\n", g.ID, g.P, g.Q, g.S, g.V, g.F)
	return output
}

func (g *gen) GetID() string {
	return g.ID
}

func (g *gen) GetAliases() []string {
	return g.Aliases
}

func (g *gen) Term() terminal {
	return terminal{g.V, 0, g.P, g.Q, g.S, g.F, g.Ph, g.Dactive, g.Dreactive, droop{}}
}
