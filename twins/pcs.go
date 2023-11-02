package main

import (
	// "fmt"
	"math"
	"time"
)

// TODO: PCS may have constant power OR constant current mode
// TODO: need to add protection limits to PCS configuration
type pcs struct {
	// PCS Model Identification
	ID      string
	Aliases []string
	// PCS physical parameters - limits, losses, etc
	Plim     float64 // Inverter max active power limit (constant)
	Qlim     float64 // Inverter max reactive power limit (constant)
	Slim     float64 // Inverter max apparent power limit (constant)
	Noise    float64 // level of kW noise on Pcmd and Qcmd following
	Rte      float64 // Round trip efficiency at full load
	pesr     float64 // power normalized ESR, see Init()
	Idleloss float64 // Fixed losses across load
	IvCurve  float64 // not used
	// PCS Measured values
	Vac     float64
	Vdc     float64
	Iac     float64
	Idc     float64
	Di      float64 // Active power current
	Qi      float64 // Reactive power current
	P       float64 // AC side power
	Pdc     float64 // DC side power
	Q       float64
	S       float64
	Pf      float64
	F       float64 // Real frequency, after any adjustments made by pcs in gridforming mode
	Ph      float64 // phase angle
	Fadjust float64 // Unadjusted frequency (frequency to adjust to). This is the frequency due to active power droop losses for gridforming mode
	// PCS Communications - includes configurable I/O
	Heart        hearttime
	Heartbeat    int
	Time         time.Time
	Year         float64
	Month        float64
	Day          float64
	Hour         float64
	Minute       float64
	Second       float64
	Status       []bitfield
	StatusCfg    []bitfieldcfg
	CtrlWord1    int
	CtrlWord1Cfg []ctrlwordcfg
	CtrlWord2    int
	CtrlWord2Cfg []ctrlwordcfg
	CtrlWord3    int
	CtrlWord3Cfg []ctrlwordcfg
	CtrlWord4    int
	CtrlWord4Cfg []ctrlwordcfg
	// On, off, and contactor controls
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
	// Feedback Control Modes and commands
	PMode            int     // sets type of real power control
	QMode            int     // sets type of reactive power control
	Pcmd             float64 // Inverter active power command
	Qcmd             float64 // Inverter reactive power command
	Idcmd            float64 // Inverter d-axis current command
	Iqcmd            float64 // Inverter q-axis current command
	VACcmd           float64 // Voltage setpoint in grid forming or voltage support reactive power control
	VDCcmd           float64 // DC voltage setpoint
	Fcmd             float64 // Freqency setpoint in grid forming
	PfMode           bool    // overrides Qcmd based on PfCmd
	PfCmd            float64 // Power factor command (-1-1)
	GridForming      bool    // Grid forming status
	GridFormingCmd   bool    // Grid forming command
	GridFollowingCmd bool    // Grid following command
	// PCS Ramping Behavior
	PRampStartEnable bool    // enables real power ramping when inverter starts
	PRampStart       float64 // ramp rate for real power on inverter start
	PRampRiseEnable  bool    // enables ramping when inverter real power reference increases
	PRampRise        float64 // ramp rate for real power on power reference increase
	PRampDropEnable  bool    // enables ramping when inverter real power reference decreases
	PRampDrop        float64 // ramp rate for real power on power reference decrease
	PRampStopEnable  bool    // enables real power ramping when inverter stops
	PRampStop        float64 // ramp rate for real power on inverter stop
	QRampStartEnable bool    // enables reactive power ramping when inverter starts
	QRampStart       float64 // ramp rate for reactive power on inverter start
	QRampRiseEnable  bool    // enables ramping when inverter reactive power reference increases
	QRampRise        float64 // ramp rate for reactive power on power reference increase
	QRampDropEnable  bool    // enables ramping when inverter reactive power reference decreases
	QRampDrop        float64 // ramp rate for reactive power on power reference decrease
	QRampStopEnable  bool    // enables reactive power ramping when inverter stops
	QRampStop        float64 // ramp rate for reactive power on inverter stop
	RampEnable       bool    // if true, enable all ramp functions
	targetPcmd       float64 // assign Pcmd to this variable in updateMode to avoid Pcmd changing during execution
	targetQcmd       float64 // assign Qcmd to this varialbe in updateMode to avoid Qcmd changing during execution
	onTransition     bool    // if true, signals a transition from an off state to an on state
	offTransition    bool    // if true, signals a transition from an on state to an off state
	PstartActive     bool    // enables PRampStart ramping if there is an on state transition
	PstopActive      bool    // enables PRampStop ramping if there is an off state transition
	Dt               float64 // used for debugging, sets a DeltaT that can be used as needed
	QstartActive     bool    // enables QRampStart ramping if there is an on state transition
	QstopActive      bool    // enables QRampStop ramping if there is an off state transition
	AbsRampRate      bool    // if true, signals that the ramp rates are measured in absolute values instead of percentage power per second
	// Droop parameters
	Dactive          droop
	Dreactive        droop
	Dvoltage         droop
	DvoltageExternal droop
	// Faults and fault handling
	Fault    bool    // Indicates a fault of any kind has occured. Causes the inverter to turn off and open both the AC and DC contactors.
	Warning  bool    // Indicates a warning of any kind has occured. No action is taken immediately by the inverter.
	Watchdog float64 // Checks to make sure that the connection to the higher level controller is active. If it does not change regularly, it turns off the PCS. Not implemented.
}

