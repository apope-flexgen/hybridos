package go_metrics

import (
	"encoding/json"
	"fims"
	"fmt"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	simdjson "github.com/minio/simdjson-go"
)

func ProcessFims(msg fims.FimsMsgRaw) {
	processFimsTiming.start()
	if len(msg.Frags) > 0 && msg.Frags[0] == ProcessName {
		msg.Uri = msg.Uri[len(ProcessName)+1:]
		msg.Frags = msg.Frags[1:]
	}
	if msg.Method == "set" || msg.Method == "pub" {
		if _, ok := UriElements[msg.Uri]; ok {
			// if so, unmarshal the message body
			var err error
			pj, err = simdjson.Parse(msg.Body, pj)
			if err != nil { // could be a single naked item, or it could be invalid json
				if fmt.Sprintf("%s", err) == "Failed to find all structural indices for stage 1" {
					elementValueMutex.Lock()
					json.Unmarshal(msg.Body, &elementValue)
					handleUriElement(&msg, msg.Uri, msg.Frags[len(msg.Frags)-1])
					elementValueMutex.Unlock()
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
		_, last_uri_element_is_output := MetricsConfig.Outputs[msg.Frags[len(msg.Frags)-1]]
		_, last_uri_element_is_input := MetricsConfig.Inputs[msg.Frags[len(msg.Frags)-1]]
		_, isEchoPublishUri := echoPublishUristoEchoNum[msg.Uri]
		if outputNames, ok := uriToOutputNameMap[msg.Uri]; ok && len(outputNames) > 0 { // asking for all outputs associated with the given URI
			anyOutputsClothed, outputVals := mapOutputNamesToMsgVars(outputNames)
			// Construct the message body to go out over fims
			// If multiple values are present they will be returned as an array
			var responseBody interface{}
			if anyOutputsClothed {
				if len(outputVals) == 1 {
					responseBody = map[string]interface{}{"value": outputVals[0]}
				} else {
					responseBody = map[string]interface{}{"value": outputVals}
				}
			} else {
				if len(outputVals) == 1 {
					responseBody = outputVals[0]
				} else {
					responseBody = outputVals
				}
			}
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: responseBody})
		} else if outputVars, ok := PublishUris[msg.Uri]; ok { // asking for data from a single publish URI
			pubDataChangedMutex.Lock()
			pubDataChanged[msg.Uri] = true
			for _, outputVar := range outputVars {
				outputVarChanged[outputVar] = true
			}
			msgBody := PrepareBody(msg.Uri)
			if isEchoPublishUri {
				echoMutex.RLock()
				for echoIndex := range MetricsConfig.Echo {
					if msg.Uri == MetricsConfig.Echo[echoIndex].PublishUri {
						for key, value := range MetricsConfig.Echo[echoIndex].Echo {
							msgBody[key] = value
						}
					}
				}
				echoMutex.RUnlock()
			}
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			pubDataChangedMutex.Unlock()
		} else if last_uri_element_is_input && len(msg.Frags) > 0 && msg.Frags[0] == "inputs" { // asking for a specific input
			inputScopeMutex.RLock()
			input := InputScope[msg.Frags[len(msg.Frags)-1]]
			var val interface{}
			if len(input) > 1 {
				val_list := make([]interface{}, len(input))
				for g, union := range input {
					val_list[g] = getValueFromUnion(&union)
				}
				val = val_list
			} else if len(input) == 1 {
				val = getValueFromUnion(&input[0])
			} else {
				val = 0
			}
			inputScopeMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{msg.Frags[len(msg.Frags)-1]: val}})
		} else if len(msg.Frags) > 0 && msg.Frags[0] == "inputs" { // asking for all inputs
			msgBody := make(map[string]interface{}, len(MetricsConfig.Inputs))
			inputScopeMutex.RLock()
			for key, input := range InputScope {
				var val interface{}
				if len(input) > 1 {
					val_list := make([]interface{}, len(input))
					for g, union := range input {
						val_list[g] = getValueFromUnion(&union)
					}
					val = val_list
				} else if len(input) == 1 {
					val = getValueFromUnion(&input[0])
				} else {
					val = 0
				}

				msgBody[key] = val
			}
			inputScopeMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
		} else if last_uri_element_is_output && len(msg.Frags) > 0 && msg.Frags[0] == "outputs" { // asking for all outputs associated with the given uri (in a different way)
			outputScopeMutex.Lock()
			outputVals := make(map[string]interface{}, 0)
			// Iterate over the values associated with the URI
			// Generally, only a single value will be present, but multiple are possible
			for _, outputVar := range outputNames {
				outputName := MetricsConfig.Outputs[outputVar].Name
				output := OutputScope[msg.Frags[len(msg.Frags)-1]]
				var val interface{}
				if len(output) > 1 {
					val_list := make([]interface{}, len(output))
					for g, union := range output {
						val_list[g] = getValueFromUnion(&union)
					}
					val = val_list
				} else if len(output) == 1 {
					val = getValueFromUnion(&output[0])
				} else {
					val = 0
				}
				// It's unclear whether the names will all be the same
				// Therefore, track each unique name and construct a list any time a second value for that name is found
				switch currentVal := outputVals[outputName].(type) {
				case []interface{}:
					outputVals[outputName] = append(outputVals[outputName].([]interface{}), currentVal)
				case interface{}:
					if currentVal != nil {
						// A value already exists. Construct an array containing the values
						outputVals[outputName] = make([]interface{}, 2)
						outputVals[outputName] = append(outputVals[outputName].([]interface{}), currentVal)
					}
					outputVals[outputName] = val
				default:
					log.Errorf("Encountered invalid type when constructing get response for %s. There is a problem with the code", msg.Uri)
				}
			}
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: outputVals})
		} else if len(msg.Frags) > 0 && msg.Frags[0] == "outputs" { // asking for all outputs
			msgBody := make(map[string]interface{}, len(OutputScope))
			outputScopeMutex.RLock()
			for key, output := range OutputScope {
				var val interface{}
				if len(output) > 1 {
					val_list := make([]interface{}, len(output))
					for g, union := range output {
						val_list[g] = getValueFromUnion(&union)
					}
					val = val_list
				} else if len(output) == 1 {
					val = getValueFromUnion(&output[0])
				} else {
					val = 0
				}

				msgBody[key] = val
			}
			outputScopeMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
		} else if len(msg.Frags) >= 1 && msg.Frags[0] == "timings" {
			msgBody := make(map[string]interface{}, 11)
			processFimsTiming.calculateAverage()
			evalExpressionsTiming.calculateAverage()
			processFimsTiming.mutex.RLock()
			msgBody["total_process_up_time"] = fmt.Sprintf("%fs", time.Since(t0).Seconds())
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
		} else {
			echoMutex.RLock()
			for echoIndex := range MetricsConfig.Echo {
				if msg.Uri == MetricsConfig.Echo[echoIndex].PublishUri {
					f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: MetricsConfig.Echo[echoIndex].Echo})
				} else if echoValue, isVal := MetricsConfig.Echo[echoIndex].Echo[msg.Frags[len(msg.Frags)-1]]; GetParentUri(msg.Uri) == MetricsConfig.Echo[echoIndex].PublishUri && isVal {
					if MetricsConfig.Echo[echoIndex].Format == "clothed" {
						f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{"value": echoValue}})
					} else {
						f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: echoValue})
					}
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

