/**
 *
 * setpoint.go
 *
 * Methods for the `setpoint` data type.
 *
 */
package main

import "log"

type setpoint struct {
	id      string
	name    string
	unit    string
	uri     string
	varType string
	value   valueObj
}

// sendGet sends a GET to the setpoint's URI.
func (sp setpoint) sendGet(replyToUri string) {
	f.SendGet(sp.uri, replyToUri)
}

// sendSet sends a single setpoint to its URI via a FIMS SET.
func (sp setpoint) sendSet() {
	// if scheduler events are disabled, do not send setpoints
	if !enableSchedulerEvents {
		return
	}
	fimsBody := make(map[string]interface{})
	fimsBody["value"] = sp.get()
	f.SendSet(sp.uri, "", fimsBody)
}

// get returns the value from the register matching the setpoint's designated variable type.
func (sp setpoint) get() interface{} {
	var val interface{} = nil
	switch sp.varType {
	case "Float":
		val = sp.value.value_float
	case "Int":
		val = sp.value.value_int
	case "Bool":
		val = sp.value.value_bool
	case "String":
		val = sp.value.value_string
	}
	return val
}

// set tries to update a setpoint's value as long as the type check matches.
func (sp *setpoint) set(newValInterface interface{}) bool {
	switch sp.varType {
	case "Float":
		newVal, ok := newValInterface.(float64)
		if !ok {
			return false
		}
		sp.value.value_float = newVal
	case "Int":
		newVal, ok := newValInterface.(int)
		if !ok {
			return false
		}
		sp.value.value_int = newVal
	case "Bool":
		newVal, ok := newValInterface.(bool)
		if !ok {
			return false
		}
		sp.value.value_bool = newVal
	case "String":
		newVal, ok := newValInterface.(string)
		if !ok {
			return false
		}
		sp.value.value_string = newVal
	}
	return true
}

// addToList converts a setpoint to a map[string]interface{} and adds it to a list of setpoints for easy FIMS sending
func (sp setpoint) addToList(list *[]interface{}) {
	v := make(map[string]interface{})
	v["id"] = sp.id
	v["name"] = sp.name
	v["unit"] = sp.unit
	v["uri"] = sp.uri
	v["type"] = sp.varType
	switch sp.varType {
	case "Float":
		v["value"] = sp.value.value_float
	case "Int":
		v["value"] = sp.value.value_int
	case "Bool":
		v["value"] = sp.value.value_bool
	case "String":
		v["value"] = sp.value.value_string
	}
	*list = append(*list, v)
}

// sends default mode constants
func sendDefaultSetpoints() {
	defaultMode := modes.getMode("default")
	if defaultMode == nil {
		log.Println("events.go::sendDefaultSetpoints() ~ Default mode not found in modes map")
		return
	}
	defaultMode.sendConstants()
}