func (p *pcs) Init() {
	if p.Rte == 0 {
		p.pesr = 0
	} else {
		// This gives a "power normalized ESR" so we don't have to convert to current for loading loses
		// This means we can use P^2 * pesr for loss calculations later
		p.pesr = (p.Plim*(1/math.Sqrt(p.Rte/100.0)-1) - p.Idleloss) / math.Pow(p.Plim, 2)
	}
	p.Status = processBitfieldConfig(p, p.StatusCfg)

	// TODO: should setting Plim and Qlim come from configuration?
	if p.Plim == 0 {
		p.Plim = 990
	}
	if p.Qlim == 0 {
		p.Qlim = 990
	}
	if p.Slim == 0 {
		p.Slim = rss(p.Plim, p.Qlim)
	}
	// if p.VDCcmd == 0 {
	// 	// TODO: throw an error?
	// }

	// DC droop settings need to model a "weak grid"
	if p.VDCcmd > 0 {
		p.Dvoltage.XNom = p.VDCcmd
	} else {
		p.Dvoltage.XNom = 1330 //TODO GB: find a way to do this with default configs.
	}
	p.Dvoltage.YNom = p.Slim
	p.Dvoltage.Percent = 0.2
	p.Dvoltage.slope, p.Dvoltage.offset = getSlope(p.Dvoltage.Percent, p.Dvoltage.YNom, p.Dvoltage.XNom) // initial DC droop settings
	// fmt.Println(p.ID, p.Dactive, p.Dreactive)
	//Dactive droop is frequency (x axis) vs active power (y axis)
	if p.Fcmd > 0 {
		p.Dactive.XNom = p.Fcmd
	} else {
		p.Dactive.XNom = 60
	}
	p.Dactive.YNom = p.Plim
	p.Dactive.Percent = 0.2
	//Dreactive droop is AC voltage (x axis) vs reactive power (y axis)
	if p.VACcmd > 0 {
		p.Dreactive.XNom = p.VACcmd
	} else {
		p.Dreactive.XNom = 480
	}
	p.Dreactive.YNom = p.Qlim
	p.Dreactive.Percent = 0.2
	p.Dactive.slope, p.Dactive.offset = getSlope(p.Dactive.Percent, p.Dactive.YNom, p.Dactive.XNom)
	p.Dreactive.slope, p.Dreactive.offset = getSlope(p.Dreactive.Percent, p.Dreactive.YNom, p.Dreactive.XNom)
	p.PstartActive, p.PstopActive = false, false
}

