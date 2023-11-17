package main

import (
	"math"
)

type ess struct {
	ID                  string
	Aliases             []string
	Cap                 float64 // Capacity in kWh
	V                   float64
	VLN                 float64
	I                   float64
	Di                  float64 // Active power current
	Qi                  float64 // Reactive power current
	P                   float64
	Q                   float64
	S                   float64
	Pf                  float64
	F                   float64
	Ph                  float64
	On                  bool
	Oncmd               bool
	Offcmd              bool
	Standby             bool
	StandbyCmd          bool
	ContactorControl    bool // Allows independent control of contactors
	AcContactor         bool
	AcContactorOpenCmd  bool
	AcContactorCloseCmd bool
	DcContactor         bool
	DcContactorOpenCmd  bool
	DcContactorCloseCmd bool
	Racks               int
	RacksInService      int // Equals Racks when ESS running
	Pcmd                float64
	Phigh               float64 // Inverter max discharge limit (constant)
	Pcharge             float64 // Battery max charge limit (varies with SOC)
	Plow                float64 // Inverter max charge limit
	Pdischarge          float64 // Battery max discharge limit
	Qcmd                float64
	Qhigh               float64 // Inverter capacitive limit
	Qlow                float64 // Inverter inductive limit
	Soc                 float64
	Rte                 float64 // Round trip efficiency at full load
	pesr                float64 // power normalized ESR, see Init()
	Idleloss            float64 // Fixed losses across load
	IvCurve             float64 // not used
	Idc                 float64 // not used
	Vdc                 float64 // not used
	Vcmd                float64 // Voltage setpoint in grid forming
	Fcmd                float64 // Freqency setpoint in grid forming
	PfMode              bool    // overrides Qcmd based on PfCmd
	PfCmd               float64 // Power factor command (-1-1)
	Noise               float64 // level of kW noise on Pcmd and Qcmd following
	GridForming         bool    // Grid forming status
	GridFormingCmd      bool    // Grid forming command
	GridFollowingCmd    bool    // Grid following command
	Status              []bitfield
	StatusCfg           []bitfieldcfg
	CtrlWord1           int
	CtrlWord1Cfg        []ctrlwordcfg
	CtrlWord2           int
	CtrlWord2Cfg        []ctrlwordcfg
	CtrlWord3           int
	CtrlWord3Cfg        []ctrlwordcfg
	CtrlWord4           int
	CtrlWord4Cfg        []ctrlwordcfg
	Dactive             droop
	Dreactive           droop
	PRampRise           float64
	PRampDrop           float64
	PRampStart          float64
	PRampStop           float64
	PRampDropEnable     bool
	PRampRiseEnable     bool
	PRampStartEnable    bool
	PRampStopEnable     bool
	RampEnable          bool
	QRampRise           float64
	QRampDrop           float64
	QRampStart          float64
	QRampStop           float64
	QRampDropEnable     bool
	QRampRiseEnable     bool
	QRampStartEnable    bool
	QRampStopEnable     bool
	targetPcmd          float64 // assign Pcmd to this variable in updateMode to avoid Pcmd changing during execution
	targetQcmd          float64 // assign Qcmd to this varialbe in updateMode to avoid Qcmd changing during execution
	onTransition        bool    // if true, signals a transition from an off state to an on state
	offTransition       bool    // if true, signals a transition from an on state to an off state
	PstartActive        bool    // enables PRampStart ramping if there is an on state transition
	PstopActive         bool    // enables PRampStop ramping if there is an off state transition
	Dt                  float64 // used for debugging, sets a DeltaT that can be used as needed
	QstartActive        bool    // enables QRampStart ramping if there is an on state transition
	QstopActive         bool    // enables QRampStop ramping if there is an off state transition
	AbsRampRate         bool    // if true, signals that the ramp rates are measured in absolute values instead of percentage power per second

}

func (e *ess) Init() {
	if e.Rte == 0 {
		e.pesr = 0
	} else {
		// This gives a "power normalized ESR" so we don't have to convert to current for loading loses
		// This means we can use P^2 * pesr for loss calculations later
		e.pesr = (e.Phigh*(1/math.Sqrt(e.Rte/100.0)-1) - e.Idleloss) / math.Pow(e.Phigh, 2)
	}
	e.Status = processBitfieldConfig(e, e.StatusCfg)
	e.Dactive.slope, e.Dactive.offset = getSlope(e.Dactive.Percent, e.Dactive.YNom, e.Dactive.XNom)
	e.Dreactive.slope, e.Dreactive.offset = getSlope(e.Dreactive.Percent, e.Dreactive.YNom, e.Dreactive.XNom)
	// fmt.Println(e.ID, e.Dactive, e.Dreactive)
	e.PstartActive, e.PstopActive = false, false
}

