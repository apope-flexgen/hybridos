package go_metrics

import (
	"encoding/json"
	"fims"
	"fmt"
	"go/ast"
	"go/token"
	"math"
	"os"
	"reflect"
	"regexp"
	"sort"
	"strconv"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

func StartEvalsAndPubs(wg *sync.WaitGroup) {
	defer wg.Done()
	// connect to fims and subscribe to inputs
	var err error
	f, err = fims.Connect(ProcessName)
	if err != nil {
		log.Fatalf("Unable to connect to FIMS server: %s", err)
	}
	err = f.Subscribe(SubscribeUris...)
	if err != nil {
		log.Fatalf("Unable to subscribe to input URIs: %s", err)
	}

	sub_string := "Subscribed to:\n"

	for _, subscribeUri := range SubscribeUris {
		sub_string += "\t\t" + subscribeUri + "\n"
	}
	log.Infof(sub_string)

	fimsMap = make(chan fims.FimsMsgRaw, 100)
	// listen for messages
	go f.ReceiveChannelRaw(fimsMap)

	processFimsTiming.init()
	evalExpressionsTiming.init()
	go func() {
		for {
			if f.Connected() {
				ProcessFims(<-fimsMap) // this is blocking, so we won't create a bajillion instances of the goroutine below
			} else {
				break
			}
		}
	}()

	t0 = time.Now()
	MDOtick := time.NewTicker(10000 * time.Millisecond)
	for j := 0; j < len(tickers); j += 1 {
		go func(tickerIndex int) {
			for {
				if !f.Connected() {
					break
				}
				<-tickers[tickerIndex].C
				EvaluateExpressions()
				pubDataChangedMutex.Lock()
				for _, pubUri := range tickerPubs[tickerIndex] {
					pubDataChanged[pubUri] = true
				}
				temp_uri := ""
				for uriGroup := range PublishUris {
					if len(uriGroup) > 0 {
						if needsPub := pubDataChanged[uriGroup]; needsPub {
							PrepareBody(uriGroup)
							if strings.Contains(uriGroup, "[") {
								temp_uri = uriGroup[0:strings.Index(uriGroup, "[")]
							} else {
								temp_uri = uriGroup
							}

							directMsgMutex.RLock()
							// Check whether there are still direct messages that should be sent out
							_, is_direct_set := uriIsDirect["set"][uriGroup]
							_, is_direct_post := uriIsDirect["post"][uriGroup]
							sendDirectSet := is_direct_set && uriToDirectMsgActive["set"][uriGroup]
							sendDirectPost := is_direct_post && uriToDirectMsgActive["post"][uriGroup]
							directMsgMutex.RUnlock()

							if uriIsIntervalSet[uriGroup] && (len(directMsgBody) > 0 || uriHeartbeat[uriGroup]) {
								if uriIsLonely[uriGroup] {
									lonelyVarName := uriGroup[strings.Index(uriGroup, "[")+1 : strings.Index(uriGroup, "]")]
									if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
										lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
									}
									_, err = f.Send(fims.FimsMsg{
										Method: "set",
										Uri:    temp_uri + "/" + lonelyVarName,
										Body:   directMsgBody[lonelyVarName],
									})
								} else {
									_, err = f.Send(fims.FimsMsg{
										Method: "set",
										Uri:    temp_uri,
										Body:   directMsgBody,
									})
								}
							} else if len(directMsgBody) > 0 && (sendDirectSet || sendDirectPost) {
								if uriIsLonely[uriGroup] {
									lonelyVarName := uriGroup[strings.Index(uriGroup, "[")+1 : strings.Index(uriGroup, "]")]
									if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
										lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
									}
									// Lock and lower the direct message signal if the message is sent
									directMsgMutex.Lock()
									if sendDirectSet {
										f.Send(fims.FimsMsg{
											Method: "set",
											Uri:    (temp_uri + "/" + lonelyVarName),
											Body:   directMsgBody[lonelyVarName],
										})
										uriToDirectMsgActive["set"][uriGroup] = false
									} else if sendDirectPost {
										f.Send(fims.FimsMsg{
											Method: "post",
											Uri:    (temp_uri + "/" + lonelyVarName),
											Body:   directMsgBody[lonelyVarName],
										})
										uriToDirectMsgActive["post"][uriGroup] = false
									}
									directMsgMutex.Unlock()
								} else {
									// Lock and lower the direct message signal if the message is sent
									directMsgMutex.Lock()
									if sendDirectSet {
										f.Send(fims.FimsMsg{
											Method: "set",
											Uri:    (temp_uri),
											Body:   directMsgBody,
										})
										uriToDirectMsgActive["set"][uriGroup] = false
									} else if sendDirectPost {
										f.Send(fims.FimsMsg{
											Method: "post",
											Uri:    (temp_uri),
											Body:   directMsgBody,
										})
										uriToDirectMsgActive["post"][uriGroup] = false
									}
									directMsgMutex.Unlock()
								}
							} else if len(pubMsgBody) > 0 || uriHeartbeat[uriGroup] {
								if uriIsLonely[uriGroup] {
									lonelyVarName := uriGroup[strings.Index(uriGroup, "[")+1 : strings.Index(uriGroup, "]")]
									if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
										lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
									}
									_, err = f.Send(fims.FimsMsg{
										Method: "pub",
										Uri:    temp_uri + "/" + lonelyVarName,
										Body:   pubMsgBody[lonelyVarName],
									})
								} else {
									_, err = f.Send(fims.FimsMsg{
										Method: "pub",
										Uri:    temp_uri,
										Body:   pubMsgBody,
									})
								}
							}
							pubDataChanged[uriGroup] = false
						}

					}
				}
				pubDataChangedMutex.Unlock()

			}
		}(j)
	}

	for echoIdx := range MetricsConfig.Echo {
		go func(echoIndex int) {
			for {
				if !f.Connected() {
					break
				}
				<-MetricsConfig.Echo[echoIndex].Ticker.C
				echoMutex.RLock()
				if len(MetricsConfig.Echo[echoIndex].Heartbeat) > 0 {
					MetricsConfig.Echo[echoIndex].Echo[MetricsConfig.Echo[echoIndex].Heartbeat] = float64(time.Since(t0)) / 1000000000.0
				}
				if MetricsConfig.Echo[echoIndex].Format == "naked" {
					echoMsgBodyMutex.Lock()
					echoMsgBody = make(map[string]interface{}, 0)
					for key, value := range MetricsConfig.Echo[echoIndex].Echo {
						switch x := value.(type) {
						case map[string]interface{}:
							if value, ok := x["value"]; ok {
								echoMsgBody[key] = value
							} else {
								echoMsgBody[key] = nil
							}
						default:
							echoMsgBody[key] = value
						}
					}
					f.Send(fims.FimsMsg{
						Method: "pub",
						Uri:    MetricsConfig.Echo[echoIndex].PublishUri,
						Body:   echoMsgBody,
					})
					echoMsgBodyMutex.Unlock()
				} else if MetricsConfig.Echo[echoIndex].Format == "clothed" {
					echoMsgBodyMutex.Lock()
					echoMsgBody = make(map[string]interface{}, 0)
					for key, value := range MetricsConfig.Echo[echoIndex].Echo {
						switch value.(type) {
						case map[string]interface{}:
							echoMsgBody[key] = value
						default:
							switch value.(type) {
							case string:
								var outputElementValue interface{}
								json.Unmarshal([]byte(fmt.Sprintf("{\"value\":\"%v\"}", value)), &outputElementValue)
								echoMsgBody[key] = outputElementValue
							default:
								var outputElementValue interface{}
								json.Unmarshal([]byte(fmt.Sprintf("{\"value\":%v}", value)), &outputElementValue)
								echoMsgBody[key] = outputElementValue
							}
						}
					}
					f.Send(fims.FimsMsg{
						Method: "pub",
						Uri:    MetricsConfig.Echo[echoIndex].PublishUri,
						Body:   echoMsgBody,
					})
					echoMsgBodyMutex.Unlock()
				} else {
					f.Send(fims.FimsMsg{
						Method: "pub",
						Uri:    MetricsConfig.Echo[echoIndex].PublishUri,
						Body:   MetricsConfig.Echo[echoIndex].Echo,
					})
				}
				echoMutex.RUnlock()
			}
		}(echoIdx)
	}

	for {
		if !f.Connected() {
			break
		}
		<-MDOtick.C
		mdoBuf.Reset()
		if len(MdoFile) > 0 {
			var fd *os.File
			if strings.Contains(MdoFile, ".json") {
				fd, _ = os.OpenFile(MdoFile, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			} else {
				fd, _ = os.OpenFile(MdoFile+".json", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			}
			Mdo.Meta["name"] = ProcessName
			Mdo.Meta["timestamp"] = time.Now().Format(time.RFC3339)
			inputScopeMutex.RLock()
			for key := range Mdo.Inputs {
				input := InputScope[key]
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
				Mdo.Inputs[key]["value"] = val
				for _, attribute := range MetricsConfig.Inputs[key].Attributes {
					input := InputScope[key+"@"+attribute]
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
					Mdo.Inputs[key][attribute] = val
				}
			}
			inputScopeMutex.RUnlock()
			filtersMutex.RLock()
			for key := range MetricsConfig.Filters {
				if len(dynamicFilterExpressions[key].DynamicInputs) > 0 {
					copy(Mdo.Filters[key], dynamicFilterExpressions[key].DynamicInputs[len(dynamicFilterExpressions[key].DynamicInputs)-1])
				} else if len(staticFilterExpressions[key].DynamicInputs) > 0 {
					copy(Mdo.Filters[key], staticFilterExpressions[key].DynamicInputs[len(staticFilterExpressions[key].DynamicInputs)-1])
				} else {
					Mdo.Filters[key] = make([]string, 0)
				}
			}
			filtersMutex.RUnlock()
			outputScopeMutex.RLock()
			for key := range MetricsConfig.Outputs {
				mdooutput = MetricsConfig.Outputs[key]
				output := OutputScope[key]
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
				Mdo.Outputs[key]["value"] = val
				for attribute := range mdooutput.Attributes {
					output := OutputScope[key+"@"+attribute]
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
					Mdo.Outputs[key][attribute] = val
				}
			}
			outputScopeMutex.RUnlock()
			for k := range MetricsConfig.Metrics {
				metricsMutex[k].RLock()
				if strings.Contains(strings.ToLower(MetricsConfig.Metrics[k].Expression), "overtimescale") && !strings.Contains(strings.ToLower(MetricsConfig.Metrics[k].Expression), "integrate") {
					currentIndex := 0
					if _, ok := MetricsConfig.Metrics[k].State["currentIndex"]; ok {
						currentIndex = int(MetricsConfig.Metrics[k].State["currentIndex"][0].ui)
					}
					for stateVar, stateVal := range MetricsConfig.Metrics[k].State {
						if stateVar == "alwaysEvaluate" || strings.Contains(stateVar, "index") {
							continue
						} else if stateVar == "timestamps" {
							Mdo.Metrics[MetricsConfig.Metrics[k].Expression]["firstTimestamp"] = time.UnixMilli(stateVal[0].i).Format(time.RFC3339)
						} else {
							if len(stateVal) > 1 {
								Mdo.Metrics[MetricsConfig.Metrics[k].Expression][stateVar] = make([]interface{}, currentIndex+1)
								for q := range stateVal[0:currentIndex] {
									Mdo.Metrics[MetricsConfig.Metrics[k].Expression][stateVar].([]interface{})[q] = getValueFromUnion(&stateVal[q])
								}
							} else {
								Mdo.Metrics[MetricsConfig.Metrics[k].Expression][stateVar] = getValueFromUnion(&stateVal[0])
							}
						}
					}
				} else {
					for stateVar, stateVal := range MetricsConfig.Metrics[k].State {
						if stateVar == "alwaysEvaluate" {
							continue
						}
						Mdo.Metrics[MetricsConfig.Metrics[k].Expression][stateVar] = getValueFromUnion(&stateVal[0])
					}
				}
				metricsMutex[k].RUnlock()
			}

			echoMutex.RLock()
			for k := range MetricsConfig.Echo {
				Mdo.Echo[MetricsConfig.Echo[k].PublishUri] = MetricsConfig.Echo[k].Echo
			}
			echoMutex.RUnlock()
			err = mdoEncoder.Encode(Mdo)
			if err == nil {
				fd.Write(mdoBuf.Bytes())
			} else {
				log.Warnf("%v", err)
			}
			fd.Close()
		}

	}
}

// Construct and send the fims message to the uri provided using the appropriate method
// The caller must ensure that the pubDataChanged, outputScope, and directMsgMutex all are unlocked
// method: the fims method by which the message will be sent
// directMsgUriGroup: the uri group to which the message will be sent
func constructAndSendDirectMsgs(method string, directMsgUriGroup string) {
	var directMsgUri string
	pubDataChangedMutex.Lock()
	PrepareBody(directMsgUriGroup)
	if strings.Contains(directMsgUriGroup, "[") {
		directMsgUri = directMsgUriGroup[0:strings.Index(directMsgUriGroup, "[")]
	} else {
		directMsgUri = directMsgUriGroup
	}

	if len(directMsgBody) > 0 {
		if uriIsLonely[directMsgUriGroup] {
			lonelyVarName := directMsgUriGroup[strings.Index(directMsgUriGroup, "[")+1 : strings.Index(directMsgUriGroup, "]")]
			if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
				lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
			}
			f.Send(fims.FimsMsg{
				Method: method,
				Uri:    (directMsgUri + "/" + lonelyVarName),
				Body:   directMsgBody[lonelyVarName],
			})
		} else {
			f.Send(fims.FimsMsg{
				Method: method,
				Uri:    directMsgUri,
				Body:   directMsgBody,
			})
		}
	}
	pubDataChanged[directMsgUriGroup] = false
	directMsgMutex.Lock()
	uriToDirectMsgActive[method][directMsgUriGroup] = false
	directMsgMutex.Unlock()
	pubDataChangedMutex.Unlock()
}

