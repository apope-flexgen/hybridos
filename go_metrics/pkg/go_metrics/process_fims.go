package go_metrics

import (
	"encoding/json"
	"fims"
	"fmt"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
	simdjson "github.com/minio/simdjson-go"
)

func ProcessFims(msg fims.FimsMsgRaw) {
	processFimsTiming.start()
	if len(msg.Uri) >= len(ProcessName)+1 && msg.Uri[0:len(ProcessName)+1] == "/"+ProcessName {
		msg.Uri = msg.Uri[len(ProcessName)+1:]
	}
	if msg.Method == "pub" || msg.Method == "set" {
		//check to see if the value is something we care about
		_, isEchoOutputRegister := echoOutputToInputNum[msg.Uri]
		_, isEchoPublishUri := echoPublishUristoEchoNum[msg.Uri]
		if !isEchoOutputRegister {
			_, isEchoOutputRegister = echoPublishUristoEchoNum[GetParentUri(msg.Uri)]
		}
		if _, ok := UriElements[msg.Uri]; ok || isEchoOutputRegister || isEchoPublishUri {
			// if so, unmarshal the message body
			var err error
			pj, err = simdjson.Parse(msg.Body, pj)
			if err != nil { // could be a single naked item, or it could be invalid json
				if fmt.Sprintf("%s", err) == "Failed to find all structural indices for stage 1" {
					handleNakedMessage(&msg)
				} else {
					log.Warnf("problem with parsing message body into JSON: %v", err)
					return
				}
			} else {
				// extract the necessary values from the message body
				iter = pj.Iter()
				handleJsonMessage(&msg)
			}

			//This set reply may be overly broad since while the set will be for
			// a URI that metrics cares about (from the subscribes), it may be for a value
			// that metrics is not responsible for (BUT THIS IS WHAT METRICS DOES SO THERE)
			if msg.Method == "set" && len(msg.Replyto) > 0 {
				msgBodyInMutex.Lock()
				json.Unmarshal(msg.Body, &msgBodyIn)
				f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBodyIn})
				msgBodyInMutex.Unlock()
			}
		}
	} else if msg.Method == "get" && len(msg.Replyto) > 0 {
		if _, no_response := noGetResponse[msg.Uri]; no_response {
			return
		}
		// find metric in outputs and set back out
		outputsMutex.RLock()
		_, last_uri_element_is_output := MetricsConfig.Outputs[msg.Frags[len(msg.Frags)-1]]
		outputsMutex.RUnlock()
		inputsMutex.RLock()
		_, last_urilast_uri_element_is_input := MetricsConfig.Inputs[msg.Frags[len(msg.Frags)-1]]
		inputsMutex.RUnlock()
		if outputVar, ok := uriToOutputNameMap[msg.Uri]; ok { // asking for a single output value
			inputsMutex.Lock()
			EvaluateExpressions()
			inputsMutex.Unlock()
			outputsMutex.Lock()
			output = MetricsConfig.Outputs[outputVar]
			val := getValueFromUnion(&(output.Value))
			outputsMutex.Unlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{outputVar: val}})
		} else if outputVars, ok := PublishUris[msg.Uri]; ok { // asking for data from a single publish URI
			inputsMutex.Lock()
			EvaluateExpressions()
			inputsMutex.Unlock()
			msgBodyMutex.Lock()
			pubDataChangedMutex.Lock()
			pubDataChanged[msg.Uri] = true
			for _, outputVar := range outputVars {
				outputVarChanged[outputVar] = true
			}
			PrepareBody(msg.Uri)
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			pubDataChangedMutex.Unlock()
			msgBodyMutex.Unlock()
		} else if _, ok := OutputUriElements[msg.Uri]; ok { // asking for data from a set of publish URIs
			inputsMutex.Lock()
			EvaluateExpressions()
			inputsMutex.Unlock()
			msgBodyMutex.Lock()
			msgBody = GetOutputMsgBody(msg.Uri).(map[string]interface{})
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			msgBodyMutex.Unlock()
		} else if last_urilast_uri_element_is_input && len(msg.Frags) > 1 && msg.Frags[1] == "inputs" { // asking for a specific input
			inputsMutex.RLock()
			input = MetricsConfig.Inputs[msg.Frags[len(msg.Frags)-1]]
			val := getValueFromUnion(&(input.Value))
			inputsMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{input.Name: val}})
		} else if len(msg.Frags) > 1 && msg.Frags[1] == "inputs" { // asking for all inputs
			msgBodyMutex.Lock()
			msgBody = make(map[string]interface{}, len(MetricsConfig.Inputs))
			inputsMutex.RLock()
			for key, input := range MetricsConfig.Inputs {
				val := getValueFromUnion(&(input.Value))
				msgBody[key] = val
			}
			inputsMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			msgBodyMutex.Unlock()
		} else if last_uri_element_is_output && len(msg.Frags) > 1 && msg.Frags[1] == "outputs" { // asking for a specific output (in a different way)
			outputsMutex.RLock()
			output = MetricsConfig.Outputs[msg.Frags[len(msg.Frags)-1]]
			val := getValueFromUnion(&(output.Value))
			outputsMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{msg.Frags[len(msg.Frags)-1]: val}})
		} else if len(msg.Frags) > 1 && msg.Frags[1] == "outputs" { // asking for all outputs
			msgBodyMutex.Lock()
			msgBody = make(map[string]interface{}, len(MetricsConfig.Outputs))
			outputsMutex.RLock()
			for key, output := range MetricsConfig.Outputs {
				val := getValueFromUnion(&(output.Value))
				msgBody[key] = val
			}
			outputsMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			msgBodyMutex.Unlock()
		} else if len(msg.Frags) >= 1 && msg.Frags[0] == ProcessName && msg.Frags[1] == "timings" {
			msgBodyMutex.Lock()
			msgBody = make(map[string]interface{}, 10)
			processFimsTiming.calculateAverage()
			evalExpressionsTiming.calculateAverage()
			processFimsTiming.mutex.RLock()
			msgBody["fims_message_process_time_total"] = fmt.Sprintf("%fs", float64(processFimsTiming.total)/1000000000.0)
			msgBody["fims_message_process_count"] = processFimsTiming.count
			msgBody["fims_message_process_time_average"] = fmt.Sprintf("%.3fus", float64(processFimsTiming.average)/1000)
			msgBody["fims_message_process_time_min"] = fmt.Sprintf("%.3fus", float64(processFimsTiming.min)/1000)
			msgBody["fims_message_process_time_max"] = fmt.Sprintf("%.3fus", float64(processFimsTiming.max)/1000)
			processFimsTiming.mutex.RUnlock()
			evalExpressionsTiming.mutex.RLock()
			msgBody["eval_expressions_time_total"] = fmt.Sprintf("%fs", float64(evalExpressionsTiming.total)/1000000000.0)
			msgBody["eval_expressions_func_calls"] = evalExpressionsTiming.count
			msgBody["eval_expressions_time_average"] = fmt.Sprintf("%.3fus", float64(evalExpressionsTiming.average)/1000)
			msgBody["eval_expressions_time_min"] = fmt.Sprintf("%.3fus", float64(evalExpressionsTiming.min)/1000)
			msgBody["eval_expressions_time_max"] = fmt.Sprintf("%.3fus", float64(evalExpressionsTiming.max)/1000)
			evalExpressionsTiming.mutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			msgBodyMutex.Unlock()
		} else {
			echoMutex.RLock()
			for echoIndex, _ := range MetricsConfig.Echo {
				if msg.Uri == MetricsConfig.Echo[echoIndex].PublishUri {
					f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: MetricsConfig.Echo[echoIndex].Echo})
				} else if echoValue, isVal := MetricsConfig.Echo[echoIndex].Echo[msg.Frags[len(msg.Frags)-1]]; GetParentUri(msg.Uri) == MetricsConfig.Echo[echoIndex].PublishUri && isVal {
					f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: echoValue})
				}
			}
			echoMutex.RUnlock()
		}
	}
	// } else if msg.Method == "del" {
	// 	if outputVar, ok := uriToOutputNameMap[msg.Uri]; ok {
	// 		initialVal := initialValues[outputVar]
	// 		output := MetricsConfig.Outputs[outputVar]
	// 		output.Value = getUnionFromValue(&initialVal)
	// 		MetricsConfig.Outputs[outputVar] = output
	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{GetUriElement(msg.Uri): initialVal}})
	// 	} else if _, ok := PublishUris[msg.Uri]; ok {
	// 		msgBody := resetBody(msg.Uri)
	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
	// 	}
	// }
	processFimsTiming.stop()
}