// PCS UpdateMode() processes commands either through control words or direct
// commands.
func (p *pcs) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(p, p.CtrlWord1Cfg, p.CtrlWord1)
	processCtrlWordConfig(p, p.CtrlWord2Cfg, p.CtrlWord2)
	processCtrlWordConfig(p, p.CtrlWord3Cfg, p.CtrlWord3)
	processCtrlWordConfig(p, p.CtrlWord4Cfg, p.CtrlWord4)

	// ContactorControl allows discrete control over DC and AC contactors
	// If not set, only Oncmd or Offcmd are needed
	// A Fault overrides ContactorControl, forcing both contactors to open
	p.onTransition, p.offTransition = false, false

	if p.GridFormingCmd {
		p.GridForming, p.GridFormingCmd = true, false
	} else if p.GridFollowingCmd && p.GridForming {
		p.GridForming, p.GridFollowingCmd = false, false
	}

	// Turn on if conditions allow it
	if p.Oncmd && (!p.On || p.Standby) && p.AcContactor && p.DcContactor && !(p.PstopActive || p.QstopActive) {
		p.On = true
		p.Oncmd = false
		p.Standby = false
		p.onTransition = true
		p.targetPcmd = p.Pcmd
		p.targetQcmd = p.Qcmd
	} else if (p.On || p.PstopActive || p.QstopActive) && p.Offcmd {
		p.On = false
		p.Offcmd = false
		p.Standby = false
		p.offTransition = true
		p.targetPcmd = 0
		p.targetQcmd = 0
	}

	// If Grid Following, set up flag to ensure that Ramping occurs when a power transition is present
	if p.On && p.StandbyCmd {
		p.Standby = true
		p.StandbyCmd = false
	}
	// If Grid Following, set up flags to determine whether there should be onTransition ramping or offTransition ramping
	if !p.GridForming {
		if (p.offTransition || p.PstopActive) && (p.PRampStopEnable || p.RampEnable) {
			p.PstopActive = true
		}
		if (!p.PRampStopEnable && !p.RampEnable) || p.onTransition || p.Pdc == 0 {
			p.PstopActive = false
		}
		if (p.onTransition || p.PstartActive) && (p.PRampStartEnable || p.RampEnable) {
			p.PstartActive = true
		}
		if (p.Pcmd != p.targetPcmd && !p.onTransition) || (!p.PRampStartEnable && !p.RampEnable) || p.offTransition {
			p.PstartActive = false
		}

		// Reactive power for setting up flags to determine whether there should be onTransition ramping or offTransition ramping
		if (p.offTransition || p.QstopActive) && (p.QRampStopEnable || p.RampEnable) {
			p.QstopActive = true
		}
		if (p.onTransition || p.QstartActive) && (p.QRampStartEnable || p.RampEnable) {
			p.QstartActive = true
		}
		if (!p.QRampStopEnable && !p.RampEnable) || p.onTransition || p.Q == 0 {
			p.QstopActive = false
		}
		if (p.Qcmd != p.targetQcmd && !p.onTransition) || (!p.QRampStartEnable && !p.RampEnable) || p.offTransition {
			p.QstartActive = false
		}
	}
	if !(p.PstopActive || p.QstopActive) {
		p.targetPcmd = p.Pcmd // Guarentee that Pcmd used is constant at a given timestamp
		p.targetQcmd = p.Qcmd // Guarentee that Qcmd used is constant at a given timestamp

		if p.Fault {
			p.AcContactorOpenCmd, p.DcContactorOpenCmd = true, true
		} else if !p.ContactorControl {
			if !p.On && p.Oncmd {
				p.AcContactorCloseCmd, p.DcContactorCloseCmd = true, true
			} else if p.On && p.Offcmd {
				p.AcContactorOpenCmd, p.DcContactorOpenCmd = true, true
			}
		}
		if !p.AcContactor && p.AcContactorCloseCmd {
			p.AcContactor, p.AcContactorCloseCmd = true, false
		} else if p.AcContactor && p.AcContactorOpenCmd {
			p.AcContactor, p.AcContactorOpenCmd = false, false
			p.Offcmd = true
		}
		if !p.DcContactor && p.DcContactorCloseCmd {
			p.DcContactor, p.DcContactorCloseCmd = true, false
		} else if p.DcContactor && p.DcContactorOpenCmd {
			p.DcContactor, p.DcContactorOpenCmd = false, false
			p.Offcmd = true
		}
	}

	// if PCS is on and contactors closed, pass information down
	if (p.On || p.PstopActive || p.QstopActive) && p.AcContactor {
		if p.GridForming {
			p.Vac = p.VACcmd
			p.F, p.Ph = input.f, input.ph
		} else {
			p.P, p.Vac = input.p, input.v
			p.F, p.Ph = input.f, input.ph
		}
	} else {
		p.P, p.Q, p.Vac = 0, 0, 0
		p.F, p.Ph = 0, 0
		p.targetPcmd, p.targetQcmd = 0, 0
	}

	return output
}