// Handle and send out messages directly
func ProcessDirectMsgs() {
	EvaluateExpressions()
	inputScopeMutex.Lock()

	// Get the direct messages that are active
	activeDirectSets := make([]string, 0)
	activeDirectPosts := make([]string, 0)
	directMsgMutex.RLock()
	for directMsgUriGroup, active := range uriToDirectMsgActive["set"] {
		if active {
			activeDirectSets = append(activeDirectSets, directMsgUriGroup)
		}
	}
	for directMsgUriGroup, active := range uriToDirectMsgActive["post"] {
		if active {
			activeDirectPosts = append(activeDirectPosts, directMsgUriGroup)
		}
	}
	directMsgMutex.RUnlock()

	// Send out the direct messages to their given uris
	for _, directMsgUriGroup := range activeDirectSets {
		constructAndSendDirectMsgs("set", directMsgUriGroup)
	}
	for _, directMsgUriGroup := range activeDirectPosts {
		constructAndSendDirectMsgs("post", directMsgUriGroup)
	}
	inputScopeMutex.Unlock()
}

func addFilterVars(filterName string) []Union {
	outputFilterVars := make([]Union, 0)
	var intermediateVars []Union
	if _, ok := InputScope[filterName]; ok {
		outputFilterVars = append(outputFilterVars, InputScope[filterName]...)
	} else if _, ok := FilterScope[filterName]; ok {
		for _, inputUnion := range FilterScope[filterName] {
			intermediateVars = addFilterVars(inputUnion)
			outputFilterVars = append(outputFilterVars, intermediateVars...)
		}
	}
	return outputFilterVars
}

