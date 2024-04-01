package main

import (
	"fmt"
	"math"
	"math/rand"
	"reflect"
	"strings"
	"time"
)

//droop struct contains variables to define voltage and frequency droop characteristics of an asset instance
type droop struct {
	YNom    float64 //y output at 100% droop e.g. nominal power
	Percent float64 //droop percentage of x for 100% y output
	XNom    float64 //nominal x value for comparison to instantaneous x value to get percent droop e.g. nominal frequency
	slope   float64 //slope of droop response vector, m when (y = m*(x) + b) e.g. watts/hertz or vars/volts
	offset  float64 //offset of droop response vector when x = 0: (y = m*(x) + b) e.g. watts or vars
}

type terminal struct {
	v      float64
	vdc    float64
	p      float64
	q      float64
	s      float64
	f      float64
	ph     float64
	dHertz droop
	dVolts droop
	dVdc   droop
}

type asset interface {
	UpdateMode(input terminal) (output terminal)
	GetLoadLines(input terminal, dt float64) (output terminal)
	DistributeVoltage(input terminal) (output terminal)
	CalculateState(input terminal, dt float64) (output terminal)
	DistributeLoad(input terminal) (output terminal)
	UpdateState(input terminal, dt float64) (output terminal)
	GetID() string
	GetAliases() []string
	Term() terminal
	Init()
}

func rss(x, y float64) float64 {
	// Root Sum Square, implements sqrt(x^2 + y^2)
	return math.Sqrt(math.Pow(x, 2) + math.Pow(y, 2))
}

const sqrt3 = 1.73205080756887729352744634150587236694280525381038062805580697945193 // https://oeis.org/A002194

func pToI(p, v float64) float64 {
	// simple P = V * I conversion
	if v == 0 {
		return 0
	}
	return p * 1000 / v
}

func sToI(s, v float64) float64 {
	// s (3ph power in kW)/(v*sqrt(3) = i (1ph current)
	if v == 0 {
		return 0
	}
	return (s * 1000) / (v * sqrt3)
}

func pf(p, q, s float64) (pf float64) {
	// IEEE sign convention
	// PF sign is correlates with the PF lead/lag convention, in other words, the effective load type (inductive or capacitive)
	// PF sign also follows sign of P, for loadlike assets
	if s == 0 {
		pf = 1.0
	} else {
		pf = math.Abs(p) / math.Abs(s)
		if pf > 1.0 {
			pf = 1.0
		}
		if math.Signbit(q) {
			pf = pf * -1
		}
		if math.Signbit(p) {
			pf = pf * -1
		}
	}
	return pf
}

func pfToQ(p, pf float64) float64 {
	// q = sqrt( (p^2/pf^2) - p^2)
	// sign of q depends on sign of pf
	// first draft saturating pf to .1, or else 1/pf^2 term blows up
	sign := 1.0
	if math.Signbit(pf) { // true means negative
		sign = -1.0
	}
	if math.Signbit(p) {
		sign = sign * -1.0
	}
	p2 := math.Pow(p, 2)
	if (pf > -.1) && (pf < .1) {
		pf = .1 // don't need sign here since it comes back later
	} else if (pf < -1.0) || (pf > 1.0) {
		pf = 1
	}
	q := sign * math.Sqrt(p2/math.Pow(pf, 2)-p2)
	if math.IsNaN(q) {
		return 0
	}
	return q
}

func powerNoise(noise float64) float64 {
	// Normally distributed noise with std. deviation set to half of noise kW setting
	return rand.NormFloat64() * (noise / 2.0)
}

func floatEq(x, y, threshold float64) bool {
	return (math.Abs(x-y) < threshold)
}

func loadNoise(min, max float64) float64 {
	// Uniformly distributed noise between min and max
	r := rand.Float64() // uniform [0.0,1.0)
	r *= max - min
	r += min
	return r
}

type bitfield struct {
	Value  int    `json:"value"`
	String string `json:"string"`
}

type bitfieldcfg struct {
	bitfield
	Field  string `json:"field"`
	Invert bool   `json:"invert"`
}