// ESS UpdateMode() processes commands either through control words or direct
// commands.
func (e *ess) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(e, e.CtrlWord1Cfg, e.CtrlWord1)
	processCtrlWordConfig(e, e.CtrlWord2Cfg, e.CtrlWord2)
	processCtrlWordConfig(e, e.CtrlWord3Cfg, e.CtrlWord3)
	processCtrlWordConfig(e, e.CtrlWord4Cfg, e.CtrlWord4)

	// ContactorControl allows discrete control over DC and AC contactors
	// If not set, only Oncmd or Offcmd are needed
	if !e.ContactorControl {
		if !e.On && e.Oncmd {
			e.AcContactorCloseCmd, e.DcContactorCloseCmd = true, true
		} else if e.On && e.Offcmd {
			e.AcContactorOpenCmd, e.DcContactorOpenCmd = true, true
		}
	}

	if !e.AcContactor && e.AcContactorCloseCmd {
		e.AcContactor, e.AcContactorCloseCmd = true, false
	} else if e.AcContactor && e.AcContactorOpenCmd {
		e.AcContactor, e.AcContactorOpenCmd = false, false
		e.Offcmd = true
	}

	if !e.DcContactor && e.DcContactorCloseCmd {
		e.DcContactor, e.DcContactorCloseCmd = true, false
		e.RacksInService = e.Racks
	} else if e.DcContactor && e.DcContactorOpenCmd {
		e.DcContactor, e.DcContactorOpenCmd = false, false
		e.RacksInService = 0
		e.Offcmd = true

	}

	if e.GridFormingCmd {
		e.GridForming, e.GridFormingCmd = true, false
	} else if e.GridFollowingCmd && e.GridForming {
		e.GridForming, e.GridFollowingCmd = false, false
	}

	e.onTransition, e.offTransition = false, false

	// Turn on if conditions allow it
	if e.Oncmd && (!e.On || e.Standby) && e.AcContactor && e.DcContactor && !(e.PstopActive || e.QstopActive) {
		e.On = true
		e.Oncmd = false
		e.Standby = false
		e.onTransition = true
		e.targetPcmd = e.Pcmd
		e.targetQcmd = e.Qcmd
	} else if (e.On || e.PstopActive || e.QstopActive) && e.Offcmd {
		e.On = false
		e.Offcmd = false
		e.Standby = false
		e.offTransition = true
		e.targetPcmd = 0
		e.targetQcmd = 0
	}

	// If Grid Following, set up flag to ensure that Ramping occurs when a power transition is present
	if e.On && e.StandbyCmd {
		e.Standby = true
		e.StandbyCmd = false
	}
	// If Grid Following, set up flags to determine whether there should be onTransition ramping or offTransition ramping
	if !e.GridForming {
		if (e.offTransition || e.PstopActive) && (e.PRampStopEnable || e.RampEnable) {
			e.PstopActive = true
		}
		if (!e.PRampStopEnable && !e.RampEnable) || e.onTransition || e.P == 0 {
			e.PstopActive = false
		}
		if (e.onTransition || e.PstartActive) && (e.PRampStartEnable || e.RampEnable) {
			e.PstartActive = true
		}
		if (e.Pcmd != e.targetPcmd && !e.onTransition) || (!e.PRampStartEnable && !e.RampEnable) || e.offTransition {
			e.PstartActive = false
		}

		// Reactive power for setting up flags to determine whether there should be onTransition ramping or offTransition ramping
		if (e.offTransition || e.QstopActive) && (e.QRampStopEnable || e.RampEnable) {
			e.QstopActive = true
		}
		if (e.onTransition || e.QstartActive) && (e.QRampStartEnable || e.RampEnable) {
			e.QstartActive = true
		}
		if (!e.QRampStopEnable && !e.RampEnable) || e.onTransition || e.Q == 0 {
			e.QstopActive = false
		}
		if (e.Qcmd != e.targetQcmd && !e.onTransition) || (!e.QRampStartEnable && !e.RampEnable) || e.offTransition {
			e.QstartActive = false
		}
	}
	if !(e.PstopActive || e.QstopActive) {
		e.targetPcmd = e.Pcmd // Guarentee that Pcmd used is constant at a given timestamp
		e.targetQcmd = e.Qcmd // Guarentee that Qcmd used is constant at a given timestamp
	}

	return output // returning zero terminal, since ESS has no assets below it
}

