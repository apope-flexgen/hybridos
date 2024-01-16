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
		_, last_urilast_uri_element_is_input := MetricsConfig.Inputs[msg.Frags[len(msg.Frags)-1]]
		_, isEchoPublishUri := echoPublishUristoEchoNum[msg.Uri]
		if outputVar, ok := uriToOutputNameMap[msg.Uri]; ok { // asking for a single output value
			outputScopeMutex.Lock()
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
			outputScopeMutex.Unlock()
			if stringInSlice(MetricsConfig.Outputs[outputVar].Flags, "clothed") {
				f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{"value": val}})
			} else {
				f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: val})
			}

		} else if outputVars, ok := PublishUris[msg.Uri]; ok { // asking for data from a single publish URI
			msgBodyMutex.Lock()
			pubDataChangedMutex.Lock()
			pubDataChanged[msg.Uri] = true
			for _, outputVar := range outputVars {
				outputVarChanged[outputVar] = true
			}
			PrepareBody(msg.Uri)
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
			msgBodyMutex.Unlock()
		} else if _, ok := UriElements[msg.Uri]; ok { // asking for data from a set of publish URIs

			msgBodyMutex.Lock()
			msgBody = GetOutputMsgBody(msg.Uri).(map[string]interface{})
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
			msgBodyMutex.Unlock()
		} else if last_urilast_uri_element_is_input && len(msg.Frags) > 0 && msg.Frags[0] == "inputs" { // asking for a specific input
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
			msgBodyMutex.Lock()
			msgBody = make(map[string]interface{}, len(MetricsConfig.Inputs))
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
			msgBodyMutex.Unlock()
		} else if last_uri_element_is_output && len(msg.Frags) > 0 && msg.Frags[0] == "outputs" { // asking for a specific output (in a different way)
			outputName := MetricsConfig.Outputs[outputVar].Name
			outputScopeMutex.RLock()
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
			outputScopeMutex.RUnlock()
			f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{outputName: val}})
		} else if len(msg.Frags) > 0 && msg.Frags[0] == "outputs" { // asking for all outputs
			msgBodyMutex.Lock()
			msgBody = make(map[string]interface{}, len(OutputScope))
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
			msgBodyMutex.Unlock()
		} else if len(msg.Frags) >= 1 && msg.Frags[0] == "timings" {
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
			findValuesInMap(msg, (*msg).Uri, (*msg).Frags[len((*msg).Frags)-1], jsonPaths2, &iter)
		case interface{}:
			elementValueMutex.Lock()
			elementValue, _ = iter.Interface()
			handleUriElement(msg, (*msg).Uri, (*msg).Frags[len((*msg).Frags)-1])
			elementValueMutex.Unlock()
		}
	}
}

func findValuesInMap(msg *fims.FimsMsgRaw, currentUri string, currentKey string, valuesToLookFor map[string]interface{}, iter *simdjson.Iter) {
	fullUri := ""
	var err error
	typ := iter.Type()
	for {
		switch typ {
		case simdjson.TypeRoot:
			if typ, iterMap[currentUri], err = iter.Root(iterMap[currentUri]); err != nil {
				return
			}
		case simdjson.TypeObject:
			if _, ok := iterMap[currentUri]; ok {
				if objMap[currentUri], err = iterMap[currentUri].Object(objMap[currentUri]); err != nil {
					return
				}
			} 
			// else { // I don't think this is necessary, but it was here just in case.
			// 	if objMap[currentUri], err = iter.Object(objMap[currentUri]); err != nil {
			// 		return
			// 	}
			// }
			
			if _, ok := elemMap[currentUri]; !ok{
				elemMap[currentUri] = &simdjson.Iter{}
			}
			if _, ok := objMap[currentUri]; ok {
				name, typ, err := objMap[currentUri].NextElement(elemMap[currentUri]); 
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
							findValuesInMap(msg, fullUri, name, value, elemMap[currentUri])
						} else {
							elementValueMutex.Lock()
							elementValue, _ = elemMap[currentUri].Interface()
							handleUriElement(msg, fullUri, name)
							elementValueMutex.Unlock()
						}
					}
					name, typ, err = objMap[currentUri].NextElement(elemMap[currentUri]); 
				} 
				return
			}
		case simdjson.TypeNone:
			return
		}

		//typ = iter.Advance() // not sure if we can reach this line // need to test that
	}
}