func processBitfieldConfig(s interface{}, bfcfg []bitfieldcfg) []bitfield {
	// The input s should be a pointer to a struct
	rv := reflect.Indirect(reflect.ValueOf(s).Elem()) // Reflect (and dereference) so we can get struct fields by string compare
	status := []bitfield{}
	for _, b := range bfcfg {
		// FieldByName doesn't work here because it's case sensitive and I don't want people
		// writing json to have to know about the casing on these
		f := rv.FieldByNameFunc(func(n string) bool { return strings.ToLower(n) == b.Field })
		if f.Kind() == reflect.Bool {
			if f.Bool() != b.Invert {
				status = append(status, bitfield{b.Value, b.String})
				// fmt.Println("b: ", b, "status: ", status, "value", b.Value, "String", b.String)
			}
		}
	}
	return status
}

type ctrlcfg struct {
	Field string `json:"field"`
	Value bool   `json:"value"`
}

type ctrlwordcfg struct {
	Value    int       `json:"value"`
	Controls []ctrlcfg `json:"controls"`
}

func processCtrlWordConfig(a interface{}, ctrlwordcfg []ctrlwordcfg, ctrlword int) {
	rv := reflect.Indirect(reflect.ValueOf(a).Elem()) // Reflect (and dereference) so we can get struct fields by string compare
	for _, cwc := range ctrlwordcfg {
		if cwc.Value == ctrlword {
			for _, c := range cwc.Controls {
				// FieldByName doesn't work here because it's case sensitive and I don't want people
				// writing json to have to know about the casing on these
				f := rv.FieldByNameFunc(func(n string) bool { return strings.ToLower(n) == c.Field })
				if f.Kind() == reflect.Bool && f.CanSet() {
					f.SetBool(c.Value)
				}
			}
		}
	}
}

type measurement struct {
	Value              float64
	FaultHigh          bool
	FaultHighThreshold float64
	AlarmHigh          bool
	AlarmHighThreshold float64
	AlarmLow           bool
	AlarmLowThreshold  float64
	FaultLow           bool
	FaultLowThreshold  float64
	FaultsMsg          string
	AlarmsMsg          string
}

func monitorMeasurement(m interface{}, reset bool, name ...string) (fault bool, alarm bool, faults string, alarms string) {
	// This function takes an interface containing a Reflect Addr() address pointing to a measurement structure. It also takes a reset command as a boolean.
	// This function presumes that the measurement structure has fields in a particular order. If that order changes, this function must be updated. Order will be indicated with comments in the code.
	// The function checks whether it has crossed any of the alarm or fault limits, and sets the alarms and faults as appropriate
	// Faults and alarms must be reset manually by setting the Reset to true
	meas := m.(*measurement)
	//meas := reflect.ValueOf(m).Elem() // Get the measurement from its address. This gets the measurement as a Reflect.Value type, allowing Reflect.Set actions

	if reset {
		meas.FaultHigh = false
		meas.AlarmHigh = false
		meas.AlarmLow = false
		meas.FaultLow = false
	}
	meas.FaultsMsg = ""
	meas.AlarmsMsg = ""

	var vname string
	if len(name) > 0 {
		vname = name[0]
	}

	var msg string
	if meas.Value > meas.FaultHighThreshold {
		if !meas.FaultHigh && vname != "" {
			msg = fmt.Sprintf("%s High Fault: Value %.2f exceeded fault high threshold %.2f", vname, meas.Value, meas.FaultHighThreshold)
			meas.FaultsMsg = appendDelimMsg(meas.FaultsMsg, msg)
		}
		meas.FaultHigh = true // Field 1 of the measurement struct is the FaultHigh flag
		fault = true
	}
	if meas.Value > meas.AlarmHighThreshold {
		if !meas.AlarmHigh && vname != "" {
			msg = fmt.Sprintf("%s High Alarm: Value %.2f exceeded alarm high threshold %.2f", vname, meas.Value, meas.AlarmHighThreshold)
			meas.AlarmsMsg = appendDelimMsg(meas.AlarmsMsg, msg)
		}
		meas.AlarmHigh = true // Field 3 of the measurement struct is the AlarmHigh flag
		alarm = true
	}
	if meas.Value < meas.FaultLowThreshold {
		if !meas.FaultLow && vname != "" {
			msg = fmt.Sprintf("%s Low Fault: Value %.2f exceeded fault low threshold %.2f", vname, meas.Value, meas.FaultLowThreshold)
			meas.FaultsMsg = appendDelimMsg(meas.FaultsMsg, msg)
		}
		meas.FaultLow = true // Field 7 of the measurement struct is the FaultLow flag
		fault = true
	}
	if meas.Value < meas.AlarmLowThreshold {
		if !meas.AlarmLow && vname != "" {
			msg = fmt.Sprintf("%s Low Alarm: Value %.2f exceeded alarm low threshold %.2f", vname, meas.Value, meas.AlarmLowThreshold)
			meas.AlarmsMsg = appendDelimMsg(meas.AlarmsMsg, msg)
		}
		meas.AlarmLow = true // Field 5 of the measurement struct is the AlarmLow flag
		alarm = true
	}

	return fault, alarm, meas.FaultsMsg, meas.AlarmsMsg
}

