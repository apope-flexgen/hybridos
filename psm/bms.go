package main

import (
	"log"
	"math"
	"math/rand"
	"time"
	// "fmt"
	//"reflect"
)

type bms struct {
	// BMS Model Identification
	ID                     string
	Aliases                []string
	// BMS physical parameters - limits, losses, etc
	Cap                    float64   // Nominal Capacity in kWh. Note this is a measure of battery ENERGY not battery CAPACITY - vendor spec sheets often list this as a 'capacity' value, so it is named such here in psm as well.
	CapMeas                float64   // Achieved Capacity in kWh, based on capacity of individual SBMUs
	Vnom                   float64   // Battery nominal (or normal) voltage
	Idleloss               float64   // Fixed losses across load
	Rte                    float64   // Round trip efficiency at full load
	pesr                   float64   // power normalized ESR, see Init()
	Inom                   float64   // Battery nominal current
	Pmax                   float64   // Battery max power, charge and discharge
	VdcProfile             []float64 // Used to calculate charging S-curve for bus and cell voltages. Entry at each index indicates DC bus voltage at the SOC at the same index in SOCprofile
	SocProfile             []float64 // Used to calculate charging S-curve for bus and cell voltages. Entry at each index indicates SOC at the DC bus voltage at the same index in Vdcprofile
	// Racks and cells
	Sbmu                   []rack
	CapRange               float64 // Variance in sbmu capacity, where 0.01 = 1%. SBMU may be above or below nominal by this percentage.
	LossRange              float64 // Variance in sbmu IdleLoss and RTE, where 0.01 = 1%. SBMU may be above or below nominal by this percentage.
	CellVoltRange          float64 // Variance in sbmu cell voltages, where 0.01 = 1%. SBMU may be above or below nominal by this percentage.
	NumSbmus               int     // Number of sbmus in the bms system
	// BMS Measured Values
	Vdc         measurement `name:"DC Voltage"` // Measured dc bus voltage
	Idc         measurement `name:"DC Current"` // Measured dc current
	P           float64     // Measured dc power
	Pdc         float64     // TODO
	Pcharge     float64     // Battery max charge limit (varies with SOC)
	Pdischarge  float64     // Battery max discharge limit (varies with SOC)
	Icharge     float64     // Battery max charge current (varies with SOC)
	Idischarge  float64     // Battery max discharge current (varies with SOC)
	Echarge     float64     // pulled from capacity
	Edischarge  float64     // pulled from capacity
	Soc         measurement `name:"SOC"` // Battery state of charge
	Soh         float64     // Battery state of health
	MaxCellVolt measurement `name:"Maximum Cell Voltage"` // maximum cell voltage in the BMS
	AvgCellVolt float64     // average cell voltage in the BMS
	MinCellVolt measurement `name:"Minimum Cell Voltage"`     // minimum cell voltage in the BMS
	MaxCellTemp measurement `name:"Maximum Cell Temperature"` // maximum cell temperature in the BMS
	AvgCellTemp float64     // average cell temperature in the BMS
	MinCellTemp measurement `name:"Minimum Cell Temperature"` // minimum cell temperature in the BMS

	SocChargeVec    []float64
	PChargeVec      []float64
	SocDischargeVec []float64
	PDischargeVec   []float64
	DisableLUTs     bool //flag to disable look-up-table based power limiting from BMS.

	// BMS Communications - includes configurable I/O
	Heart                 hearttime
	Status                []bitfield
	StatusCfg             []bitfieldcfg
	CtrlWord1             int
	CtrlWord1Cfg          []ctrlwordcfg
	CtrlWord2             int
	CtrlWord2Cfg          []ctrlwordcfg
	CtrlWord3             int
	CtrlWord3Cfg          []ctrlwordcfg
	CtrlWord4             int
	CtrlWord4Cfg          []ctrlwordcfg
	CtrlWord1Reset        int
	CtrlWord2Reset        int
	CtrlWord3Reset        int
	CtrlWord4Reset        int
	// On, off, and contactor controls
	On                    bool
	Oncmd                 bool
	Offcmd                bool
	ContactorControl      bool // Allows independent control of contactors
	DcContactor           bool
	DcContactorOpenCmd    bool
	DcContactorCloseCmd   bool
	NumRacks              int
	NumRacksEnabled       int
	ContactorClsDeltaV    float64
	ContactorClsMaxI	  float64
	// Faults and fault handling
	Fault                 bool    // Indicates that a fault of any kind is present, turns off the BMS, and opens the dc contactor
	Faults				  string
	Alarms	    		  string
	Warning               bool    // Indicates that a warning of any kind is present. No immediate action is taken.
	Watchdog              float64 // Checks to make sure that the connection to the higher level controller is active. If it does not change regularly, it turns off the BMS. Not implemented.
	Reset                 bool    // Resets warnings and faults
	DisableFault          bool    // Disable fault behavior
	EnableVoltageDynamics bool    // Enables characteristic voltage response to battery current (resistive drop and first order capacitive decay)
	R0					  float64  // Battery internal resistance for voltage response
	Tau					  float64  // Battery voltage response time constant. 
	R1					  float64  // Polarization resistance (usually much smaller than R0)'
	V1				      float64  // First time constant voltage (represents the voltage across R1||C1 in battery equivalent circuit model)
	// Droop Parameters
	Dvoltage              droop
	faultskip             bool 		// non public parameter. Skip the first fault detection instance in updateMode() to prevent nuisance faults. 
}

