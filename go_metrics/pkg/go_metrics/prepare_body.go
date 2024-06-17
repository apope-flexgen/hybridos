package go_metrics

import (
	"crypto/md5"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/google/uuid"
)

// Adds the alerting attributes to the fims object if the variable is an alert.
// The attributes added will be the active/inactive status and a list of timestamps in the form of a
// details array that indicates when the alert was triggered.
// Uses the outputScopeMutex to read the values of these attributes
// outputVar: the current output variable being sent out on fims
// clothedOutputVal: the current clothed fims object that has been constructed
// return if the variable is an alert and any errors that occurred
func addAlertingAttributesToOutput(outputVar string, clothedOutputVal *map[string]interface{}, is_get bool) (bool, error) {
	if !IsAlertingInstance {
		return false, nil
	}

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
		outputScopeMutex.RUnlock()
		return true, fmt.Errorf("alert does not have a status attribute. There is an error in the code")
	} else {
		if len(alertStatus) != 1 || alertStatus[0].tag != BOOL {
			outputScopeMutex.RUnlock()
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
	valueChangedList, valueChangedOk := OutputScope[outputVar+"@alertAttributeChanged"]
	if !messagesOk || !timestampsOk || !valueChangedOk || len(messagesList) != len(timestampsList) || len(messagesList) != len(valueChangedList) {
		outputScopeMutex.RUnlock()
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

func prepareSingleOutputVar(outputUri, outputVar string, naked, clothed, checkFormat, interval_set, direct_set, direct_post, is_get bool) (map[string]interface{}, map[string]interface{}, map[string]interface{}) {
	msgBody := make(map[string]interface{}, 0)
	tempDirectMsgBody := make(map[string]interface{}, 0)
	tempPubMsgBody := make(map[string]interface{}, 0)
	if !uriIsSparse[outputUri] || (uriIsSparse[outputUri] && outputVarChanged[outputVar]) || is_get {
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
				// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
				if stringInSlice(output.Flags, "flat") && stringInSlice(output.Flags, "lonely") {
					clothedOutputVal["name"] = output.Name
					msgBody = clothedOutputVal
				} else {
					// Otherwise the other fields will be nested under the name, which will be used as the key of the object
					msgBody[output.Name] = clothedOutputVal
				}
			} else {
				err := castUnionType(&outputVals[0], INT)
				if err == nil {
					enumIndex, okInt := getValueFromUnion(&outputVals[0]).(int64)
					if okInt {
						if pos, ok := output.EnumMap[int(enumIndex)]; ok {
							msgBody[output.Name] = []EnumObject{output.Enum[pos]}
						} else {
							msgBody[output.Name] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						msgBody[output.Name] = []EnumObject{{Value: enumIndex, String: "Unknown"}}
					}
				} else {
					msgBody[output.Name] = []EnumObject{{Value: -1, String: "Unknown"}}
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
				// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
				if stringInSlice(output.Flags, "flat") && stringInSlice(output.Flags, "lonely") {
					clothedOutputVal["name"] = output.Name
					msgBody = clothedOutputVal
				} else {
					// Otherwise the other fields will be nested under the name, which will be used as the key of the object
					msgBody[output.Name] = clothedOutputVal
				}
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
				msgBody[output.Name] = outputList
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
					// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
					if stringInSlice(output.Flags, "flat") && stringInSlice(output.Flags, "lonely") {
						clothedOutputVal["name"] = output.Name
						msgBody = clothedOutputVal
					} else {
						// Otherwise the other fields will be nested under the name, which will be used as the key of the object
						msgBody[output.Name] = clothedOutputVal
					}
				} else {
					msgBody[output.Name] = outputVal
				}
			}
		}

		if stringInSlice(output.Flags, "generate_uuid") {
			// Add a unique id to the output. This unique id is generated based on the hash of the current name, and will always be consistent
			// It's used primarily for templated outputs to ensure each of them has a unique id of a known format of uuid4
			uuid4 := uuid.NewHash(md5.New(), [16]byte{}, []byte(output.Name), 4)
			clothedOutputVal["uuid"] = uuid4.String()
		}

		// Add the alerting attributes to the fims message if this is an alert
		if isAlert, err := addAlertingAttributesToOutput(outputVar, &clothedOutputVal, is_get); err != nil {
			log.Errorf("Error adding alerting attributes to output variable %s: %v.", outputVar, err)
		} else if isAlert {
			// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
			if stringInSlice(output.Flags, "flat") && stringInSlice(output.Flags, "lonely") {
				clothedOutputVal["name"] = output.Name
				msgBody = clothedOutputVal
			} else {
				// Otherwise the other fields will be nested under the name, which will be used as the key of the object
				msgBody[output.Name] = clothedOutputVal
			}
		}
		if interval_set || direct_set || direct_post {
			// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
			if stringInSlice(output.Flags, "flat") && stringInSlice(output.Flags, "lonely") {
				tempDirectMsgBody[output.Name] = msgBody
			} else {
				// Otherwise the other fields will be nested under the name, which will be used as the key of the object
				tempDirectMsgBody[output.Name] = msgBody[output.Name]
			}
		} else {
			// If the output has the "flat" and "lonely" flags, format its output with the name at the same level as the other fields
			if stringInSlice(output.Flags, "flat") {
				tempPubMsgBody[output.Name] = msgBody
			} else {
				// Otherwise the other fields will be nested under the name, which will be used as the key of the object
				tempPubMsgBody[output.Name] = msgBody[output.Name]
			}
		}
	}
	return msgBody, tempDirectMsgBody, tempPubMsgBody
}

func PrepareBody(outputUri string) map[string]interface{} {
	msgBody := make(map[string]interface{}, len(PublishUris[outputUri]))
	directMsgBody = make(map[string]interface{}, 0)
	pubMsgBody = make(map[string]interface{}, 0)
	var tempMsgBody map[string]interface{}
	var tmpDirectMsgBody map[string]interface{}
	var tempPubMsgBody map[string]interface{}
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
		tempMsgBody, tmpDirectMsgBody, tempPubMsgBody = prepareSingleOutputVar(outputUri, outputVar, naked, clothed, checkFormat, interval_set, direct_set, direct_post, false)
		output, ok := MetricsConfig.Outputs[outputVar]
		if ok {
			msgBody[output.Name] = tempMsgBody[output.Name]
			if interval_set || direct_set || direct_post {
				directMsgBody[output.Name] = tmpDirectMsgBody[output.Name]
			} else {
				pubMsgBody[output.Name] = tempPubMsgBody[output.Name]
			}
		}
	}

	if msgBody == nil {
		// Make sure we return something that's valid json if the body is nil
		return map[string]interface{}{}
	}
	return msgBody
}