// messages in the form of a json object can be
//
//   - multi-valued naked messages (for echo or metrics)     e.g. "/components/bms_1":    {"max_current": 3456, "vnom": 78910, "active_power": 1234}
//
//   - single-valued clothed input WITHOUT the word "value"  e.g. "/components/bms_1":    {"max_current": 3456}
//     ^^ These two are treated the same way
//
//   - multi-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}, "vnom": {"value":3456....}, ...}
//
//   - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1":    {"max_current": {"value":3456, "enabled": true, "scale": 1000}}
//     ^^ These two are treated the same way
//
//   - multi- or single- message containing input attributes e.g. "/components/bms_1/max_current":    {"enabled": true, "scale": 1000}
//
//   - single-valued clothed input WITH the word "value"     e.g. "/components/bms_1/max_current":    {"value": 3456, "enabled": true, "scale": 1000}
//
// note that multiple options are possible for a single message
func handleJsonMessage(msg *fims.FimsMsgRaw) {
	if jsonPaths, ok := UriElements[(*msg).Uri]; ok {
		switch jsonPaths2 := jsonPaths.(type) {
		case map[string]interface{}:
			iter.Advance()
			findValuesInMap(msg, (*msg).Uri, jsonPaths2, &iter)
		case interface{}:
			elementValueMutex.Lock()
			elementValue, _ = iter.Interface()
			handleUriElement(msg, (*msg).Uri, (*msg).Frags[len((*msg).Frags)-1])
			elementValueMutex.Unlock()
		}
	}
}

