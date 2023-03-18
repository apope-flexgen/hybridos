package main

import (
	"fmt"
	"strings"

	fg "github.com/flexgen-power/go_flexgen"
)

const genSuffix string = "_gen"
const lenGenSuffix int = len(genSuffix)
const loadSuffix string = "_load"
const lenLoadSuffix int = len(loadSuffix)

//-------------------------------MUX BOOL---------------------------------------------------------------------
// muxBool encapsulates the four values used in a manual override logic block using a struct and the
// getSelect() function
type muxBool struct {
	actual   bool
	manual   bool
	override bool
}

// performs the manual override selection logic (if override is true: take manual value;
// else take actual value)
func (mb *muxBool) getSelect() bool {
	if mb.override {
		return mb.manual
	}
	return mb.actual
}

// Sets the corresponding field of muxBool to the input boolean value.
func (mb *muxBool) setField(search string, newVal bool) (valueChanged bool, err error) {
	switch search {
	case "_manual":
		valueChanged = mb.manual != newVal
		mb.manual = newVal
	case "_override":
		valueChanged = mb.override != newVal
		mb.override = newVal
	case "_actual":
		valueChanged = mb.actual != newVal
		mb.actual = newVal
	default:
		return false, fmt.Errorf("%s is not a valid suffix", search)
	}
	return valueChanged, nil
}

// Adds the actual/manual/override fields and select mux output to the given object.
func (mb *muxBool) addToObj(obj map[string]interface{}, idRoot string) {
	obj[idRoot+"_actual"] = mb.actual
	obj[idRoot+"_manual"] = mb.manual
	obj[idRoot+"_override"] = mb.override
	obj[idRoot+"_select"] = mb.getSelect()
}

//-------------------------------MUX FLOAT-------------------------------------------------------

// muxFloat encapsulates the four values used in a manual override logic block using a struct and the
// getSelect() function
type muxFloat struct {
	actual   float64
	manual   float64
	override bool
}

// Performs the manual override selection logic (if override is true: take manual value;
// else take actual value).
func (mf *muxFloat) getSelect() float64 {
	if mf.override {
		return mf.manual
	}
	return mf.actual
}

// Sets the corresponding field "search" of muxFloat to the given new value.
// Returns true if the new value is not the same as what the old value was.
func (mf *muxFloat) setField(search string, newVal interface{}) (valueChanged bool, err error) {
	newVal = fg.UnwrapVariable(newVal)
	switch search {
	case "_manual":
		confirmedVal, ok := newVal.(float64)
		if !ok {
			return false, fmt.Errorf("expected float64 for %s but got %T", search, newVal)
		}
		if mf.manual != confirmedVal {
			mf.manual = confirmedVal
			return true, nil
		}
	case "_override":
		confirmedVal, ok := newVal.(bool)
		if !ok {
			return false, fmt.Errorf("expected bool for %s but got %T", search, newVal)
		}
		if mf.override != confirmedVal {
			mf.override = confirmedVal
			return true, nil
		}
	case "_actual":
		confirmedVal, ok := newVal.(float64)
		if !ok {
			return false, fmt.Errorf("expected float64 for %s but got %T", search, newVal)
		}
		if mf.actual != confirmedVal {
			mf.actual = confirmedVal
			return true, nil
		}
	default:
		return false, fmt.Errorf("%s is not a valid suffix", search)
	}
	return false, nil
}

// Adds the actual, manual, and override fields to the given object with the respective ID suffixes
// "_actual", "_manual", and "_override" being added to the root variable ID.
func (mf *muxFloat) addToObj(obj map[string]interface{}, idRoot string) {
	obj[idRoot+"_actual"] = mf.actual
	obj[idRoot+"_manual"] = mf.manual
	obj[idRoot+"_override"] = mf.override
	obj[idRoot+"_select"] = mf.getSelect()
}

//-------------------------------MUX INT-------------------------------------------------------

// muxInt encapsulates the four values used in a manual override logic block using a struct and the
// getSelect() function
type muxInt struct {
	actual   int
	manual   int
	override bool
}

// Performs the manual override selection logic (if override is true: take manual value;
// else take actual value).
func (mi *muxInt) getSelect() int {
	if mi.override {
		return mi.manual
	}
	return mi.actual
}