// ESS GetLoadLines() returns droop parameters up the tree if grid forming
func (e *ess) GetLoadLines(input terminal, dt float64) (output terminal) {
	if e.GridForming && (e.On || e.PstopActive || e.QstopActive) {
		output.dHertz = e.Dactive   // Nominal rating is saved in dHertz, but
		output.dHertz.XNom = e.Fcmd // commands can override
		output.dVolts = e.Dreactive
		output.dVolts.XNom = e.Vcmd
		output.p = e.P // e.P and e.Q contain the last iteration's output at this
		output.q = e.Q // point in the solver, so droop has a one tick delay
	}
	return output
}

// ESS DistributeVoltage() accepts the voltage set from upstream, either
// by a grid directly or through droop. The ESS turns off if no voltage
// is present and updates its status output.
func (e *ess) DistributeVoltage(input terminal) (output terminal) {
	// fmt.Println("input.v:  ", input.v)
	if input.v <= 0 { // Turn off if input voltage is 0 after grid forming phase
		e.On, e.Oncmd, e.Offcmd = false, false, false
		e.Standby, e.StandbyCmd = false, false
	}
	assetStatus := processBitfieldConfig(e, e.StatusCfg)
	copy(e.Status, assetStatus)
	e.V, e.Ph, e.F = input.v, input.ph, input.f
	return output // returning zero terminal, since ESS has no assets below it
}

// ESS CalculateState() calculates and returns grid following output P and Q,
// taking into considering power factor mode, charge/discharge limits and SOC
func (e *ess) CalculateState(input terminal, dt float64) (output terminal) {
	if e.GridForming {
		return output
	}
	e.F, e.Ph = input.f, input.ph

	pcmd := e.targetPcmd
	qcmd := e.targetQcmd

	// Implement Ramping Functions
	activePRamp := 0.0
	PrampActive := false
	activeQRamp := 0.0
	QrampActive := false

	// Check which Active power ramp rate to implement and calculate the requested ramp rate
	if e.PstartActive {
		activePRamp = e.PRampStart
		PrampActive = true
	} else if e.PstopActive {
		activePRamp = e.PRampStop
	} else if (e.P < e.targetPcmd) && (e.PRampRiseEnable || e.RampEnable) {
		activePRamp = e.PRampRise
		PrampActive = true
	} else if (e.P > e.targetPcmd) && (e.PRampDropEnable || e.RampEnable) {
		activePRamp = e.PRampDrop
		PrampActive = true
	}

	// Check which Reactive power ramp rate to implement and calculate the requested ramp rate
	if e.QstartActive {
		activeQRamp = e.QRampStart
		QrampActive = true
	} else if e.QstopActive {
		activeQRamp = e.QRampStop
	} else if (e.Q < e.targetQcmd) && (e.RampEnable || e.QRampRiseEnable) {
		activeQRamp = e.QRampRise
		QrampActive = true
	} else if (e.Q > e.targetQcmd) && (e.RampEnable || e.QRampDropEnable) {
		activeQRamp = e.QRampDrop
		QrampActive = true
	}

	if !e.AbsRampRate {
		activePRamp = (activePRamp / 100) * e.Phigh
		activeQRamp = (activeQRamp / 100) * e.Qhigh
	} else {
		activePRamp = (activePRamp * 1000) / 60
		activeQRamp = (activeQRamp * 1000) / 60
	}

	if PrampActive {
		pcmd = getSlew(e.P, e.targetPcmd, activePRamp, dt)
	}
	if QrampActive {
		qcmd = getSlew(e.Q, e.targetQcmd, activeQRamp, dt)
	}

	if e.PstopActive {
		pcmd = getSlew(e.P, 0, activePRamp, dt)
	}
	if e.QstopActive {
		qcmd = getSlew(e.Q, 0, activeQRamp, dt)
	}

	// Limit power command to inverter rated active power limit
	if pcmd > e.Phigh {
		pcmd = e.Phigh
	} else if pcmd < -e.Phigh {
		pcmd = -e.Phigh
	}
	// Limit power command to inverter rated reactive power limit
	if qcmd > e.Qhigh {
		qcmd = e.Qhigh
	} else if qcmd < -e.Qhigh {
		qcmd = -e.Qhigh
	}

	soc := e.Soc / 100.0
	pchg, pdschg := e.Pcharge, e.Pdischarge
	// Limit power command to charge and discharge limits
	// Charge and discharge limits are currently hard limits (-> 0) when fully charged or discharged
	if pcmd > pdschg {
		pcmd = pdschg
	} else if pcmd < -pchg {
		pcmd = -pchg
	}
	// If power factor mode is set, override qcmd with a power factor based Q command
	if e.PfMode {
		qcmd = pfToQ(pcmd, e.PfCmd)
	}
	pcmd += powerNoise(e.Noise)
	qcmd += powerNoise(e.Noise)
	// TODO: add ramp
	if (e.On || e.PstopActive || e.QstopActive) && !e.Standby {
		socloss := e.Idleloss + e.pesr*math.Pow(pcmd, 2)
		// SOC output from getIntegral not used in CalculateState(), see UpdateState()
		_, plimited, under, over := getIntegral(soc, -(pcmd+socloss)/e.Cap, dt/3600, 0, 1)
		pchg = e.Phigh
		pdschg = e.Phigh
		if over {
			output.p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 100%
			pchg = 0
		} else if under {
			output.p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 0%
			pdschg = 0
		} else {
			output.p = pcmd
		}
		output.q = qcmd // limit Q output based on S rating, maybe move PF mode here
	} else {
		output.p, output.q = 0, 0
	}
	e.P, e.Q = output.p, output.q
	e.Pcharge, e.Pdischarge = pchg, pdschg
	// fmt.Printf("Battery %v is P: %.0fkW\tSOC: %.2f%%\tabs SOC: %.1fkWh\tdt: %.3f\n", e.ID, e.P, e.Soc, e.Soc*e.Cap/100.0, dt)
	return output
}