// naked messages with a single value can be:
//      - naked input values (for echo or metrics)              e.g. "/components/bms_1/max_current": 3456
//      - and/or naked attribute values (for metrics)			e.g. "/components/bms_1/max_current/enabled": true   OR   "/components/bms_1/enabled": true
// note that multiple options are possible
func handleNakedMessage(msg *fims.FimsMsgRaw) {
	var err error
	// check if the uri has a corresponding echo input uri
	// (if it's naked, it must be a single variable)
	if echoObj, ok := uriToEchoObjectInputMap[GetParentUri((*msg).Uri)]; ok {

		for echoIndex, inputIndex := range echoObj {
			echoMutex.Lock()
			for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
				if oldName == (*msg).Frags[len((*msg).Frags)-1] {
					elementValueMutex.Lock()
					err = json.Unmarshal((*msg).Body, &elementValue) // decode the byte array into an interface{}; there are definitely other ways to do this, but it's probably the simplest way to do it
					if err == nil {
						MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue // set the echo value
					}
					elementValueMutex.Unlock()
					continue
				}
			}
			echoMutex.Unlock()
		}

	}

	// check if the uri has a corresponding echo output
	if (*msg).Method == "set" {
		echoMutex.Lock()
		if inputNum, ok := echoOutputToInputNum[(*msg).Uri]; ok {
			echoInput = MetricsConfig.Echo[echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]].Inputs[inputNum]
			echoInputRegisterName, ok := echoInput.Registers[(*msg).Frags[len((*msg).Frags)-1]] // this should always come out with something, but just in case...
			if ok {
				echoInputUri := echoInput.Uri + "/" + echoInputRegisterName
				elementValueMutex.Lock()
				err = json.Unmarshal((*msg).Body, &elementValue)
				if err == nil {
					f.Send(fims.FimsMsg{Method: "set", Uri: echoInputUri, Replyto: "", Body: elementValue})
				}
				elementValueMutex.Unlock()
			}
		} else if echoNum, ok := echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]; ok {
			_, ok := MetricsConfig.Echo[echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]].Echo[GetUriElement((*msg).Uri)]
			if ok {
				elementValueMutex.Lock()
				err = json.Unmarshal((*msg).Body, &elementValue)
				if err == nil {
					MetricsConfig.Echo[echoNum].Echo[GetUriElement((*msg).Uri)] = elementValue
				}
				elementValueMutex.Unlock()
			}
		}
		echoMutex.Unlock()
	}

	// check if the uri corresponds directly to an input
	// i.e.    /path/to/input/var corresponds directly to an input's uri
	// (if it's naked, it must be a single variable)
	if inputNames, ok := uriToInputNameMap[(*msg).Uri]; ok {
		elementValueMutex.Lock()
		err = json.Unmarshal((*msg).Body, &elementValue) // decode the byte array into an interface{}
		if err == nil {
			for i, _ := range inputNames {
				handleDecodedMetricsInputValue(inputNames[i])
			}
		}
		elementValueMutex.Unlock()
	}

	// check if the uri corresponds to an input's attribute
	// i.e. if /path/to/input/var corresponds directly to an input's uri
	// then /path/to/input/var/attribute is one possible path to the input's attribute
	if inputNames, ok := uriToInputNameMap[GetParentUri((*msg).Uri)]; ok {
		for i, _ := range inputNames {
			inputsMutex.RLock()
			input := MetricsConfig.Inputs[inputNames[i]]
			inputsMutex.RUnlock()
			attributeName := (*msg).Frags[len((*msg).Frags)-1]
			if len(input.Attributes) > 0 && stringInSlice(input.Attributes, attributeName) {
				elementValueMutex.Lock()
				err = json.Unmarshal((*msg).Body, &elementValue)
				if err == nil {
					handleDecodedMetricsAttributeValue(inputNames[i], input.AttributesMap[attributeName])
				}
				elementValueMutex.Unlock()
			}
		}
	} else if _, ok := UriElements[GetParentUri((*msg).Uri)]; ok { // /path/to/input/attribute is another possible path to the input's attribute
		inputsMutex.RLock()
		for inputName, input := range MetricsConfig.Inputs {
			if len(input.Attributes) > 0 && GetParentUri((*msg).Uri) == GetParentUri(input.Uri) {
				for _, attributeName := range input.Attributes {
					if attributeName == (*msg).Frags[len((*msg).Frags)-1] {
						elementValueMutex.Lock()
						err = json.Unmarshal((*msg).Body, &elementValue)
						if err == nil {
							handleDecodedMetricsAttributeValue(inputName, input.AttributesMap[attributeName])
						}
						elementValueMutex.Unlock()
					} else if strings.Contains((*msg).Frags[len((*msg).Frags)-1], "@") && inputName == strings.Split((*msg).Frags[len((*msg).Frags)-1], "@")[0] && attributeName == strings.Split((*msg).Frags[len((*msg).Frags)-1], "@")[1] {
						elementValueMutex.Lock()
						err = json.Unmarshal((*msg).Body, &elementValue)
						if err == nil {
							handleDecodedMetricsAttributeValue(inputName, input.AttributesMap[attributeName])
						}
						elementValueMutex.Unlock()
					}
				}
			}
		}
		inputsMutex.RUnlock()
	}
}