// Sets the corresponding field "search" of muxInt to the given new value.
func (mi *muxInt) setField(search string, newVal interface{}) (valueChanged bool, err error) {
	newVal = fg.UnwrapVariable(newVal)
	switch search {
	case "_manual":
		confirmedVal, err := fg.CastToInt(newVal)
		if err != nil {
			return false, fmt.Errorf("expected int for %s but got %T", search, newVal)
		}
		valueChanged = mi.manual != confirmedVal
		mi.manual = confirmedVal
	case "_override":
		confirmedVal, ok := newVal.(bool)
		if !ok {
			return false, fmt.Errorf("expected bool for %s but got %T", search, newVal)
		}
		valueChanged = mi.override != confirmedVal
		mi.override = confirmedVal
	case "_actual":
		confirmedVal, err := fg.CastToInt(newVal)
		if err != nil {
			return false, fmt.Errorf("expected int for %s but got %T", search, newVal)
		}
		valueChanged = mi.actual != confirmedVal
		mi.actual = confirmedVal
	default:
		return false, fmt.Errorf("%s is not a valid suffix", search)
	}
	return valueChanged, nil
}

// Adds the actual, manual, and override fields to the given object with the respective ID suffixes
// "_actual", "_manual", and "_override" being added to the root variable ID.
func (mi *muxInt) addToObj(obj map[string]interface{}, root string) {
	obj[root+"_actual"] = mi.actual
	obj[root+"_manual"] = mi.manual
	obj[root+"_override"] = mi.override
	obj[root+"_select"] = mi.getSelect()
}

//-------------------------------SCHEDULED SERVICE-------------------------------------------------------

// encapsulates the reponsibility, requirement, and scheduled amount for a service/frequency response
// into one object
type scheduledService struct {
	responsibility muxFloat
	requirement    muxFloat
	scheduled      float64
}

