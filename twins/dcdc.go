package main

import (

	"log"
	//"fmt"
	"math"
	"time"
)

type dcdc struct {
	// DCDC Model Identification
	ID       string
	Aliases  []string
	Plim     float64 // DCDC max power limit
	Noise1   float64 // level of noise in kW on Vdc1
	Noise2   float64 // level of noise in kW on Vdc2
	Rte      float64 // Round trip efficiency at full load
	pesr     float64 // power normalized ESR, see Init()
	Idleloss float64 // Fixed losses across load
	Vdc1     float64 // Child-side (usually BMS) DC voltage at DCDC terminal
	Vdc2     float64 // Parent-side (usually PCS) DC voltage at DCDC terminal
	Idc1     float64 // DC Current at child-side DCDC terminal
	Idc2     float64 // DC Current at parent-side DCDC terminal
	Pdc1     float64 // Child-side power
	Pdc2     float64 // Parent-side power
	// DCDC Communications - includes configurable I/O
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
	On               bool
	Oncmd            bool
	Offcmd           bool
	Standby          bool
	StandbyCmd       bool
	PMode            int     // sets type of real power control (buck/boost wrt Vdc1 - BMS)
	Pcmd             float64 // DCDC active power command
	VDC1cmd          float64 // Child-side DC voltage setpoint
	VDC2cmd          float64 // Parent-side DC voltage setpoint
	VDC1max			 float64 // Max voltage on child side
	VDC2max			 float64 // Max voltage on parent side. 
	GridForming      bool    // Grid forming status
	GridFormingCmd   bool    // Grid forming command
	GridFollowingCmd bool    // Grid following command
	DVdc1            droop   //Dvoltage in PCS
	DVdc2            droop   //Dactive in PCS
	DvoltageExternal droop
	PvTerminal       terminal
	// Faults and fault handling
	Fault    bool    // Indicates a fault of any kind has occured. Causes the inverter to turn off and open both the AC and DC contactors.
	Warning  bool    // Indicates a warning of any kind has occured. No action is taken immediately by the inverter.
	Watchdog float64 // Checks to make sure that the connection to the higher level controller is active. If it does not change regularly, it turns off the DCDC. Not implemented.
}

func (d *dcdc) Init() {
	if d.Rte == 0 {
		d.pesr = 0
	} else {
		// This gives a "power normalized ESR" so we don't have to convert to current for loading loses
		// This means we can use P^2 * pesr for loss calculations later
		d.pesr = (d.Plim*(1/math.Sqrt(d.Rte/100.0)-1) - d.Idleloss) / math.Pow(d.Plim, 2)
	}
	d.Status = processBitfieldConfig(d, d.StatusCfg)

	// TODO: should setting Plim and Qlim come from configuration?
	if d.Plim == 0 {
		d.Plim = 990
	}
	// if p.VDC1cmd == 0 {
	// 	// TODO: throw an error?
	// }
	//set max voltages to very large number if not configured.
	if d.VDC1max == 0 {
		d.VDC1max = 100000  
	}
	if d.VDC2max == 0 {
		d.VDC2max = 100000
	}
	if d.DVdc2.Percent != 0 && d.DVdc2.XNom != 0 {
		d.DVdc2.slope, d.DVdc2.offset = getSlope(d.DVdc2.Percent, d.DVdc2.YNom, d.DVdc2.XNom)
	}

	// DC droop settings need to model a "weak grid"
	if d.VDC1cmd <=0 {
		d.DVdc1.XNom = 480
	} else {
		d.DVdc1.XNom = d.VDC1cmd
	}
	d.DVdc1.YNom = 3000   //YNOM interpreted as max power - 3kW
	d.DVdc1.Percent = 0.9 //Weak source, so high percentage
	if d.VDC2cmd <= 0 {
		d.DVdc2.XNom = 480
	} else {
		d.DVdc2.XNom = d.VDC2cmd
	}
	d.DVdc2.YNom = 3000
	d.DVdc2.Percent = 0.9

	d.DVdc1.slope, d.DVdc1.offset = getSlope(d.DVdc1.Percent, d.DVdc1.YNom, d.DVdc1.XNom) // initial child-side droop settings
	d.DVdc2.slope, d.DVdc2.offset = getSlope(d.DVdc2.Percent, d.DVdc2.YNom, d.DVdc2.XNom)
	//fmt.Println("Child side droop init:", d.DVdc1)
	//fmt.Println("Parent side droop init:", d.DVdc2)
	// fmt.Println(p.ID, p.Dactive, p.Dreactive)

}

