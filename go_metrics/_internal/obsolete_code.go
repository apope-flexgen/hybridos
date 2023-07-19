package main

// /**
//  * This function parses an "expression array" (an array that represents an algebraic expression)
//  * into a string format.
//  *
//  * For example, given ["3", "+", "5", "+", {"avg": ["3","4","5"]}], ParseToString would
//  * output "3+5+Avg(3,4,5)".
//  */
//  func ParseToString(expressionArray *[]interface{}) (string, error) {
// 	var wg sync.WaitGroup //not really sure if multithreading is necessary here...

// 	// minimize each element of the array to a single string
// 	for i, expression := range *expressionArray {
// 		if _, ok := expression.(string); !ok {
// 			wg.Add(1)
// 			go func(i int, exp interface{}) {
// 				defer wg.Done()
// 				switch exp := exp.(type) {
// 				case map[string]interface{}: // these are meant to represent functions
// 					for key, val := range exp { // there should really only be one key and an associated list
// 						valArr, _ := val.([]interface{})
// 						answer, err := evalFunction(key, &valArr)
// 						if err != nil {
// 							answer = ""
// 						}
// 						(*expressionArray)[i] = answer
// 					} //end looping through function map
// 				case []interface{}: // these are usually the arguments provided to a function
// 					answer, err := ParseToString(&exp)
// 					if err != nil {
// 						answer = ""
// 					}
// 					(*expressionArray)[i] = answer
// 				case string: // not entirely sure why we get here, but we do...
// 					(*expressionArray)[i] = exp
// 				default:
// 					fmt.Printf("Unknown type %v\n", reflect.TypeOf(exp))
// 					(*expressionArray)[i] = ""
// 				}

// 			}(i, expression) //end goroutine
// 		} //end "if expression is not a string" block
// 	} //end iterating through expressions in expressionArray
// 	wg.Wait()

// 	containsBinaryOperator := false // if an array of strings contains NO binary operator, add commas between terms (assuming the array is a list of arguments to a function)
// 	containsEmptyString := false    // if an empty string exists, the expression was likely parsed incorrectly
// 	strExpressionArray := make([]string, len(*expressionArray))
// 	for i, val := range *expressionArray {
// 		str, _ := val.(string)
// 		strExpressionArray[i] = str
// 		if strings.ContainsAny(str, "+-*/&|!~^<>=%") {
// 			containsBinaryOperator = true
// 		}
// 		if len(str) == 0 {
// 			containsEmptyString = true
// 		}
// 	}

// 	if !containsEmptyString {
// 		if containsBinaryOperator {
// 			return strings.Join(strExpressionArray, ""), nil
// 		} else {
// 			return strings.Join(strExpressionArray, ","), nil
// 		}
// 	} else {
// 		if containsBinaryOperator {
// 			return strings.Join(strExpressionArray, ""), fmt.Errorf("one or more terms is an empty string. Expression %s likely contains an error", strings.Join(strExpressionArray, ""))
// 		} else {
// 			return strings.Join(strExpressionArray, ","), fmt.Errorf("one or more terms is an empty string. Expression %s likely contains an error", strings.Join(strExpressionArray, ","))
// 		}
// 	}

// }

// // When parsing an array into a string expression, this function
// // converts common "functions" into a standard form of FunctionName(arguments).
// // Accepts a number of abbreviations and alternative function calls.
// func evalFunction(function string, expressionArray *[]interface{}) (string, error) {
// 	switch strings.ToLower(function) {