func (b *bms) Init() {
	// Initialize losses
	if b.Rte == 0 {
		b.pesr = 0
	} else {
		// This gives a "power normalized ESR" so we don't have to convert to current for loading losses
		// This means we can use P^2 * pesr for loss calculations later
		b.pesr = (b.Pmax*(1/math.Sqrt(b.Rte/100.0)-1) - b.Idleloss) / math.Pow(b.Pmax, 2)
	}

	// Find highest and lowest possible voltage and maximum current from profile
	vmin := math.Inf(+1)
	vmax := math.Inf(-1)
	imax := 0.0
	for i, e := range b.VdcProfile {
		if i == 0 || e < vmin {
			vmin = e
		}
		if i == 0 || e > vmax {
			vmax = e
		}
	}
	if vmin > 0 {
		imax = b.Pmax * 1000 / vmin
	}

	// Initialize status
	b.Status = processBitfieldConfig(b, b.StatusCfg)

	// Initialize warning and fault limits for BMS measurements
	// Vdc
	if b.Vdc.FaultHighThreshold <= vmin || b.Vdc.FaultHighThreshold > (1.25*vmax) {
		b.Vdc.FaultHighThreshold = 1.25 * vmax
	}
	if b.Vdc.AlarmHighThreshold <= vmin || b.Vdc.AlarmHighThreshold > b.Vdc.FaultHighThreshold {
		b.Vdc.AlarmHighThreshold = 0.95 * b.Vdc.FaultHighThreshold
	}
	if b.Vdc.AlarmLowThreshold < 0.8*vmin || b.Vdc.AlarmLowThreshold > b.Vdc.AlarmHighThreshold {
		b.Vdc.AlarmLowThreshold = 0.8 * vmin
	}
	if b.Vdc.FaultLowThreshold <= 0.75*vmin || b.Vdc.FaultLowThreshold > b.Vdc.AlarmLowThreshold {
		b.Vdc.FaultLowThreshold = 0.95 * b.Vdc.AlarmLowThreshold
	}
	// Idc
	if b.Idc.FaultHighThreshold <= imax || b.Idc.FaultHighThreshold > (3*imax) {
		b.Idc.FaultHighThreshold = 3 * imax
	}
	if b.Idc.AlarmHighThreshold <= imax || b.Idc.AlarmHighThreshold > b.Idc.FaultHighThreshold {
		b.Idc.AlarmHighThreshold = 0.95 * b.Idc.FaultHighThreshold
	}
	b.Idc.AlarmLowThreshold = -b.Idc.AlarmHighThreshold
	b.Idc.FaultLowThreshold = -b.Idc.FaultHighThreshold
	// SOC
	if b.Soc.FaultHighThreshold <= 50 || b.Soc.FaultHighThreshold > 100 {
		b.Soc.FaultHighThreshold = 100
	}
	if b.Soc.AlarmHighThreshold <= 50 || b.Soc.AlarmHighThreshold > b.Soc.FaultHighThreshold {
		b.Soc.AlarmHighThreshold = 0.95 * b.Soc.FaultHighThreshold
	}
	if b.Soc.AlarmLowThreshold < 3 || b.Soc.AlarmLowThreshold > b.Soc.AlarmHighThreshold {
		b.Soc.AlarmLowThreshold = 3
	}
	if b.Soc.FaultLowThreshold <= 0 || b.Soc.FaultLowThreshold > b.Soc.AlarmLowThreshold {
		b.Soc.FaultLowThreshold = 0.95 * b.Soc.AlarmLowThreshold
	}

	// Find the dc bus voltage based on the SOC of the battery
	pro_len := len(b.SocProfile)
	if pro_len != len(b.VdcProfile) {
		log.Println("[", b.ID, "] Inequal lengths for SocProfile and VdcProfile vectors. Unrecoverable fault. Please check configurations.")
		log.Fatalln("Exiting...")
	}
	if pro_len == 1 || b.Soc.Value < b.SocProfile[0] {
		b.Vdc.Value = b.VdcProfile[0]
	} else if b.Soc.Value >= b.SocProfile[pro_len-1] {
		b.Vdc.Value = b.VdcProfile[pro_len-1]
	} else {
		for i := 0; i < (pro_len - 1); i++ {
			if b.Soc.Value > b.SocProfile[i] && b.Soc.Value < b.SocProfile[i+1] {
				deltaSOC := (b.Soc.Value - b.SocProfile[i]) / (b.SocProfile[i+1] - b.SocProfile[i])
				b.Vdc.Value = ((b.VdcProfile[i+1] - b.VdcProfile[i]) * deltaSOC) + b.VdcProfile[i]
			}
		}
	}

	if len(b.Sbmu) == 1 && b.NumRacks != 0 {
		for i:=0; i<b.NumRacks - 1; i++ {
			b.Sbmu = append(b.Sbmu, rack{})
		}
	}
	// Initialize the sbmus and extract overall data such as capacity and system max cell voltage
	// If not specified, randomize capacity, losses, and cell voltages
	b.NumSbmus = len(b.Sbmu)
	b.DcContactor = false
	b.NumRacks = 0
	b.NumRacksEnabled = b.NumSbmus // Racks are enabled by default
	var capacitySum float64
	var avgVoltSum float64
	var avgTempSum float64
	var cellVoltNom float64
	var sbmuImax float64
	if b.NumSbmus > 0 {
		sbmuImax = imax / float64(b.NumSbmus)
	} else {
		sbmuImax = imax
	}
	for i, _ := range b.Sbmu {
		// initialize the random number generator
		rand.Seed(time.Now().UnixNano())
		// set the default parameters of the SBMU
		b.Sbmu[i].Vdc = b.Vdc.Value
		if b.Sbmu[i].Cap <= 0.0 {
			b.Sbmu[i].Cap = (1 + b.CapRange*2*(0.5-rand.Float64())) * (b.Cap / float64(b.NumSbmus))
		}
		if b.Sbmu[i].Pmax <= 0.0 {
			b.Sbmu[i].Pmax = b.Pmax / float64(b.NumSbmus)
		}
		if b.Sbmu[i].Idleloss <= 0.0 {
			b.Sbmu[i].Idleloss = (1 + b.LossRange*2*(0.5-rand.Float64())) * (b.Idleloss / float64(b.NumSbmus))
		}
		if b.Sbmu[i].Rte <= 0.0 {
			b.Sbmu[i].Rte = (1 + b.LossRange*2*(0.5-rand.Float64())) * (b.Rte)
		}
		if b.Sbmu[i].Rte == 0 {
			b.Sbmu[i].pesr = 0
		} else {
			b.Sbmu[i].pesr = (b.Sbmu[i].Pmax*(1/math.Sqrt(b.Sbmu[i].Rte/100.0)-1) - b.Sbmu[i].Idleloss) / math.Pow(b.Sbmu[i].Pmax, 2)
		}
		// All SBMUs are presumed to start with the same SOC and perfect state of health
		b.Sbmu[i].Soc.Value = b.Soc.Value
		b.Sbmu[i].Soh = 100.0
		// If not specified, NumCells is calculated using a presumed nominal cell voltage of 3.5 V, and rounds down
		if b.Sbmu[i].NumCells <= 0 && b.Vnom > 0 {
			b.Sbmu[i].NumCells = int(b.Vnom / 3.5)
		} else if b.Sbmu[i].NumCells > 0 && b.Vnom <= 0 {
			log.Fatal(b.ID, ": Nominal voltage (Vnom) is zero or less.")
		}
		// Find the nominal cell voltage
		cellVoltNom = b.Vnom / float64(b.Sbmu[i].NumCells)
		// Find cell voltages and temperatures
		b.Sbmu[i].cellVoltOver = b.CellVoltRange * rand.Float64()
		b.Sbmu[i].cellVoltUnder = b.CellVoltRange * rand.Float64()
		b.Sbmu[i].AvgCellVolt = b.Sbmu[i].Vdc / float64(b.Sbmu[i].NumCells)
		b.Sbmu[i].MaxCellVolt.Value = (1 + b.Sbmu[i].cellVoltOver) * b.Sbmu[i].AvgCellVolt
		b.Sbmu[i].MinCellVolt.Value = (1 - b.Sbmu[i].cellVoltUnder) * b.Sbmu[i].AvgCellVolt
		// Temperature model is not implemented, so cell temperatures are set to an unobjectionable value in degrees C
		b.Sbmu[i].MaxCellTemp.Value = b.MaxCellTemp.Value
		b.Sbmu[i].AvgCellTemp = b.AvgCellTemp
		b.Sbmu[i].MinCellTemp.Value = b.MinCellTemp.Value
		// Set the warning and fault thresholds
		// Idc - overloading
		if b.Sbmu[i].Idc.FaultHighThreshold <= sbmuImax || b.Sbmu[i].Idc.FaultHighThreshold > (3*sbmuImax) {
			b.Sbmu[i].Idc.FaultHighThreshold = 3 * sbmuImax
		}
		if b.Sbmu[i].Idc.AlarmHighThreshold <= sbmuImax || b.Sbmu[i].Idc.AlarmHighThreshold > b.Sbmu[i].Idc.FaultHighThreshold {
			b.Sbmu[i].Idc.AlarmHighThreshold = 0.95 * b.Sbmu[i].Idc.FaultHighThreshold
		}
		b.Sbmu[i].Idc.AlarmLowThreshold = -b.Sbmu[i].Idc.AlarmHighThreshold
		b.Sbmu[i].Idc.FaultLowThreshold = -b.Idc.FaultHighThreshold
		// SOC
		if b.Sbmu[i].Soc.FaultHighThreshold <= 50 || b.Sbmu[i].Soc.FaultHighThreshold > 100 {
			b.Sbmu[i].Soc.FaultHighThreshold = 100
		}
		if b.Sbmu[i].Soc.AlarmHighThreshold <= 50 || b.Sbmu[i].Soc.AlarmHighThreshold > b.Sbmu[i].Soc.FaultHighThreshold {
			b.Sbmu[i].Soc.AlarmHighThreshold = 0.95 * b.Sbmu[i].Soc.FaultHighThreshold
		}
		if b.Sbmu[i].Soc.AlarmLowThreshold < 3 || b.Sbmu[i].Soc.AlarmLowThreshold > b.Sbmu[i].Soc.AlarmHighThreshold {
			b.Sbmu[i].Soc.AlarmLowThreshold = 3
		}
		if b.Sbmu[i].Soc.FaultLowThreshold <= 0 || b.Sbmu[i].Soc.FaultLowThreshold > b.Sbmu[i].Soc.AlarmLowThreshold {
			b.Sbmu[i].Soc.FaultLowThreshold = 0.95 * b.Sbmu[i].Soc.AlarmLowThreshold
		}
		// Max Cell Voltage
		if b.Sbmu[i].MaxCellVolt.FaultHighThreshold <= cellVoltNom || b.Sbmu[i].MaxCellVolt.FaultHighThreshold > (1.15*cellVoltNom) {
			b.Sbmu[i].MaxCellVolt.FaultHighThreshold = 1.15 * cellVoltNom
		}
		if b.Sbmu[i].MaxCellVolt.AlarmHighThreshold <= cellVoltNom || b.Sbmu[i].MaxCellVolt.AlarmHighThreshold > b.Sbmu[i].MaxCellVolt.FaultHighThreshold {
			b.Sbmu[i].MaxCellVolt.AlarmHighThreshold = 0.95 * b.Sbmu[i].MaxCellVolt.FaultHighThreshold
		}
		b.Sbmu[i].MaxCellVolt.AlarmLowThreshold = 0.1
		b.Sbmu[i].MaxCellVolt.FaultLowThreshold = 0
		// Min Cell Voltage
		b.Sbmu[i].MinCellVolt.FaultHighThreshold = 3 * cellVoltNom
		b.Sbmu[i].MinCellVolt.AlarmHighThreshold = 2.9 * cellVoltNom
		if b.Sbmu[i].MinCellVolt.AlarmLowThreshold <= 0 || b.Sbmu[i].MinCellVolt.AlarmLowThreshold > (0.9*cellVoltNom) {
			b.Sbmu[i].MinCellVolt.AlarmLowThreshold = 0.9 * cellVoltNom
		}
		if b.Sbmu[i].MinCellVolt.FaultLowThreshold <= 0 || b.Sbmu[i].MinCellVolt.FaultLowThreshold > b.Sbmu[i].MinCellVolt.AlarmLowThreshold {
			b.Sbmu[i].MinCellVolt.FaultLowThreshold = 0.95 * b.Sbmu[i].MinCellVolt.AlarmLowThreshold
		}
		// Max Cell Temperature
		if b.Sbmu[i].MaxCellTemp.FaultHighThreshold <= 0 || b.Sbmu[i].MaxCellTemp.FaultHighThreshold > 90 {
			b.Sbmu[i].MaxCellTemp.FaultHighThreshold = 90
		}
		if b.Sbmu[i].MaxCellTemp.AlarmHighThreshold <= 0 || b.Sbmu[i].MaxCellTemp.AlarmHighThreshold > b.Sbmu[i].MaxCellTemp.FaultHighThreshold {
			b.Sbmu[i].MaxCellTemp.AlarmHighThreshold = 0.95 * b.Sbmu[i].MaxCellTemp.FaultHighThreshold
		}
		b.Sbmu[i].MaxCellTemp.AlarmLowThreshold = -100
		b.Sbmu[i].MaxCellTemp.FaultLowThreshold = -110
		// Min Cell Temperature
		b.Sbmu[i].MinCellTemp.FaultHighThreshold = 110
		b.Sbmu[i].MinCellTemp.AlarmHighThreshold = 100
		if b.Sbmu[i].MinCellTemp.AlarmLowThreshold > -10 || b.Sbmu[i].MinCellTemp.AlarmLowThreshold < -50 {
			b.Sbmu[i].MinCellTemp.AlarmLowThreshold = -10
		}
		if b.Sbmu[i].MinCellTemp.FaultLowThreshold <= -50 || b.Sbmu[i].MinCellTemp.FaultLowThreshold > b.Sbmu[i].MinCellTemp.AlarmLowThreshold {
			b.Sbmu[i].MinCellTemp.FaultLowThreshold = 1.05 * b.Sbmu[i].MinCellTemp.AlarmLowThreshold
		}
		// Find overall bms capacity, cell voltages, and cell temperatures
		capacitySum = capacitySum + b.Sbmu[i].Cap
		avgVoltSum = avgVoltSum + b.Sbmu[i].AvgCellVolt
		avgTempSum = avgTempSum + b.Sbmu[i].AvgCellTemp
		b.MaxCellVolt.Value = math.Max(b.MaxCellVolt.Value, b.Sbmu[i].MaxCellVolt.Value)
		b.MinCellVolt.Value = math.Min(b.MinCellVolt.Value, b.Sbmu[i].MinCellVolt.Value)
		b.MaxCellTemp.Value = math.Max(b.MaxCellTemp.Value, b.Sbmu[i].MaxCellTemp.Value)
		b.MinCellTemp.Value = math.Min(b.MinCellTemp.Value, b.Sbmu[i].MinCellTemp.Value)
		// Ensure that the dc contactor on each SBMU is closed at the start of the simulation and the open/close commands are off
		b.Sbmu[i].DcContactor = false
		b.Sbmu[i].DcContactorOpenCmd = false
		b.Sbmu[i].DcContactorCloseCmd = false

		// Ensure battery rack is enabled at the start of the simulation and the enable/disable commands are off
		b.Sbmu[i].Enabled = true
		b.Sbmu[i].EnableCmd = false
		b.Sbmu[i].DisableCmd = false
	}
	b.CapMeas = capacitySum
	b.AvgCellVolt = avgVoltSum / float64(b.NumSbmus)
	b.AvgCellTemp = avgTempSum / float64(b.NumSbmus)
	b.Soh = 100.0

	// Initialize fault thresholds for aggregate cell measurements// Max Cell Voltage
	if b.MaxCellVolt.FaultHighThreshold <= cellVoltNom || b.MaxCellVolt.FaultHighThreshold > (1.15*cellVoltNom) {
		b.MaxCellVolt.FaultHighThreshold = 1.15 * cellVoltNom
	}
	if b.MaxCellVolt.AlarmHighThreshold <= cellVoltNom || b.MaxCellVolt.AlarmHighThreshold > b.MaxCellVolt.FaultHighThreshold {
		b.MaxCellVolt.AlarmHighThreshold = 0.95 * b.MaxCellVolt.FaultHighThreshold
	}
	b.MaxCellVolt.AlarmLowThreshold = 0.1
	b.MaxCellVolt.FaultLowThreshold = 0
	// Min Cell Voltage
	b.MinCellVolt.FaultHighThreshold = 3 * cellVoltNom
	b.MinCellVolt.AlarmHighThreshold = 2.9 * cellVoltNom
	if b.MinCellVolt.AlarmLowThreshold <= 0 || b.MinCellVolt.AlarmLowThreshold > (0.9*cellVoltNom) {
		b.MinCellVolt.AlarmLowThreshold = 0.9 * cellVoltNom
	}
	if b.MinCellVolt.FaultLowThreshold <= 0 || b.MinCellVolt.FaultLowThreshold > b.MinCellVolt.AlarmLowThreshold {
		b.MinCellVolt.FaultLowThreshold = 0.95 * b.MinCellVolt.AlarmLowThreshold
	}
	// Max Cell Temperature
	if b.MaxCellTemp.FaultHighThreshold <= 0 || b.MaxCellTemp.FaultHighThreshold > 90 {
		b.MaxCellTemp.FaultHighThreshold = 90
	}
	if b.MaxCellTemp.AlarmHighThreshold <= 0 || b.MaxCellTemp.AlarmHighThreshold > b.MaxCellTemp.FaultHighThreshold {
		b.MaxCellTemp.AlarmHighThreshold = 0.95 * b.MaxCellTemp.FaultHighThreshold
	}
	b.MaxCellTemp.AlarmLowThreshold = -100
	b.MaxCellTemp.FaultLowThreshold = -110
	// Min Cell Temperature
	b.MinCellTemp.FaultHighThreshold = 110
	b.MinCellTemp.AlarmHighThreshold = 100
	if b.MinCellTemp.AlarmLowThreshold > -10 || b.MinCellTemp.AlarmLowThreshold < -50 {
		b.MinCellTemp.AlarmLowThreshold = -10
	}
	if b.MinCellTemp.FaultLowThreshold <= -50 || b.MinCellTemp.FaultLowThreshold > b.MinCellTemp.AlarmLowThreshold {
		b.MinCellTemp.FaultLowThreshold = 1.05 * b.MinCellTemp.AlarmLowThreshold
	}

	// Find remaining charge and discharge energy based on the SOC and capacity of the battery
	b.Echarge = b.Cap * (1 - b.Soc.Value/100)
	b.Edischarge = b.Cap * b.Soc.Value / 100

	//Droop parameters. BMS is modeled as a stiff source
	if b.Pmax <= 0 {
		b.Pmax = 10000 //TODO GB: move this to default configs.
	}
	b.Dvoltage.Percent = 0.001
	b.Dvoltage.XNom = b.Vnom
	b.Dvoltage.YNom = b.Pmax

	if b.Dvoltage.Percent != 0 && b.Dvoltage.XNom != 0 {
		b.Dvoltage.slope, b.Dvoltage.offset = getSlope(b.Dvoltage.Percent, b.Dvoltage.YNom, b.Dvoltage.XNom)
	}
	// fmt.Println(b.ID, b.Dactive, b.Dreactive)

	//check configured soc power limit vectors, and disable the lookup tables if any issue found.
	if len(b.SocChargeVec) != len(b.PChargeVec) || len(b.SocDischargeVec) != len(b.PDischargeVec) {
		log.Println("[", b.ID, "] Incorrect vector lengths for chargeable and dischargeable power limits. Reverting to default behavior")
		b.DisableLUTs = true
	}
	if len(b.SocChargeVec) == 0 || len(b.SocDischargeVec) == 0 {
		log.Println("[", b.ID, "] zero length vector for chargeable or dischargeable power limits. Reverting to default behavior")
		b.DisableLUTs = true
	}
	if b.ContactorClsDeltaV <= 0 {
		b.ContactorClsDeltaV = 100000000		// Calibrate off
	}
	if b.ContactorClsMaxI <= 0 {
		b.ContactorClsMaxI = 100000000
	}
	b.faultskip = true // skip fault detection for the first iteration. 

	flt1, flt2, flt3, flt4 := false, false, false, false
	b.CtrlWord1Reset, flt1 = initCtrlWord(b.CtrlWord1Cfg, 0)
	b.CtrlWord2Reset, flt2 = initCtrlWord(b.CtrlWord2Cfg, 0)
	b.CtrlWord3Reset, flt3 = initCtrlWord(b.CtrlWord3Cfg, 0)
	b.CtrlWord4Reset, flt4 = initCtrlWord(b.CtrlWord4Cfg, 0)

	if flt1 || flt2 || flt3 || flt4 {
		log.Println("[", b.ID, "] Unable to set good reset values for control words. This may cause unintended behavior")
	}
}

