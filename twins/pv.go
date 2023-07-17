package main

import (
	"time"
	"log"
)

type pv struct {
	ID             string
	Aliases        []string
	Idc            float64
	Pdc            float64
	Vdc            float64
	Fault          bool
	MaxCurr        float64 //maximum PV current (corresponds with 0V in I/V transfer curve). Dependent variable Imax = Pmax / Vmax
	MaxVlt         float64 //PV output voltage at 100% insolation and 0A current draw.
	Pmax		   float64 //Rated PV maximum power
	Isc            float64 //this iteration short circuit current (after irradiation effects)
	Voc            float64 //this iteration open circuit voltage (after irradiation effects)
	Status         []bitfield
	StatusCfg      []bitfieldcfg
	CtrlWord1      int
	CtrlWord1Cfg   []ctrlwordcfg
	CtrlWord2      int
	CtrlWord2Cfg   []ctrlwordcfg
	VdcProfile     []float64 //Percent of max VDC.
	VdcProfileInv  []float64 //reverse order of the VDC profile for power control mode voltage lookup. 
	IdcProfile     []float64 //Percent of max current. Allows same I-V profile to be scaled without changing profile breakpoints
	PdcProfile	   []float64 //Percent of max power. Equal to IdcProfile .* VdcProfile (in MATLAB/Octave parlance. In other words element-by-element multiplication)
	PdcProfileInv  []float64 //reverse order of the PDC profile for power control mode voltage lookup. 
	IrrPctBreakpts []float64
	IrrCurrEffect  []float64
	IrrVoltEffect  []float64
	IrradiationCmd float64 //Commanded irradiation percent
	IrradiationPct float64 //solar irradiation in percentage of maximum solar flux density
	IrrIdcScalar   float64 //effect of irradiation on PV short circuit current
	IrrVdcScalar   float64 //effect of irradiation on PV open circuit voltage
	DVolts         droop
	Heart          hearttime
	Heartbeat      int
	Time           time.Time
	Year           float64
	Month          float64
	Day            float64
	Hour           float64
	Minute         float64
	Second         float64
	PCtrlMode	   bool
	PCtrlModeCmd   bool
	VltCtrlModeCmd bool
	Pdel		   float64 //delta power to make up if operating in power control mode. 
}

func (pv *pv) Init() {
	pv.PCtrlModeCmd = true //For now hard code PV model to be in Power control mode. remove this once voltage control mode is more mature. 
	pv.VltCtrlModeCmd = false
	if len(pv.VdcProfile) != len(pv.IdcProfile) {
		log.Fatal("PV:", pv.ID, "VdcProfile has different dimensions than IdcProfile. Please check configs")
	}
	if pv.PCtrlModeCmd && pv.VltCtrlModeCmd {
		log.Fatal("PV:", pv.ID, "Power and Voltage control command flags both set. Only one should be set at a time. Please check configs")
	}
	if pv.PCtrlModeCmd {
		pv.PCtrlMode = true
	}
	pv.PCtrlModeCmd = false
	pv.VltCtrlModeCmd = false

	pv.Vdc = pv.MaxVlt
	pv.DVolts.XNom = pv.MaxVlt
	pv.DVolts.YNom = pv.Pmax
	pv.DVolts.Percent = 0.0000001 //Model PV as extremely stiff source. We will be handling I-V transfer curve ourselves by interpolation.
	pv.DVolts.slope, pv.DVolts.offset = getSlope(pv.DVolts.XNom, pv.DVolts.YNom, pv.DVolts.Percent)
	//Determine Imax from Vmax, Pmax, and I-V profile
	//First determine percent of max voltage that corresponds to maximum power
	for i := range pv.VdcProfile {
		pv.PdcProfile = append(pv.PdcProfile, pv.IdcProfile[i] * pv.VdcProfile[i]) //TODO GB: Future upgrade may be to define a more granluar vdc step, interpolate Pdc/IdcProfile to that step, and then find a tighter PmaxPct. 
	}
	PmaxVal, PmaxIdx := maximum(pv.PdcProfile ...)
	for i,val := range pv.PdcProfile {
		pv.PdcProfile[i] = val / PmaxVal
	}
	//next scale pv.MaxCurr to this value. This allows us to configure the PV asset with a Pmax and Voc which is closer to nameplate ratings than Vmax and Imax. 
	VMpp := pv.VdcProfile[PmaxIdx] * pv.MaxVlt
	IMpp := (pv.Pmax * 1000) / VMpp
	pv.MaxCurr = IMpp / pv.IdcProfile[PmaxIdx]
	if pv.IrradiationCmd >= 0 && pv.IrradiationCmd <= 1.0 {
		pv.IrradiationPct = pv.IrradiationCmd
	} else {
		pv.IrradiationPct = 1.0
	}
	//yi = interpl(xvec, yvec, xi) linearly interpolates a value yi from an input xi along a curve defined by [xvec,yvec]
	//these set the irradiation effect on Voc and Imax, in other words reduces them if insolation is not at maximum. 	
	pv.IrrIdcScalar, _ = interpl(pv.IrrPctBreakpts, pv.IrrCurrEffect, pv.IrradiationPct)
	pv.IrrVdcScalar, _ = interpl(pv.IrrPctBreakpts, pv.IrrVoltEffect, pv.IrradiationPct)
	pv.Voc = pv.MaxVlt * pv.IrrVdcScalar
	pv.Isc = pv.MaxCurr * pv.IrrIdcScalar
}