// Sets the corresponding field "search" of scheduledService to the new value.
// Returns true if the new value is not the same as what the old value was.
func (ss *scheduledService) setField(search string, newVal interface{}) (valueChanged bool, err error) {
	newVal = fg.UnwrapVariable(newVal)
	if strings.HasPrefix(search, "_responsibility") {
		return ss.responsibility.setField(search[len("_responsibility"):], newVal)
	} else if strings.HasPrefix(search, "_requirement") {
		return ss.requirement.setField(search[len("_requirement"):], newVal)
	} else if search == "_scheduled" {
		confirmedVal, ok := newVal.(float64)
		if !ok {
			return false, fmt.Errorf("expected a float64 for _scheduled field but got %T", newVal)
		}
		if ss.scheduled != confirmedVal {
			ss.scheduled = confirmedVal
			return true, nil
		}
		return false, nil
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all the fields of the responsibility/requirement objects and the scheduled field to the given object.
func (ss *scheduledService) addToObj(obj map[string]interface{}, root string) {
	ss.responsibility.addToObj(obj, root+"_responsibility")
	ss.requirement.addToObj(obj, root+"_requirement")
	obj[root+"_scheduled"] = ss.scheduled
}

//-------------------------------PARTICIPATING SERVICE-------------------------------------------------------

// encapsulates the reponsibility, requirement, and participation amount for a service/frequency response
// into one object
type participatingService struct {
	responsibility muxFloat
	requirement    muxFloat
	participation  float64
}

// Sets the corresponding field "search" of participatingService to the new value.
func (ps *participatingService) setField(search string, newVal interface{}) (valueChanged bool, err error) {
	newVal = fg.UnwrapVariable(newVal)
	if strings.HasPrefix(search, "_responsibility") {
		return ps.responsibility.setField(search[len("_responsibility"):], newVal)
	} else if strings.HasPrefix(search, "_requirement") {
		return ps.requirement.setField(search[len("_requirement"):], newVal)
	} else if search == "_participation" {
		confirmedVal, ok := newVal.(float64)
		if !ok {
			return false, fmt.Errorf("expected a float64 for participation but got %T", newVal)
		}
		if ps.participation != confirmedVal {
			ps.participation = confirmedVal
			return true, nil
		}
		return false, nil
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all the fields from the responsibility and requirement objects to the given object
// with respective suffixes "_responsibility" and "_requirement". Also adds the participation
// field with a suffix of "_participation".
func (ps *participatingService) addToObj(obj map[string]interface{}, root string) {
	ps.responsibility.addToObj(obj, root+"_responsibility")
	ps.requirement.addToObj(obj, root+"_requirement")
	obj[root+"_participation"] = ps.participation
}

//-------------------------------GEN/LOAD MUX INT-------------------------------------------------------

// encapsulates the muxInt values of both the gen and load equivalents into one object
type genLoadMuxInt struct {
	idRoot string
	gen    muxInt
	load   muxInt
}

// Sets the corresponding gen or load value based on the prefix and suffix of "search" with the new value.
func (glmi *genLoadMuxInt) setGenLoad(search string, newVal interface{}) (valueChanged bool, err error) {
	if strings.HasPrefix(search, genSuffix) {
		return glmi.gen.setField(search[lenGenSuffix:], newVal)
	} else if strings.HasPrefix(search, loadSuffix) {
		return glmi.load.setField(search[lenLoadSuffix:], newVal)
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all fields from the gen and the load objects to the given object, with the
// respective suffixes "_gen" and "_load" being added to the root variable ID.
func (glmi *genLoadMuxInt) addToObj(obj map[string]interface{}) {
	glmi.gen.addToObj(obj, glmi.idRoot+genSuffix)
	glmi.load.addToObj(obj, glmi.idRoot+loadSuffix)
}

// parses through the given configuration "cfg" and passes the gen and load configs to lower parsing fcns
func parseGLMuxInt(cfg map[string]interface{}, idRoot string) genLoadMuxInt {
	glmi := genLoadMuxInt{
		idRoot: idRoot,
	}

	gen, err := fg.ExtractAsInt(cfg, idRoot+"_gen_actual")
	if err != nil {
		glmi.gen.actual = 0
	} else {
		glmi.gen.actual = gen
	}
	gen, err = fg.ExtractAsInt(cfg, idRoot+"_gen_manual")
	if err != nil {
		glmi.gen.manual = 0
	} else {
		glmi.gen.manual = gen
	}
	genInterface, err := fg.ExtractValueWithType(cfg, idRoot+"_gen_override", fg.BOOL)
	if err != nil {
		glmi.gen.override = false
	} else {
		glmi.gen.override = genInterface.(bool)
	}

	load, err := fg.ExtractAsInt(cfg, idRoot+"_load_actual")
	if err != nil {
		glmi.load.actual = 0
	} else {
		glmi.load.actual = load
	}
	load, err = fg.ExtractAsInt(cfg, idRoot+"_load_manual")
	if err != nil {
		glmi.load.manual = 0
	} else {
		glmi.load.manual = load
	}
	loadInterface, err := fg.ExtractValueWithType(cfg, idRoot+"_load_override", fg.BOOL)
	if err != nil {
		glmi.load.override = false
	} else {
		glmi.load.override = loadInterface.(bool)
	}
	return glmi
}

//-------------------------------GEN/LOAD MUX FLOAT-------------------------------------------------------

// encapsulates the muxFloat values of both the gen and load equivalents into one object
type genLoadMuxFloat struct {
	idRoot string
	gen    muxFloat
	load   muxFloat
}

// sets the corresponding gen or load value named "search" with the input value (float) "newVal"
func (glmf *genLoadMuxFloat) setGenLoad(search string, newVal interface{}) (valueChanged bool, err error) {
	if strings.HasPrefix(search, genSuffix) {
		return glmf.gen.setField(search[lenGenSuffix:], newVal)
	} else if strings.HasPrefix(search, loadSuffix) {
		return glmf.load.setField(search[lenLoadSuffix:], newVal)
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all fields from the gen and the load objects to the given object, with the
// respective suffixes "_gen" and "_load" being added to the root variable ID.
func (glmf *genLoadMuxFloat) addToObj(obj map[string]interface{}) {
	glmf.gen.addToObj(obj, glmf.idRoot+genSuffix)
	glmf.load.addToObj(obj, glmf.idRoot+loadSuffix)
}

// parses through the given configuration "cfg" and passes the gen and load configs to lower parsing fcns
func parseGLMuxFloat(cfg map[string]interface{}, idRoot string) genLoadMuxFloat {
	glmf := genLoadMuxFloat{
		idRoot: idRoot,
	}

	gen, err := fg.ExtractValueWithType(cfg, idRoot+"_gen_actual", fg.FLOAT64)
	if err != nil {
		glmf.gen.actual = 0.0
	} else {
		glmf.gen.actual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_manual", fg.FLOAT64)
	if err != nil {
		glmf.gen.manual = 0.0
	} else {
		glmf.gen.manual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_override", fg.BOOL)
	if err != nil {
		glmf.gen.override = false
	} else {
		glmf.gen.override = gen.(bool)
	}

	load, err := fg.ExtractValueWithType(cfg, idRoot+"_load_actual", fg.FLOAT64)
	if err != nil {
		glmf.load.actual = 0.0
	} else {
		glmf.load.actual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_manual", fg.FLOAT64)
	if err != nil {
		glmf.load.manual = 0.0
	} else {
		glmf.load.manual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_override", fg.BOOL)
	if err != nil {
		glmf.load.override = false
	} else {
		glmf.load.override = load.(bool)
	}
	return glmf
}

//-------------------------------GEN/LOAD SCHEDULED SERVICE-------------------------------------------------------

// encapsulates the scheduledService values of both the gen and load equivalents into one object
type genLoadScheduledService struct {
	idRoot string
	gen    scheduledService
	load   scheduledService
}

// sets the corresponding gen or load value named "search" with the input value (float) "newVal"
func (glss *genLoadScheduledService) setGenLoad(search string, newVal interface{}) (valueChanged bool, err error) {
	if strings.HasPrefix(search, genSuffix) {
		return glss.gen.setField(search[lenGenSuffix:], newVal)
	} else if strings.HasPrefix(search, loadSuffix) {
		return glss.load.setField(search[lenLoadSuffix:], newVal)
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all fields from the gen/load objects to the given object.
func (glss *genLoadScheduledService) addToObj(obj map[string]interface{}) {
	glss.gen.addToObj(obj, glss.idRoot+genSuffix)
	glss.load.addToObj(obj, glss.idRoot+loadSuffix)
}

// parses through the given configuration "cfg" and passes the gen and load configs to lower parsing fcns
func parseGLSService(cfg map[string]interface{}, idRoot string) genLoadScheduledService {
	glss := genLoadScheduledService{
		idRoot: idRoot,
	}
	gen, err := fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_actual", fg.FLOAT64)
	if err != nil {
		glss.gen.responsibility.actual = 0.0
	} else {
		glss.gen.responsibility.actual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_manual", fg.FLOAT64)
	if err != nil {
		glss.gen.responsibility.manual = 0.0
	} else {
		glss.gen.responsibility.manual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_override", fg.BOOL)
	if err != nil {
		glss.gen.responsibility.override = false
	} else {
		glss.gen.responsibility.override = gen.(bool)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_actual", fg.FLOAT64)
	if err != nil {
		glss.gen.requirement.actual = 0.0
	} else {
		glss.gen.requirement.actual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_manual", fg.FLOAT64)
	if err != nil {
		glss.gen.requirement.manual = 0.0
	} else {
		glss.gen.requirement.manual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_override", fg.BOOL)
	if err != nil {
		glss.gen.requirement.override = false
	} else {
		glss.gen.requirement.override = gen.(bool)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_scheduled", fg.FLOAT64)
	if err != nil {
		glss.gen.scheduled = 0.0
	} else {
		glss.gen.scheduled = gen.(float64)
	}

	load, err := fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_actual", fg.FLOAT64)
	if err != nil {
		glss.load.responsibility.actual = 0.0
	} else {
		glss.load.responsibility.actual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_manual", fg.FLOAT64)
	if err != nil {
		glss.load.responsibility.manual = 0.0
	} else {
		glss.load.responsibility.manual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_override", fg.BOOL)
	if err != nil {
		glss.load.responsibility.override = false
	} else {
		glss.load.responsibility.override = load.(bool)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_actual", fg.FLOAT64)
	if err != nil {
		glss.load.requirement.actual = 0.0
	} else {
		glss.load.requirement.actual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_manual", fg.FLOAT64)
	if err != nil {
		glss.load.requirement.manual = 0.0
	} else {
		glss.load.requirement.manual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_override", fg.BOOL)
	if err != nil {
		glss.load.requirement.override = false
	} else {
		glss.load.requirement.override = load.(bool)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_scheduled", fg.FLOAT64)
	if err != nil {
		glss.load.scheduled = 0.0
	} else {
		glss.load.scheduled = load.(float64)
	}
	return glss
}

//-------------------------------GEN/LOAD PARTICIPATING SERVICE-------------------------------------------------------

// encapsulates the participatingService values of both the gen and load equivalents into one object
type genLoadParticipatingService struct {
	idRoot string
	gen    participatingService
	load   participatingService
}

// sets the corresponding gen or load value named "search" with the input value (float) "newVal"
func (glps *genLoadParticipatingService) setGenLoad(search string, newVal interface{}) (valueChanged bool, err error) {
	if strings.HasPrefix(search, genSuffix) {
		return glps.gen.setField(search[lenGenSuffix:], newVal)
	} else if strings.HasPrefix(search, loadSuffix) {
		return glps.load.setField(search[lenLoadSuffix:], newVal)
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
}

// Adds all of the fields form the gen and load objects to the given object, adding respective
// suffixes "_gen" and "_load" to the root variable ID.
func (glps *genLoadParticipatingService) addToObj(obj map[string]interface{}) {
	glps.gen.addToObj(obj, glps.idRoot+genSuffix)
	glps.load.addToObj(obj, glps.idRoot+loadSuffix)
}

// parses through the given configuration "cfg" and passes the gen and load configs to lower parsing fcns
func parseGLPService(cfg map[string]interface{}, idRoot string) genLoadParticipatingService {
	glps := genLoadParticipatingService{
		idRoot: idRoot,
	}

	gen, err := fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_actual", fg.FLOAT64)
	if err != nil {
		glps.gen.responsibility.actual = 0.0
	} else {
		glps.gen.responsibility.actual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_manual", fg.FLOAT64)
	if err != nil {
		glps.gen.responsibility.manual = 0.0
	} else {
		glps.gen.responsibility.manual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility_override", fg.BOOL)
	if err != nil {
		glps.gen.responsibility.override = false
	} else {
		glps.gen.responsibility.override = gen.(bool)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_actual", fg.FLOAT64)
	if err != nil {
		glps.gen.requirement.actual = 0.0
	} else {
		glps.gen.requirement.actual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_manual", fg.FLOAT64)
	if err != nil {
		glps.gen.requirement.manual = 0.0
	} else {
		glps.gen.requirement.manual = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement_override", fg.BOOL)
	if err != nil {
		glps.gen.requirement.override = false
	} else {
		glps.gen.requirement.override = gen.(bool)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_participation", fg.FLOAT64)
	if err != nil {
		glps.gen.participation = 0.0
	} else {
		glps.gen.participation = gen.(float64)
	}

	load, err := fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_actual", fg.FLOAT64)
	if err != nil {
		glps.load.responsibility.actual = 0.0
	} else {
		glps.load.responsibility.actual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_manual", fg.FLOAT64)
	if err != nil {
		glps.load.responsibility.manual = 0.0
	} else {
		glps.load.responsibility.manual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility_override", fg.BOOL)
	if err != nil {
		glps.load.responsibility.override = false
	} else {
		glps.load.responsibility.override = load.(bool)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_actual", fg.FLOAT64)
	if err != nil {
		glps.load.requirement.actual = 0.0
	} else {
		glps.load.requirement.actual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_manual", fg.FLOAT64)
	if err != nil {
		glps.load.requirement.manual = 0.0
	} else {
		glps.load.requirement.manual = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement_override", fg.BOOL)
	if err != nil {
		glps.load.requirement.override = false
	} else {
		glps.load.requirement.override = load.(bool)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_participation", fg.FLOAT64)
	if err != nil {
		glps.load.participation = 0.0
	} else {
		glps.load.participation = load.(float64)
	}

	return glps
}

//-------------------------------GEN/LOAD SERVICE (NO MUX)-------------------------------------------------------

// encapsulates the scheduled service (with no multiplexor functionality) values of both the gen and load equivalents into one object
type genLoadServiceNoMux struct {
	idRoot             string
	genRequirement     float64
	genResponsibility  float64
	genScheduled       float64
	loadRequirement    float64
	loadResponsibility float64
	loadScheduled      float64
}

// sets the corresponding gen or load value named "search" with the input value (float) "newVal"
func (glsnm *genLoadServiceNoMux) setGenLoad(search string, newVal interface{}) (valueChanged bool, err error) {
	// handle both clothed and naked values
	newVal = fg.UnwrapVariable(newVal)

	confirmedVal, ok := newVal.(float64)
	if !ok {
		return false, fmt.Errorf("value for %s was expected to be a float. Received value of type %T", search, newVal)
	}
	if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "responsibility") {
		valueChanged = glsnm.genResponsibility != confirmedVal
		glsnm.genResponsibility = confirmedVal
	} else if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "requirement") {
		valueChanged = glsnm.genRequirement != confirmedVal
		glsnm.genRequirement = confirmedVal
	} else if strings.HasPrefix(search, genSuffix) && strings.Contains(search, "scheduled") {
		valueChanged = glsnm.genScheduled != confirmedVal
		glsnm.genScheduled = confirmedVal
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "responsibility") {
		valueChanged = glsnm.loadResponsibility != confirmedVal
		glsnm.loadResponsibility = confirmedVal
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "requirement") {
		valueChanged = glsnm.loadRequirement != confirmedVal
		glsnm.loadRequirement = confirmedVal
	} else if strings.HasPrefix(search, loadSuffix) && strings.Contains(search, "scheduled") {
		valueChanged = glsnm.loadScheduled != confirmedVal
		glsnm.loadScheduled = confirmedVal
	} else {
		return false, fmt.Errorf("%s is not a valid endpoint", search)
	}
	return valueChanged, nil
}

// Adds all of the gen/load responsibility/requirement/scheduled fields to the given object.
func (glsnm *genLoadServiceNoMux) addToObj(obj map[string]interface{}) {
	obj[glsnm.idRoot+"_gen_responsibility"] = glsnm.genResponsibility
	obj[glsnm.idRoot+"_gen_requirement"] = glsnm.genRequirement
	obj[glsnm.idRoot+"_gen_scheduled"] = glsnm.genScheduled
	obj[glsnm.idRoot+"_load_responsibility"] = glsnm.loadResponsibility
	obj[glsnm.idRoot+"_load_requirement"] = glsnm.loadRequirement
	obj[glsnm.idRoot+"_load_scheduled"] = glsnm.loadScheduled
}

// parses through the given configuration "cfg" and passes the gen and load configs to lower parsing fcns
func parseGLSNoMux(cfg map[string]interface{}, idRoot string) genLoadServiceNoMux {
	glsnm := genLoadServiceNoMux{
		idRoot: idRoot,
	}

	gen, err := fg.ExtractValueWithType(cfg, idRoot+"_gen_responsibility", fg.FLOAT64)
	if err != nil {
		glsnm.genResponsibility = 0.0
	} else {
		glsnm.genResponsibility = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_requirement", fg.FLOAT64)
	if err != nil {
		glsnm.genRequirement = 0.0
	} else {
		glsnm.genRequirement = gen.(float64)
	}
	gen, err = fg.ExtractValueWithType(cfg, idRoot+"_gen_scheduled", fg.FLOAT64)
	if err != nil {
		glsnm.genScheduled = 0.0
	} else {
		glsnm.genScheduled = gen.(float64)
	}

	load, err := fg.ExtractValueWithType(cfg, idRoot+"_load_responsibility", fg.FLOAT64)
	if err != nil {
		glsnm.loadResponsibility = 0.0
	} else {
		glsnm.loadResponsibility = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_requirement", fg.FLOAT64)
	if err != nil {
		glsnm.loadRequirement = 0.0
	} else {
		glsnm.loadRequirement = load.(float64)
	}
	load, err = fg.ExtractValueWithType(cfg, idRoot+"_load_scheduled", fg.FLOAT64)
	if err != nil {
		glsnm.loadScheduled = 0.0
	} else {
		glsnm.loadScheduled = load.(float64)
	}

	return glsnm
}
