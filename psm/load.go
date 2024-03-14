package main

// 1. Passive loads
// 2. Time varying according to file input
// 3. Time varying according to FIMS input (infinite source, if you set something negative)

type load struct {
	ID           string
	Aliases      []string
	V            float64
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
	Pcmd         float64 // Overridden if RandomWalk enabled
	Qcmd         float64
	Pramp        float64 // kW/min
	Qramp        float64
	Noise        float64 // Noise std dev. in kW/kVAR when not RandomWalk
	RandomWalk   bool    // When true, assign uniform random number (Pmin,Pmax) to Pcmd, same for Qcmd
	Pmin         float64 // For RandomWalk
	Pmax         float64 // For RandomWalk
	Qmin         float64 // For RandomWalk
	Qmax         float64 // For RandomWalk
	Dactive      droop
	Dreactive    droop
	CtrlWord1    int
	CtrlWord1Cfg []ctrlwordcfg
}

func (l *load) Init() {
	return
}

func (l *load) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(l, l.CtrlWord1Cfg, l.CtrlWord1)

	if !l.On && l.Oncmd {
		l.On = true
		l.Oncmd = false
	} else if l.On && l.Offcmd {
		l.On = false
		l.Offcmd = false
	}
	return
}
func (l *load) GetLoadLines(input terminal, dt float64) (output terminal) {
	output.p = l.P //Last step P for droop calculations.
	return output
}
func (l *load) DistributeVoltage(input terminal) (output terminal) {
	if input.v == 0 {
		l.On, l.Oncmd, l.Offcmd = false, false, false
	}
	l.V = input.v
	l.F, l.Ph = input.f, input.ph
	return
}
func (l *load) CalculateState(input terminal, dt float64) (output terminal) {
	// fmt.Println(l.ID, l.On, l.V, l.Pcmd)
	var p, q float64
	if l.On {
		if l.RandomWalk { // This randomness is different from powerNoise
			p = loadNoise(l.Pmin, l.Pmax)
			q = loadNoise(l.Qmin, l.Qmax)
		} else {
			p = l.Pcmd
			q = l.Qcmd
		}
		p += powerNoise(l.Noise)
		q += powerNoise(l.Noise)
		output.p = getSlew(l.P, p, l.Pramp, dt/60)
		output.q = getSlew(l.Q, q, l.Qramp, dt/60)
	} else {
		output.p, output.q = 0, 0
	}
	l.P, l.Q = output.p, output.q
	// fmt.Printf("Gen %v is P: %.0fkW\tdt: %.3f\n", l.ID, l.P, dt)
	return output
}
func (l *load) DistributeLoad(input terminal) (output terminal) {
	return input
}
func (l *load) UpdateState(input terminal, dt float64) (output terminal) {
	l.S = rss(l.P, l.Q)
	l.Pf = pf(l.P, l.Q, l.S)
	l.I, l.Di, l.Qi = sToI(l.S, l.V), sToI(l.P, l.V), sToI(l.Q, l.V)
	output.p, output.q, output.s = l.P, l.Q, l.S
	return output
}

func (l *load) GetID() string {
	return l.ID
}

func (l *load) GetAliases() []string {
	return l.Aliases
}

func (l *load) Term() terminal {
	return terminal{l.V, 0, l.P, l.Q, l.S, l.F, l.Ph, l.Dactive, l.Dreactive, droop{}}
}