//PV UpdateMode() get insolation from command
//updates internal state appropriately
//return zero termninal as PV is leaf node.
//Direction: down (1)
func (pv *pv) UpdateMode(input terminal) (output terminal) {
	// if pv.PCtrlModeCmd {
	// 	pv.PCtrlMode = true
	// 	pv.PCtrlModeCmd = false
	// } else if pv.VltCtrlModeCmd {
	// 	pv.PCtrlMode = false 
	// 	pv.VltCtrlModeCmd = false
	// }
	_, PmaxIdx := maximum(pv.PdcProfile ...)
	//next scale pv.MaxCurr to this value. This allows us to configure the PV asset with a Pmax and Voc which is closer to nameplate ratings than Vmax and Imax. 
	VMpp := pv.VdcProfile[PmaxIdx] * pv.MaxVlt
	IMpp := (pv.Pmax * 1000) / VMpp
	pv.MaxCurr = IMpp / pv.IdcProfile[PmaxIdx]
	
	if pv.IrradiationCmd >= 0 && pv.IrradiationCmd <= 1.0 {
		pv.IrradiationPct = pv.IrradiationCmd
	} else {
		pv.IrradiationPct = 1.0
	}
	//yi = interpl(xvec, yvec, xi) linearly interpolates a value yi from an input xi along a curve defined by [xvec,yvec]
	//these set the irradiation effect on Voc and Imax, in other words reduces them if insolation is not at maximum. 	
	pv.IrrIdcScalar, _ = interpl(pv.IrrPctBreakpts, pv.IrrCurrEffect, pv.IrradiationPct)
	pv.IrrVdcScalar, _ = interpl(pv.IrrPctBreakpts, pv.IrrVoltEffect, pv.IrradiationPct)
	pv.Voc = pv.MaxVlt * pv.IrrVdcScalar
	pv.Isc = pv.MaxCurr * pv.IrrIdcScalar
	if pv.PCtrlMode {
		pv.PdcProfileInv = reverse(pv.PdcProfile)
		pv.VdcProfileInv = reverse(pv.VdcProfile)
	}
	return output
}

//PV GetLoadLines() returns zero terminal. PCS arbitrates voltage setpoint between all independent (active) sources at this step.
//PV is dependent source, so return zero (will respond to active source setpoints in a later step)
//Direction: up (2)
func (pv *pv) GetLoadLines(input terminal, dt float64) (output terminal) {
	//output.p, output.vdc = pv.Pdc, pv.Vdc
	//return zero terminal at this step since pv 'reacts' to voltage setpoint from power electronics.
	return output
}

//PV DistributeVoltage() input p from PCS at this point is difference between PCS power and sum of
//PCS children power. PV determines its share of this delta power it must 'make up' in the
//current iteration via droop share.
//return zero terminal as PV is leaf node
//Direction: down (3)
func (pv *pv) DistributeVoltage(input terminal) (output terminal) {
	if pv.PCtrlMode { //in power control mode we will set the VDC ourselves by looking up a setpoint backwards against 
		pv.Pdel = input.p
		return output
	}
	pv.Vdc = input.vdc //input.vdc at this point is arbitrated PV setpoint voltage from parent (either DCDC or PCS) 
	if pv.Vdc > pv.Voc {
		pv.Vdc = pv.Voc
	}
	return output
}