func handleUriElement(msg *fims.FimsMsgRaw, uri string, elementName string) {

	// handle inputs
	if inputNames, ok := uriToInputNameMap[uri]; ok {
		for _, inputName := range inputNames {
			handleDecodedMetricsInputValue(inputName)
		}
	}

	// handle attributes at uris like /some/uri/input/attribute
	if inputsWithAttribute, ok := allPossibleAttributes[elementName]; ok {
		if inputNames, ok := uriToInputNameMap[GetParentUri(uri)]; ok {
			for _, inputName := range inputNames {
				if stringInSlice(inputsWithAttribute, inputName +"@" + elementName){
					handleDecodedMetricsAttributeValue(inputName,inputName +"@" + elementName)
				}
			}
		}
	}

	// handle attributes at uris like /some/uri/input@attribute
	if strings.Contains(uri,"@") {
		input := strings.Split(elementName,"@")[0]
		elementName2 := strings.Split(uri, "@")[1]
		if inputsWithAttribute, ok := allPossibleAttributes[elementName2]; ok {
			if inputNames, ok := uriToInputNameMap[GetParentUri(uri) + "/" + input]; ok {
				for _, inputName := range inputNames {
					if stringInSlice(inputsWithAttribute, inputName +"@" + elementName2){
						handleDecodedMetricsAttributeValue(inputName,inputName +"@" + elementName2)
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

	// handle echo "set" forwarding
	if (*msg).Method == "set" {

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

	if !unionListsMatch(InputScope[inputName], union_array) || containedInValChanged[inputName] || inputYieldsDirectSet[inputName] {
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
	if containedInValChanged[inputName] || inputYieldsDirectSet[inputName] {
		ProcessDirectSets()
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
								clothedOutputVal["value"] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							clothedOutputVal["value"] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						clothedOutputVal["value"] = []EnumObject{EnumObject{Value: -1, String: "Unknown"}}
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
								msgBody[outputVar] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
							}
						} else {
							msgBody[outputVar] = []EnumObject{EnumObject{Value: enumIndex, String: "Unknown"}}
						}
					} else {
						msgBody[outputVar] = []EnumObject{EnumObject{Value: -1, String: "Unknown"}}
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
}

func GetOutputMsgBody(uri string) interface{} {
	if outputVar, ok := uriToOutputNameMap[uri]; ok {
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
		if output2,ok := MetricsConfig.Outputs[outputVar]; ok {
				if stringInSlice(output2.Flags,"clothed") {
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
		
		outputScopeMutex.RUnlock()
		return val
	} else {
		outputMap := make(map[string]interface{}, 0)
		for outputName, output := range MetricsConfig.Outputs {
			if len(output.Uri) == len(uri) && strings.HasPrefix(output.Uri, uri) { // if the uri is the first part of output.Uri
				if len(output.Name) > 0 {
					outputMap[output.Name] = GetOutputMsgBody(uri + "/" + outputName)
				}
			} else if strings.HasPrefix(output.Uri, uri) {
				uriSuffix := output.Uri[len(uri)+1:] // get rid of the uri + first slash
				uriFrags := strings.Split(uriSuffix,"/")
				if len(uriFrags) > 0 {
					outputMap[uriFrags[0]] = GetOutputMsgBody(uri + "/" + uriFrags[0])
				}
			}
		}
		echoMutex.RLock()
		for echoIndex := range MetricsConfig.Echo {
			if uri == MetricsConfig.Echo[echoIndex].PublishUri {
				for key, value := range MetricsConfig.Echo[echoIndex].Echo {
					if _,ok := outputMap[key]; ok {
						if val,ok := outputMap[key].(map[string]interface{}); ok {
							val["value"] = value
						} else {
							outputMap[key] = value // overwrite the metrics calculation
						}
					} else {
						outputMap[key] = value
					}
				}
			} else if GetParentUri(uri) == MetricsConfig.Echo[echoIndex].PublishUri{
				uriFrags := strings.Split(uri,"/")
				if len(uriFrags) > 0 {
					key := uriFrags[len(uriFrags)-1]
					if echoValue, isVal := MetricsConfig.Echo[echoIndex].Echo[key]; isVal{
						if _,ok := outputMap[key]; ok {
							if val,ok := outputMap[key].(map[string]interface{}); ok {
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
	return nil
}