// 	case "root", "rt":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Root(%s)", inside), nil
// 	case "pow", "power", "exp":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Pow(%s)", inside), nil
// 	case "sum", "add":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Add(%s)", inside), nil
// 	case "mult", "multiply":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Mult(%s)", inside), nil
// 	case "max", "maximum":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Max(%s)", inside), nil
// 	case "min", "minimum":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Min(%s)", inside), nil
// 	case "floor":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Floor(%s)", inside), nil
// 	case "ceil", "ceiling", "ciel", "cieling": //because people are bad at spelling
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Ceil(%s)", inside), nil
// 	case "avg", "average", "mean":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Avg(%s)", inside), nil
// 	case "sqrt":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Sqrt(%s)", inside), nil
// 	case "lt", "lessthan", "less_than":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("LessThan(%s)", inside), nil
// 	case "gt", "greaterthan", "greater_than":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("GreaterThan(%s)", inside), nil
// 	case "lte", "lessthanorequal", "less_than_or_equal":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("LessThanOrEqual(%s)", inside), nil
// 	case "gte", "greaterthanorequal", "greater_than_or_equal":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("GreaterThanOrEqual(%s)", inside), nil
// 	case "eq", "equ", "equal", "equalto", "isequal", "equal_to", "is_equal":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Equal(%s)", inside), nil
// 	case "neq", "nequ", "nequal", "notequalto", "notequal", "not_equal_to", "not_equal":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("NotEqual(%s)", inside), nil
// 	case "scale", "divide", "quo", "quotient", "div":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Div(%s)", inside), nil
// 	case "subtract", "sub", "minus":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Sub(%s)", inside), nil
// 	case "pctOf", "percentof", "pct_of", "percent_of":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Pct(%s)", inside), nil
// 	case "if", "ifthen", "ifelse", "ifthenelse", "if_then", "if_else", "if_then_else":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			err := errCheck(len(*expressionArray), 3, function)
// 			if err != nil {
// 				return "", nil
// 			}
// 			inside, err := ParseToString(expressionArray)
// 			if err != nil {
// 				return "", err
// 			}
// 			return fmt.Sprintf("IfElse(%s)", inside), nil
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("If(%s)", inside), nil
// 	case "mod", "modulus", "modulo":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Mod(%s)", inside), nil
// 	case "fdiv", "floordiv", "fdivide", "floordivide", "floor_divide":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("FloorDiv(%s)", inside), nil
// 	case "and":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("And(%s)", inside), nil
// 	case "or":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Or(%s)", inside), nil
// 	case "not", "neg":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Not(%s)", inside), nil
// 	case "bool", "boolean":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Bool(%s)", inside), nil
// 	case "int", "integer":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Int(%s)", inside), nil
// 	case "uint", "unsigned int", "unsigned integer":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("UInt(%s)", inside), nil
// 	case "float":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Float(%s)", inside), nil
// 	case "string", "str", "tostring":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("String(%s)", inside), nil
// 	case "attribute":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Attribute(%s)", inside), nil
// 	case "integrate":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Integrate(%s)", inside), nil
// 	case "integrateovertimescale":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("IntegrateOverTimescalse(%s)", inside), nil
// 	case "currenttimemilliseconds":
// 		err := errCheck(len(*expressionArray), 0, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("CurrentTimeMilliseconds(%s)", inside), nil
// 	case "time":
// 		err := errCheck(len(*expressionArray), 0, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("CurrentTimeMilliseconds(%s)", inside), nil
// 	case "millisecondssince":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("MillisecondsSince(%s)", inside), nil
// 	case "millisecondstorfc3339":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("MillisecondsToRFC3339(%s)", inside), nil
// 	case "rfc3339":
// 		err := errCheck(len(*expressionArray), 1, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("MillisecondsToRFC3339(%s)", inside), nil
// 	case "enum":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Enum(%s)", inside), nil
// 	case "srff":
// 		err := errCheck(len(*expressionArray), 2, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Srff(%s)", inside), nil
// 	case "rss":
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Rss(%s)", inside), nil
// 	case "pulse":
// 		err := errCheck(len(*expressionArray), 3, function)
// 		if err != nil {
// 			return "", err
// 		}
// 		inside, err := ParseToString(expressionArray)
// 		if err != nil {
// 			return "", err
// 		}
// 		return fmt.Sprintf("Pulse(%s)", inside), nil
// 	default:
// 		return "", fmt.Errorf("%s is not a valid command", function)
// 	}
// }

// // Checks if number of parameters passed in is the correct amount. Returns an error if not true
// func errCheck(vars int, needed int, cmd string) error {
// 	if vars != needed {
// 		return fmt.Errorf("\"%s\" command has an incorrect amount of parameters; has %d, needs %d", cmd, vars, needed)
// 	}
// 	return nil
// }

// // list of string filters are for regexp only right now...
// func ParseStringFilterList(filters []string, inputs *map[string]Input) []Union {
// 	output := make([]Union, 0)
// 	for _, filter := range filters {
// 		regEx, err := regexp.Compile(filter)
// 		if err != nil {
// 			log.Println("Could not parse regular expression. See https://github.com/google/re2/wiki/Syntax for help constructing.")
// 		}