// UpdateMode DCDC UpdateMode() processes commands either through control words or direct
// commands.
func (d *dcdc) UpdateMode(input terminal) (output terminal) {
	processCtrlWordConfig(d, d.CtrlWord1Cfg, d.CtrlWord1)
	processCtrlWordConfig(d, d.CtrlWord2Cfg, d.CtrlWord2)
	processCtrlWordConfig(d, d.CtrlWord3Cfg, d.CtrlWord3)
	processCtrlWordConfig(d, d.CtrlWord4Cfg, d.CtrlWord4)

	// Turn on if conditions allow it
	if d.Oncmd && (!d.On || d.Standby) /*&& p.AcContactor && p.DcContactor*/ {
		d.On = true
		d.Oncmd = false
		d.Standby = false
	} else if d.On && d.Offcmd {
		d.On = false
		d.Offcmd = false
		d.Standby = false
	}
	if d.On && d.StandbyCmd {
		d.Standby = true
		d.StandbyCmd = false
	}

	// if DCDC is on pass information down
	if d.On {
		d.Pdc2, d.Vdc2 = input.p, input.vdc
	} else {
		d.Pdc2, d.Vdc2 = 0, 0
	}

	// fmt.Println("[UpdateMode] input.v:", input.v, " input.vdc", input.vdc, " input.p", input.p, " input.s", input.s)
	// fmt.Println("[UpdateMode] dVdc:", input.dVdc)
	return output
}

// DCDC GetLoadLines() returns droop parameters up the tree if grid forming
func (d *dcdc) GetLoadLines(input terminal, dt float64) (output terminal) {
	// fmt.Println("[DCDC][GetLoadLines] Input terminal", input)
	// Set XNom in the droops based on VDCcmd if GridForming, because it might have changed
	// otherwise XNom should be input voltage in gridfollowing mode
	if d.GridForming {
		d.DVdc1.XNom = d.VDC1cmd
	} else {
		d.DVdc1.XNom = input.vdc
	}
	d.DVdc1.slope, d.DVdc1.offset = getSlope(d.DVdc1.Percent, d.DVdc1.YNom, d.DVdc1.XNom)
	dcTerminal := terminal{
		p: d.Pdc1, dVdc: d.DVdc1,
	}
	// Recalculate slope and offset to take commands into account
	combined := combineTerminals(dcTerminal, input) // input is coming from the BMS "children" - this is the voltage on the combined bus shared with DCDC and batteries.
	d.DvoltageExternal = combined.dVdc
	d.Vdc1 = getX(d.Pdc1, d.DvoltageExternal.slope, d.DvoltageExternal.offset)
	// fmt.Println("[DCDC][GetLoadLines] d.DvoltageExternal", d.DvoltageExternal)
	output.dVdc = d.DVdc2                                     //parent-side droop.
	//In PV power control mode, we will be simply using asset Pcmd to control PV. This is used in cases where we aren't testing the actual electrical characteristics of 
	//DC- coupled storage, but instead are integrating with another controller that does those lower-level decisions.
	//in this case, we need the assets to behave as ideally as possible so we don't have to implement the low level logic of the other controller ourselves. 
	//assume that in power control mode that Vdc2cmd will be zero. 
	//TODO GB: This likely needs to be adjusted when implementing the voltage control mode, as it may be possible to 'silently' get into a non-running state with this logic. 
	if d.VDC2cmd == 0 {
		output.p = d.Pcmd
	} else {
		output.p = getX(d.VDC2cmd, d.DVdc2.slope, d.DVdc2.offset) //encode VDC cmd in droop so PCS (parent side) can share node voltage with PV siblings
	}
	
	// fmt.Println("[DCDC][GetLoadLines] output terminal (d.Vdc2):", output)
	//Now handle child side. Need to encode child side DC voltage cmd for use if PV is child of DCDC
	//Basically DCDC is acting here similar to how PCS DC side handles its children's voltage
	d.PvTerminal.p = getY(d.VDC1cmd, d.DVdc1.slope, d.DVdc1.offset) //encode vdc cmd in droop to send down to PV For PV power arbitration
	d.PvTerminal.dVdc = d.DVdc1
	d.PvTerminal = combineTerminals(d.PvTerminal, input) //DCDC may have other indpendent voltage sources as children alongside dependent sources, so handle those.
	// if !d.GridForming {
	// 	output = terminal{} //grid following assets should return zero terminal here
	// }
	return output
}

