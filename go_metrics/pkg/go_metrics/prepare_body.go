package go_metrics

func PrepareBody(outputUri string) map[string]interface{} {
	msgBody := make(map[string]interface{}, len(PublishUris[outputUri]))
	setMsgBody = make(map[string]interface{}, 0)
	pubMsgBody = make(map[string]interface{}, 0)
	clothed := false
	naked := false
	checkFormat := false
	interval_set := uriIsSet[outputUri]
	_, direct_set := uriToDirectSetActive[outputUri]

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
			if interval_set || direct_set {
				setMsgBody[outputVar] = msgBody[outputVar]
			} else {
				pubMsgBody[outputVar] = msgBody[outputVar]
			}
		}
	}
	return msgBody
}