func monitorStruct(inputStruct reflect.Value, reset bool) (fault bool, alarm bool, faultsMsg string, alarmsMsg string) {
	// This function inspects a structure to see if it contains measurements structures.
	// If a measurement structure is found, this function calls monitorMeasurement to update its faults and alarms.
	// If the structure contains a structure or slice, it calls monitorStruct or monitorSlice to see if they contain measurements.

	// Create variables for measurement fault and alarm status
	var lowerFault bool
	var lowerAlarm bool
	var lowerFaultsMsg string
	var lowerAlarmsMsg string

	for i := 0; i < inputStruct.NumField(); i++ {
		thisField := inputStruct.Field(i)
		if thisField.CanInterface() {
			fieldInter := thisField.Interface()
			fieldAddr := thisField.Addr().Interface()
			fieldType := reflect.ValueOf(fieldInter).Type().String() // Type will give us the named type of the variable, including user defined types such as measurement. The .String method converts this output to a string for comparison later
			fieldKind := reflect.ValueOf(fieldInter).Kind()          // Kind gives us the underlying type of the variable, ignorning user defined types, meaning that it will say that measurement is a structure.
			if fieldType == "main.measurement" {
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorMeasurement(fieldAddr, reset)
			} else if fieldKind == reflect.Struct {
				// If the field is a struct, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorStruct(thisField, reset)
			} else if fieldKind == reflect.Slice {
				// If the field is a slice, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorSlice(thisField, reset)
			}
		}
		fault = fault || lowerFault
		alarm = alarm || lowerAlarm
		faultsMsg = appendDelimMsg(faultsMsg, lowerFaultsMsg)
		alarmsMsg = appendDelimMsg(alarmsMsg, lowerAlarmsMsg)
	}

	return fault, alarm, faultsMsg, alarmsMsg
}

func monitorSlice(inputSlice reflect.Value, reset bool) (fault bool, alarm bool, faultsMsg string, alarmsMsg string) {
	// This function inspects a slice to see if it contains measurements structures.
	// If a measurement structure is found, this function calls monitorMeasurement to update its faults and alarms.
	// If the structure contains a structure or slice, it calls monitorStruct or monitorSlice to see if they contain measurements.

	// Create variables for measurement fault and alarm status
	var lowerFault bool
	var lowerAlarm bool
	var lowerFaultsMsg string
	var lowerAlarmsMsg string

	for i := 0; i < inputSlice.Len(); i++ {
		thisIndex := inputSlice.Index(i) // thisIndex represents each index in the slice
		if thisIndex.CanInterface() {
			indexInter := thisIndex.Interface()
			indexAddr := thisIndex.Addr().Interface()
			indexType := reflect.ValueOf(indexInter).Type().String() // Type will give us the named type of the variable, including user defined types such as measurement. The .String method converts this output to a string for comparison later
			indexKind := reflect.ValueOf(indexInter).Kind()          // Kind gives us the underlying type of the variable, ignorning user defined types, meaning that it will say that measurement is a structure.
			if indexType == "main.measurement" {
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorMeasurement(indexAddr, reset)
			} else if indexKind == reflect.Struct {
				// If the field is a struct, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorStruct(thisIndex, reset)
			} else if indexKind == reflect.Slice {
				// If the field is a slice, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorSlice(thisIndex, reset)
			}
		}
		fault = fault || lowerFault
		alarm = alarm || lowerAlarm
		faultsMsg = appendDelimMsg(faultsMsg, lowerFaultsMsg)
		alarmsMsg = appendDelimMsg(alarmsMsg, lowerAlarmsMsg)
	}

	return fault, alarm, faultsMsg, alarmsMsg
}