// evaluate a filter, including static and dynamic expressions
func EvaluateFilter(filterName string) {
	var needsEval bool
	filterNeedsEvalMutex.RLock()
	needsEval = filterNeedsEval[filterName]
	filterNeedsEvalMutex.RUnlock()

	if needsEval {
		if debug {
			if stringInSlice(debug_filters, filterName) {
				log.Debugf("Evaluating filter [%s]", filterName)
			}
		}
		// this loop for static filters is actually necessary because inputs can change values
		if filter, ok := staticFilterExpressions[filterName]; ok {
			InputScope[filterName] = make([]Union, 0)
			for _, dynamicInput := range filter.DynamicInputs[0] {
				InputScope[filterName] = append(InputScope[filterName], InputScope[dynamicInput]...)
			}
		}

		if filter, ok := dynamicFilterExpressions[filterName]; ok {
			filtersMutex.RLock()
			intermediateInputs := filter.DynamicInputs[0]
			filtersMutex.RUnlock()
			for p, exp := range filter.DynamicFilterExpressions {
				stringArr, _ := EvaluateDynamicFilter(&(exp.Ast), intermediateInputs)
				intermediateInputs = stringArr
				sort.Strings(intermediateInputs)
				filtersMutex.Lock()
				FilterScope[filterName] = make([]string, len(intermediateInputs))
				copy(FilterScope[filterName], intermediateInputs)
				copy(dynamicFilterExpressions[filterName].DynamicInputs[p+1], intermediateInputs)
				filtersMutex.Unlock()
				if len(intermediateInputs) == 0 {
					break
				}
			}
			InputScope[filterName] = make([]Union, 0)
			for _, str := range intermediateInputs {
				InputScope[filterName] = append(InputScope[filterName], addFilterVars(str)...)
			}
			if debug {
				if stringInSlice(debug_filters, filterName) {
					log.Debugf("Filter [%s] contains input vars %v", filterName, intermediateInputs)
				}
			}
		}
		filterNeedsEvalMutex.Lock()
		filterNeedsEval[filterName] = false
		filterNeedsEvalMutex.Unlock()
	}
}

// Store the automatically generated alerting attributes in the OutputScope. This includes the true/false status
// of the alert as well as the timestamp(s) when the alert was triggered.
// Uses the outputScopeMutex before writing to the OutputScope
// outputVar: the current output variable that should use the attributes
// outputVals: the result of the metrics expression
// metricsObject: the current metricsObject being evaluated
// return any errors that occurred
func storeAlertingAttributes(outputVar string, outputVals []Union, metricsObject *MetricsObject) error {
	if !metricsObject.Alert {
		return nil
	}

	// Ensure the status and details array exist for the given output variable
	outputScopeMutex.Lock()
	if len(OutputScope[outputVar+"@alertStatus"]) == 0 {
		// alertStatus tracks whether the alert is current active (true) or inactive (false)
		// this is essentially just the value, but it's routed through the outputScope for consistency with other attributes
		OutputScope[outputVar+"@alertStatus"] = []Union{{tag: BOOL, b: false}}
	}
	if len(outputVals) == 0 {
		return fmt.Errorf("no result available for the expression")
	}
	expressionResult := outputVals[0]
	if err := castUnionType(&expressionResult, BOOL); err != nil {
		return fmt.Errorf("cannot cast a boolean result from the metrics expression")
	}
	OutputScope[outputVar+"@alertStatus"][0].b = expressionResult.b

	// Reset details and add the ones currently active
	OutputScope[outputVar+"@activeMessages"] = make([]Union, 0)
	OutputScope[outputVar+"@activeTimestamps"] = make([]Union, 0)
	if expressionResult.b {
		// If true, populate the details array with the timestamp and messages
		// The two will be added together in the outputs, with each active message getting the latest timestamp
		OutputScope[outputVar+"@activeMessages"] = append(OutputScope[outputVar+"@activeMessages"], (*metricsObject).State["activeMessages"]...)
		OutputScope[outputVar+"@activeTimestamps"] = append(OutputScope[outputVar+"@activeTimestamps"], (*metricsObject).State["activeTimestamps"]...)
	}
	outputScopeMutex.Unlock()
	return nil
}

// Return whether there are new active messages indicating the metric has changed
func (metricsObject *MetricsObject) alertAttributesHaveChanged() bool {
	return len(metricsObject.State["activeMessages"]) > 0
}

// Clear out the metricsObject state from the last iteration
func (metricsObject *MetricsObject) clearAlertingState() {
	if !metricsObject.Alert {
		return
	}

	if len(metricsObject.State["activeMessages"]) > 0 {
		metricsObject.State["activeMessages"] = make([]Union, 0)
	}
	if len(metricsObject.State["activeTimestamps"]) > 0 {
		metricsObject.State["activeTimestamps"] = make([]Union, 0)
	}
}