// DCDC DistributeVoltage() accepts the voltage set from upstream, either
// by a grid directly or through droop. The DCDC turns off if no voltage
// is present and updates its status output.
func (d *dcdc) DistributeVoltage(input terminal) (output terminal) {
	// fmt.Println("[DCDC][DistributeVoltage] input terminal", input)
	var vltLimit float64
	if d.GridForming {
		vltLimit = d.VDC2cmd * 1.1
	} else {
		vltLimit = d.VDC2max * 1.1
	}
	if input.vdc <= 0 || input.vdc > vltLimit { // Turn off if input voltage is 0 after grid forming phase or if input voltage is 10% higher than max voltage or voltage command. 
		if d.On {
			log.Println("DCDC", d.ID, "input voltage out of range, turning off. Input voltage:", input.vdc)
		}
		d.On, d.Oncmd, d.Offcmd = false, false, false
		d.Standby, d.StandbyCmd = false, false
		
	}
	assetStatus := processBitfieldConfig(d, d.StatusCfg)
	for i, v := range assetStatus {
		d.Status[i] = v
	}

	//If grid following set XNOM to parent-side vdc voltage, otherwise set to voltage setpoint.
	if !d.GridForming && d.On {
		d.DVdc2.XNom = input.vdc
	} else if d.On {
		d.DVdc2.XNom = d.VDC2cmd
	}
	d.DVdc2.slope, d.DVdc2.offset = getSlope(d.DVdc2.Percent, d.DVdc2.YNom, d.DVdc2.XNom)
	//fmt.Println("[DCDC][DistributeVoltage] d.DVdc2", d.DVdc2)
	// pass DC voltage down BMS downstream of DCDC
	if d.On /*&& p.DcContactor*/ {
		output = d.PvTerminal
	} else {
		output.vdc = 0
		output.dVdc = droop{}
	}
	return output
}

func (d *dcdc) CalculateState(input terminal, dt float64) (output terminal) {
	//TODO GB: confirm DCDC acts as grid-forming asset
	//fmt.Println("[DCDC][CalculateState] input terminal:", input)
	if d.GridForming {
		return output
	}
	pcmd := d.Pcmd //to PCS
	// Limit power command to inverter rated active power limit
	if pcmd > d.Plim {
		pcmd = d.Plim
	} else if pcmd < -d.Plim {
		pcmd = -d.Plim
	}
	pcmd += powerNoise(d.Noise2)
	// Calculate losses and delivered power
	dcdcloss := d.Idleloss + d.pesr*math.Pow(pcmd, 2)
	pdel := pcmd + dcdcloss //from BMS

	if d.On && !d.Standby {
		output.p = pcmd
	} else {
		output.p = 0
	}

	d.Pdc1, d.Pdc2 = pdel, pcmd
	d.PvTerminal = input
	// fmt.Printf("Battery %v is P: %.0fkW\tSOC: %.2f%%\tabs SOC: %.1fkWh\tdt: %.3f\n", p.ID, p.P, p.Soc, p.Soc*p.Cap/100.0, dt)
	//fmt.Println("[DCDC][CalculateState] output terminal", output)
	output.dVdc = d.DVdc2
	return output
}

