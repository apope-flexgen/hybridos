/*
 * scadapoint.go
 *
 * A scadapoint represents a site-connected variable that may or may not be able to be set by Fleet Manager but always
 * has a status value reported back by the site.
 *
 */
package main

import (
	"fmt"
	"math"
	"reflect"

	fg "github.com/flexgen-power/go_flexgen"
	log "github.com/flexgen-power/go_flexgen/logger"
)

type scadapoint struct {
	id      string
	control interface{}
	status  interface{}
}

func newFloatControl(id string) scadapoint {
	return scadapoint{
		id:      id,
		control: float64(0.0),
		status:  float64(0.0),
	}
}

func newFloatStatus(id string) scadapoint {
	return scadapoint{
		id:      id,
		control: nil,
		status:  float64(0.0),
	}
}

func newIntControl(id string) scadapoint {
	return scadapoint{
		id:      id,
		control: int(0),
		status:  int(0),
	}
}

func newIntStatus(id string) scadapoint {
	return scadapoint{
		id:      id,
		control: nil,
		status:  int(0),
	}
}

func newBoolStatus(id string) scadapoint {
	return scadapoint{
		id:      id,
		control: nil,
		status:  bool(false),
	}
}

func (sp *scadapoint) addToObject(m map[string]interface{}) {
	m[sp.id] = sp.status
}

func (sp *scadapoint) optionalParse(m map[string]interface{}, siteId string) {
	// DNP3 endpoints can't be named the same so a DNP3 variable with an input and output has
	// a "_in" appended to its input URI, while the Fleet Manager URI remains the same and the values
	// are stored in control (output) and status (input) fields of a scadapoint
	val, found := m[sp.id]
	if !found {
		//checks if input has suffix
		val, found = m[sp.id+"_in"]
		if !found {
			return
		}
	}
	val = fg.UnwrapVariable(val)
	// for ints, allow data types that are castable to ints (since JSON will deliver ints as float64s)
	expectedType := reflect.TypeOf(sp.status)
	var varInt int
	if expectedType == reflect.TypeOf(varInt) {
		castedVal, err := fg.CastToInt(val)
		if err == nil {
			val = castedVal
		}
	}
	// validate data type
	receivedType := reflect.TypeOf(val)
	if receivedType != expectedType {
		log.Errorf("Failed to parse %s for %s: expected %s but got %s", sp.id, siteId, expectedType.String(), receivedType.String())
	}
	// save the new value
	sp.status = val
}

func (sp *scadapoint) withinTolerance(tolerance float64) bool {
	return math.Abs(sp.status.(float64)-sp.control.(float64)) <= tolerance
}

// sendControlValue sends the control value to the given URI if the
// status value and control value do not match each other.
func (sp scadapoint) sendControlValue(baseUri string) {
	if sp.status == sp.control {
		return
	}
	if err := f.SendSet(fmt.Sprintf("%s/%s", baseUri, sp.id), "", sp.control); err != nil {
		log.Errorf("Failed to send control scadapoint %s to URI %s: %s", sp.id, fmt.Sprintf("%s/%s", baseUri, sp.id), err.Error())
	}
}

func (sp *scadapoint) sendFloatControlValue(baseUri string) {
	if sp.withinTolerance(0.0001) {
		return
	}
	sp.sendControlValue(baseUri)
}
