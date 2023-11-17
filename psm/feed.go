package main

import (
	"math"
)

type feed struct {
	ID              string
	Aliases         []string
	I               float64
	Di              float64 // Active power current
	Qi              float64 // Reactive power current
	P               float64
	Q               float64
	S               float64
	Pf              float64
	F1              float64
	F2              float64
	Ph              float64
	Pmax            float64 // Bidirectional W trip threshold
	Qmax            float64 // Bidirectional VAR trip threshold
	Smax            float64 // VA trip threshold
	V1              float64 // Voltage of node pointing up the tree
	V1LN            float64
	V2              float64 // Voltage of node pointing down the tree
	V2LN            float64
	V               float64
	Polrev          bool // Reverses meter polarity (negative P means import rather than export)
	Closed          bool
	Closecmd        bool
	Opencmd         bool
	Resetcmd        bool // Reset Tripped state so you can reclose
	Tripcmd         bool // Normally set internally in response to Pmax, Qmax, Smax exceeded
	Tripped         bool
	Status          []bitfield
	StatusCfg       []bitfieldcfg
	CtrlWordEnabled bool
	CtrlWord1Cfg    []ctrlwordcfg
	CtrlWord1       int
	Dactive         droop
	Dreactive       droop
}

func (f *feed) Init() {
	f.Status = processBitfieldConfig(f, f.StatusCfg)
	return
}

// Feeder UpdateMode() processes commands from control words or
// direct commands, as well as trip command from last iteration
func (f *feed) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(f, f.CtrlWord1Cfg, f.CtrlWord1)

	// On the way down, see if the feeder has been commanded
	// Can reset, open or close
	if f.Resetcmd && f.Tripped {
		f.Tripped = false
		f.Resetcmd = false
	}
	if f.Tripcmd {
		f.Tripped = true
		f.Closed = false
		f.Tripcmd = false
		// fmt.Printf("%v tripped\n", f.id)
	} else if f.Opencmd && f.Closed {
		f.Closed = false
		f.Opencmd = false
		// fmt.Printf("%v opened\n", f.id)
	} else if f.Closecmd && !f.Closed && !f.Tripped {
		f.Closed = true
		f.Closecmd = false
		// fmt.Printf("%v closed\n", f.id)
	}

	if f.Closed {
		f.V1, f.V2 = input.v, input.v
		f.F1, f.F2, f.Ph = input.f, input.f, input.ph
	} else {
		f.V1, f.V2 = input.v, 0
		f.F1, f.F2 = input.f, 0
	}

	output.v, output.f, output.ph = f.V2, f.F2, f.Ph
	return output
}

// Feeder GetLoadLines() passes up droop parameters if closed, otherwise
// store the drooped voltage and frequency for use in DistributeVoltage()
func (f *feed) GetLoadLines(input terminal, dt float64) (output terminal) {
	if f.Closed {
		return input
	}
	f.V2, f.F2, f.Ph = input.v, input.f, input.ph //TODO GB: This input.f is the result after gridforming assets have corrected phase. Need to store in an overall "f.F" for reporting
	f.Dactive, f.Dreactive = input.dHertz, input.dVolts
	return output // Return zeroed terminal when not closed; no droop goes up the tree
}

// Feeder DistributeVoltage() passes down the incoming voltage and frequency
// if closed, otherwise pass down the stored drooped voltage and frequency
func (f *feed) DistributeVoltage(input terminal) (output terminal) {
	if f.Closed {
		f.V1, f.V2 = input.v, input.v
		f.F1, f.F2, f.Ph = input.f, input.f, input.ph
	} else {
		f.V1 = input.v
		f.F1 = input.f
	}
	assetStatus := processBitfieldConfig(f, f.StatusCfg)
	for i, v := range assetStatus {
		f.Status[i] = v
	}
	output.v, output.f, output.ph = f.V2, f.F2, f.Ph //from GetLoadLines() step if !f.Closed
	return output
}

// Feeder CalculateState() collects and passes up P and Q from grid following
// assets if closed. P and Q are stored either way. Tripping is deferred
// to UpdateState().
func (f *feed) CalculateState(input terminal, dt float64) (output terminal) {
	f.P, f.Q, f.S = input.p, input.q, input.s
	if f.Polrev {
		f.P, f.Q = -f.P, -f.Q
		output = terminal{p: -f.P, q: -f.Q, s: f.S}
	} else {
		output = terminal{p: f.P, q: f.Q, s: f.S}
	}
	if !f.Closed {
		output = terminal{p: 0, q: 0, s: 0}
	}
	return output
}

// Feeder DistributeLoad() passes down the collected P and Q of the tree
// for grid forming assets to respond to. It is either passed down from
// above if closed, otherwise returned from the P and Q stored on
// CalculateState()
func (f *feed) DistributeLoad(input terminal) (output terminal) {
	if f.Closed {
		return input
	}
	var p, q, s float64
	if f.Polrev {
		p, q, s = -f.P, -f.Q, f.S
	} else {
		p, q, s = f.P, f.Q, f.S
	}
	output = terminal{p: p, q: q, s: s}
	output.f = getX(-p, f.Dactive.slope, f.Dactive.offset)     // determine next tick's v and f
	output.v = getX(-q, f.Dreactive.slope, f.Dreactive.offset) // based on collected droop and p, q
	return output
}

// Feeder UpdateState() collects P and Q again now that grid forming
// assets have had a chance to participate. This time, trip thresholds
// are processed to update the mode on the next iteration.
func (f *feed) UpdateState(input terminal, dt float64) (output terminal) {
	// Check only if breaker should trip, commands are handled in UpdateMode()
	f.P, f.Q, f.S = input.p, input.q, input.s
	if f.Closed {
		if math.Abs(input.p) > f.Pmax ||
			math.Abs(input.q) > f.Qmax ||
			input.s > f.Smax {
			f.Tripcmd = true
			// fmt.Printf("%v tripped! P: %.2f\tQ: %.2f\tS: %.2f\tV1: %.2f\tV2: %.2f\n", f.ID, input.p, input.q, input.s, f.V1, f.V2)
		}
	}
	if f.Closed {
		f.P, f.Q, f.S = input.p, input.q, input.s
	} else {
		f.P, f.Q, f.S = 0, 0, 0
	}
	if f.Polrev {
		f.P, f.Q = -f.P, -f.Q
	}
	f.Pf = pf(f.P, f.Q, f.S)
	//TODO GB: Why do these use f.V instead of f.V1?
	f.I, f.Di, f.Qi = sToI(f.S, f.V), sToI(f.P, f.V), sToI(f.Q, f.V)
	f.V1LN = f.V1 / sqrt3
	f.V2LN = f.V2 / sqrt3
	// Return un-reversed polarity for solver
	if f.Polrev {
		output = terminal{p: -f.P, q: -f.Q, s: f.S}
	} else {
		output = terminal{p: f.P, q: f.Q, s: f.S}
	}
	if !f.Closed {
		output = terminal{p: 0, q: 0, s: 0}
	}
	return output
}

func (f *feed) GetID() string {
	return f.ID
}

func (f *feed) GetAliases() []string {
	return f.Aliases
}

func (f *feed) Term() terminal {
	return terminal{f.V, 0, f.P, f.Q, f.S, f.F2, f.Ph, f.Dactive, f.Dreactive, droop{}}
}