func EvaluateExpressions() {
	inputScopeMutex.Lock()
	evalExpressionsTiming.start()
	for _, filterName := range FiltersList {
		EvaluateFilter(filterName)
	}

	if len(MetricsConfig.Metrics) > 0 {
		metricsMutex[0].RLock()
	}
	for q, metricsObject := range MetricsConfig.Metrics {
		metricsObjectPointer := &MetricsConfig.Metrics[q]
		metricsMutex[q].RUnlock()
		expressionNeedsEvalMutex.RLock()
		needEval := expressionNeedsEval[q]
		expressionNeedsEvalMutex.RUnlock()
		if needEval {
			originalType := metricsObjectPointer.Type
			// Clear out the previous state for alerting values that should not persist
			metricsMutex[q].Lock()

			metricsObjectPointer.clearAlertingState()
			outputVals, err := Evaluate(&(metricsObject.ParsedExpression), &(metricsObjectPointer.State))
			metricsMutex[q].Unlock()
			if err != nil {
				log.Warnf("Error with metrics expression  [   %s   ]:\n%s\n", metricsObject.Expression, err)
			}

			if err == nil && outputVals[0].tag != NIL {
				if originalType != NIL {
					castUnionType(&outputVals[0], originalType)
				}

				for _, fullOutputName := range metricsObject.Outputs {
					if strings.Contains(fullOutputName, "@") {
						splitOutputVar := strings.Split(fullOutputName, "@")
						if len(splitOutputVar) == 2 {
							outputVar := splitOutputVar[0]
							attribute := splitOutputVar[1]

							directMsgMutex.RLock()
							// Check for both methods of direct messages in determining whether to send messages
							_, is_direct_set := uriIsDirect["set"][outputToUriGroup[outputVar]]
							_, is_direct_post := uriIsDirect["post"][outputToUriGroup[outputVar]]
							directMsgMutex.RUnlock()

							// Storage the alerting attributes if this is an alert
							if err := storeAlertingAttributes(outputVar, outputVals, metricsObjectPointer); err != nil {
								log.Errorf("Cannot generate alert for %s metric: %v", metricsObjectPointer.Id, err)
							}

							// Update the scope here to allow for direct messages to occur below
							outputScopeMutex.Lock()
							OutputScope[fullOutputName] = make([]Union, len(outputVals))
							copy(OutputScope[fullOutputName], outputVals)
							outputScopeMutex.Unlock()

							is_sparse := uriIsSparse[outputToUriGroup[outputVar]]
							// Check if the values have changed from the last evaluation, then copy the new values into the OutputScope
							valuesChanged := !unionListsMatch(OutputScope[fullOutputName], outputVals) || metricsObjectPointer.alertAttributesHaveChanged()

							if valuesChanged || ((is_direct_set || is_direct_post) && !is_sparse) {
								pubDataChangedMutex.Lock()
								outputVarChanged[outputVar] = true
								pubDataChangedMutex.Unlock()

								// Lock and raise the direct message flag
								if is_direct_set {
									directMsgMutex.Lock()
									// Mark the uri as holding an active set
									uriToDirectMsgActive["set"][outputVar] = true
									directMsgMutex.Unlock()
									constructAndSendDirectMsgs("set", outputToUriGroup[outputVar])
								}
								if is_direct_post {
									directMsgMutex.Lock()
									// Mark the uri as holding an active post
									uriToDirectMsgActive["post"][outputVar] = true
									directMsgMutex.Unlock()
									constructAndSendDirectMsgs("post", outputToUriGroup[outputVar])
								}

								pubDataChangedMutex.Lock()
								pubDataChanged[outputToUriGroup[outputVar]] = false
								pubDataChangedMutex.Unlock()

								if debug {
									if stringInSlice(debug_outputs, outputVar) {
										log.Debugf("Output [%s] attribute [%s] changed value to [%v]", outputVar, attribute, tempValue)
									}
								}
							}
						} else {
							log.Errorf("Something went wrong when trying to change the value of " + fullOutputName)
						}
					} else {
						outputScopeMutex.Lock()
						output := make([]Union, len(OutputScope[fullOutputName]))
						copy(output, OutputScope[fullOutputName])
						outputScopeMutex.Unlock()

						directMsgMutex.RLock()
						// Check for both methods of direct messages in determining whether to send messages
						_, is_direct_set := uriIsDirect["set"][outputToUriGroup[fullOutputName]]
						_, is_direct_post := uriIsDirect["post"][outputToUriGroup[fullOutputName]]
						directMsgMutex.RUnlock()

						// Storage the alerting attributes if this is an alert
						if err := storeAlertingAttributes(fullOutputName, outputVals, metricsObjectPointer); err != nil {
							log.Errorf("Cannot generate alert for %s metric: %v", metricsObjectPointer.Id, err)
						}

						is_sparse := uriIsSparse[outputToUriGroup[fullOutputName]]
						// Check if the values have changed from the last evaluation, then copy the new values into the OutputScope
						valuesChanged := !unionListsMatch(OutputScope[fullOutputName], outputVals) || metricsObjectPointer.alertAttributesHaveChanged()

						// Update the scope here to allow for direct messages to occur below
						outputScopeMutex.Lock()
						OutputScope[fullOutputName] = make([]Union, len(outputVals))
						copy(OutputScope[fullOutputName], outputVals)
						outputScopeMutex.Unlock()

						if valuesChanged || ((is_direct_set || is_direct_post) && !is_sparse) {
							pubDataChangedMutex.Lock()
							outputVarChanged[fullOutputName] = true
							pubDataChangedMutex.Unlock()

							// Lock and raise the direct message flag
							if is_direct_set {
								directMsgMutex.Lock()
								// Mark the uri as holding an active set
								uriToDirectMsgActive["set"][fullOutputName] = true
								directMsgMutex.Unlock()
								constructAndSendDirectMsgs("set", outputToUriGroup[fullOutputName])
							}
							if is_direct_post {
								directMsgMutex.Lock()
								// Mark the uri as holding an active post
								uriToDirectMsgActive["post"][fullOutputName] = true
								directMsgMutex.Unlock()
								constructAndSendDirectMsgs("post", outputToUriGroup[fullOutputName])
							}

							pubDataChangedMutex.Lock()
							pubDataChanged[outputToUriGroup[fullOutputName]] = false
							pubDataChangedMutex.Unlock()
							if debug {
								if stringInSlice(debug_outputs, fullOutputName) {
									log.Debugf("Output [%s] changed value to [%v]", fullOutputName, getValueFromUnion(&outputVals[0]))
								}
							}
						}
					}
				}

				if len(metricsObject.InternalOutput) > 0 {
					input := InputScope[metricsObject.InternalOutput]
					if !unionListsMatch(input, outputVals) {
						for _, expNum := range inputToMetricsExpression[metricsObject.InternalOutput] {
							expressionNeedsEvalMutex.Lock()
							expressionNeedsEval[expNum] = true
							expressionNeedsEvalMutex.Unlock()
						}
						if debug {
							if stringInSlice(debug_inputs, metricsObject.InternalOutput) {
								log.Debugf("Internal input [%s] changed value to [%v]", metricsObject.InternalOutput, getValueFromUnion(&outputVals[0]))
							}
						}
					}
					InputScope[metricsObject.InternalOutput] = make([]Union, len(outputVals))
					copy(InputScope[metricsObject.InternalOutput], outputVals)
				}
			}

			metricsMutex[q].Lock()
			metricsObjectPointer.State["value"] = make([]Union, len(outputVals))
			copy(metricsObjectPointer.State["value"], outputVals)
			metricsMutex[q].Unlock()
			if !metricsObjectPointer.State["alwaysEvaluate"][0].b {
				expressionNeedsEvalMutex.Lock()
				expressionNeedsEval[q] = false
				expressionNeedsEvalMutex.Unlock()
			}
		}
		if q < len(MetricsConfig.Metrics)-1 {
			metricsMutex[q+1].RLock() // lock this value before we try to access the next metrics object
		}
	}
	evalExpressionsTiming.stop()
	inputScopeMutex.Unlock()
}