// Get all the values associated with the list of input names.
// Will return the list of values as well as whether any were clothed.
// Uses the outputScopeMutex
// outputNames: the list of names to check
func mapOutputNamesToMsgVars(outputNames []string) (bool, []interface{}) {
	outputScopeMutex.Lock()
	// If there are multiple outputs and any of them are clothed, all of them must be clothed
	anyOutputsClothed := false
	outputVals := make([]interface{}, 0)
	// Iterate over the values associated with the URI
	// Generally, only a single value will be present, but multiple are possible
	for _, outputVar := range outputNames {
		output := OutputScope[outputVar]
		var val interface{}
		if len(output) > 1 {
			val_list := make([]interface{}, len(output))
			for g, union := range output {
				val_list[g] = getValueFromUnion(&union)
			}
			val = val_list
		} else if len(output) == 1 {
			val = getValueFromUnion(&output[0])
		} else {
			val = 0
		}
		if stringInSlice(MetricsConfig.Outputs[outputVar].Flags, "clothed") {
			anyOutputsClothed = true
		}
		outputVals = append(outputVals, val)
	}
	return anyOutputsClothed, outputVals
}

func findValuesInMap(msg *fims.FimsMsgRaw, currentUri string, valuesToLookFor map[string]interface{}, iter *simdjson.Iter) {
	fullUri := ""
	var err error
	typ := iter.Type()
	for {
		switch typ {
		case simdjson.TypeRoot:
			// iterMap is used to store the current simdjson.Iter object
			// We use a map so that we don't have to keep reallocating memory
			if typ, iterMap[currentUri], err = iter.Root(iterMap[currentUri]); err != nil {
				return
			}
		case simdjson.TypeObject:
			if _, ok := iterMap[currentUri]; ok {
				// objMap is used to store the current simdjson.Object object
				// We use a map so that we don't have to keep reallocating memory
				if objMap[currentUri], err = iterMap[currentUri].Object(objMap[currentUri]); err != nil {
					return
				}
			}
			// else { // I don't think this is necessary, but it was here just in case.
			// 	if objMap[currentUri], err = iter.Object(objMap[currentUri]); err != nil {
			// 		return
			// 	}
			// }

			// elemMap is used to store the current simdjson.Iter object for the element that we're on
			// We use a map so that we don't have to keep reallocating memory
			if _, ok := elemMap[currentUri]; !ok {
				elemMap[currentUri] = &simdjson.Iter{}
			}
			if _, ok := objMap[currentUri]; ok {
				name, typ, err := objMap[currentUri].NextElement(elemMap[currentUri])
				for typ != simdjson.TypeNone && err == nil {
					if val, ok := valuesToLookFor[name]; ok || name == "value" {
						if name == "value" {
							name = GetUriElement(currentUri)
							fullUri = currentUri
						} else {
							fullUri = currentUri + "/" + name
						}
						if typ == simdjson.TypeObject {
							value := val.(map[string]interface{})
							iterMap[fullUri] = elemMap[currentUri]
							findValuesInMap(msg, fullUri, value, elemMap[currentUri])
						} else {
							elementValueMutex.Lock()
							elementValue, _ = elemMap[currentUri].Interface()
							handleUriElement(msg, fullUri, name)
							elementValueMutex.Unlock()
						}
					} else if strings.EqualFold(name, "reevaluate") {
						// Reevaluation case: all output uris accept a command to reevaluate their metric
						elementValueMutex.Lock()
						elementValue, _ = elemMap[currentUri].Interface()
						handleUriElement(msg, fullUri, name)
						elementValueMutex.Unlock()
					}
					name, typ, err = objMap[currentUri].NextElement(elemMap[currentUri])
				}
				return
			}
		case simdjson.TypeNone:
			return
		}
		//typ = iter.Advance() // not sure if we can reach this line // need to test that
	}
}

