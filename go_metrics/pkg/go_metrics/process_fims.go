package go_metrics

import (
	"encoding/json"
	"fims"
	"fmt"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/go_flexgen/parsemap"
	simdjson "github.com/minio/simdjson-go"
)

func ProcessFims(msg fims.FimsMsgRaw) {
	metricsConfigMutex.RLock()
	processFimsTiming.start()
	go_metrics_prefix := false
	if len(msg.Frags) > 0 && msg.Frags[0] == ProcessName {
		msg.Uri = msg.Uri[len(ProcessName)+1:]
		msg.Frags = msg.Frags[1:]
		go_metrics_prefix = true
	}
	if msg.Method == "set" || msg.Method == "pub" {
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
				metricsConfigMutex.RUnlock()
				return
			}
		} else {
			// extract the necessary values from the message body
			iter = pj.Iter()
			handleJsonMessage(&msg)
		}

		// Set handling for URIs that we're certain should be owned by go_metrics
		if msg.Method == "set" && len(msg.Replyto) > 0 {
			// only reply if it's something we're responsible for
			send_set := true
			// either it's prefixed with 'go_metrics'
			if !go_metrics_prefix {
				for i := 0; i < len(msg.Frags) && send_set; i++ {
					uri := "/" + strings.Join(msg.Frags, "/")
					_, no_response := noGetResponse[uri]
					_, is_input := uriToInputNameMap[uri]
					_, is_echo_input := uriToEchoObjectInputMap[uri]
					if no_response || is_echo_input || is_input {
						send_set = false
						break
					}
				}
				// or it's an output that IS NOT "no response"
				// (if it isn't in uriToOutputNameMap[msg.Uri], we're assuming it's not an output)
				_, isOutputUri := PublishUris[msg.Uri]
				_, isOutVarUri := uriToOutputNameMap[msg.Uri]
				if !isOutputUri && !isOutVarUri {
					send_set = false
				}
			}
			if send_set {
				msgBodyInMutex.Lock()
				json.Unmarshal(msg.Body, &msgBodyIn)
				f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBodyIn})
				msgBodyInMutex.Unlock()
			}
		}
	} else if msg.Method == "get" && len(msg.Replyto) > 0 {
		if _, no_response := noGetResponse[msg.Uri]; no_response && !go_metrics_prefix {
			metricsConfigMutex.RUnlock()
			return
		}
		// find metric in outputs and set back out
		_, last_uri_element_is_output := MetricsConfig.Outputs[msg.Frags[len(msg.Frags)-1]]
		_, last_uri_element_is_input := MetricsConfig.Inputs[msg.Frags[len(msg.Frags)-1]]
		_, isEchoPublishUri := echoPublishUristoEchoNum[msg.Uri]
		if outputNames, ok := uriToOutputNameMap[msg.Uri]; ok && len(outputNames) > 0 { // asking for all outputs associated with the given URI
			msgBody := make(map[string]interface{}, 0)
			var tmpDirectMsgBody map[string]interface{}
			var tempPubMsgBody map[string]interface{}

			for _, outputName := range outputNames {
				outputUri := outputToUriGroup[outputName]
				interval_set := uriIsIntervalSet[outputUri]
				_, direct_set := uriIsDirect["set"][outputUri]
				_, direct_post := uriIsDirect["post"][outputUri]
				_, tmpDirectMsgBody, tempPubMsgBody = prepareSingleOutputVar(outputUri, outputName, false, false, true, interval_set, direct_set, direct_post, true)
				output, ok := MetricsConfig.Outputs[outputName]
				if ok {
					if interval_set || direct_set || direct_post {
						msgBody[output.Name] = tmpDirectMsgBody[output.Name]
					} else {
						msgBody[output.Name] = tempPubMsgBody[output.Name]
					}
				}
			}
			var responseBody interface{}
			if len(msgBody) > 1 {
				responseBody = msgBody
			} else if len(outputNames) > 0 {
				outputUri := outputToUriGroup[outputNames[0]]
				if _, ok := PublishUris[outputUri]; !ok {
					output, ok := MetricsConfig.Outputs[outputNames[0]]
					if ok {
						responseBody = msgBody[output.Name]
					}
				} else {
					responseBody = msgBody
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
			outputScopeMutex.Unlock()
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
	metricsConfigMutex.RUnlock()
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
//   - configuration documents are also accepted if the message is specifically to the "/go_metrics/configuration" endpoint
//

// note that multiple options are possible for a single message
func handleJsonMessage(msg *fims.FimsMsgRaw) {
	jsonPaths, ok := UriElements[(*msg).Uri]
	if !ok {
		return
	}
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

// handle new configuration fims messages received at runtime that should be appended to the configuration being used.
// Wrapper function that handles fims messages and
// msg: the message containing the doc
// id: the unique id for the configuration
// return whether the metric was updated
func handleNewRuntimeConfiguration(msg *fims.FimsMsgRaw, id string) (bool, error) {
	// Empty body received
	if msg.Body == nil || string(msg.Body) == "null" {
		if msg.Method != "del" {
			return false, fmt.Errorf("received empty configuration")
		}
	} else if err := handleNewConfiguration(msg.Body, id); err != nil {
		return false, err
	}

	// Backup the config to DBI if the configuration was handled successfully
	// Always send a set unless the message is a deletion
	fimsMethod := "set"
	if (*msg).Method == "del" {
		fimsMethod = "del"
	}
	f.SendRaw(fims.FimsMsgRaw{
		Method: fimsMethod,
		Uri:    "/dbi/go_metrics/" + id,
		Body:   msg.Body,
	})

	return true, nil
}

// handle new configuration documents received in raw byte form
// This can be either startup configuration or configuration received via fims messages at runtime
// config: configuration document in raw byte form
// id: unique id for the configuration
// return whether any errors occurred
func handleNewConfiguration(config []byte, id string) error {
	// Ignore warnings as they will be reported during the config processing itself
	new_metrics_info, _, wasError := UnmarshalConfig(config, id)
	new_metrics_info = GetPubTickers(new_metrics_info)
	GetSubscribeUris(new_metrics_info)

	if wasError {
		return fmt.Errorf("error handling configuration received from fims")
	} else {
		log.Infof("Received new configuration [%s] via FIMS", id)
	}

	// Start using new configuration
	var wg sync.WaitGroup
	wg.Add(1)
	metricsConfigMutex.Lock()
	is_restarting = true
	f.Close()
	// don't close the channel because that's still being used
	StartEvalsAndPubs(new_metrics_info, &wg)
	is_restarting = false
	metricsConfigMutex.Unlock()
	wg.Wait()

	return nil
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

// Request a list of configurations that exist in dbi, then request any additional configurations from this
// list that are not the base config and have not yet been processed.
// This will run whether the base config source is dbi or a file path
func GetAdditionalConfigurations() {
	// Start a FIMS channel that will receive FIMS requests.
	// connect to FIMS server
	cfgConn, err := fims.Connect(ProcessName + "_config")
	if err != nil {
		log.Errorf("failed to connect to FIMS server: %v", err)
		return
	}
	defer cfgConn.Close()
	configUri := fmt.Sprintf("/cfg_%s", ProcessName)
	err = cfgConn.Subscribe(configUri)
	if err != nil {
		log.Errorf("failed to subscribe to config reply uri %s: %v", configUri, err)
	}
	fimsReceive := make(chan fims.FimsMsg)
	go cfgConn.ReceiveChannel(fimsReceive)
	waitSeconds := 10
	timeout := time.NewTicker(time.Duration(waitSeconds) * time.Second)
	done := false
	defer timeout.Stop()
	// start request-receive cycle
	for {
		// TODO: better way to break from for+select?
		if !cfgConn.Connected() || done {
			break
		}
		// request DBI on every attempt, expecting data to be loaded to DBI
		err := cfgConn.SendGet(fmt.Sprintf("/dbi/%s", ProcessName), configUri)
		if err != nil {
			delaySeconds := 2
			log.Errorf("Error sending GET for configuration data to DBI: %s. Waiting %d seconds then trying again.", err.Error(), delaySeconds)
			delay := time.NewTimer(time.Duration(delaySeconds) * time.Second)
			<-delay.C
			timeout.Reset(time.Duration(waitSeconds) * time.Second)
		}
		// wait for DBI to reply, and re-request if it takes too long
		select {
		case data := <-fimsReceive:
			cfgMap, ok := data.Body.(map[string]interface{})
			// TODO: file and no config response case
			if !ok {
				log.Errorf("Received invalid configuration map: %v", cfgMap)
				done = true
				break
			}
			// TODO: file and only additional config sources
			if _, ok := cfgMap["configuration"]; !ok {
				log.Errorf("Could not find base configuration in config document")
			}
			for configId, doc := range cfgMap {
				// Skip the base configuration that was already configured
				if configId == "configuration" {
					continue
				}
				configBytes, err := json.Marshal(doc)
				if err != nil {
					log.Errorf("Could not process additional configuration document: %s, %v", configId, err)
				}
				handleNewConfiguration(configBytes, configId)
			}
			done = true
		case <-timeout.C:
			log.Errorf("Timed out after waiting %d seconds for configuration from DBI. Resending GET to DBI.", waitSeconds)
			done = true
		}
	}
}