// BMS UpdateMode() processes commands either through control words or direct
// commands.
func (b *bms) UpdateMode(input terminal) (output terminal) {
	// Process Control Words
	processCtrlWordConfig(b, b.CtrlWord1Cfg, b.CtrlWord1)
	processCtrlWordConfig(b, b.CtrlWord2Cfg, b.CtrlWord2)
	processCtrlWordConfig(b, b.CtrlWord3Cfg, b.CtrlWord3)
	processCtrlWordConfig(b, b.CtrlWord4Cfg, b.CtrlWord4)
	// Reset control words back to default value after their action is parsed.
	b.CtrlWord1 = b.CtrlWord1Reset
	b.CtrlWord2 = b.CtrlWord2Reset
	b.CtrlWord3 = b.CtrlWord3Reset
	b.CtrlWord4 = b.CtrlWord4Reset
	// fmt.Println(b.DcContactor, b.DcContactorOpenCmd, b.DcContactorCloseCmd, b.On, b.Oncmd, b.Offcmd)

	// Update measurement fault and alarm states with the updateMeasurements function call
	if !b.DisableFault && !b.faultskip {
		if b.Reset {
			b.Fault, b.Warning = false, false
			b.Faults, b.Alarms = "", ""
		}
		lowerFault, lowerWarning, lowerFaultsMsg, lowerAlarmsMsg := updateMeasurements(b, b.Reset)
		b.Fault = b.Fault || lowerFault
		b.Warning = b.Warning || lowerWarning
		b.Faults = appendDelimMsg(b.Faults, lowerFaultsMsg)
		b.Alarms = appendDelimMsg(b.Alarms, lowerAlarmsMsg)
		if b.Reset {
			b.Reset = false
		}
	} else {
		b.Fault, b.Warning, b.Reset = false, false, false
		b.Faults, b.Alarms = "", ""
	}
	b.faultskip = false // this variable is only used to skip the fault detection upon first pass through.
	// Control DC contactor
	// ContactorControl allows discrete control over DC and AC contactors
	// If not set, only Oncmd or Offcmd are needed
	// A Fault overrides ContactorControl, forcing the contactor to open
	if b.Fault && !b.DisableFault {
		b.Oncmd = false
		if b.DcContactor {
			b.DcContactorOpenCmd = true
		}
	} else /* if !b.ContactorControl */ {
		if !b.On && b.Oncmd {
			b.DcContactorCloseCmd = true
		} else if b.On && b.Offcmd {
			b.DcContactorOpenCmd = true
		}
	}
	// Update each battery rack state and dc contactors
	for i := range b.Sbmu {
		processCtrlWordConfig(&(b.Sbmu[i]), b.Sbmu[i].CtrlWord1Cfg, b.Sbmu[i].CtrlWord1)

		// enable/disable each battery rack
		if !b.Sbmu[i].Enabled && b.Sbmu[i].EnableCmd {
			b.Sbmu[i].Enabled, b.Sbmu[i].EnableCmd = true, false
			b.NumRacksEnabled++
		} else if b.Sbmu[i].Enabled && b.Sbmu[i].DisableCmd {
			b.Sbmu[i].Enabled, b.Sbmu[i].DisableCmd = false, false
			b.NumRacksEnabled--
		}

		// control the DC contactors on each SBMU
		if b.Sbmu[i].Enabled && math.Abs(b.Sbmu[i].Idc.Value) <= b.ContactorClsMaxI {
			if !b.Sbmu[i].DcContactor && b.DcContactorCloseCmd && (b.NumRacks == 0 || (b.Sbmu[i].Vdc - b.Vdc.Value) < b.ContactorClsDeltaV)  {
				b.Sbmu[i].DcContactor, b.Sbmu[i].DcContactorCloseCmd = true, false
				b.NumRacks++
			} else if b.Sbmu[i].DcContactor && b.DcContactorOpenCmd {
				b.Sbmu[i].DcContactor, b.Sbmu[i].DcContactorOpenCmd = false, false
				b.NumRacks--
			}
		}
	}

	// Update DC contactor status based on incoming command
	if !b.DcContactor && b.DcContactorCloseCmd {
		if b.NumRacks > 0 {
			b.DcContactor, b.DcContactorCloseCmd = true, false
		} else {
			b.DcContactor, b.DcContactorCloseCmd = false, false
		}
	} else if b.DcContactor && b.DcContactorOpenCmd {
		b.DcContactor, b.DcContactorOpenCmd = false, false
		b.Offcmd = true
	}

	// Turn on if conditions allow it
	if b.Oncmd && (!b.On) && b.DcContactor {
		b.On = true
		b.Oncmd = false
	} else if b.On && b.Offcmd {
		b.On = false
		b.Offcmd = false
	}
	// Contactor commands may be sent to close rack contactors while mains are open
	// In that case the command wouldn'tbe reset above so set it back to false here
	// This prevents the bms from holding on to old contactor close commands. 
	b.DcContactorCloseCmd = false
	b.DcContactorOpenCmd = false
	return output // returning zero terminal, since BMS has no assets below it
}