// handle reevaluation signal that maps an output uri to a metric that needs reevaluation
// uri: the output uri that should be mapped to the metric
// return whether the metric was updated
func handleReevaluationSignal(uri string, elementName string) (bool, error) {
	// We got here with a different set somehow
	if elementName != "reevaluate" {
		return false, nil
	}

	// Check if the output resolves to the given name
	outputNames, uriToNameOk := uriToOutputNameMap[uri]
	if !uriToNameOk {
		return false, fmt.Errorf("failed to find an output associated with the given uri: %s", uri)
	}

	for _, outputName := range outputNames {
		// Check if the name can be mapped to a metrics expression
		metricsObjects, nameToMetricOk := outputToMetricsObject[outputName]
		if !nameToMetricOk {
			return false, fmt.Errorf("failed to find the metric associated with the given output: %s", outputName)
		}

		// If this is a valid endpoint, make sure the set object is in the right format
		// Handle either a naked or clothed object
		unwrappedVal := parsemap.UnwrapVariable(elementValue)
		val, ok := unwrappedVal.(bool)
		if !ok || !val {
			return false, fmt.Errorf("reevaluation signal must be true")
		}

		// The output should map to an expression, so ensure there's an associated expression number
		if len(metricsObjects) == 0 {
			return false, fmt.Errorf("failed to find the metric number associated with the given output: %s", outputName)
		}
		// Update all the expressions associated with the output (typically just 1)
		for expNum, object := range metricsObjects {
			// Also clear their output and state so they're evaluated as new
			warn, err := configureStateAndOutputs(object, expNum)
			if warn != "" {
				log.Warnf("Warning reconfiguring metric state and scope: %s", warn)
			}
			if err != nil {
				return false, fmt.Errorf("failed to reconfigure the metric's state and scope: %w", err)
			}
		}

		// Clear the output scope as well so the metric is reevaluated as new
		if _, ok = OutputScope[outputName]; !ok {
			return false, fmt.Errorf("could not clear the outputScope for output: %s", outputName)
		}
		outputScopeMutex.Lock()
		OutputScope[outputName] = nil
		outputScopeMutex.Unlock()
	}

	return true, nil
}

func handleUriElement(msg *fims.FimsMsgRaw, uri string, elementName string) {

	// handle inputs
	if inputNames, ok := uriToInputNameMap[uri]; ok {
		for _, inputName := range inputNames {
			if MetricsConfig.Inputs[inputName].Method == "" || MetricsConfig.Inputs[inputName].Method == "both" || msg.Method == MetricsConfig.Inputs[inputName].Method {
				handleDecodedMetricsInputValue(inputName)
			}
		}
	}

	// handle attributes at uris like /some/uri/input/attribute
	if inputsWithAttribute, ok := allPossibleAttributes[elementName]; ok {
		if inputNames, ok := uriToInputNameMap[GetParentUri(uri)]; ok {
			for _, inputName := range inputNames {
				if MetricsConfig.Inputs[inputName].Method == "" || MetricsConfig.Inputs[inputName].Method == "both" || msg.Method == MetricsConfig.Inputs[inputName].Method {
					if stringInSlice(inputsWithAttribute, inputName+"@"+elementName) {
						handleDecodedMetricsAttributeValue(inputName, inputName+"@"+elementName)
					}
				}
			}
		}
	}

	// handle attributes at uris like /some/uri/input@attribute
	if strings.Contains(uri, "@") {
		input := strings.Split(elementName, "@")[0]
		elementName2 := strings.Split(uri, "@")[1]
		if inputsWithAttribute, ok := allPossibleAttributes[elementName2]; ok {
			if inputNames, ok := uriToInputNameMap[GetParentUri(uri)+"/"+input]; ok {
				for _, inputName := range inputNames {
					if MetricsConfig.Inputs[inputName].Method == "" || MetricsConfig.Inputs[inputName].Method == "both" || msg.Method == MetricsConfig.Inputs[inputName].Method {
						if stringInSlice(inputsWithAttribute, inputName+"@"+elementName2) {
							handleDecodedMetricsAttributeValue(inputName, inputName+"@"+elementName2)
						}
					}
				}
			}
		}
	}

	// update echo registers
	if echoObj, ok := uriToEchoObjectInputMap[GetParentUri(uri)]; ok {
		for echoIndex, inputIndex := range echoObj {
			echoMutex.Lock()
			for newName, oldName := range MetricsConfig.Echo[echoIndex].Inputs[inputIndex].Registers {
				if oldName == elementName {
					MetricsConfig.Echo[echoIndex].Echo[newName] = elementValue
				}
			}
			echoMutex.Unlock()
		}
	}

	// handle sets
	if (*msg).Method == "set" {
		// Check for reevaluation signal
		if ok, err := handleReevaluationSignal(msg.Uri, elementName); err != nil {
			log.Errorf("Error reevaluating metric %s: %v", msg.Uri, err)
			return
		} else if ok {
			// Return early if the reevaluation signal was received
			return
		}

		// handle echo "set" forwarding
		// handle echo register "sets" that need to be forwarded to original inputs
		sentAsSet := false
		if inputNum, isEchoOutputRegister := echoOutputToInputNum[uri]; isEchoOutputRegister {
			echoMutex.Lock()
			echoInput = MetricsConfig.Echo[echoPublishUristoEchoNum[GetParentUri(uri)]].Inputs[inputNum]
			echoInputRegisterName, ok := echoInput.Registers[elementName]
			if ok {
				echoInputUri := echoInput.Uri + "/" + echoInputRegisterName
				f.Send(fims.FimsMsg{Method: "set", Uri: echoInputUri, Replyto: "", Body: elementValue})
				sentAsSet = true
			}
			echoMutex.Unlock()
		}

		// update static echo registers from "sets"
		if echoIndex, isEchoPublishUri := echoPublishUristoEchoNum[GetParentUri(uri)]; isEchoPublishUri {
			echoMutex.Lock()
			_, isEchoRegister := MetricsConfig.Echo[echoIndex].Echo[elementName]
			if isEchoRegister {
				if !sentAsSet {
					MetricsConfig.Echo[echoIndex].Echo[elementName] = elementValue
				}
			}
			echoMutex.Unlock()
		}
	}
}