// messages in the form of a json object can be
//      - multi-valued naked messages (for echo or metrics)     e.g. "/components/bms_1":    {"max_current": 3456, "vnom": 78910, "active_power": 1234}
//      - single-valued clothed input WITHOUT the word "value"  e.g. "/components/bms_1":    {"max_current": 3456}
//         ^^ These two are treated the same way
//
//      - multi-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}, "vnom": {"value":3456....}, ...}
//      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}}
//         ^^ These two are treated the same way
//
//      - multi- or single- message containing input attributes e.g. "/components/bms_1/max_current":    {"enabled": true, "scale": 1000}
//      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1/max_current":    {"value": 3456, "enabled": true, "scale": 1000}
// note that multiple options are possible for a single message
func handleJsonMessage(msg *fims.FimsMsgRaw) {
	if listOfPaths, ok := UriElements[(*msg).Uri]; ok { // if we are expecting components from this Uri (could be echo input, metrics input, or metrics attribute)
		if len(listOfPaths) == 0 { // it must contain either ("value"), ("value" + attributes), or (attributes) --- because we already know that it's clothed
			element, err := iter.FindElement(nil, "value")
			if err == nil {
				// check if it's a clothed single echo input
				//      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1/max_current":    {"value": 3456, "enabled": true, "scale": 1000}
				if echoObj, ok_echo := uriToEchoObjectInputMap[GetParentUri((*msg).Uri)]; ok_echo {
					varName := GetUriElement((*msg).Uri)
					for echoIndex, inputIndex := range echoObj {
						echoMutex.Lock()
						for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
							if oldName == varName {
								elementValueMutex.Lock()
								if MetricsConfig.Echo[echoIndex].Format == "naked" {
									elementValue, _ = (element.Iter.Interface())
								} else if MetricsConfig.Echo[echoIndex].Format == "clothed" {
									elementValue, _ = (iter.Interface())
									if _, ok := elementValue.([]interface{}); ok && len(elementValue.([]interface{})) == 1 {
										elementValue = (elementValue.([]interface{}))[0]
									}
								} else {
									elementValue, _ = (iter.Interface())
									if _, ok := elementValue.([]interface{}); ok && len(elementValue.([]interface{})) == 1 {
										elementValue = (elementValue.([]interface{}))[0]
									}
								}
								MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue
								elementValueMutex.Unlock()
							}
						}
						echoMutex.Unlock()
					}
				}

				if inputNames, ok_metrics := uriToInputNameMap[(*msg).Uri]; ok_metrics {
					elementValueMutex.Lock()
					for i, _ := range inputNames {
						elementValue, err = element.Iter.Interface()
						if err == nil {
							handleDecodedMetricsInputValue(inputNames[i])
						}
						inputsMutex.RLock()
						input := MetricsConfig.Inputs[inputNames[i]]
						inputsMutex.RUnlock()
						if len(input.Attributes) > 0 {
							for _, attribute := range input.Attributes {
								element, err := iter.FindElement(nil, attribute)
								if err == nil {
									elementValue, _ = element.Iter.Interface()
									handleDecodedMetricsAttributeValue(inputNames[i], input.AttributesMap[attribute])
								}
							}
						}
					}
					elementValueMutex.Unlock()
				}
			} else { // it might contain only attributes
				inputsMutex.RLock()
				for inputName, input := range MetricsConfig.Inputs {
					// I don't actually think you can get here...
					if len(input.Attributes) > 0 && (*msg).Uri == GetParentUri(input.Uri) {
						for _, attribute := range input.Attributes {
							element, err := iter.FindElement(nil, attribute)
							if err == nil {
								elementValueMutex.Lock()
								elementValue, _ = element.Iter.Interface()
								handleDecodedMetricsAttributeValue(inputName, input.AttributesMap[attribute])
								elementValueMutex.Unlock()
							}
						}
					} else if len(input.Attributes) > 0 && (*msg).Uri == input.Uri {
						for _, attribute := range input.Attributes {
							element, err := iter.FindElement(nil, attribute)
							if err == nil {
								elementValueMutex.Lock()
								elementValue, _ = element.Iter.Interface()
								handleDecodedMetricsAttributeValue(inputName, input.AttributesMap[attribute])
								elementValueMutex.Unlock()
							}
						}
					}
				}
				inputsMutex.RUnlock()
			}
		} else {
			for _, json_path := range listOfPaths {
				if element, err := (&iter).FindElement(nil, json_path...); err == nil { // try to find the published value in (uri + json_path)
					fullUri := (*msg).Uri + "/" + strings.Join(json_path, "/")

					// Echo stuff
					////////////////////////////////////////////////////////////////////////
					// check if it's a naked (multi) echo input
					//      - multi-valued naked messages (for echo or metrics)     e.g. "/components/bms_1":    {"max_current": 3456, "vnom": 78910, "active_power": 1234}
					//      - single-valued clothed input WITHOUT the word "value"  e.g. "/components/bms_1":    {"max_current": 3456}
					if echoObj, ok := uriToEchoObjectInputMap[(*msg).Uri]; ok {
						for echoIndex, inputIndex := range echoObj {
							echoMutex.Lock()
							for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
								if oldName == json_path[len(json_path)-1] {
									elementValueMutex.Lock()
									elementValue, _ = (element.Iter.Interface())
									MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue
									elementValueMutex.Unlock()
									break
								}
							}
							echoMutex.Unlock()
						}
					}

					// check if it's a clothed (multi) echo input
					//      - multi-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}, "vnom": {"value":3456....}, ...}
					//      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}}
					if echoObj, ok := uriToEchoObjectInputMap[(*msg).Uri]; ok {
						for echoIndex, inputIndex := range echoObj {
							echoMutex.Lock()
							for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
								fullUri2 := fmt.Sprintf("%s/%s", GetParentUri(fullUri), oldName)
								if fullUri2 == fullUri {
									element2, err := element.Iter.FindElement(nil, "value")
									if err == nil {
										elementValueMutex.Lock()
										if MetricsConfig.Echo[echoIndex].Format == "naked" {
											elementValue, _ = (element2.Iter.Interface())
										} else if MetricsConfig.Echo[echoIndex].Format == "clothed" {
											elementValue, _ = (element.Iter.Interface())
										} else {
											elementValue, _ = (element.Iter.Interface())
										}
										MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue
										elementValueMutex.Unlock()
									}
									break
								}
							}
							echoMutex.Unlock()
						}
					}

					// I think this does the same thing as above...
					// // check if it's a clothed single echo input
					// //      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1/max_current":    {"value": 3456, "enabled": true, "scale": 1000}
					// for echoIndex, _ := range MetricsConfig.Echo {
					// 	for inputIndex, _ := range MetricsConfig.Echo[echoIndex].Inputs {
					// 		for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
					// 			fullUri2 := fmt.Sprintf("%s/%s", GetParentUri(fullUri), oldName)
					// 			if fullUri2 == fullUri {
					// 				element2, err := element.Iter.FindElement(nil, "value") // if we don't want to echo the entire body, we would want to change that here.... (currently just sends the entire body)
					// 				if err == nil {
					// 					if MetricsConfig.Echo[echoIndex].Format == "naked" {
					// 						elementValue, _ = (element2.Iter.Interface())
					// 					} else if MetricsConfig.Echo[echoIndex].Format == "clothed" {
					// 						elementValue, _ = (element.Iter.Interface())
					// 					} else {
					// 						elementValue, _ = (element.Iter.Interface())
					// 					}
					// 					MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue
					// 				}
					// 				break
					// 			}
					// 		}
					// 	}
					// }

					// Metrics stuff
					////////////////////////////////////////////////////////////////////////

					if inputNames, ok_input := uriToInputNameMap[fullUri]; ok_input { // uri + json_path = /path/to/input/var
						if element.Type == simdjson.TypeObject {
							//      - multi-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}, "vnom": {"value":3456....}, ...}
							//      - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}}
							for i, _ := range inputNames {
								if element2, err := element.Iter.FindElement(nil, "value"); err == nil {
									elementValueMutex.Lock()
									elementValue, _ = element2.Iter.Interface()
									handleDecodedMetricsInputValue(inputNames[i])
									elementValueMutex.Unlock()
								}

								inputsMutex.RLock()
								input := MetricsConfig.Inputs[inputNames[i]]
								inputsMutex.RUnlock()
								if len(input.Attributes) > 0 {
									for _, attributeName := range input.Attributes {
										if element2, err := element.Iter.FindElement(nil, attributeName); err == nil {
											elementValueMutex.Lock()
											elementValue, _ = element2.Iter.Interface()
											handleDecodedMetricsAttributeValue(inputNames[i], input.AttributesMap[attributeName])
											elementValueMutex.Unlock()
										}
									}
								}
							}
						} else {
							//      - multi-valued naked messages (for echo or metrics)     e.g. "/components/bms_1":    {"max_current": 3456, "vnom": 78910, "active_power": 1234}
							//      - single-valued clothed input WITHOUT the word "value"  e.g. "/components/bms_1":    {"max_current": 3456}
							for i, _ := range inputNames {
								elementValueMutex.Lock()
								elementValue, _ = element.Iter.Interface()
								handleDecodedMetricsInputValue(inputNames[i])
								elementValueMutex.Unlock()
								inputsMutex.RLock()
								input := MetricsConfig.Inputs[inputNames[i]]
								inputsMutex.RUnlock()
								if len(input.Attributes) > 0 && (*msg).Uri == GetParentUri(input.Uri) {
									for _, attribute := range input.Attributes {
										element, err := iter.FindElement(nil, attribute)
										if err == nil {
											elementValueMutex.Lock()
											elementValue, _ = element.Iter.Interface()
											handleDecodedMetricsAttributeValue(inputNames[i], input.AttributesMap[attribute])
											elementValueMutex.Unlock()
										}
									}
								}
							}

						}
					}
				} else {
					inputsMutex.RLock()
					for inputName, input := range MetricsConfig.Inputs {
						if len(input.Attributes) > 0 && (*msg).Uri == GetParentUri(input.Uri) {
							for _, attribute := range input.Attributes {
								element, err := iter.FindElement(nil, attribute)
								if err == nil {
									elementValueMutex.Lock()
									elementValue, _ = element.Iter.Interface()
									handleDecodedMetricsAttributeValue(inputName, input.AttributesMap[attribute])
									elementValueMutex.Unlock()
								}
							}
						}
					}
					inputsMutex.RUnlock()
				}
			}
		}
	}
	// or it could be a single echo output register or an echo publish URI
	//check if the uri has a corresponding echo output
	if (*msg).Method == "set" {
		echoMutex.RLock()
		if inputNum, ok := echoOutputToInputNum[(*msg).Uri]; ok {
			element, err := iter.FindElement(nil, "value")
			if err == nil {
				echoInput = MetricsConfig.Echo[echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]].Inputs[inputNum]
				echoInputRegisterName, ok := echoInput.Registers[(*msg).Frags[len((*msg).Frags)-1]] // this should always come out with something, but just in case...
				if ok {
					echoInputUri := echoInput.Uri + "/" + echoInputRegisterName
					elementValueMutex.Lock()
					elementValue, _ = element.Iter.Interface()
					f.Send(fims.FimsMsg{Method: "set", Uri: echoInputUri, Replyto: "", Body: elementValue})
					elementValueMutex.Unlock()
				}
			}
		} else if echoIndex, ok := echoPublishUristoEchoNum[(*msg).Uri]; ok {
			echoMap := make(map[string]interface{}, 0)
			sentAsSet := make(map[string]bool, 0)
			err := json.Unmarshal((*msg).Body, &echoMap)
			if err == nil {
				echoMsgBodyMutex.Lock()
				containsOneValue := false
				for inputIndex, _ := range MetricsConfig.Echo[echoIndex].Inputs {
					echoMsgBody = make(map[string]interface{}, 0)
					containsOneValue = false
					for newName, value := range echoMap {
						if oldName, ok := MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers[newName]; ok {
							echoMsgBody[oldName] = value
							containsOneValue = true
							sentAsSet[newName] = true
						}
					}
					if containsOneValue {
						f.Send(fims.FimsMsg{Method: "set", Uri: MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Uri, Replyto: "", Body: echoMsgBody})
					}
				}
				echoMsgBodyMutex.Unlock()
				for newName, value := range echoMap {
					_, isEchoRegister := MetricsConfig.Echo[echoIndex].Echo[newName]
					if isEchoRegister {
						if _, sent := sentAsSet[newName]; !sent {
							echoMutex.RUnlock()
							echoMutex.Lock()
							MetricsConfig.Echo[echoIndex].Echo[newName] = value
							echoMutex.Unlock()
							echoMutex.RLock()
						}
					}
				}
			}
		} else if echoNum, ok := echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]; ok {
			_, ok := MetricsConfig.Echo[echoPublishUristoEchoNum[GetParentUri((*msg).Uri)]].Echo[GetUriElement((*msg).Uri)]
			if ok {
				elementValueMutex.Lock()
				err := json.Unmarshal((*msg).Body, &elementValue)
				if err == nil {
					echoMutex.RUnlock()
					echoMutex.Lock()
					MetricsConfig.Echo[echoNum].Echo[GetUriElement((*msg).Uri)] = elementValue
					echoMutex.Unlock()
					echoMutex.RLock()
				}
				elementValueMutex.Unlock()
			}
		}
		echoMutex.RUnlock()
	}
}