// BMS GetLoadLines() returns droop parameters up the tree if grid forming
func (b *bms) GetLoadLines(input terminal, dt float64) (output terminal) {
	if b.On {
		output.dVdc = b.Dvoltage
		// Get DC bus voltage from the current SOC
		pro_len := len(b.SocProfile)
		if pro_len == 1 || b.Soc.Value < b.SocProfile[0] {
			b.Vdc.Value = b.VdcProfile[0]
		} else if b.Soc.Value >= b.SocProfile[pro_len-1] {
			b.Vdc.Value = b.VdcProfile[pro_len-1]
		} else {
			for i := 0; i < (pro_len - 1); i++ {
				if b.Soc.Value > b.SocProfile[i] && b.Soc.Value < b.SocProfile[i+1] {
					deltaSOC := (b.Soc.Value - b.SocProfile[i]) / (b.SocProfile[i+1] - b.SocProfile[i])
					b.Vdc.Value = ((b.VdcProfile[i+1] - b.VdcProfile[i]) * deltaSOC) + b.VdcProfile[i]
				}
			}
		}
		output.dVdc.XNom = b.Vdc.Value
		output.p = b.P // b.P and b.Q contain the last iteration's output at this point in the solver,
		// so droop has a one tick delay

		// Update the rack DC bus voltages
		// for i, _ := range b.Sbmu {
		// 	b.Sbmu[i].Vdc = b.Vdc.Value
		// }
	}
	return output
}