// make it so that we recalculate all filters using the input
// and all expressions using the result of that filter
// AND all expressions using the input
func handleDecodedMetricsInputValue(inputName string) {
	inputScopeMutex.RLock()
	var union_array []Union
	switch elementValueCast := elementValue.(type) {
	case []interface{}:
		if MetricsConfig.Inputs[inputName].Type == "bitfield" {
			union_array = make([]Union, 2*len(elementValueCast))
		} else {
			union_array = make([]Union, len(elementValueCast))
		}
		for element_idx, elementValue := range elementValueCast {
			elementValue, ok := elementValue.(map[string]interface{})
			if ok {
				if MetricsConfig.Inputs[inputName].Type == "bitfield" {
					union_array[2*element_idx] = getUnionFromValue(elementValue["value"])
					union_array[2*element_idx+1] = getUnionFromValue(elementValue["string"])
				} else if MetricsConfig.Inputs[inputName].Type == "bitfield_int" {
					union_array[element_idx] = castValueToUnionType(elementValue["value"], UINT)
				} else if MetricsConfig.Inputs[inputName].Type == "bitfield_string" {
					union_array[element_idx] = castValueToUnionType(elementValue["string"], STRING)
				} else {
					union_array[element_idx] = castValueToUnionType(elementValue["value"], MetricsConfig.Inputs[inputName].Value.tag)
				}
			} else {
				union_array[element_idx] = Union{}
			}
		}
		if debug {
			if stringInSlice(debug_inputs, inputName) {
				log.Debugf("Received input [%s] value [%v]", inputName, elementValueCast)
			}
		}
	default:
		union_array = make([]Union, 1)
		union_array[0] = castValueToUnionType(elementValue, MetricsConfig.Inputs[inputName].Value.tag)
		if debug {
			if stringInSlice(debug_inputs, inputName) {
				log.Debugf("Received input [%s] value [%v]", inputName, elementValue)
			}
		}
	}

	if !unionListsMatch(InputScope[inputName], union_array) || containedInValChanged[inputName] || inputYieldsDirectMsg[inputName] {
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
		for _, expNum := range inputToMetricsExpression[inputName] {
			expressionNeedsEvalMutex.Lock()
			expressionNeedsEval[expNum] = true
			expressionNeedsEvalMutex.Unlock()
		}
	}
	inputScopeMutex.RUnlock()

	inputScopeMutex.Lock()
	InputScope[inputName] = union_array
	inputScopeMutex.Unlock()
	if containedInValChanged[inputName] || inputYieldsDirectMsg[inputName] {
		ProcessDirectMsgs()
	}
}