// make it so that we recalculate all filters using the input
// and all expressions using the result of that filter
// AND all expressions using the input
func handleDecodedMetricsInputValue(inputName string) {
	inputsMutex.RLock()
	input := MetricsConfig.Inputs[inputName]
	inputsMutex.RUnlock()
	union = castValueToUnionType(elementValue, input.Value.tag)
	if debug {
		if stringInSlice(debug_inputs, inputName) {
			log.Debugf("Received input [%s] value [%v]", inputName, elementValue)
		}
	}
	if input.Value != union || containedInValChanged[inputName] || inputYieldsDirectSet[inputName] {
		input.Value = union
		for _, filterName := range inputToFilterExpression[input.Name] {
			filterNeedsEvalMutex.Lock()
			filterNeedsEval[filterName] = true
			filterNeedsEvalMutex.Unlock()
			for _, expNum := range inputToMetricsExpression[filterName] {
				expressionNeedsEvalMutex.Lock()
				expressionNeedsEval[expNum] = true
				expressionNeedsEvalMutex.Unlock()
			}
		}
		for _, expNum := range inputToMetricsExpression[input.Name] {
			expressionNeedsEvalMutex.Lock()
			expressionNeedsEval[expNum] = true
			expressionNeedsEvalMutex.Unlock()
		}
	}

	inputsMutex.Lock()
	MetricsConfig.Inputs[inputName] = input
	inputsMutex.Unlock()
	scopeMutex.Lock()
	Scope[inputName] = []Input{input}
	scopeMutex.Unlock()
	if containedInValChanged[inputName] || inputYieldsDirectSet[inputName] {
		inputsMutex.Lock()
		ProcessDirectSets()
		inputsMutex.Unlock()
	}
}

