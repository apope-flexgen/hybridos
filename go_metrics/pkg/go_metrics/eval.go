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
			ProcessFims(<-fimsMap) // this is blocking, so we won't create a bajillion instances of the goroutine below
		}
	}()
	t0 := time.Now()
	MDOtick := time.NewTicker(10000 * time.Millisecond)
	for j := 0; j < len(tickers); j += 1 {
		go func(tickerIndex int) {
			for {
				select {
				case <-tickers[tickerIndex].C:
					inputsMutex.Lock()
					EvaluateExpressions()
					inputsMutex.Unlock()
					pubDataChangedMutex.Lock()
					for _, pubUri := range tickerPubs[tickerIndex] {
						pubDataChanged[pubUri] = true
					}
					temp_uri := ""
					for uriGroup, _ := range PublishUris {
						if len(uriGroup) > 0 {
							if needsPub, _ := pubDataChanged[uriGroup]; needsPub {
								msgBodyMutex.Lock()
								PrepareBody(uriGroup)
								if strings.Contains(uriGroup, "[") {
									temp_uri = uriGroup[0:strings.Index(uriGroup, "[")]
								} else {
									temp_uri = uriGroup
								}

								if uriIsSet[uriGroup] && (len(setMsgBody) > 0 || uriHeartbeat[uriGroup]) {
									if uriIsLonely[uriGroup] {
										outputsMutex.RLock()
										lonelyVarName := uriGroup[strings.Index(uriGroup, "[")+1 : strings.Index(uriGroup, "]")]
										if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
											lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
										}
										outputsMutex.RUnlock()
										_, err = f.Send(fims.FimsMsg{
											Method: "set",
											Uri:    temp_uri + "/" + lonelyVarName,
											Body:   setMsgBody[lonelyVarName],
										})
									} else {
										_, err = f.Send(fims.FimsMsg{
											Method: "set",
											Uri:    temp_uri,
											Body:   setMsgBody,
										})
									}
								} else if len(pubMsgBody) > 0 || uriHeartbeat[uriGroup] {
									if uriIsLonely[uriGroup] {
										outputsMutex.RLock()
										lonelyVarName := uriGroup[strings.Index(uriGroup, "[")+1 : strings.Index(uriGroup, "]")]
										if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
											lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
										}
										outputsMutex.RUnlock()
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
								msgBodyMutex.Unlock()
								pubDataChanged[uriGroup] = false
							}

						}
					}
					pubDataChangedMutex.Unlock()
				}
			}
		}(j)
	}
	go func() {
		if len(MetricsConfig.Echo) == 0 {
			return
		}
		for {
			for echoIndex, _ := range MetricsConfig.Echo {
				select {
				case <-MetricsConfig.Echo[echoIndex].Ticker.C:
					if len(MetricsConfig.Echo[echoIndex].Heartbeat) > 0 {
						MetricsConfig.Echo[echoIndex].Echo[MetricsConfig.Echo[echoIndex].Heartbeat] = float64(time.Since(t0)) / 1000000000.0
					}
					if MetricsConfig.Echo[echoIndex].Format == "naked" {
						echoMsgBodyMutex.Lock()
						echoMsgBody = make(map[string]interface{}, 0)
						for key, value := range MetricsConfig.Echo[echoIndex].Echo {
							switch value.(type) {
							case map[string]interface{}:
								if value, ok := value.(map[string]interface{})["value"]; ok {
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
									outputElementMutex.Lock()
									json.Unmarshal([]byte(fmt.Sprintf("{\"value\":\"%v\"}", value)), &outputElementValue)
									echoMsgBody[key] = outputElementValue
									outputElementMutex.Unlock()
								default:
									outputElementMutex.Lock()
									json.Unmarshal([]byte(fmt.Sprintf("{\"value\":%v}", value)), &outputElementValue)
									echoMsgBody[key] = outputElementValue
									outputElementMutex.Unlock()
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
				}
			}
		}
	}()

	for {
		select {
		case <-MDOtick.C:
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
				inputsMutex.RLock()
				for key, _ := range Mdo.Inputs {
					mdoinput = MetricsConfig.Inputs[key]
					Mdo.Inputs[key]["value"] = getValueFromUnion(&(mdoinput.Value))
					for _, attribute := range MetricsConfig.Inputs[key].Attributes {
						scopeMutex.RLock()
						Mdo.Inputs[key][attribute] = getValueFromUnion(&(Scope[key+"@"+attribute][0].Value))
						scopeMutex.RUnlock()
					}
				}
				inputsMutex.RUnlock()
				filtersMutex.RLock()
				for key, _ := range MetricsConfig.Filters {
					if len(dynamicFilterExpressions[key].DynamicInputs) > 0 {
						copy(Mdo.Filters[key], dynamicFilterExpressions[key].DynamicInputs[len(dynamicFilterExpressions[key].DynamicInputs)-1])
					} else if len(staticFilterExpressions[key].DynamicInputs) > 0 {
						copy(Mdo.Filters[key], staticFilterExpressions[key].DynamicInputs[len(staticFilterExpressions[key].DynamicInputs)-1])
					} else {
						Mdo.Filters[key] = make([]string, 0)
					}
				}
				filtersMutex.RUnlock()
				outputsMutex.RLock()
				for key, _ := range MetricsConfig.Outputs {
					mdooutput = MetricsConfig.Outputs[key]
					Mdo.Outputs[key]["value"] = getValueFromUnion(&(mdooutput.Value))
					for attribute, attributeVal := range mdooutput.Attributes {
						Mdo.Outputs[key][attribute] = attributeVal
					}
				}
				outputsMutex.RUnlock()
				for k, _ := range MetricsConfig.Metrics {
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
									for q, _ := range stateVal[0:currentIndex] {
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
				for k, _ := range MetricsConfig.Echo {
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
}

func ProcessDirectSets() {
	// inputsMutex.Lock()
	EvaluateExpressions()
	// inputsMutex.Unlock()
	directSetMutex.Lock()
	for directSetUriGroup, active := range uriToDirectSetActive {
		if active {
			pubDataChangedMutex.Lock()
			msgBodyMutex.Lock()
			PrepareBody(directSetUriGroup)
			if strings.Contains(directSetUriGroup, "[") {
				directSetUri = directSetUriGroup[0:strings.Index(directSetUriGroup, "[")]
			} else {
				directSetUri = directSetUriGroup
			}

			if len(setMsgBody) > 0 {
				if uriIsLonely[directSetUriGroup] {
					outputsMutex.RLock()
					lonelyVarName := directSetUriGroup[strings.Index(directSetUriGroup, "[")+1 : strings.Index(directSetUriGroup, "]")]
					if _, ok := MetricsConfig.Outputs[lonelyVarName]; ok && len(MetricsConfig.Outputs[lonelyVarName].Name) > 0 {
						lonelyVarName = MetricsConfig.Outputs[lonelyVarName].Name
					}
					outputsMutex.RUnlock()
					f.Send(fims.FimsMsg{
						Method: "set",
						Uri:    (directSetUri + "/" + lonelyVarName),
						Body:   setMsgBody[lonelyVarName],
					})
				} else {
					f.Send(fims.FimsMsg{
						Method: "set",
						Uri:    directSetUri,
						Body:   setMsgBody,
					})
				}
			}
			msgBodyMutex.Unlock()
			pubDataChanged[directSetUriGroup] = false
			uriToDirectSetActive[directSetUriGroup] = false
			pubDataChangedMutex.Unlock()
		}
	}
	directSetMutex.Unlock()
}

func EvaluateExpressions() {
	evalExpressionsTiming.start()
	filterNeedsEvalMutex.RLock()
	filterNeedsEvalCopy := make(map[string]bool, len(filterNeedsEval))
	for key, val := range filterNeedsEval {
		filterNeedsEvalCopy[key] = val
	}
	filterNeedsEvalMutex.RUnlock()
	for filterName, needsEval := range filterNeedsEvalCopy {
		if needsEval {
			// this loop for static filters is actually necessary because inputs can change values
			if filter, ok := staticFilterExpressions[filterName]; ok {
				scopeMutex.Lock()
				Scope[filterName] = make([]Input, 0)
				scopeMutex.Unlock()
				for _, dynamicInput := range filter.DynamicInputs[0] {
					scopeMutex.Lock()
					Scope[filterName] = append(Scope[filterName], Scope[dynamicInput]...)
					scopeMutex.Unlock()
				}
			}
			if filter, ok := dynamicFilterExpressions[filterName]; ok {
				filtersMutex.RLock()
				intermediateInputs := filter.DynamicInputs[0]
				filtersMutex.RUnlock()
				for p, exp := range filter.DynamicFilterExpressions {
					stringArr, _ := EvaluateDynamicFilter(&(exp.Ast), intermediateInputs)
					intermediateInputs = stringArr
					filtersMutex.Lock()
					copy(dynamicFilterExpressions[filterName].DynamicInputs[p+1], intermediateInputs)
					filtersMutex.Unlock()
					if len(intermediateInputs) == 0 {
						break
					}
				}
				scopeMutex.Lock()
				Scope[filterName] = make([]Input, 0)
				scopeMutex.Unlock()
				for _, str := range intermediateInputs {
					scopeMutex.Lock()
					Scope[filterName] = append(Scope[filterName], Scope[str]...)
					scopeMutex.Unlock()

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
	if len(MetricsConfig.Metrics) > 0 {
		metricsMutex[0].RLock()
	}
	for q, metricsObject := range MetricsConfig.Metrics {
		metricsMutex[q].RUnlock()
		expressionNeedsEvalMutex.RLock()
		needEval := expressionNeedsEval[q]
		expressionNeedsEvalMutex.RUnlock()
		if needEval {
			var originalType DataType
			originalType = metricsObject.Type
			outputVal, err := Evaluate(&(metricsObject.ParsedExpression), &(metricsObject.State))
			if err != nil {
				log.Warnf("Error with metrics expression  [   %s   ]:\n%s\n", metricsObject.Expression, err)
			}

			if err == nil && outputVal.tag != NIL {
				if originalType != NIL {
					castUnionType(&outputVal, originalType)
				}
				for _, outputVar := range metricsObject.Outputs {
					if strings.Contains(outputVar, "@") {
						splitOutputVar := strings.Split(outputVar, "@")
						if len(splitOutputVar) == 2 {
							outputVar = splitOutputVar[0]
							attribute := splitOutputVar[1]
							tempValue = getValueFromUnion(&outputVal)
							directSetMutex.RLock()
							_, is_direct_set := uriToDirectSetActive[outputToUriGroup[outputVar]]
							directSetMutex.RUnlock()
							is_sparse := uriIsSparse[outputToUriGroup[outputVar]]
							outputsMutex.Lock()
							if MetricsConfig.Outputs[outputVar].Attributes[attribute] != tempValue || (is_direct_set && !is_sparse) {
								pubDataChangedMutex.Lock()
								outputVarChanged[outputVar] = true
								pubDataChanged[outputToUriGroup[outputVar]] = true
								pubDataChangedMutex.Unlock()
								directSetMutex.Lock()
								if is_direct_set {
									uriToDirectSetActive[outputToUriGroup[outputVar]] = true
								}
								directSetMutex.Unlock()
								if debug {
									if stringInSlice(debug_outputs, outputVar) {
										log.Debugf("Output [%s] attribute [%s] changed value to [%v]", outputVar, attribute, tempValue)
									}
								}
							}
							MetricsConfig.Outputs[outputVar].Attributes[attribute] = tempValue
							outputsMutex.Unlock()
						} else {
							log.Errorf("Something went wrong when trying to change the value of " + outputVar)
						}
					} else {
						outputsMutex.RLock()
						output = MetricsConfig.Outputs[outputVar]
						outputsMutex.RUnlock()
						directSetMutex.RLock()
						_, is_direct_set := uriToDirectSetActive[outputToUriGroup[outputVar]]
						directSetMutex.RUnlock()
						is_sparse := uriIsSparse[outputToUriGroup[outputVar]]
						if output.Value != outputVal || (is_direct_set && !is_sparse) {
							pubDataChangedMutex.Lock()
							outputVarChanged[outputVar] = true
							pubDataChanged[outputToUriGroup[outputVar]] = true
							pubDataChangedMutex.Unlock()
							directSetMutex.Lock()
							if is_direct_set {
								uriToDirectSetActive[outputToUriGroup[outputVar]] = true
							}
							directSetMutex.Unlock()
							if debug {
								if stringInSlice(debug_outputs, outputVar) {
									log.Debugf("Output [%s] changed value to [%v]", outputVar, getValueFromUnion(&outputVal))
								}
							}
						}
						output.Value = outputVal
						outputsMutex.Lock()
						MetricsConfig.Outputs[outputVar] = output
						outputsMutex.Unlock()
					}
				}

				if len(metricsObject.InternalOutput) > 0 {
					scopeMutex.RLock()
					input = Scope[metricsObject.InternalOutput][0]
					scopeMutex.RUnlock()
					if input.Value != outputVal {
						for _, expNum := range inputToMetricsExpression[metricsObject.InternalOutput] {
							expressionNeedsEvalMutex.Lock()
							expressionNeedsEval[expNum] = true
							expressionNeedsEvalMutex.Unlock()
						}
						if debug {
							if stringInSlice(debug_inputs, metricsObject.InternalOutput) {
								log.Debugf("Internal input [%s] changed value to [%v]", metricsObject.InternalOutput, getValueFromUnion(&outputVal))
							}
						}
					}
					input.Value = outputVal
					scopeMutex.Lock()
					Scope[metricsObject.InternalOutput] = []Input{input}
					scopeMutex.Unlock()
				}
			}

			metricsObject.State["value"][0] = outputVal
			metricsMutex[q].Lock()
			MetricsConfig.Metrics[q] = metricsObject
			metricsMutex[q].Unlock()
			if !metricsObject.State["alwaysEvaluate"][0].b {
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
}

/**
* This function takes a parsed expression and map of input variables to their
* run-time values. Calls an auxiliary function to do the dirty work.
 */
// TODO: Handle filter functions ("aliases") as input types
func Evaluate(parsed *Expression, state *map[string][]Union) (Union, error) {

	result, err := evaluate(&(parsed.Ast), state)

	if err != nil || len(result) == 0 {
		return Union{}, err
	}

	return result[0], nil
}

// Looks at a node in an AST and determines the type of the node.
// Based on the type, it calls an auxiliary function to do the actual evaluation.
func evaluate(node *ast.Node, state *map[string][]Union) (values []Union, err error) {
	switch (*node).(type) {
	case *ast.Ident:
		values, err = evaluateIdent((*node).(*ast.Ident))
	case *ast.CallExpr:
		values, err = evaluateCallExpr((*node).(*ast.CallExpr), state)
	case *ast.BinaryExpr:
		values, err = evaluateBinary((*node).(*ast.BinaryExpr), state)
	case *ast.ParenExpr:
		newNode := ast.Node((*node).(*ast.ParenExpr).X)
		values, err = evaluate(&newNode, state)
	case *ast.UnaryExpr:
		values, err = evaluateUnary((*node).(*ast.UnaryExpr), state)
	case *ast.BasicLit:
		values, err = evaluateBasicLit((*node).(*ast.BasicLit))
	case *ast.SelectorExpr:
		values, err = evaluateSelector((*node).(*ast.SelectorExpr))
	default:
		err = fmt.Errorf("unsupported node %+v (type %+v)", *node, reflect.TypeOf(*node))
	}

	return values, err
}

// pull a variable out of the map of inputs
func evaluateIdent(node *ast.Ident) ([]Union, error) {
	if node.Name == "true" {
		return []Union{Union{tag: BOOL, b: true}}, nil
	} else if node.Name == "false" {
		return []Union{Union{tag: BOOL, b: false}}, nil
	}
	scopeMutex.RLock()
	inputs, found := Scope[node.Name]
	scopeMutex.RUnlock()
	if !found {
		return []Union{}, fmt.Errorf("could not find variable %v in Scope", node.Name)
	}
	values := make([]Union, len(inputs))
	for i, input := range inputs {
		values[i] = input.Value
	}
	if len(values) == 0 {
		values = []Union{Union{}} // if the variable exists, but doesn't have a value, give it a default value of 0
	}
	return values, nil
}

// pull a variable out of the map of inputs
func evaluateSelector(node *ast.SelectorExpr) ([]Union, error) {
	nodeName := node.X.(*ast.Ident).Name + "@" + node.Sel.Name
	scopeMutex.RLock()
	inputs, found := Scope[nodeName]
	scopeMutex.RUnlock()
	if !found {
		return []Union{}, fmt.Errorf("could not find variable %v in Scope", nodeName)
	}
	values := make([]Union, len(inputs))
	for i, input := range inputs {
		values[i] = input.Value
	}
	if len(values) == 0 {
		values = []Union{Union{}} // if the variable exists, but doesn't have a value, give it a default value of 0
	}
	return values, nil
}

// evaluate the result of a binary operation
func evaluateBinary(node *ast.BinaryExpr, state *map[string][]Union) ([]Union, error) {
	newNodeL := ast.Node(node.X)

	lValues, err := evaluate(&newNodeL, state) // evaluate the left side before combining with the right
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
	rValues, err := evaluate(&newNodeR, state) // evaluate the right side before combining with the left

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
func evaluateCallExpr(node *ast.CallExpr, state *map[string][]Union) ([]Union, error) {
	num_args := len(node.Args)
	id, ok := node.Fun.(*ast.Ident)
	if ok {
		//if statements are a little different since they only evaluate the condition and ONE statement
		if stringInSlice([]string{"if", "ifthen", "ifelse", "ifthenelse", "if_then", "if_else", "if_then_else"}, strings.ToLower(id.Name)) {
			if num_args <= 1 {
				return []Union{}, fmt.Errorf("incorrect number of argments for function evaulation")
			}
			nodeArg := ast.Node(node.Args[0])
			vals, err := evaluate(&nodeArg, state)
			if err != nil {
				return []Union{}, err
			}
			if len(vals) == 0 {
				return []Union{}, fmt.Errorf("no condition given to 'if' statement")
			} else if len(vals) > 1 && num_args == 2 {
				nodeArg := ast.Node(node.Args[1])
				output, err := evaluate(&nodeArg, state)
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
				output1, err := evaluate(&nodeArg1, state)
				if len(output1) != len(vals) {
					return []Union{}, fmt.Errorf("cannot do variadic pairwise if statement if arguments don't match in size")
				}
				nodeArg2 := ast.Node(node.Args[2])
				output2, err := evaluate(&nodeArg2, state)
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
					values, err := evaluate(&nodeArg, state)
					return values, err
				} else {
					if num_args == 3 {
						nodeArg := ast.Node(node.Args[2])
						values, err := evaluate(&nodeArg, state)
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
			vals, err := evaluate(&nodeArg, state)
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
		default:
			return []Union{}, fmt.Errorf("unrecognized function %v", id.Name)
		}
		return []Union{result}, err
	} else {
		return []Union{}, fmt.Errorf("could not evaluate %v", (*node).Fun)
	}
}

// evaluate unary expressions
func evaluateUnary(node *ast.UnaryExpr, state *map[string][]Union) ([]Union, error) {

	newValue := ast.Node(node.X)
	rValues, err := evaluate(&newValue, state) // evaluate the operand before applying the unary operator to the result

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
func evaluateBasicLit(node *ast.BasicLit) ([]Union, error) {
	switch node.Kind {
	case token.INT:
		value, err := strconv.ParseInt(node.Value, 10, 64)
		if err != nil {
			return []Union{}, err
		}
		return []Union{Union{
			tag: INT,
			i:   value,
		}}, nil
	case token.FLOAT:
		value, err := strconv.ParseFloat(node.Value, 64)
		if err != nil {
			return []Union{}, err
		}
		return []Union{Union{
			tag: FLOAT,
			f:   value,
		}}, nil
	case token.STRING:
		return []Union{Union{
			tag: STRING,
			s:   strings.ReplaceAll(node.Value, "\"", ""),
		}}, nil
	default:
		return []Union{}, fmt.Errorf("error evaluating ast.BasicLit %v", node.Value)
	}
}