/**
* This function takes a parsed expression and map of input variables to their
* run-time values. Calls an auxiliary function to do the dirty work.
 */
// TODO: Handle filter functions ("aliases") as input types
func Evaluate(parsed *Expression, state *map[string][]Union) ([]Union, error) {

	result, err := parsed.evaluate(&(parsed.Ast), state)

	if err != nil || len(result) == 0 {
		return []Union{{}}, err
	}

	return result, nil
}

// Looks at a node in an AST and determines the type of the node.
// Based on the type, it calls an auxiliary function to do the actual evaluation.
func (exp *Expression) evaluate(node *ast.Node, state *map[string][]Union) (values []Union, err error) {
	switch (*node).(type) {
	case *ast.Ident:
		values, err = exp.evaluateIdent((*node).(*ast.Ident))
	case *ast.CallExpr:
		values, err = exp.evaluateCallExpr((*node).(*ast.CallExpr), state)
	case *ast.BinaryExpr:
		values, err = exp.evaluateBinary((*node).(*ast.BinaryExpr), state)
	case *ast.ParenExpr:
		newNode := ast.Node((*node).(*ast.ParenExpr).X)
		values, err = exp.evaluate(&newNode, state)
	case *ast.UnaryExpr:
		values, err = exp.evaluateUnary((*node).(*ast.UnaryExpr), state)
	case *ast.BasicLit:
		values, err = exp.evaluateBasicLit((*node).(*ast.BasicLit))
	case *ast.SelectorExpr:
		values, err = exp.evaluateSelector((*node).(*ast.SelectorExpr))
	default:
		err = fmt.Errorf("unsupported node %+v (type %+v)", *node, reflect.TypeOf(*node))
	}

	// Check each of the subexpressions to see if any are matched against the list that should be reported
	if msgErr := exp.trackActiveSubexpressions(node, state, &values); msgErr != nil {
		if err != nil {
			// Combine errors
			err = fmt.Errorf("%w, and error matching subexpression: %w", err, msgErr)
		} else {
			err = fmt.Errorf("error matching subexpression: %w", err)
		}
	}

	return values, err
}

// pull a variable out of the map of inputs
func (exp *Expression) evaluateIdent(node *ast.Ident) ([]Union, error) {
	if node.Name == "true" {
		return []Union{{tag: BOOL, b: true}}, nil
	} else if node.Name == "false" {
		return []Union{{tag: BOOL, b: false}}, nil
	} else if node.Name == "nil" {
		return []Union{{}}, nil
	}
	inputs, found := InputScope[node.Name]
	if !found {
		return []Union{}, fmt.Errorf("could not find variable %v in Scope", node.Name)
	}
	values := make([]Union, len(inputs))
	copy(values, inputs)
	if len(values) == 0 {
		values = []Union{{}} // if the variable exists, but doesn't have a value, give it a default value of 0
	}
	return values, nil
}

// pull a variable out of the map of inputs
func (exp *Expression) evaluateSelector(node *ast.SelectorExpr) ([]Union, error) {
	nodeName := node.X.(*ast.Ident).Name + "@" + node.Sel.Name
	inputs, found := InputScope[nodeName]
	if !found {
		return []Union{}, fmt.Errorf("could not find variable %v in Scope", nodeName)
	}
	values := make([]Union, len(inputs))
	copy(values, inputs)
	if len(values) == 0 {
		values = []Union{{}} // if the variable exists, but doesn't have a value, give it a default value of 0
	}
	return values, nil
}

// evaluate the result of a binary operation
func (exp *Expression) evaluateBinary(node *ast.BinaryExpr, state *map[string][]Union) ([]Union, error) {
	newNodeL := ast.Node(node.X)

	lValues, err := exp.evaluate(&newNodeL, state) // evaluate the left side before combining with the right
	if err != nil {
		return []Union{}, err
	}
	if len(lValues) == 0 { // not sure if this is technically possible
		return []Union{}, fmt.Errorf("left operand of binary expression has length 0")
	} else if len(lValues) > 1 { // but this definitely is possible
		return []Union{}, fmt.Errorf("left operand of binary expression has more than one element")
	}
	lValue := lValues[0]
	newNodeR := ast.Node(node.Y)
	rValues, err := exp.evaluate(&newNodeR, state) // evaluate the right side before combining with the left

	if err != nil {
		return []Union{}, err
	}
	if len(rValues) == 0 { // not sure if this is technically possible
		return []Union{}, fmt.Errorf("right operand of binary expression has length 0")
	} else if len(rValues) > 1 { // but this definitely is possible
		return []Union{}, fmt.Errorf("right operand of binary expression has more than one element")
	}
	rValue := rValues[0]
	// we can only do string operations between strings
	if lValue.tag == STRING || rValue.tag == STRING {
		if lValue.tag == STRING && rValue.tag == STRING {
			// we can only add and/or compare strings
			if !(node.Op == token.ADD || node.Op == token.EQL || node.Op == token.LSS || node.Op == token.GTR || node.Op == token.NEQ || node.Op == token.LEQ || node.Op == token.GEQ || node.Op == token.LAND || node.Op == token.LOR) {
				return []Union{}, fmt.Errorf("cannot perform binary operation %s between two strings", (node.Op).String())
			}
		} else if !(node.Op == token.ADD || node.Op == token.EQL || node.Op == token.NEQ || node.Op == token.LAND || node.Op == token.LOR) {
			return []Union{}, fmt.Errorf("cannot perform binary operation %s between string and non-string", (node.Op).String())
		}
	}

	// I'm lazy so I'm just using the functions to perform binary operations between left and right operands
	value := Union{}
	switch node.Op {
	case token.ADD:
		value, err = Add(lValue, rValue)
	case token.SUB:
		value, err = Sub(lValue, rValue)
	case token.MUL:
		value, err = Mult(lValue, rValue)
	case token.QUO:
		value, err = Div(lValue, rValue)
	case token.REM:
		value, err = Mod(lValue, rValue)
	case token.AND:
		value, err = BitwiseAnd(lValue, rValue)
	case token.OR:
		value, err = BitwiseOr(lValue, rValue)
	case token.XOR:
		value, err = BitwiseXor(lValue, rValue)
	case token.SHL:
		value, err = LeftShift(lValue, rValue)
	case token.SHR:
		value, err = RightShift(lValue, rValue)
	case token.AND_NOT:
		value, err = BitwiseAndNot(lValue, rValue)
	case token.LAND:
		value, err = And(lValue, rValue)
	case token.LOR:
		value, err = Or(lValue, rValue)
	case token.EQL:
		value, err = Equal(lValue, rValue)
	case token.LSS:
		value, err = LessThan(lValue, rValue)
	case token.GTR:
		value, err = GreaterThan(lValue, rValue)
	case token.NEQ:
		value, err = NotEqual(lValue, rValue)
	case token.LEQ:
		value, err = LessThanOrEqual(lValue, rValue)
	case token.GEQ:
		value, err = GreaterThanOrEqual(lValue, rValue)
	default:
		err = fmt.Errorf("unsupported binary operation: %s", node.Op) // not sure if we can technically get here...
	}

	return []Union{value}, err
}