func (d *dcdc) DistributeLoad(input terminal) (output terminal) {
	if !d.GridForming {
		d.DVdc2.XNom = input.vdc
	}
	power := d.Pcmd
	//Calculate losses and delivered power
	dcdcloss := d.Idleloss + d.pesr*math.Pow(power, 2)
	//fmt.Println("[DCDC][UpdateState] dcdcloss", dcdcloss)
	pdel := dcdcloss + d.Pcmd

	if d.PvTerminal.p != 0 {
		if d.PvTerminal.p > d.Plim {
			output.p = d.PvTerminal.p - d.Plim
		} else if d.PvTerminal.p < -d.Plim {
			output.p = d.Plim + d.PvTerminal.p
		} else {
			output.p = d.PvTerminal.p
		}
	} else {
		if pdel > d.Plim {
			output.p = pdel - d.Plim
		} else if pdel < -d.Plim {
			output.p = d.Plim + pdel
		} else {
			output.p = pdel
		}
	}
	output.dVdc = d.PvTerminal.dVdc //contains the combined slope information for all children, so they can figure out their share of power to make up based on the ratio of their droop slope to this droop slope
	output.vdc = getX(d.Pdc2, d.DvoltageExternal.slope, d.DvoltageExternal.offset)
	return output
}

func (d *dcdc) UpdateState(input terminal, dt float64) (output terminal) {

	// fmt.Println("[DCDC][UpdateState] input terminal:", input)
	//update heartbeat and time
	updateHeartTime(&d.Heart)
	d.Heartbeat = d.Heart.Heartbeat
	d.Time = d.Heart.Time
	d.Year = d.Heart.Year
	d.Month = d.Heart.Month
	d.Day = d.Heart.Day
	d.Hour = d.Heart.Hour
	d.Minute = d.Heart.Minute
	d.Second = d.Heart.Second

	//Update child-side DC bus voltage
	power := d.Pcmd
	//fmt.Println("[DCDC][UpdateState] power:", power)

	//Calculate losses and delivered power
	dcdcloss := d.Idleloss + d.pesr*math.Pow(power, 2)
	//fmt.Println("[DCDC][UpdateState] dcdcloss", dcdcloss)
	pdel := 0.0
	if d.On && !d.Standby {
		if power > d.Plim {
			power = d.Plim
		} else if power < -d.Plim {
			power = -d.Plim
		}
		pdel = power + dcdcloss
		if input.p == 0 { //BMS contactors open
			power, pdel = 0, 0
		}
	} else { //dcdc not converting power.
		power, pdel = 0, 0
	}
	d.Pdc2, d.Pdc1 = power, pdel  //Parent side, child side
	d.Idc1 = pToI(d.Pdc1, d.Vdc1) // child side ('input') current (from BMS)
	d.Idc2 = pToI(d.Pdc2, d.Vdc2) // parent side ('output') current (to PCS)

	output.p = d.Pdc2
	//output.vdc = getX(pdel, d.DVdc2.slope, d.DVdc2.offset) //too low of a voltage. DCDC should 'tell' PCS it is outputting 480v and let PCS handle what voltage really is with droop combination. TODO GB this <-
	output.vdc = d.DVdc2.XNom //What DCDC is 'trying' to output
	//d.DVdc2.XNom = d.Vdc2
	// fmt.Println("[DCDC][UpdateState] parent side voltage", d.Vdc2)
	output.dVdc = d.DVdc2
	// fmt.Println("[DCDC][UpdateState] output", output)
	return output
}

func (d *dcdc) GetID() string {
	return d.ID
}

func (d *dcdc) GetAliases() []string {
	return d.Aliases
}

func (d *dcdc) Term() terminal {
	return terminal{0, d.Vdc1, d.Pdc1, 0, 0, 0, 0, droop{}, droop{}, d.DVdc1}
}