//PV CalculateState() calculates voltage setpoint from reverse lookup of inverted pdc profile. Then sets current setpoint by I = P/V
// in voltage control mode, look up current from arbitrated DC voltage and determine power by P=VI
//Then look up new output voltage from current demand
//Direction: up (4)
func (pv *pv) CalculateState(input terminal, dt float64) (output terminal) {
	if pv.PCtrlMode {
		if pv.Pdel > pv.Pmax {
			pv.Pdc = pv.Pmax
		} else {
			pv.Pdc = pv.Pdel
		}
		VdcPct,_ := interpl(pv.PdcProfileInv, pv.VdcProfileInv, pv.Pdc/pv.Pmax) //look up voltage from power curve backwards
		pv.Vdc = VdcPct * pv.Voc 
		pv.Idc = (pv.Pdel * 1000)/ pv.Vdc
		pv.DVolts.YNom = pv.Pmax
	} else {
		//Determine x coordinate (percentage of max voltage) for interpolation. Default to 76% if undefined (decent guess at MPP)
		VdcPct := 0.0
		if pv.Voc == 0 {
			VdcPct = 0.76
			pv.Fault = true
		} else {
			VdcPct = pv.Vdc / pv.Voc
		}
		//look up current setpoint from I-V curve.
		IdcPct, flt := interpl(pv.VdcProfile, pv.IdcProfile, VdcPct)
		pv.Idc = IdcPct * pv.Isc
		pv.Pdc = (pv.Vdc * pv.Idc) / 1000 //kW
		if flt {
			pv.Fault = true
		}
		pv.DVolts.YNom = pv.Pdc
	}
	pv.DVolts.XNom = pv.Vdc
	pv.DVolts.slope, pv.DVolts.offset = getSlope(pv.DVolts.Percent, pv.DVolts.YNom, pv.DVolts.XNom)
	output.dVdc = pv.DVolts
	output.p = pv.Pdc
	output.vdc = pv.Vdc
	return output
}

//Direction: down(5)
func (pv *pv) DistributeLoad(input terminal) (output terminal) {
	//check if PV is trying to deliver more power than PCS/DCDC can handle, then scale it back and find the 'true' operating point
	if input.p != 0 {
		// pDelta := (input.p * (pv.DVolts.slope / input.dVdc.slope)) * 1000 //power split by resistive droop share of all siblings (represented by droop slope), scaled to watts (input in kW)
		// IDelta := pDelta / pv.Vdc
		// pv.Idc = pv.Idc - IDelta
		// vdcPct, flt := interpl(pv.IdcProfile, pv.VdcProfile, pv.Idc/pv.Isc)
		// pv.Vdc = vdcPct * pv.Voc
		// pv.Pdc = (pv.Vdc * pv.Idc) / 1000
		// if flt {
		// 	pv.Fault = true
		// }
		//TODO GB: is this needed? Do these setpoints ever change between here and the last time they were calculated?
		if !pv.PCtrlMode {
			pv.Vdc = input.vdc
			vdcPct := 0.0
			if pv.Voc == 0 {
				vdcPct = 0.76
			} else {
				vdcPct = pv.Vdc/pv.Voc
			}
			idcPct, flt := interpl(pv.VdcProfile, pv.IdcProfile, vdcPct)
			pv.Idc = idcPct * pv.Isc
			pv.Pdc = (pv.Vdc * pv.Idc) / 1000
			if flt {
				pv.Fault = true
			}
		}
	}
	return input
}

//Direction: up (6)
func (pv *pv) UpdateState(input terminal, dt float64) (output terminal) {
	if pv.PCtrlMode {
		if pv.Pdc > pv.Pmax {
			pv.Pdc = pv.Pmax
		}
		VdcPct,_ := interpl(pv.PdcProfileInv, pv.VdcProfileInv, pv.Pdc/pv.Pmax) //look up voltage from power curve backwards
		pv.Vdc = VdcPct * pv.Voc
		pv.Idc = (pv.Pdc * 1000) / pv.Vdc
		pv.DVolts.YNom = pv.Pmax
	} else {
		pv.DVolts.YNom = pv.Pdc
	}
	pv.DVolts.XNom = pv.Vdc
	pv.DVolts.slope, pv.DVolts.offset = getSlope(pv.DVolts.Percent, pv.DVolts.YNom, pv.DVolts.XNom)
	output.dVdc = pv.DVolts
	output.p = pv.Pdc
	output.vdc = pv.Vdc

	// Update heartbeat and time
	updateHeartTime(&pv.Heart)
	pv.Heartbeat = pv.Heart.Heartbeat
	pv.Time = pv.Heart.Time
	pv.Year = pv.Heart.Year
	pv.Month = pv.Heart.Month
	pv.Day = pv.Heart.Day
	pv.Hour = pv.Heart.Hour
	pv.Minute = pv.Heart.Minute
	pv.Second = pv.Heart.Second
	return output
}

func (pv *pv) GetID() string {
	return pv.ID
}

func (pv *pv) GetAliases() []string {
	return pv.Aliases
}

func (pv *pv) Term() terminal {
	return terminal{0, pv.Vdc, pv.Pdc, 0, 0, 0, 0, droop{}, droop{}, pv.DVolts}
}