// BMS DistributeVoltage() accepts the voltage set from upstream, either
// by a BMS directly or through droop. The BMS turns off if no voltage
// is present and updates its status output.
func (b *bms) DistributeVoltage(input terminal) (output terminal) {
	// TODO: review if this is actually needed; battery should hold voltage regardless of input terminal
	/*
		if input.vdc <= 0 { // Turn off if input voltage is 0 after BMS forming phase
			b.On, b.Oncmd, b.Offcmd = false, false, false
		}
	*/

	assetStatus := processBitfieldConfig(b, b.StatusCfg)
	for i, v := range assetStatus {
		b.Status[i] = v
	}
	// fmt.Println("BMS STATUS", b.Status, "Val", b.Status[0].Value, "ADDRESS", &(b.Status[0].Value))
	// fmt.Printf("Status Address: %p Val Address: %p String: %s Val: %d\n", &b.Status, &(b.Status[0].Value), b.Status[0].String, b.Status[0].Value)
	// Find the dc bus voltage based on the SOC of the battery
	// pro_len := len(b.SocProfile)
	// if pro_len == 1 || b.Soc.Value < b.SocProfile[0] {
	// 	b.Vdc.Value = b.VdcProfile[0]
	// } else if b.Soc.Value >= b.SocProfile[pro_len-1] {
	// 	b.Vdc.Value = b.VdcProfile[pro_len-1]
	// } else {
	// 	for i := 0; i < (pro_len - 1); i++ {
	// 		if b.Soc.Value > b.SocProfile[i] && b.Soc.Value < b.SocProfile[i+1] {
	// 			deltaSOC := (b.Soc.Value - b.SocProfile[i]) / (b.SocProfile[i+1] - b.SocProfile[i])
	// 			b.Vdc.Value = ((b.VdcProfile[i+1] - b.VdcProfile[i]) * deltaSOC) + b.VdcProfile[i]
	// 		}
	// 	}
	// }

	return output // returning zero terminal, since BMS has no assets below it
}