// make it so that we recalculate all filters using the attribute
// and all expressions using the result of that filter
func handleDecodedMetricsAttributeValue(inputName, scopeVar string) {
	attributeUnion = getUnionFromValue(elementValue)
	if debug {
		if stringInSlice(debug_inputs, inputName) {
			log.Debugf("Received input [%s] attribute [%s] value [%v]", inputName, scopeVar, elementValue)
		}
	}
	scopeMutex.Lock()
	if len(Scope[scopeVar]) == 0 || attributeUnion != Scope[scopeVar][0].Value {
		attributeObject, ok := MetricsConfig.Attributes[scopeVar]
		if ok {
			attributeObject.Value = attributeUnion
			MetricsConfig.Attributes[scopeVar] = attributeObject
		}
		Scope[scopeVar] = []Input{Input{Value: attributeUnion}}
		for _, filterName := range inputToFilterExpression[inputName] {
			filterNeedsEvalMutex.Lock()
			filterNeedsEval[filterName] = true
			filterNeedsEvalMutex.Unlock()
			for _, expNum := range inputToMetricsExpression[filterName] {
				expressionNeedsEvalMutex.Lock()
				expressionNeedsEval[expNum] = true
				expressionNeedsEvalMutex.Unlock()
			}
		}
		for _, filterName := range inputToFilterExpression[scopeVar] {
			filterNeedsEvalMutex.Lock()
			filterNeedsEval[filterName] = true
			filterNeedsEvalMutex.Unlock()
			for _, expNum := range inputToMetricsExpression[filterName] {
				expressionNeedsEvalMutex.Lock()
				expressionNeedsEval[expNum] = true
				expressionNeedsEvalMutex.Unlock()
			}
		}
		for _, expNum := range inputToMetricsExpression[scopeVar] {
			expressionNeedsEvalMutex.Lock()
			expressionNeedsEval[expNum] = true
			expressionNeedsEvalMutex.Unlock()
		}
	}
	scopeMutex.Unlock()
}