// evaluate the result of a function call
func (exp *Expression) evaluateCallExpr(node *ast.CallExpr, state *map[string][]Union) ([]Union, error) {
	num_args := len(node.Args)
	id, ok := node.Fun.(*ast.Ident)
	if ok {
		//if statements are a little different since they only evaluate the condition and ONE statement
		if stringInSlice([]string{"if", "ifthen", "ifelse", "ifthenelse", "if_then", "if_else", "if_then_else"}, strings.ToLower(id.Name)) {
			if num_args <= 1 {
				return []Union{}, fmt.Errorf("incorrect number of argments for function evaulation")
			}
			nodeArg := ast.Node(node.Args[0])
			vals, err := exp.evaluate(&nodeArg, state)
			if err != nil {
				return []Union{}, err
			}
			if len(vals) == 0 {
				return []Union{}, fmt.Errorf("no condition given to 'if' statement")
			} else if len(vals) > 1 && num_args == 2 {
				nodeArg := ast.Node(node.Args[1])
				output, err := exp.evaluate(&nodeArg, state)
				outputUnions := make([]Union, 0)
				if len(output) != len(vals) {
					return []Union{}, fmt.Errorf("cannot do variadic pairwise if statement if arguments don't match in size")
				}
				for i, val := range vals {
					if val.b {
						outputUnions = append(outputUnions, output[i])
					}
				}
				return outputUnions, err
			} else if len(vals) > 1 && num_args == 3 {
				nodeArg1 := ast.Node(node.Args[1])
				output1, _ := exp.evaluate(&nodeArg1, state)
				if len(output1) != len(vals) {
					return []Union{}, fmt.Errorf("cannot do variadic pairwise if statement if arguments don't match in size")
				}
				nodeArg2 := ast.Node(node.Args[2])
				output2, err := exp.evaluate(&nodeArg2, state)
				if len(output2) != len(vals) {
					return []Union{}, fmt.Errorf("cannot do variadic pairwise if statement if arguments don't match in size")
				}
				outputUnions := make([]Union, 0)
				for i, val := range vals {
					if val.b {
						outputUnions = append(outputUnions, output1[i])
					} else {
						outputUnions = append(outputUnions, output2[i])
					}
				}
				return outputUnions, err
			} else if len(vals) == 1 {
				val := vals[0]
				if val.b {
					nodeArg := ast.Node(node.Args[1])
					values, err := exp.evaluate(&nodeArg, state)
					return values, err
				} else {
					if num_args == 3 {
						nodeArg := ast.Node(node.Args[2])
						values, err := exp.evaluate(&nodeArg, state)
						return values, err
					} else {
						return []Union{}, nil
					}
				}
			}
		}

		// convert the arguments of the functions into their actual values
		// (not just leaving them as expressions)
		argVals := make([]Union, 0)
		for _, arg := range node.Args {
			nodeArg := ast.Node(arg)
			vals, err := exp.evaluate(&nodeArg, state)
			if err != nil {
				return []Union{}, err
			}
			argVals = append(argVals, vals...)
		}
		var result Union
		var err error
		// pass the arguments to the function specified and return the result
		switch strings.ToLower(id.Name) {
		case "sum", "add":
			result, err = Add(argVals...)
		case "subtract", "sub", "minus":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for subtract function")
			}
			result, err = Sub(argVals[0], argVals[1])
		case "mult", "multiply", "product":
			result, err = Mult(argVals...)
		case "scale", "divide", "quo", "quotient", "div":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for divide function")
			}
			result, err = Div(argVals[0], argVals[1])
		case "mod", "modulus", "modulo":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for mod function")
			}
			result, err = Mod(argVals[0], argVals[1])
		case "bitwiseand", "bitwise_and":
			result, err = BitwiseAnd(argVals...)
		case "bitwiseor", "bitwise_or":
			result, err = BitwiseOr(argVals...)
		case "bitwisexor", "bitwise_xor":
			result, err = BitwiseXor(argVals...)
		case "leftshift", "left_shift", "lsh":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for leftshift function")
			}
			result, err = LeftShift(argVals[0], argVals[1])
		case "rightshift", "right_shift", "rsh":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for rightshift function")
			}
			result, err = RightShift(argVals[0], argVals[1])
		case "bitwiseandnot", "bitwise_and_not", "bitwise_andnot":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for bitwise &^ function")
			}
			result, err = BitwiseAndNot(argVals[0], argVals[1])
		case "and":
			result, err = And(argVals...)
		case "or":
			result, err = Or(argVals...)
		case "not":
			return Not(argVals...)
		case "eq", "equ", "equal", "equalto", "isequal", "equal_to", "is_equal":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for equals function")
			}
			result, err = Equal(argVals...)
		case "neq", "nequ", "nequal", "notequalto", "notequal", "not_equal_to", "not_equal":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for not equals function")
			}
			result, err = NotEqual(argVals...)
		case "lt", "lessthan", "less_than":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for less than function")
			}
			result, err = LessThan(argVals...)
		case "gt", "greaterthan", "greater_than":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for greater than function")
			}
			result, err = GreaterThan(argVals...)
		case "lte", "lessthanorequal", "less_than_or_equal":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for less than or equal function")
			}
			result, err = LessThanOrEqual(argVals...)
		case "gte", "greaterthanorequal", "greater_than_or_equal":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for greater than or equal function")
			}
			result, err = GreaterThanOrEqual(argVals...)
		case "root", "rt":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for root function")
			}
			result, err = Root(argVals[0], argVals[1])
		case "pow", "power", "exp":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for power function")
			}
			result, err = Pow(argVals[0], argVals[1])
		case "max", "maximum":
			result, err = Max(argVals...)
		case "min", "minimum":
			result, err = Min(argVals...)
		case "avg", "average", "mean":
			result, err = Avg(argVals...)
		case "floor":
			return Floor(argVals...)
		case "ceil", "ceiling", "ciel", "cieling": //because people are bad at spelling
			return Ceil(argVals...)
		case "sqrt":
			return Sqrt(argVals...)
		case "pct", "pctOf", "percentof", "pct_of", "percent_of":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for percent function")
			}
			result, err = Pct(argVals[0], argVals[1])
		case "fdiv", "floordiv", "fdivide", "floordivide", "floor_divide":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for floor division function")
			}
			result, err = FloorDiv(argVals[0], argVals[1])
		case "abs", "absolutevalue", "absolute_value":
			return Abs(argVals...)
		case "round":
			return Round(argVals...)
		case "bool", "boolean":
			return Bool(argVals...)
		case "int", "integer":
			return Int(argVals...)
		case "uint", "unsigned_int", "unsigned_integer":
			return UInt(argVals...)
		case "float":
			return Float(argVals...)
		case "string", "str", "tostring":
			return String(argVals...)
		case "integrate":
			if num_args == 1 {
				result, err = Integrate(argVals[0], Union{tag: FLOAT, f: 1.0}, Union{tag: INT, i: -1}, Union{tag: INT, i: 0}, state)
			} else if num_args == 2 {
				result, err = Integrate(argVals[0], argVals[1], Union{tag: INT, i: -1}, Union{tag: INT, i: 0}, state)
			} else if num_args == 3 {
				result, err = Integrate(argVals[0], argVals[1], argVals[2], Union{tag: INT, i: 0}, state)
			} else if num_args == 4 {
				result, err = Integrate(argVals[0], argVals[1], argVals[2], argVals[3], state)
			} else {
				return []Union{}, fmt.Errorf("incorrect number of arguments for integrate function")
			}
		case "currenttimemilliseconds":
			if num_args != 0 {
				return []Union{}, fmt.Errorf("unexpected arguments for CurrentTimeMilliseconds function")
			}
			result, err = CurrentTimeMilliseconds()
		case "time":
			if num_args != 0 {
				return []Union{}, fmt.Errorf("unexpected arguments for Time function")
			}
			result, err = CurrentTimeMilliseconds()
		case "millisecondssince":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for MillisecondsSince function")
			}
			result, err = MillisecondsSince(argVals[0])
		case "millisecondstorfc3339":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for MillisecondsToRFC3339 function")
			}
			result, err = MillisecondsToRFC3339(argVals[0])
		case "rfc3339":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for MillisecondsToRFC3339 function")
			}
			result, err = MillisecondsToRFC3339(argVals[0])
		case "srff":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for srff function")
			}
			result, err = Srff(argVals[0], argVals[1], state)
		case "rss":
			result, err = Rss(argVals...)
		case "selectn":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for select N function")
			}
			result, err = SelectN(argVals[0], argVals[1:]...)
		case "enum":
			if num_args < 3 || ((num_args-1)%2 != 0) {
				return []Union{}, fmt.Errorf("incorrect number of arguments for enum function")
			}
			result, err = Enum(argVals[0], argVals[1:]...)
		case "selectorn":
			if num_args < 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for selector N function")
			}
			result, err = SelectorN(argVals...)
		case "pulse":
			if num_args != 3 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for pulse function")
			}
			result, err = Pulse(argVals[0], argVals[1], argVals[2], state)
		case "compare":
			if num_args < 3 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for compare function")
			}
			return Compare(argVals[0], argVals[1], argVals[2:]...)
		case "compareor":
			if num_args < 3 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for compareor function")
			}
			result, err = CompareOr(argVals[0], argVals[1], argVals[2:]...)
		case "compareand":
			if num_args < 3 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for compareand function")
			}
			result, err = CompareAnd(argVals[0], argVals[1], argVals[2:]...)
		case "maxovertimescale":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for MaxOverTimescale")
			}
			result, err = MaxOverTimescale(argVals[0], argVals[1], state)
		case "minovertimescale":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for MinOverTimescale")
			}
			result, err = MinOverTimescale(argVals[0], argVals[1], state)
		case "avgovertimescale":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for AvgOverTimescale")
			}
			result, err = AvgOverTimescale(argVals[0], argVals[1], state)
		case "sumovertimescale":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for SumOverTimescale")
			}
			result, err = SumOverTimescale(argVals[0], argVals[1], state)
		case "valuechanged":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for valueChanged")
			}
			result, err = ValueChanged(argVals[0], state)
		case "valuechangedovertimescale":
			if num_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for ValueChangedOverTimescale")
			}
			result, err = ValueChangedOverTimescale(argVals[0], argVals[1], state)
		case "quadtosigned":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for QuadToSigned")
			}
			result, err = QuadToSigned(argVals[0])
		case "signedtoquad":
			if num_args != 1 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for SignedToQuad")
			}
			result, err = SignedToQuad(argVals[0])
		case "runtime":
			if num_args == 3 {
				result, err = Runtime(argVals[0], argVals[1], argVals[2], Union{tag: FLOAT, f: 1.0}, Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 0}, Union{tag: FLOAT, f: 0})
			} else if num_args == 4 {
				result, err = Runtime(argVals[0], argVals[1], argVals[2], argVals[3], Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 0}, Union{tag: FLOAT, f: 0})
			} else if num_args == 5 {
				result, err = Runtime(argVals[0], argVals[1], argVals[2], argVals[3], argVals[4], Union{tag: FLOAT, f: 0}, Union{tag: FLOAT, f: 0})
			} else if num_args == 6 {
				result, err = Runtime(argVals[0], argVals[1], argVals[2], argVals[3], argVals[4], argVals[5], argVals[5])
			} else if num_args == 7 {
				result, err = Runtime(argVals[0], argVals[1], argVals[2], argVals[3], argVals[4], argVals[5], argVals[6])
			} else {
				return []Union{}, fmt.Errorf("incorrect number of arguments for runtime function")
			}
		case "unicompare":
			if num_args == 1 {
				result, err = Unicompare(argVals[0], Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false})
			} else if num_args == 2 {
				result, err = Unicompare(argVals[0], argVals[1], Union{tag: BOOL, b: false})
			} else if num_args == 3 {
				result, err = Unicompare(argVals[0], argVals[1], argVals[2])
			} else {
				return []Union{}, fmt.Errorf("incorrect number of arguments for unicompare function")
			}
		case "count":
			result, err = Count(argVals)
		case "combinebits":
			result, err = CombineBits(argVals)
		case "in":
			if num_args < 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for In function")
			}
			result, err = In(argVals[0], argVals[1:])
		case "duration":
			// Check arguments
			if num_eval_args := len(argVals); num_eval_args != 2 {
				return []Union{}, fmt.Errorf("incorrect number of arguments for duration function. Expected 2 but got %d", num_eval_args)
			}
			// TODO: only boolean expression outputs are supported for now
			expressionResult := argVals[0]
			if expressionResult.tag != BOOL {
				if err := castUnionType(&expressionResult, BOOL); err != nil {
					return []Union{}, fmt.Errorf("incorrect type of expression for duration function. Requires boolean but got %s", argVals[0].tag.String())
				}
			}
			durationArg := argVals[1]
			// Internal timing functions use int64 so use the same type to match
			if durationArg.tag != FLOAT {
				if err := castUnionType(&durationArg, FLOAT); err != nil {
					return []Union{}, fmt.Errorf("incorrect type of duration for duration function. Requires float but got %s", argVals[0].tag.String())
				}
			}

			result, err = Duration(expressionResult.b, durationArg.f, state)
			if err != nil {
				return []Union{}, fmt.Errorf("failed to evaluate Duration function, %w", err)
			}
		default:
			return []Union{}, fmt.Errorf("unrecognized function %v", id.Name)
		}
		return []Union{result}, err
	} else {
		return []Union{}, fmt.Errorf("could not evaluate %v", (*node).Fun)
	}
}