// PCS GetLoadLines() returns droop parameters up the tree if grid forming
// TODO GB: Why is this using DC-side information to send up the tree (AC-side?)
func (p *pcs) GetLoadLines(input terminal, dt float64) (output terminal) {
	dcTerminal := terminal{
		p: p.P, dVdc: p.Dvoltage,
	}
	p.Dt = dt
	// Set XNom in the droops based because they might have changed
	if p.VDCcmd > 0 {
		dcTerminal.dVdc.XNom = p.VDCcmd
	}
	// Recalculate slope and offset to take commands into account
	dcTerminal.dVdc.slope, dcTerminal.dVdc.offset = getSlope(dcTerminal.dVdc.Percent, dcTerminal.dVdc.YNom, dcTerminal.dVdc.XNom)

	// Do combine terminals here since the PCS is the root of the tree
	// and needs to store external droop information for DistributeVoltage()
	output = combineTerminals(dcTerminal, input) // input is coming from the BMS "children"
	p.DvoltageExternal = output.dVdc

	// "AC-side" of the PCS, similar to ESS
	if p.GridForming && (p.On || p.PstopActive || p.QstopActive) {
		output.dHertz = p.Dactive // Nominal rating is saved in dHertz, but
		if p.Fcmd > 0 {
			output.dHertz.XNom = p.Fcmd // commands can override
		}
		output.dVolts = p.Dreactive
		if p.VACcmd > 0 {
			output.dVolts.XNom = p.VACcmd
		}
		output.p = p.P // p.P and p.Q contain the last iteration's output at this
		output.q = p.Q // point in the solver, so droop has a one tick delay
		//output.f = getX(p.P, p.Dactive.slope, p.Dactive.offset)
	}

	return output
}

// PCS DistributeVoltage() accepts the voltage set from upstream, either
// by a grid directly or through droop. The PCS turns off if no voltage
// is present and updates its status output.
func (p *pcs) DistributeVoltage(input terminal) (output terminal) {
	// fmt.Println("DistributeVoltage V:", input.v)
	if input.v <= 0 { // Turn off if input voltage is 0 after grid forming phase
		p.On, p.Oncmd, p.Offcmd = false, false, false
		p.Standby, p.StandbyCmd = false, false
	}
	assetStatus := processBitfieldConfig(p, p.StatusCfg)
	copy(p.Status, assetStatus)
	// fmt.Println(p.Status)
	// if p.GridForming && p.On {
	// 	p.Vac, p.Ph, p.F = p.VACcmd, 0, p.Fcmd
	// } else {
	p.Vac, p.Ph, p.F = input.v, input.ph, input.f
	// }

	// pass DC voltage down BMS downstream of PCS
	// in this way, PCS is more of a feeder
	// send power setpoint down for dependent sources (pv) that need it to arbitrate their own setpoints.
	if (p.On || p.PstopActive || p.QstopActive) && p.DcContactor {
		output.vdc = p.VDCcmd
		output.p = p.targetPcmd
	} else {
		output.vdc = 0
	}
	return output
}

