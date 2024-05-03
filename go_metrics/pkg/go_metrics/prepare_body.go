package go_metrics

import (
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Adds the alerting attributes to the fims object if the variable is an alert.
// The attributes added will be the active/inactive status and a list of timestamps in the form of a
// details array that indicates when the alert was triggered.
// Uses the outputScopeMutex to read the values of these attributes
// outputVar: the current output variable being sent out on fims
// clothedOutputVal: the current clothed fims object that has been constructed
// return if the variable is an alert and any errors that occurred
func addAlertingAttributesToOutput(outputVar string, clothedOutputVal *map[string]interface{}) (bool, error) {
	// Check if the name can be mapped to a metrics expression and the expression is an alert
	metricsObjects, ok := outputToMetricsObject[outputVar]
	if !ok {
		return false, nil
	}

	isAlert := false
	// Check all the expressions associated with the output (typically just 1)
	for _, metricObject := range metricsObjects {
		if metricObject.Alert {
			isAlert = true
			break
		}
	}
	if !isAlert {
		return false, nil
	}

	outputScopeMutex.RLock()
	alertStatus, ok := OutputScope[outputVar+"@alertStatus"]
	if !ok {
		return true, fmt.Errorf("alert does not have a status attribute. There is an error in the code")
	} else {
		if len(alertStatus) != 1 || alertStatus[0].tag != BOOL {
			return true, fmt.Errorf("invalid number of alert status values. Expected 1 but got %d. There is an error in the code", len(alertStatus))
		}
		// Update the alert status based on the value of the metric
		if alertStatus[0].b {
			(*clothedOutputVal)["status"] = "active"
		} else {
			(*clothedOutputVal)["status"] = "inactive"
		}
	}

	// messages and timestamps are separated but stored at the same index. Make sure both lists match up
	// An empty list is acceptable and should be sent out if no message/timestamp pairs exist
	messagesList, messagesOk := OutputScope[outputVar+"@activeMessages"]
	timestampsList, timestampsOk := OutputScope[outputVar+"@activeTimestamps"]
	if !messagesOk || !timestampsOk || len(messagesList) != len(timestampsList) {
		return true, fmt.Errorf("issue with alert details list. There is an error in the code")
	} else {
		// Create an array of all the trigger times for the alert.
		// This tracks separate timestamps for OR'd conditions so we know
		// when each individual subcondition became true.
		// It is not a history of the alert
		var details []map[string]interface{}
		details = make([]map[string]interface{}, 0)
		for index, activeMessage := range messagesList {
			details = append(details, map[string]interface{}{"message": activeMessage.s, "timestamp": timestampsList[index].s})
		}
		// We still want to send an empty array if no entries are present
		(*clothedOutputVal)["details"] = details
	}
	outputScopeMutex.RUnlock()

	return true, nil
}

func PrepareBody(outputUri string) map[string]interface{} {
	msgBody := make(map[string]interface{}, len(PublishUris[outputUri]))
	directMsgBody = make(map[string]interface{}, 0)
	pubMsgBody = make(map[string]interface{}, 0)
	clothed := false
	naked := false
	checkFormat := false
	interval_set := uriIsIntervalSet[outputUri]
	_, direct_set := uriIsDirect["set"][outputUri]
	_, direct_post := uriIsDirect["post"][outputUri]

	if stringInSlice(PubUriFlags[outputUri], "clothed") {
		clothed = true
	}
	if stringInSlice(PubUriFlags[outputUri], "naked") {
		naked = true
	}

	if naked && clothed {
		checkFormat = true
	}
	for _, outputVar := range PublishUris[outputUri] {
		if !uriIsSparse[outputUri] || (uriIsSparse[outputUri] && outputVarChanged[outputVar]) {
			output := MetricsConfig.Outputs[outputVar]
			clothedOutputVal := make(map[string]interface{}, len(output.Attributes))
			var outputVal interface{}
			outputScopeMutex.RLock()
			outputVarChanged[outputVar] = false
			outputVals := make([]Union, len(OutputScope[outputVar]))
			copy(outputVals, OutputScope[outputVar])
			outputScopeMutex.RUnlock()
			if len(outputVals) >= 1 {
				castUnionType(&outputVals[0], output.Value.tag)
				outputVal = getValueFromUnion(&outputVals[0])
			} else {
				outputVals = []Union{{tag: NIL}}
				castUnionType(&outputVals[0], output.Value.tag)
				outputVal = getValueFromUnion(&outputVals[0])
			}

			if checkFormat {
				if stringInSlice(output.Flags, "clothed") {
					clothed = true
					naked = false
				} else if stringInSlice(output.Flags, "naked") {
					clothed = false
					naked = true
				}
			}
			if len(output.Name) > 0 {
				outputVar = output.Name
			}
			if len(output.Enum) > 0 {
				if clothed {
					err := castUnionType(&outputVals[0], INT)
					if err == nil {
						enumIndex, okInt := getValueFromUnion(&outputVals[0]).(int64)
						if okInt {
							if pos, ok := output.EnumMap[int(enumIndex)]; ok {
								clothedOutputVal["value"] = []EnumObject{output.Enum[pos]}
							} else {
								clothedOutputVal["value"] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							clothedOutputVal["value"] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						clothedOutputVal["value"] = []EnumObject{{Value: -1, String: "Unknown"}}
					}
					outputScopeMutex.RLock()
					for attributeName := range output.Attributes {
						outputVals := []Union{}
						if _, ok := OutputScope[outputVar+"@"+attributeName]; ok {
							outputVals = make([]Union, len(OutputScope[outputVar+"@"+attributeName]))
							copy(outputVals, OutputScope[outputVar+"@"+attributeName])
						}

						if len(outputVals) >= 1 {
							clothedOutputVal[attributeName] = getValueFromUnion(&outputVals[0])
						} else {
							clothedOutputVal[attributeName] = output.Attributes[attributeName]
						}
					}
					outputScopeMutex.RUnlock()
					msgBody[outputVar] = clothedOutputVal
				} else {
					err := castUnionType(&outputVals[0], INT)
					if err == nil {
						enumIndex, okInt := getValueFromUnion(&outputVals[0]).(int64)
						if okInt {
							if pos, ok := output.EnumMap[int(enumIndex)]; ok {
								msgBody[outputVar] = []EnumObject{output.Enum[pos]}
							} else {
								msgBody[outputVar] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							msgBody[outputVar] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						msgBody[outputVar] = []EnumObject{{Value: -1, String: "Unknown"}}
					}
				}
			} else if len(output.Bitfield) > 0 {
				if clothed {
					outputList := make([]EnumObject, 0)
					err := castUnionType(&outputVals[0], INT)
					if err == nil {
						enumIndex, okInt := getValueFromUnion(&outputVals[0]).(int64)
						if okInt {
							for position := 0; position < len(output.Bitfield); position += 1 {
								if (enumIndex & (1 << position)) != 0 {
									outputList = append(outputList, output.Bitfield[position])
								}
							}
						}
					}
					outputScopeMutex.RLock()
					for attributeName := range output.Attributes {
						outputVals := []Union{}
						if _, ok := OutputScope[outputVar+"@"+attributeName]; ok {
							outputVals = make([]Union, len(OutputScope[outputVar+"@"+attributeName]))
							copy(outputVals, OutputScope[outputVar+"@"+attributeName])
						}

						if len(outputVals) >= 1 {
							clothedOutputVal[attributeName] = getValueFromUnion(&outputVals[0])
						} else {
							clothedOutputVal[attributeName] = output.Attributes[attributeName]
						}
					}
					outputScopeMutex.RUnlock()
					clothedOutputVal["value"] = outputList
					msgBody[outputVar] = clothedOutputVal
				} else {
					outputList := make([]EnumObject, 0)
					err := castUnionType(&outputVals[0], INT)
					if err == nil {
						enumIndex, okInt := getValueFromUnion(&outputVals[0]).(int64)
						if okInt {
							for position := 0; position < len(output.Bitfield); position += 1 {
								if (enumIndex & (1 << position)) != 0 {
									outputList = append(outputList, output.Bitfield[position])
								}
							}
						}
					}
					msgBody[outputVar] = outputList
				}
			} else {
				if outputVal != nil {
					if clothed {
						clothedOutputVal["value"] = outputVal
						outputScopeMutex.RLock()
						for attributeName := range output.Attributes {
							outputVals := []Union{}
							if _, ok := OutputScope[outputVar+"@"+attributeName]; ok {
								outputVals = make([]Union, len(OutputScope[outputVar+"@"+attributeName]))
								copy(outputVals, OutputScope[outputVar+"@"+attributeName])
							}

							if len(outputVals) >= 1 {
								clothedOutputVal[attributeName] = getValueFromUnion(&outputVals[0])
							} else {
								clothedOutputVal[attributeName] = output.Attributes[attributeName]
							}
						}
						outputScopeMutex.RUnlock()
						msgBody[outputVar] = clothedOutputVal
					} else {
						msgBody[outputVar] = outputVal
					}
				}
			}

			// Add the alerting attributes to the fims message if this is an alert
			if isAlert, err := addAlertingAttributesToOutput(outputVar, &clothedOutputVal); err != nil {
				log.Errorf("Error adding alerting attributes to output variable %s: %v.", outputVar, err)
			} else if isAlert {
				msgBody[outputVar] = clothedOutputVal
			}

			if interval_set || direct_set || direct_post {
				directMsgBody[outputVar] = msgBody[outputVar]
			} else {
				pubMsgBody[outputVar] = msgBody[outputVar]
			}
		}
	}
	return msgBody
}