// ESS DistributeLoad() returns the input to it since it has no assets below it in the tree
func (e *ess) DistributeLoad(input terminal) (output terminal) {
	return input
}

// ESS UpdateState() calculates grid forming power output P and Q, considering
// the P and Q demand of the tree and its droop parameters. Then update SOC
// based on the actual output from grid following or grid forming
func (e *ess) UpdateState(input terminal, dt float64) (output terminal) {
	var p, q float64
	if !e.GridForming {
		p, q = e.P, e.Q
	} else {
		p = getY(input.f, e.Dactive.slope, e.Dactive.offset)
		q = getY(input.v, e.Dreactive.slope, e.Dreactive.offset)
	}

	if (e.On || e.PstopActive || e.QstopActive) && !e.Standby {
		socloss := e.Idleloss + e.pesr*math.Pow(p, 2)
		soc, plimited, under, over := getIntegral(e.Soc/100, -(p+socloss)/e.Cap, dt/3600, 0, 1)
		e.Pcharge = e.Phigh
		e.Pdischarge = e.Phigh
		if over {
			p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 100%
			e.Pcharge = 0
		} else if under {
			p = -plimited*e.Cap - socloss // limit power output for this tick to bring SOC to 0%
			e.Pdischarge = 0
		}
		// Actually update SOC
		e.Soc = soc * 100
	} else {
		p, q = 0, 0
	}
	e.P, e.Q = p, q
	e.S = rss(e.P, e.Q)
	e.Pf = pf(e.P, e.Q, e.S)
	e.I, e.Di, e.Qi = sToI(e.S, e.V), sToI(e.P, e.V), sToI(e.Q, e.V)
	e.VLN = e.V / sqrt3
	output.p, output.q, output.s = e.P, e.Q, e.S
	// fmt.Printf("ESS %s: P: %.1f\tQ: %.1f\tS: %.1f\tV: %.1f\tF: %.1f\n", e.ID, e.P, e.Q, e.S, e.V, e.F)
	return output
}

func (e *ess) GetID() string {
	return e.ID
}

func (e *ess) GetAliases() []string {
	return e.Aliases
}

func (e *ess) Term() terminal {
	return terminal{e.V, 0, e.P, e.Q, e.S, e.F, e.Ph, e.Dactive, e.Dreactive, droop{}}
}