// PCS CalculateState() calculates and returns grid following output P and Q,
// taking into considering power factor mode, power limits
func (p *pcs) CalculateState(input terminal, dt float64) (output terminal) {
	if p.GridForming {
		return output
	}
	p.F, p.Ph = input.f, input.ph //TODO GB: Why is PCS taking F and Ph from below (DC side?)
	pcmd := p.targetPcmd
	qcmd := p.targetQcmd

	// Implement Ramping Functions
	activePRamp := 0.0
	PrampActive := false
	activeQRamp := 0.0
	QrampActive := false

	// Check which Active power ramp rate to implement and calculate the requested ramp rate
	if p.PstartActive {
		activePRamp = p.PRampStart
		PrampActive = true
	} else if p.PstopActive {
		activePRamp = p.PRampStop
	} else if (p.Pdc < p.targetPcmd) && (p.PRampRiseEnable || p.RampEnable) {
		activePRamp = p.PRampRise
		PrampActive = true
	} else if (p.Pdc > p.targetPcmd) && (p.PRampDropEnable || p.RampEnable) {
		activePRamp = p.PRampDrop
		PrampActive = true
	}

	// Check which Reactive power ramp rate to implement and calculate the requested ramp rate
	if p.QstartActive {
		activeQRamp = p.QRampStart
		QrampActive = true
	} else if p.QstopActive {
		activeQRamp = p.QRampStop
	} else if (p.Q < p.targetQcmd) && (p.RampEnable || p.QRampRiseEnable) {
		activeQRamp = p.QRampRise
		QrampActive = true
	} else if (p.Q > p.targetQcmd) && (p.RampEnable || p.QRampDropEnable) {
		activeQRamp = p.QRampDrop
		QrampActive = true
	}

	if !p.AbsRampRate {
		activePRamp = (activePRamp / 100) * p.Plim
		activeQRamp = (activeQRamp / 100) * p.Qlim
	} else {
		activePRamp = (activePRamp * 1000) / 60
		activeQRamp = (activeQRamp * 1000) / 60
	}

	if PrampActive {
		pcmd = getSlew(p.Pdc, p.targetPcmd, activePRamp, dt)
	}
	if QrampActive {
		qcmd = getSlew(p.Q, p.targetQcmd, activeQRamp, dt)
	}

	if p.PstopActive {
		pcmd = getSlew(p.Pdc, 0, activePRamp, dt)
	}
	if p.QstopActive {
		qcmd = getSlew(p.Q, 0, activeQRamp, dt)
	}
	p.Pdc = pcmd

	// Limit power command to inverter rated active power limit
	if pcmd > p.Plim {
		pcmd = p.Plim
	} else if pcmd < -p.Plim {
		pcmd = -p.Plim
	}
	// Limit power command to inverter rated reactive power limit
	if qcmd > p.Qlim {
		qcmd = p.Qlim
	} else if qcmd < -p.Qlim {
		qcmd = -p.Qlim
	}
	// If power factor mode is set, override qcmd with a power factor based Q command
	if p.PfMode {
		qcmd = pfToQ(pcmd, p.PfCmd)
	}

	pcmd += powerNoise(p.Noise)
	qcmd += powerNoise(p.Noise)

	// Calculate losses and delivered power
	pcsloss := p.Idleloss + p.pesr*math.Pow(pcmd, 2)
	pdel := pcmd + pcsloss

	if (p.On || p.PstopActive || p.QstopActive) && !p.Standby {
		output.p, output.q = pdel, qcmd
	} else {
		output.p, output.q = 0, 0
	}
	p.P, p.Q = output.p, output.q
	// fmt.Printf("Battery %v is P: %.0fkW\tSOC: %.2f%%\tabs SOC: %.1fkWh\tdt: %.3f\n", p.ID, p.P, p.Soc, p.Soc*p.Cap/100.0, dt)
	return output
}

