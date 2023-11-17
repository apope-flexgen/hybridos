package main

import (

)

type dcbus struct {
	ID              string
	Vdc				float64
	Pdc				float64
	P1Conv			float64     //Power draw from parent-side converting assets (e.g. PCS)
	P2Conv			float64		//Power from child-side converting assets (e.g. DCDC)
	Pind 			float64     //Power to be made up by independent source child assets (bms, PV, etc)
	Aliases         []string
	dcTerminal		terminal
	Dvolts          droop
	Plim			float64 	//Mostly a pass through for default configuration of child assets. No fault behavior for this configuration
	Vnom			float64 	//Same as above
}

func (d *dcbus) Init() {
	return
}

//Direction: down (1)
func (d *dcbus) UpdateMode(input terminal) (output terminal) {
	//processCtrlWordConfig(f, f.CtrlWord1Cfg, f.CtrlWord1)
	//d.Vdc = input.v //TODO GB this perhaps should be last step arbitrated voltage instead of coming from above. 
	d.Pind = 0 			//Independent source power should not carry over from previous step. 
	d.Pdc = 0
	output.vdc = d.Vdc
	return output
}

//Power converting assets (independent sources) send their power commands up at this stage
//store them and combine with parent power command in next step to determine delta to be made up by dependent power sources (pv, bms, etc)
//Direction: up (2)
//NO
func (d *dcbus) GetLoadLines(input terminal, dt float64) (output terminal) {
	d.Pdc = -input.p
	//terminal{f.V, 0, f.P, f.Q, f.S, f.F2, f.Ph, f.Dactive, f.Dreactive, droop{}}
	d.dcTerminal = combineTerminals(terminal{0, d.Vdc, d.Pdc, 0, 0, 0, 0, droop{}, droop{}, d.Dvolts}, input)
	output.vdc = d.dcTerminal.vdc
	return output 
}

//Determine share of power that independent child sources must make up. 
//Send down arbitrated voltage setpoint between parent and dependent children. 
//Direction: down (3)
func (d *dcbus) DistributeVoltage(input terminal) (output terminal) {
	//d.Pind = d.Pdc + input.p //Difference between parent power and child independent source power is the delta that independent sources must attempt to make up
	d.P1Conv = input.p 
	output.p = d.Pind //need to negate the delta here as PV will need to supply the opposite. 
	output.vdc = d.Vdc
	return output
}


//Direction: up (4)
func (d *dcbus) CalculateState(input terminal, dt float64) (output terminal) {
	d.Vdc = input.vdc
	d.P2Conv = input.p 
	d.Pind = d.P1Conv - d.P2Conv
	return output
}

//Direction: down (5)
func (d *dcbus) DistributeLoad(input terminal) (output terminal) {
	// pDelta := input.p - d.Pdc //Now this is the difference in power between all assets on the bus. Independent sources must make this up. Likely this will be due to losses
	output.p = d.Pind
	output.vdc = d.Vdc
	return output
}

// Feeder UpdateState() collects P and Q again now that grid forming
// assets have had a chance to participate. This time, trip thresholds
// are processed to update the mode on the next iteration.
//Direction: up (6)
func (d *dcbus) UpdateState(input terminal, dt float64) (output terminal) {
	// Check only if breaker should trip, commands are handled in UpdateMode()
	d.Vdc = input.vdc
	output.p = input.p
	return input //DC bus should pass up droop/power/voltage from children. 
}

func (d *dcbus) GetID() string {
	return d.ID
}

func (d *dcbus) GetAliases() []string {
	return d.Aliases
}

func (d *dcbus) Term() terminal {
	return terminal{0, d.Vdc, d.Pdc, 0, 0, 0, 0, droop{}, droop{}, d.Dvolts}
}