// make it so that we recalculate all filters using the attribute
// and all expressions using the result of that filter
func handleDecodedMetricsAttributeValue(inputName, scopeVar string) {
	attributeUnion := getUnionFromValue(elementValue)
	if debug {
		if stringInSlice(debug_inputs, inputName) {
			log.Debugf("Received input [%s] attribute [%s] value [%v]", inputName, scopeVar, elementValue)
		}
	}
	inputScopeMutex.Lock()
	if len(InputScope[scopeVar]) == 0 || attributeUnion != InputScope[scopeVar][0] {
		InputScope[scopeVar] = []Union{attributeUnion}
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
	inputScopeMutex.Unlock()
}

// TODO: this function is not called anywhere in the code
func GetOutputMsgBody(uri string) interface{} {
	if outputNames, ok := uriToOutputNameMap[uri]; ok {
		outputVals := make([]interface{}, 0)
		for _, outputVar := range outputNames {
			outputScopeMutex.RLock()
			output := OutputScope[outputVar]
			var val interface{}
			if len(output) > 1 {
				val_list := make([]interface{}, len(output))
				for g, union := range output {
					val_list[g] = getValueFromUnion(&union)
				}
				val = val_list
			} else if len(output) == 1 {
				val = getValueFromUnion(&output[0])
			} else {
				val = 0
			}
			if output2, ok := MetricsConfig.Outputs[outputVar]; ok {
				if stringInSlice(output2.Flags, "clothed") {
					val = map[string]interface{}{
						"value": val,
					}
					for attributeName := range output2.Attributes {
						outputVals := []Union{}
						if _, ok := OutputScope[outputVar+"@"+attributeName]; ok {
							outputVals = make([]Union, len(OutputScope[outputVar+"@"+attributeName]))
							copy(outputVals, OutputScope[outputVar+"@"+attributeName])
						}

						if len(outputVals) >= 1 {
							val.(map[string]interface{})[attributeName] = getValueFromUnion(&outputVals[0])
						} else {
							val.(map[string]interface{})[attributeName] = output2.Attributes[attributeName]
						}
					}
				}
			}
			outputVals = append(outputVals, val)
		}
		outputScopeMutex.RUnlock()
		// Maintaining the old convention that a single value will be returned, unless there are multiple values present (new use case)
		if len(outputVals) == 1 {
			return outputVals[0]
		} else {
			return outputVals
		}
	} else {
		outputMap := make(map[string]interface{}, 0)
		for outputName, output := range MetricsConfig.Outputs {
			if len(output.Uri) == len(uri) && strings.HasPrefix(output.Uri, uri) { // if the uri is the first part of output.Uri
				if len(output.Name) > 0 {
					outputMap[output.Name] = GetOutputMsgBody(uri + "/" + outputName)
				}
			} else if strings.HasPrefix(output.Uri, uri) {
				uriSuffix := output.Uri[len(uri)+1:] // get rid of the uri + first slash
				uriFrags := strings.Split(uriSuffix, "/")
				if len(uriFrags) > 0 {
					outputMap[uriFrags[0]] = GetOutputMsgBody(uri + "/" + uriFrags[0])
				}
			}
		}
		echoMutex.RLock()
		for echoIndex := range MetricsConfig.Echo {
			if uri == MetricsConfig.Echo[echoIndex].PublishUri {
				for key, value := range MetricsConfig.Echo[echoIndex].Echo {
					if _, ok := outputMap[key]; ok {
						if val, ok := outputMap[key].(map[string]interface{}); ok {
							val["value"] = value
						} else {
							outputMap[key] = value // overwrite the metrics calculation
						}
					} else {
						outputMap[key] = value
					}
				}
			} else if GetParentUri(uri) == MetricsConfig.Echo[echoIndex].PublishUri {
				uriFrags := strings.Split(uri, "/")
				if len(uriFrags) > 0 {
					key := uriFrags[len(uriFrags)-1]
					if echoValue, isVal := MetricsConfig.Echo[echoIndex].Echo[key]; isVal {
						if _, ok := outputMap[key]; ok {
							if val, ok := outputMap[key].(map[string]interface{}); ok {
								val["value"] = echoValue
							} else {
								outputMap[key] = echoValue // overwrite the metrics calculation
							}
						} else {
							outputMap[key] = echoValue
						}
					}
				}
			}
		}
		echoMutex.RUnlock()
		return outputMap
	}
}