func PrepareBody(outputUri string) {
	msgBody = make(map[string]interface{}, len(PublishUris[outputUri]))
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
			outputsMutex.RLock()
			outputVarChanged[outputVar] = false
			output = MetricsConfig.Outputs[outputVar]
			outputsMutex.RUnlock()
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
					outputElementMutex.Lock()
					err := castUnionType(&output.Value, INT)
					if err == nil {
						outputElementValue = getValueFromUnion(&(output.Value))
						enumIndex, okInt := outputElementValue.(int64)
						if okInt {
							if pos, ok := output.EnumMap[int(enumIndex)]; ok {
								output.Attributes["value"] = []EnumObject{output.Enum[pos]}
							} else {
								output.Attributes["value"] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							output.Attributes["value"] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						output.Attributes["value"] = []EnumObject{EnumObject{Value: -1, String: "Unknown"}}
					}
					msgBody[outputVar] = output.Attributes
					outputElementMutex.Unlock()
				} else {
					outputElementMutex.Lock()
					err := castUnionType(&output.Value, INT)
					if err == nil {
						outputElementValue = getValueFromUnion(&(output.Value))

						enumIndex, okInt := outputElementValue.(int64)
						if okInt {
							if pos, ok := output.EnumMap[int(enumIndex)]; ok {
								msgBody[outputVar] = []EnumObject{output.Enum[pos]}
							} else {
								msgBody[outputVar] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							msgBody[outputVar] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						msgBody[outputVar] = []EnumObject{EnumObject{Value: -1, String: "Unknown"}}
					}
					outputElementMutex.Unlock()
				}
			} else if len(output.Bitfield) > 0 {
				if clothed {
					outputElementMutex.Lock()
					outputList := make([]EnumObject, 0)
					err := castUnionType(&output.Value, INT)
					if err == nil {
						outputElementValue = getValueFromUnion(&(output.Value))

						enumIndex, okInt := outputElementValue.(int64)
						if okInt {
							for position := 0; position < len(output.Bitfield); position += 1 {
								if (enumIndex & (1 << position)) != 0 {
									outputList = append(outputList, output.Bitfield[position])
								}
							}
						}
					}
					output.Attributes["value"] = outputList
					msgBody[outputVar] = output.Attributes
					outputElementMutex.Unlock()
				} else {
					outputElementMutex.Lock()
					outputList := make([]EnumObject, 0)
					err := castUnionType(&output.Value, INT)
					if err == nil {
						outputElementValue = getValueFromUnion(&(output.Value))
						enumIndex, okInt := outputElementValue.(int64)
						if okInt {
							for position := 0; position < len(output.Bitfield); position += 1 {
								if (enumIndex & (1 << position)) != 0 {
									outputList = append(outputList, output.Bitfield[position])
								}
							}
						}
					}
					msgBody[outputVar] = outputList
					outputElementMutex.Unlock()
				}
			} else {
				tempPrepareMsgBodyValueMutex.Lock()
				tempPrepareMsgBodyValue = getValueFromUnion(&(output.Value))
				if tempPrepareMsgBodyValue != nil {
					if clothed {
						output.Attributes["value"] = tempPrepareMsgBodyValue
						msgBody[outputVar] = output.Attributes
					} else {
						msgBody[outputVar] = tempPrepareMsgBodyValue
					}
				}
				tempPrepareMsgBodyValueMutex.Unlock()
			}
			if interval_set || direct_set {
				setMsgBody[outputVar] = msgBody[outputVar]
			} else {
				pubMsgBody[outputVar] = msgBody[outputVar]
			}
		}
	}
}