// evaluate unary expressions
func (exp *Expression) evaluateUnary(node *ast.UnaryExpr, state *map[string][]Union) ([]Union, error) {

	newValue := ast.Node(node.X)
	rValues, err := exp.evaluate(&newValue, state) // evaluate the operand before applying the unary operator to the result

	if err != nil {
		return []Union{}, err
	}
	if len(rValues) == 0 { // not sure if we can technically get here
		return []Union{}, fmt.Errorf("operand of unary expression has length 0")
	} else if len(rValues) > 1 { // but we can definitely get here
		return []Union{}, fmt.Errorf("operand of unary expression has more than one element")
	}
	rValue := rValues[0]

	if rValue.tag == STRING && node.Op != token.NOT {
		return []Union{}, fmt.Errorf("cannot perform unary operation %s on string", (node.Op).String())
	}

	value := Union{}
	switch node.Op {
	case token.NOT:
		return Not(rValue)
	case token.SUB:
		value, err = Mult(rValue, Union{tag: INT, i: -1})
	default:
		err = fmt.Errorf("unsupported unary operation: %s", node.Op)
	}

	return []Union{value}, err
}

// evaluate numbers as numbers (note that these have to fall within the constraints of node.Kind
// which is why there isn't a BOOL conversion)
func (exp *Expression) evaluateBasicLit(node *ast.BasicLit) ([]Union, error) {
	switch node.Kind {
	case token.INT:
		value, err := strconv.ParseInt(node.Value, 10, 64)
		if err != nil {
			return []Union{}, err
		}
		return []Union{{
			tag: INT,
			i:   value,
		}}, nil
	case token.FLOAT:
		value, err := strconv.ParseFloat(node.Value, 64)
		if err != nil {
			return []Union{}, err
		}
		return []Union{{
			tag: FLOAT,
			f:   value,
		}}, nil
	case token.STRING:
		return []Union{{
			tag: STRING,
			s:   strings.ReplaceAll(node.Value, "\"", ""),
		}}, nil
	default:
		return []Union{}, fmt.Errorf("error evaluating ast.BasicLit %v", node.Value)
	}
}