// PCS DistributeLoad() passes down the collected P and Q of the tree
// for grid forming assets to respond to. It is either passed down from
// above if closed, otherwise returned from the P and Q stored on
// CalculateState()
func (p *pcs) DistributeLoad(input terminal) (output terminal) {
	// TODO: implement losses here from AC power to DC power during conversion
	if p.On {
		if p.GridForming {
			p.Fadjust = input.f
			output.p = getY(p.Fadjust, p.Dactive.slope, p.Dactive.offset) // send power down, because we return zero-terminal in CalculateState() in grid-forming TODO GB: should this be p.P?
			output.vdc = getX(input.p, p.DvoltageExternal.slope, p.DvoltageExternal.offset)
		} else {
			//Previously this was p.P, however that includes losses and our convention is to put losses on output and draw pcmd kW from battery.
			//TODO GB: Is this convention right? Should we instead output pcmd kW and draw pcmd + losses?
			output.p = p.Pdc // still send power down, because we calculated PCS power at the end of CalculateState() in grid-following.
			output.vdc = getX(p.P, p.DvoltageExternal.slope, p.DvoltageExternal.offset)
		}
		//Limit output to Plim to prevent assets below from being overpowered. This can happen especially in gridforming if the BMS power is particularly low.
		if output.p > p.Plim {
			output.p = p.Plim
		} else if output.p < -p.Plim {
			output.p = -p.Plim
		}
	}

	return output
}

// PCS UpdateState() calculates grid forming power output P and Q, considering
// the P and Q demand of the tree and its droop parameters.
func (p *pcs) UpdateState(input terminal, dt float64) (output terminal) {
	var active, reactive float64
	if !p.GridForming {
		active, reactive = p.Pdc, p.Q
	} else {
		// grid-forming mode, power commands are not available and must be calculated based on droop
		p.Dactive.slope, p.Dactive.offset = getSlope(p.Dactive.Percent, p.Dactive.YNom, p.Dactive.XNom)
		p.Dreactive.slope, p.Dreactive.offset = getSlope(p.Dreactive.Percent, p.Dreactive.YNom, p.Dreactive.XNom)
		active = getY(p.Fadjust, p.Dactive.slope, p.Dactive.offset)
		reactive = getY(p.Vac, p.Dreactive.slope, p.Dreactive.offset)
	}

	// Update heartbeat and time
	updateHeartTime(&p.Heart)
	p.Heartbeat = p.Heart.Heartbeat
	p.Time = p.Heart.Time
	p.Year = p.Heart.Year
	p.Month = p.Heart.Month
	p.Day = p.Heart.Day
	p.Hour = p.Heart.Hour
	p.Minute = p.Heart.Minute
	p.Second = p.Heart.Second

	// Update DC bus voltage
	p.Vdc = input.vdc

	// TODO: throw a fault here if you are above the limits of the PCS
	if (p.On || p.PstopActive || p.QstopActive) && !p.Standby {
		// Limit power to inverter rated active power limit
		if active > p.Plim {
			active = p.Plim
		} else if active < -p.Plim {
			active = -p.Plim
		}
		// Limit power to power available from the BMS units
		if active > 0 && active > input.p {
			active = input.p
		} else if active < 0 && active < input.p {
			active = input.p
		}
		// Limit power to inverter rated reactive power limit
		if reactive > p.Qlim {
			reactive = p.Qlim
		} else if reactive < -p.Qlim {
			reactive = -p.Qlim
		}
	} else {
		active, reactive = 0, 0
	}

	// Calculate losses and delivered power
	pcsloss := p.Idleloss + p.pesr*math.Pow(active, 2)
	pdel := active - pcsloss

	p.P, p.Q = pdel, reactive
	p.Pdc = active
	p.S = rss(p.P, p.Q)
	p.Pf = pf(p.P, p.Q, p.S)
	p.Iac, p.Di, p.Qi = sToI(p.S, p.Vac), sToI(p.P, p.Vac), sToI(p.Q, p.Vac)
	p.Idc = pToI(p.Pdc, p.Vdc)
	output.p, output.q, output.s = p.P, p.Q, p.S
	//	p.Dvoltage.YNom = p.Pdc //set up DC droop for next execution.
	if p.VDCcmd > 0 {
		p.Dvoltage.XNom = p.VDCcmd
	} else {
		p.Dvoltage.XNom = p.Vdc //set up DC droop for next execution
	}
	return output
}

func (p *pcs) GetID() string {
	return p.ID
}

func (p *pcs) GetAliases() []string {
	return p.Aliases
}

func (p *pcs) Term() terminal {
	return terminal{p.Vac, 0, p.P, p.Q, p.S, p.F, p.Ph, p.Dactive, p.Dreactive, droop{}}
}
