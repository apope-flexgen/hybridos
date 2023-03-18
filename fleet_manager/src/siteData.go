package main

type frequencyResponseData struct {
	baseloadCmd            scadapoint
	inactiveFfrCmd         scadapoint
	inactiveFrrsUpCmd      scadapoint
	inactiveFrrsDownCmd    scadapoint
	responseEnableBitField scadapoint
	frrsDownCmd            scadapoint
	frrsUpCmd              scadapoint
	rrsFfrCmd              scadapoint
	autoPfrUpCmd           scadapoint
	autoPfrDownCmd         scadapoint
}

func newFrequencyResponseData() frequencyResponseData {
	return frequencyResponseData{
		baseloadCmd:            newFloatControl("fr_baseload_cmd_mw"),
		inactiveFfrCmd:         newFloatControl("fr_ffr_inactive_cmd_mw"),
		inactiveFrrsUpCmd:      newFloatControl("fr_frrs_up_inactive_cmd_mw"),
		inactiveFrrsDownCmd:    newFloatControl("fr_frrs_down_inactive_cmd_mw"),
		responseEnableBitField: newIntControl("fr_response_enable_mask"),
		frrsUpCmd:              newFloatControl("fr_frrs_up_active_cmd_mw"),
		frrsDownCmd:            newFloatControl("fr_frrs_down_active_cmd_mw"),
		rrsFfrCmd:              newFloatControl("fr_ffr_active_cmd_mw"),
		autoPfrUpCmd:           newFloatControl("fr_pfr_up_active_cmd_mw"),
		autoPfrDownCmd:         newFloatControl("fr_pfr_down_active_cmd_mw"),
	}
}

func (data *frequencyResponseData) addToObject(obj map[string]interface{}) {
	data.baseloadCmd.addToObject(obj)
	data.inactiveFfrCmd.addToObject(obj)
	data.inactiveFrrsUpCmd.addToObject(obj)
	data.inactiveFrrsDownCmd.addToObject(obj)
	data.responseEnableBitField.addToObject(obj)
	data.frrsUpCmd.addToObject(obj)
	data.frrsDownCmd.addToObject(obj)
	data.rrsFfrCmd.addToObject(obj)
	data.autoPfrDownCmd.addToObject(obj)
	data.autoPfrUpCmd.addToObject(obj)
}

func (data *frequencyResponseData) handleClientPub(m map[string]interface{}, siteId string) {
	data.baseloadCmd.optionalParse(m, siteId)
	data.inactiveFfrCmd.optionalParse(m, siteId)
	data.inactiveFrrsUpCmd.optionalParse(m, siteId)
	data.inactiveFrrsDownCmd.optionalParse(m, siteId)
	data.responseEnableBitField.optionalParse(m, siteId)
	data.frrsUpCmd.optionalParse(m, siteId)
	data.frrsDownCmd.optionalParse(m, siteId)
	data.rrsFfrCmd.optionalParse(m, siteId)
	data.autoPfrUpCmd.optionalParse(m, siteId)
	data.autoPfrDownCmd.optionalParse(m, siteId)
}

func (data *frequencyResponseData) sendControlValues(baseUri string) {
	data.baseloadCmd.sendFloatControlValue(baseUri)
	data.inactiveFfrCmd.sendFloatControlValue(baseUri)
	data.inactiveFrrsUpCmd.sendFloatControlValue(baseUri)
	data.inactiveFrrsDownCmd.sendFloatControlValue(baseUri)
	data.responseEnableBitField.sendControlValue(baseUri)
	data.frrsUpCmd.sendControlValue(baseUri)
	data.frrsDownCmd.sendControlValue(baseUri)
	data.rrsFfrCmd.sendControlValue(baseUri)
	data.autoPfrUpCmd.sendControlValue(baseUri)
	data.autoPfrDownCmd.sendControlValue(baseUri)
}

type gridVoltageData struct {
	line1ToLine2   scadapoint
	line2ToLine3   scadapoint
	line3ToLine1   scadapoint
	avgLineVoltage float64
}

func newGridVoltageData() gridVoltageData {
	return gridVoltageData{
		line1ToLine2: newFloatStatus("grid_voltage_l1_l2"),
		line2ToLine3: newFloatStatus("grid_voltage_l2_l3"),
		line3ToLine1: newFloatStatus("grid_voltage_l3_l1"),
	}
}

func (data *gridVoltageData) addToObject(obj map[string]interface{}) {
	data.line1ToLine2.addToObject(obj)
	data.line2ToLine3.addToObject(obj)
	data.line3ToLine1.addToObject(obj)
	obj["average_voltage"] = data.avgLineVoltage
}

func (data *gridVoltageData) handleClientPub(m map[string]interface{}, siteId string) {
	data.line1ToLine2.optionalParse(m, siteId)
	data.line2ToLine3.optionalParse(m, siteId)
	data.line3ToLine1.optionalParse(m, siteId)
}

func (data *gridVoltageData) calculateAvgVoltage() {
	//SOC & Voltages
	L1L2 := data.line1ToLine2.status.(float64)
	L2L3 := data.line2ToLine3.status.(float64)
	L3L1 := data.line3ToLine1.status.(float64)
	data.avgLineVoltage = ((L1L2 + L2L3 + L3L1) / 3) * .001
}

type essData struct {
	numAvailable       scadapoint
	averageSoc         scadapoint
	chargeablePower    scadapoint
	dischargeablePower scadapoint
}

func newEssData() essData {
	return essData{
		numAvailable:       newIntStatus("num_ess_available"),
		averageSoc:         newFloatStatus("ess_average_soc"),
		chargeablePower:    newFloatStatus("ess_chargeable_power"),
		dischargeablePower: newFloatStatus("ess_dischargeable_power"),
	}
}

func (data *essData) addToObject(obj map[string]interface{}) {
	data.numAvailable.addToObject(obj)
	data.averageSoc.addToObject(obj)
	data.chargeablePower.addToObject(obj)
	data.dischargeablePower.addToObject(obj)
}

func (data *essData) handleClientPub(m map[string]interface{}, siteId string) {
	data.numAvailable.optionalParse(m, siteId)
	data.averageSoc.optionalParse(m, siteId)
	data.chargeablePower.optionalParse(m, siteId)
	data.dischargeablePower.optionalParse(m, siteId)
}
