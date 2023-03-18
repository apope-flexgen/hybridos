package main

type grid struct {
	ID                string
	Aliases           []string
	I                 float64
	Di                float64
	Qi                float64
	P                 float64
	Q                 float64
	S                 float64
	Pf                float64
	Vcmd              float64 // Commanded V
	Fcmd              float64 // Commanded F
	V                 float64 // Stored V after droop
	F                 float64 // Stored F after droop
	Ph                float64
	Dactive           droop // Intrinsic droop of grid
	Dreactive         droop // Intrinsic droop of grid
	DactiveExternal   droop // Stored droop from lower devices
	DreactiveExternal droop // Stored droop from lower devices
}

func (g *grid) Init() {
	// TODO: set slope and offset to useful defaults if not provided to preserve compatibility
	if (g.Fcmd == 0) && (g.F != 0) {
		g.Fcmd = g.F // this preserves backward compatibility where only F was set
	}
	if g.Dactive.XNom == 0 {
		g.Dactive.XNom = g.Fcmd // this preserves backward compatibility where droop didn't exist
	}
	if g.Dactive.YNom == 0 {
		g.Dactive.YNom = 1000000 // by default set to usefully high number
	}
	if (g.Vcmd == 0) && (g.V != 0) {
		g.Vcmd = g.V // this preserves backward compatibility where only V was set
	}
	if g.Dreactive.XNom == 0 {
		g.Dreactive.XNom = g.Vcmd // this preserves backward compatibility where droop didn't exist
	}
	if g.Dreactive.YNom == 0 {
		g.Dreactive.YNom = 1000000 // by default set to usefully high number
	}
	g.Dactive.slope, g.Dactive.offset = getSlope(g.Dactive.Percent, g.Dactive.YNom, g.Dactive.XNom)
	g.Dreactive.slope, g.Dreactive.offset = getSlope(g.Dreactive.Percent, g.Dreactive.YNom, g.Dreactive.XNom)
	g.V = g.Vcmd
	g.F = g.Fcmd
	// fmt.Println(g.ID, g.V, g.F, g.Vcmd, g.Fcmd, g.Dreactive)
}

// Grid UpdateMode() just passes its terminal voltages down, the
// grid is unbowed, unbent, unbroken
func (g *grid) UpdateMode(input terminal) (output terminal) {
	output.v, output.f, output.ph = g.V, g.F, g.Ph
	return output
}

// Grid GetLoadLines() takes the droop parameters passed up to it, combines
// with its own and stores for use in DistributeVoltage()
func (g *grid) GetLoadLines(input terminal, dt float64) (output terminal) {
	gridTerminal := terminal{
		p: g.P, q: g.Q,
		dHertz: g.Dactive, dVolts: g.Dreactive,
	}
	// Set XNom in the droops based on commands off FIMS
	gridTerminal.dHertz.XNom = g.Fcmd
	gridTerminal.dVolts.XNom = g.Vcmd
	// Recalculate slope and offset to take commands into account
	gridTerminal.dHertz.slope, gridTerminal.dHertz.offset = getSlope(gridTerminal.dHertz.Percent, gridTerminal.dHertz.YNom, gridTerminal.dHertz.XNom)
	gridTerminal.dVolts.slope, gridTerminal.dVolts.offset = getSlope(gridTerminal.dVolts.Percent, gridTerminal.dVolts.YNom, gridTerminal.dVolts.XNom)
	// Do combine terminals here since the grid is at the root of the tree
	// and needs to store external droop information for DistributeVoltage()
	output = combineTerminals(gridTerminal, input)
	g.DactiveExternal = output.dHertz
	g.DreactiveExternal = output.dVolts
	g.V, g.F = output.v, output.f
	// fmt.Println("getloadlines - output", output)
	// fmt.Println("getloadlines - gridTerminal", gridTerminal)
	// fmt.Println("getloadlines - dactive", g.Dactive)
	// fmt.Println("getloadlines - dactiveexternal", g.DactiveExternal)
	return output
}

// Grid DistributeVoltage() passes down the calcuated and stored droop
// frequency and voltage
func (g *grid) DistributeVoltage(input terminal) (output terminal) {
	output.v, output.f, output.ph = g.V, g.F, g.Ph
	return output
}

// Grid CalculateState() collects and stores P and Q from grid following assets
func (g *grid) CalculateState(input terminal, dt float64) (output terminal) {
	g.P, g.Q, g.S = input.p, input.q, input.s
	output = terminal{p: g.P, q: g.Q, s: g.S}
	return output
}

// Grid DistributeLoad() passes back down the collected P and Q for grid
// forming assets to respond to, along with the collected droop parameters
func (g *grid) DistributeLoad(input terminal) (output terminal) {
	output.p, output.q, output.s = g.P, g.Q, g.S
	// fmt.Println("DistributeLoad", output)
	output.f = getX(-g.P, g.DactiveExternal.slope, g.DactiveExternal.offset)     // determine sharing v and f
	output.v = getX(-g.Q, g.DreactiveExternal.slope, g.DreactiveExternal.offset) // based on collected droop and p, q
	return output
}

// Grid UpdateState() collects P and Q again now that grid forming assets
// have had a chance to participate
func (g *grid) UpdateState(input terminal, dt float64) (output terminal) {
	g.P, g.Q, g.S = input.p, input.q, input.s
	g.Pf = pf(g.P, g.Q, g.S)
	g.I, g.Di, g.Qi = sToI(g.S, g.V), sToI(g.P, g.V), sToI(g.Q, g.V)
	output = terminal{g.V, 0, g.P, g.Q, g.S, g.F, g.Ph, g.Dactive, g.Dreactive, droop{}}
	return output
}

func (g *grid) GetID() string {
	return g.ID
}

func (g *grid) GetAliases() []string {
	return g.Aliases
}

func (g *grid) Term() terminal {
	return terminal{g.V, 0, g.P, g.Q, g.S, g.F, g.Ph, g.Dactive, g.Dreactive, droop{}}
}