func updateMeasurements(a interface{}, reset bool) (fault bool, alarm bool, faultsMsg string, alarmsMsg string) {
	// This function takes a pointer to an asset, such as a bms or pcs, that is wrapped in an interface.
	// It iterates through the parameters in the asset structure to find measurement structures.
	// This function should only be called by assets with a Reset, Fault, and Alarm parameter
	// If a measurement structure is discovered, this function passes in the asset Reset value to the measurement Reset value and then runs the monitorMeasurement function.
	// If a fault is found in the monitorMeasurement function, the asset's Fault flag is set.

	// If "a" is a pointer to an asset, this command retrieves the asset structure itself
	thisAsset := reflect.ValueOf(a).Elem()
	//fmt.Printf("Value of a: %v\n", a)
	//fmt.Printf("Type of a: %T\n", a)
	//fmt.Printf("Value of thisAsset: %v\n", thisAsset)
	//fmt.Printf("Type of thisAsset: %T\n", thisAsset)

	for i := 0; i < thisAsset.NumField(); i++ {
		// Create variables for measurement fault and alarm status
		var lowerFault bool
		var lowerAlarm bool
		var lowerFaultsMsg string
		var lowerAlarmsMsg string
		// thisAsset.Field(j) gets us the field from the structure as we iterate through the fields in the structure
		thisField := thisAsset.Field(i)
		// A Field is a structure with information about a structure's fields. Converting it to an interface enables us to get at the underlying value and type information
		// Before converting it, however, we must check to be sure that it can be converted to an interface to prevent a panic.
		if thisField.CanInterface() {
			fieldInter := thisField.Interface()
			fieldAddr := thisField.Addr().Interface()
			fieldType := reflect.ValueOf(fieldInter).Type().String() // Type will give us the named type of the variable, including user defined types such as measurement. The .String method converts this output to a string for comparison later
			fieldKind := reflect.ValueOf(fieldInter).Kind()          // Kind gives us the underlying type of the variable, ignorning user defined types, meaning that it will say that measurement is a structure.
			if fieldType == "main.measurement" {
				nameTag := reflect.TypeOf(a).Elem().Field(i).Tag.Get("name")
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorMeasurement(fieldAddr, reset, nameTag)
			} else if fieldKind == reflect.Struct {
				// If the field is a struct, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorStruct(thisField, reset)
			} else if fieldKind == reflect.Slice {
				// If the field is a slice, we need to peek inside and see if it contains measurements. We should pass down the reset command to reset measurement faults and alarms.
				lowerFault, lowerAlarm, lowerFaultsMsg, lowerAlarmsMsg = monitorSlice(thisField, reset)
			}
		}
		fault = fault || lowerFault
		alarm = alarm || lowerAlarm
		faultsMsg = appendDelimMsg(faultsMsg, lowerFaultsMsg)
		alarmsMsg = appendDelimMsg(alarmsMsg, lowerAlarmsMsg)
	}

	return fault, alarm, faultsMsg, alarmsMsg
}

func combineTerminals(t1 terminal, t2 terminal) (t terminal) {
	t.p = t1.p + t2.p
	t.q = t1.q + t2.q
	t.s = t1.s + t2.s

	t.dHertz.slope = t1.dHertz.slope + t2.dHertz.slope    // the slope lines add up and let us
	t.dHertz.offset = t1.dHertz.offset + t2.dHertz.offset // find Hz from W
	t.f = getX(t.p, t.dHertz.slope, t.dHertz.offset)      // Drooped frequency given currently combined droops

	t.dVolts.slope = t1.dVolts.slope + t2.dVolts.slope    // the slope lines add up and let us
	t.dVolts.offset = t1.dVolts.offset + t2.dVolts.offset // find V from VAR
	t.v = getX(t.q, t.dVolts.slope, t.dVolts.offset)      // Drooped voltage given currently combined droops

	// droop for V to W, for PCS <-> BMS DC bus
	t.dVdc.slope = t1.dVdc.slope + t2.dVdc.slope    // the slope lines add up and let us
	t.dVdc.offset = t1.dVdc.offset + t2.dVdc.offset // find V from W
	t.vdc = getX(t.p, t.dVdc.slope, t.dVdc.offset)  // Drooped voltage given currently combined droops

	return t
}

type hearttime struct {
	// This data type tracks hearbeat and time for an asset
	Heartbeat int
	Time      time.Time
	Year      float64
	Month     float64
	Day       float64
	Hour      float64
	Minute    float64
	Second    float64
}

func updateHeartTime(h *hearttime) {
	// This function takes a pointer to a hearttime structure, updates the current time, and increments the heartbeat
	h.Heartbeat++
	if h.Heartbeat > 1000000 {
		h.Heartbeat = 0
	}
	h.Time = time.Now()
	h.Year = float64(h.Time.Year())
	h.Month = float64(h.Time.Month())
	h.Day = float64(h.Time.Day())
	h.Hour = float64(h.Time.Hour())
	h.Minute = float64(h.Time.Minute())
	h.Second = float64(h.Time.Second())
}