// 		for input, _ := range *inputs {
// 			if match := regEx.FindString(input); len(match) > 0 {
// 				output = append(output, (*inputs)[input].Value)
// 			}
// 		}
// 	}
// 	return output
// }

//OLD PROCESS FIMS STUFF THAT MIGHT STILL BE USEFUL OR NECESSARY TO INCLUDE
//
// 			//This set reply may be overly broad since while the set will be for
// 			// a URI that metrics cares about (from the subscribes), it may be for a value
// 			// that metrics is not responsible for (BUT THIS IS WHAT METRICS DOES SO THERE)
// 			if msg.Method == "set" && len(msg.Replyto) > 0 {
// 				f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msg.Body})
// 			}
// 		}
// 	}
// 	// } else if msg.Method == "get" && len(msg.Replyto) > 0 {
// 	// 	// find metric in outputs and set back out
// 	// 	if outputVar, ok := uriToOutputNameMap[msg.Uri]; ok {
// 	// 		output := metricsConfig.Outputs[outputVar]
// 	// 		val := getValueFromUnion(&(output.Value))
// 	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{GetUriElement(msg.Uri): val}})
// 	// 	} else if _, ok := publishUris[msg.Uri]; ok {
// 	// 		msgBody := prepareBody(msg.Uri)
// 	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
// 	// 	}
// 	// } else if msg.Method == "del" {
// 	// 	if outputVar, ok := uriToOutputNameMap[msg.Uri]; ok {
// 	// 		initialVal := initialValues[outputVar]
// 	// 		output := metricsConfig.Outputs[outputVar]
// 	// 		output.Value = getUnionFromValue(&initialVal)
// 	// 		metricsConfig.Outputs[outputVar] = output
// 	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: map[string]interface{}{GetUriElement(msg.Uri): initialVal}})
// 	// 	} else if _, ok := publishUris[msg.Uri]; ok {
// 	// 		msgBody := resetBody(msg.Uri)
// 	// 		f.Send(fims.FimsMsg{Method: "set", Uri: msg.Replyto, Replyto: "", Body: msgBody})
// 	// 	}
// 	// }
// }

// // Given a "filter function", use regex to find matching Input values
// // Need to think about how to implement this better...
// // https://github.com/google/re2/wiki/Syntax for regex syntax
// // TODO: make regexIn a map[string]Interface
// func extractRegularExpressions(regexIn *map[string]string, inputs *map[string]Input) map[string][]Input {
// 	output := make(map[string][]Input, 0)

// 	for key, exp := range *regexIn {
// 		output[key] = make([]Input, 0)
// 		regEx, err := regexp.Compile(exp)
// 		if err != nil {
// 			log.Fatal("Could not parse regular expression. See https://github.com/google/re2/wiki/Syntax for help constructing.")
// 		}

// 		for input, val := range *inputs {
// 			if match := regEx.FindString(input); len(match) > 0 {
// 				output[key] = append(output[key], val)
// 			}
// 		}
// 	}

// 	return output

// }

// /*
//  * clean up URI for easier storage and process
//  * removes any edge spaces
//  * removes tailing '/' to avoid complexities
//  * ensures URI starts with '/' and ALWAYS ends with alphanumeric
//  */
//  func ProcessUri(str string) string {
// 	ind := len(str) - 1
// 	found := false
// 	for ind > 0 {
// 		if str[ind] == '/' || str[ind] == ' ' {
// 			found = true
// 			ind--
// 		} else {
// 			break
// 		}
// 	}
// 	var retVal string = str
// 	if found {
// 		retVal = str[0:(ind + 1)]
// 	}
// 	retVal = strings.TrimSpace(retVal)
// 	if retVal[0] != '/' {
// 		retVal = "/" + retVal
// 	}
// 	return retVal
// }

// func resetBody(outputUri string) map[string]interface{} {
// 	msgBody := make(map[string]interface{}, len(publishUris[outputUri]))
// 	for _, child := range publishUris[outputUri] {
// 		fullUri := outputUri + "/" + child
// 		outputVar := uriToOutputNameMap[fullUri]
// 		output := metricsConfig.Outputs[outputVar]
// 		intialVal := initialValues[outputVar]
// 		output.Value = getUnionFromValue(&intialVal)
// 		metricsConfig.Outputs[outputVar] = output
// 		elementName := GetUriElement(output.Uri)
// 		msgBody[elementName] = intialVal
// 	}
// 	return msgBody
// }