func GetOutputMsgBody(uri string) interface{} {
	if _, ok := OutputUriElements[uri]; ok {
		if outputVar, ok := uriToOutputNameMap[uri]; ok {
			outputsMutex.Lock()
			output = MetricsConfig.Outputs[outputVar]
			val := getValueFromUnion(&(output.Value))
			outputsMutex.Unlock()
			return val
		} else if len(OutputUriElements[uri]) == 0 { // then it's an output's attribute
			// currently unhandled
		} else {
			keys := make(map[string]int, 0)
			output := make(map[string]interface{}, 0)
			for _, elementsList := range OutputUriElements[uri] {
				if len(elementsList) > 0 { // it should be, but just in case it isn't...
					if _, ok = keys[elementsList[0]]; ok {
						keys[elementsList[0]] += 1
					} else {
						keys[elementsList[0]] = 1
					}
				}
			}
			for key, _ := range keys {
				output[key] = GetOutputMsgBody(uri + "/" + key)
			}
			return output
		}
	}
	return nil
}

func GetInputsMsgBody(uri string) interface{} {
	if _, ok := UriElements[uri]; ok {
		if inputVar, ok := uriToInputNameMap[uri]; ok {
			inputsMutex.Lock()
			input = MetricsConfig.Inputs[inputVar[0]] // if the list is more than 1 element, then the values should all be the same
			val := getValueFromUnion(&(input.Value))
			inputsMutex.Unlock()
			return val
		} else if len(UriElements[uri]) == 0 { // then it's an output's attribute
			// currently unhandled
		} else {
			keys := make(map[string]int, 0)
			output := make(map[string]interface{}, 0)
			for _, elementsList := range OutputUriElements[uri] {
				if len(elementsList) > 0 { // it should be, but just in case it isn't...
					if _, ok = keys[elementsList[0]]; ok {
						keys[elementsList[0]] += 1
					} else {
						keys[elementsList[0]] = 1
					}
				}
			}
			for key, _ := range keys {
				output[key] = GetOutputMsgBody(uri + "/" + key)
			}
			return output
		}
	}
	return nil
}