func (b *bms) CalculateState(input terminal, dt float64) (output terminal) {
	return output // BMS is grid-forming only
}

func (b *bms) DistributeLoad(input terminal) (output terminal) {
	return input // BMS is grid-forming only
}

// BMS UpdateState() calculates grid forming power output P and Q, considering
// the P and Q demand of the tree and its droop parameters. Then update SOC
// based on the actual output from grid following or grid forming
func (b *bms) UpdateState(input terminal, dt float64) (output terminal) {
	// Update heartbeat and time
	updateHeartTime(&b.Heart)

	// Declare local variables and get power drawn from upstream pcs
	var p_sbmu, socloss, soc, plimited, socsum float64
	var under, over bool
	p_sbmu = input.p / float64(b.NumSbmus) // input power is divided among the SBMUs evenly in this simulation
	// If the BMS is on and the DC contactor is closed, update the SOC of each SBMU, then find the SOC of the total BMS.
	// TODO: throw a fault here if you are above the limits of the BMS
	if b.On && b.DcContactor {
		// Reset BMS power variables
		b.P, b.Pdc, b.Pcharge, b.Pdischarge = 0, 0, 0, 0
		for i, _ := range b.Sbmu {
			if b.Sbmu[i].DcContactor {
				b.Sbmu[i].P = p_sbmu // power delivered by battery is controlled by upstream devices
				// Get losses and update soc
				socloss = b.Sbmu[i].Idleloss + b.Sbmu[i].pesr*math.Pow(p_sbmu, 2)
				b.Sbmu[i].Pdc = b.Sbmu[i].P + socloss // power drawn from battery is delivered power plus losses
				soc, plimited, under, over = getIntegral(b.Sbmu[i].Soc.Value/100, -(b.Sbmu[i].Pdc)/b.Sbmu[i].Cap, dt/3600, 0, 1)
				// Find the actual charging and discharging power based on SOC capacity
				// TODO: implement a lookup table (C-rates) for this calculation
				b.Sbmu[i].Soc.Value = soc * 100
				if over {
					b.Sbmu[i].Pdc = -plimited * b.Sbmu[i].Cap // limit power drawn by battery for this tick to bring SOC to 100%
					b.Sbmu[i].P = b.Sbmu[i].Pdc - socloss     // adjust power delivered from battery to account for losses
				} else if under {
					b.Sbmu[i].Pdc = -plimited * b.Sbmu[i].Cap // limit power drawn by battery for this tick to bring SOC to 100%
					b.Sbmu[i].P = b.Sbmu[i].Pdc - socloss     // adjust power delivered from battery to account for losses
				}
				// Update the BMS parameters
				socsum = socsum + soc
				b.P = b.P + b.Sbmu[i].P
				b.Pdc = b.Pdc + b.Sbmu[i].Pdc
			} else {
				b.Sbmu[i].P, b.Sbmu[i].Pdc, b.Sbmu[i].Pcharge, b.Sbmu[i].Pdischarge = 0, 0, 0, 0
			}
		}
		b.Soc.Value = 100 * socsum / float64(b.NumRacks) // bms soc value is the average of the sbmu soc values. This will consider only the soc of the closed racks. 
	} else {
		b.P, b.Pdc, b.Pcharge, b.Pdischarge = 0, 0, 0, 0
		for i, _ := range b.Sbmu {
			b.Sbmu[i].P, b.Sbmu[i].Pdc, b.Sbmu[i].Pcharge, b.Sbmu[i].Pdischarge = 0, 0, 0, 0
		}
	}
	if !b.DisableLUTs {
		var ChargePCT float64
		var DischargePCT float64
		var tempFaultCharge, tempFaultDischarge bool
		ChargePCT, tempFaultCharge = interpl(b.SocChargeVec, b.PChargeVec, b.Soc.Value)
		DischargePCT, tempFaultDischarge = interpl(b.SocDischargeVec, b.PDischargeVec, b.Soc.Value)
		if !b.DisableFault && !b.faultskip {
			b.Fault = b.Fault || tempFaultCharge || tempFaultDischarge
		}
		b.Pcharge = b.Pmax * ChargePCT * -1
		b.Pdischarge = b.Pmax * DischargePCT
	} else {
		b.Pcharge = -1 * b.Pmax
		b.Pdischarge = b.Pmax
		if b.Soc.Value >= b.Soc.FaultHighThreshold {
			b.Pcharge = 0
		}
		if b.Soc.Value <= b.Soc.FaultLowThreshold {
			b.Pdischarge = 0
		}
	}
	for i, _ := range b.Sbmu {
		if b.Sbmu[i].DcContactor {
			b.Sbmu[i].Pcharge = b.Pcharge / float64(b.NumSbmus)
			b.Sbmu[i].Pdischarge = b.Pdischarge / float64(b.NumSbmus)
		}
	}

	// Once SOC is updated, find the new DC bus and update BMS currents.
	pro_len := len(b.SocProfile)
	if pro_len == 1 || b.Soc.Value < b.SocProfile[0] {
		b.Vdc.Value = b.VdcProfile[0]
	} else if b.Soc.Value >= b.SocProfile[pro_len-1] {
		b.Vdc.Value = b.VdcProfile[pro_len-1]
	} else {
		for i := 0; i < (pro_len - 1); i++ {
			if b.Soc.Value > b.SocProfile[i] && b.Soc.Value < b.SocProfile[i+1] {
				deltaSOC := (b.Soc.Value - b.SocProfile[i]) / (b.SocProfile[i+1] - b.SocProfile[i])
				b.Vdc.Value = ((b.VdcProfile[i+1] - b.VdcProfile[i]) * deltaSOC) + b.VdcProfile[i]
			}
		}
	}

	// Update the rack DC bus voltages
	// If rack contactor is closed, then take voltage based on bms soc as closed racks necessarily have the same voltage (ignoring busbar resistance and etc)
	// Otherwise, racks should keep track of their own voltage based on their individual SOCs
	for i, _ := range b.Sbmu {
		if b.Sbmu[i].DcContactor {
			b.Sbmu[i].Vdc = b.Vdc.Value
		} else {
			b.Sbmu[i].Vdc,_ = interpl(b.SocProfile, b.VdcProfile, b.Sbmu[i].Soc.Value)
		}
	}
	// If enabled, battery voltage should have a response to current.
	// We only set b.Vdc.Value and not b.Sbmu[i].Vdc as a result of this logic so we avoid nuisance max/min cell voltage faults. 
	// if b.EnableVoltageDynamics && b.Tau != 0 && b.R1 != 0 && b.R0 != 0 {
	if b.EnableVoltageDynamics &&  b.R0 >= 0 {
		rackcurrent := 0.0
		if (b.NumRacks != 0) {
			rackcurrent = b.Idc.Value / float64(b.NumRacks)
		}
		// expterm := math.Exp(-dt/b.Tau)
		// b.V1 = b.V1 * expterm + rackcurrent * (b.R1 * (1 - expterm)) //this uses last step current and that's ok for now
		b.Vdc.Value = b.Vdc.Value - rackcurrent * b.R0 
	}

	b.Idc.Value = pToI(b.P, b.Vdc.Value)
	if b.Vdc.Value != 0 {
		b.Icharge = b.Pcharge * 1000 / b.Vdc.Value
		b.Idischarge = b.Pdischarge * 1000 / b.Vdc.Value
	} else {
		b.Icharge = 0
		b.Idischarge = 0
	}

	// Update the SBMU bus and cell voltages and the currents
	// Prepare to find overall max and min cell voltages by setting up initial comparison to arbitrarily extreme values
	b.MaxCellVolt.Value = -1000.0
	b.MinCellVolt.Value = 1000.0
	for i, _ := range b.Sbmu {
		// b.Sbmu[i].Vdc = b.Vdc.Value
		b.Sbmu[i].AvgCellVolt = b.Sbmu[i].Vdc / float64(b.Sbmu[i].NumCells)
		b.Sbmu[i].MaxCellVolt.Value = (1 + b.Sbmu[i].cellVoltOver) * b.Sbmu[i].AvgCellVolt
		b.Sbmu[i].MinCellVolt.Value = (1 - b.Sbmu[i].cellVoltUnder) * b.Sbmu[i].AvgCellVolt
		b.MaxCellVolt.Value = math.Max(b.MaxCellVolt.Value, b.Sbmu[i].MaxCellVolt.Value)
		b.MinCellVolt.Value = math.Min(b.MinCellVolt.Value, b.Sbmu[i].MinCellVolt.Value)
		if b.Sbmu[i].Vdc != 0 {
			b.Sbmu[i].Icharge = b.Sbmu[i].Pcharge * 1000 / b.Sbmu[i].Vdc
			b.Sbmu[i].Idischarge = b.Sbmu[i].Pdischarge * 1000 / b.Sbmu[i].Vdc
		} else {
			b.Sbmu[i].Icharge = 0
			b.Sbmu[i].Idischarge = 0
		}
		b.Sbmu[i].Idc.Value = pToI(b.Sbmu[i].P, b.Sbmu[i].Vdc)
	}
	// TODO: Implement a thermal model which updates the temperature of the cells based on power flow and external temperature

	// Find remaining charge and discharge energy based on the SOC and capacity of the battery
	b.Echarge = b.Cap * (1 - b.Soc.Value/100)
	b.Edischarge = b.Cap * b.Soc.Value / 100

	// Send calculated output power to output terminal
	output.p = b.P
	output.dVdc = b.Dvoltage
	// fmt.Printf("BMS %s: P: %.1f\tQ: %.1f\tS: %.1f\tV: %.1f\tF: %.1f\n", b.ID, b.P, b.Q, b.S, b.V, b.F)
	return output
}

func (b *bms) GetID() string {
	return b.ID
}

func (b *bms) GetAliases() []string {
	return b.Aliases
}

func (b *bms) Term() terminal {
	return terminal{0, b.Vdc.Value, b.P, 0, b.P, 0, 0, droop{}, droop{}, b.Dvoltage}
}