// Substitute {variables} in the given string with their input values if the inputs exist
// messageString: the string on which to perform the substitution
// return the string with the completed substitution, and any errors that might have occurred
func replaceMessageVarsWithInputValues(messageString string) string {
	// Find all variables surrounded by braces which denotes that they should be substituted
	re := regexp.MustCompile(`{([^}]+)}`)
	parsedString := re.ReplaceAllStringFunc(messageString, func(rawVariable string) string {
		parsedVariable := rawVariable[1 : len(rawVariable)-1]
		if value, ok := InputScope[parsedVariable]; ok {
			return unionValueToString(&value[0])
		}
		// Return the origin string if not found
		return rawVariable
	})
	return parsedString
}

// Determine whether the AST node passed points to any subexpressions within the full expression being received. This is accomplished
// by using the node as a position within the larger expression string. Then, for all nodes that match, add their associated messages
// to the state to be reported later on for the alerting use case
// node: The current AST node being examined
// state: The state containing the subexpressions to match
// return any errors that occurred
func (exp *Expression) trackActiveSubexpressions(node *ast.Node, state *map[string][]Union, values *[]Union) error {
	// Early return if there are no expressions to match
	if state == nil || len((*state)["messageExpressions"]) == 0 {
		return nil
	}
	// Two parallel slices track the expressions and their associated messages
	// For instance, the expression might be "soc < 10" and the associated message to report might be "soc is too low"
	// Ensure the lengths match
	if len((*state)["messageExpressions"]) != len((*state)["messageStrings"]) {
		return fmt.Errorf("mismatch between message configuration: got %d expressions but %d strings", len((*state)["messageExpressions"]), len((*state)["messageStrings"]))
	}

	// Only proceed if the expression is true
	if len(*values) == 0 {
		return nil
	}
	expressionIsTrue := (*values)[0]
	if err := castUnionType(&expressionIsTrue, BOOL); err != nil {
		return fmt.Errorf("couldn't determine whether the expression is true: %w", err)
	}

	// Protect against out of bounds
	// ast node positions are offset by 1 character
	startingPos := int((*node).Pos()) - 1
	endingPos := int((*node).End()) - 1
	if startingPos < 0 || endingPos > len(exp.String) {
		return fmt.Errorf("ast node out of range for expression %s. There is a problem with the code", exp.String)
	}

	// Add any active messages that are currently matched if their value has gone from false to true
	for index, subexpression := range (*state)["messageExpressions"] {
		if exp.String[startingPos:endingPos] == subexpression.s {
			if expressionIsTrue.b {
				if index >= len((*state)["previousExpressionValues"]) {
					return fmt.Errorf("failed to get previous expression value for expression %s. There is a problem with the code", exp.String)
				}
				// Value has changed from false to true
				if !(*state)["previousExpressionValues"][index].b {
					parsedMsg := replaceMessageVarsWithInputValues((*state)["messageStrings"][index].s)
					// State holds a slice for each key rather than a map, so split the strings and timestamps into separate
					// slices with corresponding indices
					// The maps are cleared before each evaluation so these will not build up over time
					(*state)["activeMessages"] = append((*state)["activeMessages"], Union{tag: STRING, s: parsedMsg})
					(*state)["activeTimestamps"] = append((*state)["activeTimestamps"], Union{tag: STRING, s: time.Now().Format(time.RFC3339)})
				}
			}
			// Update previous value
			(*state)["previousExpressionValues"][index] = expressionIsTrue
		}
	}

	return nil

}