//linear interpolation to find yi from xi based on breakpoints supplied by xvec and yvec
func interpl(xvec []float64, yvec []float64, xi float64) (yi float64, flt bool) {
	xlen := len(xvec)
	ylen := len(yvec)
	var found bool = false
	if (xlen != ylen) || xlen < 2 {
		return 0.0, true
	}
	//linear interpolation
	//See https://en.wikipedia.org/wiki/Linear_interpolation for derivation
	if xvec[1] > xvec[0] {
		for i := 0; i < (len(xvec) - 1); i++ {
			if xi >= xvec[i] && xi <= xvec[i+1] {
				yi = (yvec[i]*(xvec[i+1]-xi) + yvec[i+1]*(xi-xvec[i])) / (xvec[i+1] - xvec[i])
				found = true
			}
		}
	} else {
		for i := 0; i < (len(xvec) - 1); i++ {
			if xi <= xvec[i] && xi >= xvec[i+1] {
				yi = (yvec[i]*(xvec[i+1]-xi) + yvec[i+1]*(xi-xvec[i])) / (xvec[i+1] - xvec[i])
				found = true
			}
		}
	}
	if found {
		return yi, false
	}
	if xi > xvec[xlen-1] { //extrapolate
		yi = (yvec[xlen-2]*(xvec[xlen-1]-xi) + yvec[xlen-1]*(xi-xvec[xlen-2])) / (xvec[xlen-1] - xvec[xlen-2])
		return yi, true //indicate to calling function that input was out of bounds. This may be ok or may not be depending on usage.
	} else if xi < xvec[0] {
		yi = (yvec[0]*(xvec[1]-xi) + yvec[1]*(xi-xvec[0])) / (xvec[1] - xvec[0])
		return yi, true
	}
	return yi, true
}

type rack struct {
	// This data type tracks the parameters of a single rack in a BMS system
	Vdc                 float64       // Measured dc voltage
	Cap                 float64       // Nominal capacity in kWh
	Pmax                float64       // Nominal rack charge/discharge limit (does not vary with SOC)
	Idleloss            float64       // Fixed losses across load
	Rte                 float64       // Round trip efficiency
	pesr                float64       // power normalized esr, see Init()
	Idc                 measurement   // Measured dc current
	P                   float64       // Power delivered to upstream devices, e.g. to BMS, can think of as "upstream" - set by upstream bms
	Pdc                 float64       // Power drawn from battery, includes losses inside the battery, used to adjust SOC, can think of as "downstream" - calculated from P and losses
	Soc                 measurement   // measured state of charge
	Soh                 float64       // Rack state of health
	NumCells            int           // Number of cells in series in a battery rack
	cellVoltOver        float64       // Percentage by which highest cell voltage is above the average
	cellVoltUnder       float64       // Percentage by which lowest cell voltage is below the average
	MaxCellVolt         measurement   // maximum cell voltage in the BMS
	AvgCellVolt         float64       // average cell voltage in the BMS
	MinCellVolt         measurement   // minimum cell voltage in the BMS
	MaxCellTemp         measurement   // maximum cell temperature in the BMS
	AvgCellTemp         float64       // average cell temperature in the BMS
	MinCellTemp         measurement   // minimum cell temperature in the BMS
	Pcharge             float64       // rack max charge limit (varies with SOC)
	Pdischarge          float64       // rack max discharge limit (varies with SOC)
	Icharge             float64       // rack max charging current, based on Pcharge and Vdc
	Idischarge          float64       // rack max discharging current, based on Pdischarge and Vdc
	DcContactor         bool          // State of the dc contactor on the rack (true = closed, false = open)
	DcContactorOpenCmd  bool          // Command to open the dc contactor
	DcContactorCloseCmd bool          // Command to close the dc contactor
	Enabled             bool          // State of the battery rack (true = enabled = dc contactors can be opened/closed, false = disabled)
	EnableCmd           bool          // Command to enable battery rack
	DisableCmd          bool          // Command to disable battery rack
	CtrlWord1           int           // Control value for enabling/disabling battery rack
	CtrlWord1Cfg        []ctrlwordcfg // Control configuration for enabling/disabling battery rack
}
