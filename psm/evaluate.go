package main

import (
	"fmt"
	"log"
	"math"
	"reflect"
	"strings"
)

//expressions contains a map of asset_id keys whose values are maps of variable name keys whose values are pointers to the
//asset_id:variable_name:[expression] or asset_id:variable_name:[[exp1], [exp2], ..., [expn]]
var expressions map[string]map[string][]interface{}
var dt interface{} //So we only have to find dT once and can keep re-using it since it is the same for all assets


//order of operations given numerical representation of precidence. Used to compare operation precidence
//iota assigns a unique integer constant to each member which is one more than the last constant assigned so order here matters. 
const (
	MINUS		int = iota
	PLUS		int = MINUS //same precidence as subtraction
	DIVIDE		int = iota
	MULT		int = DIVIDE //same precidence as division
	EXPONENT	int = iota
	LPAREN		int = iota //parens will be different priority to easily find lparen and rparen 
	RPAREN		int = iota
)
const MAX_DEPTH int = 10
// q.IsEmpty() checks if a given queue is empty
func (q *Queue) IsEmpty() bool {
	return len(*q) == 0
}
// Stack, Queue, and related methods for shunting yard algorithm and post-fix stack evaluation. 
type Stack []interface{} //LIFO
type Queue []interface{} //FIFO
// q.Enqueue adds an element elem to the end of the queue (nth element)
func (q *Queue) Enqueue(elem interface{}) {
	*q = append(*q, elem)
}
// q.Dequeue returns and removes the element at the beginning of the queue (1st element), or returns failure if unable to do so
func (q *Queue) Dequeue() (interface{}, bool){
	if q.IsEmpty() {
		return nil, false
	} else {
		ele := (*q)[0]
		*q = (*q)[1:] //slice off first element
		return ele, true
	}
}
// s.IsEmpty() checks if given stack is empty
func (s *Stack) IsEmpty() bool {
	return len(*s) == 0
}
// s.Push pushes a new value onto the stack (nth element)
func (s *Stack) Push(ele interface{}) {
	*s = append(*s, ele)
}
// s.Pop removes and returns the top element of the stack if it exists, otherwise return nothing and indicate failure (nth element)
func (s *Stack) Pop() (interface{}, bool) {
	if s.IsEmpty() {
		return nil, false
	} else {
		index := len(*s) - 1 //index of last element in the stack
		ele := (*s)[index]
		*s = (*s)[:index] // removes the top (nth element) of the stack. slice[low : high] includes low but excludes high. Not specifying low takes low as index 0
		return ele, true
	}
}
// getPrecidence() returns a numerical ranking of a given operands precidence for use in the shunting yard algorithm.
// Higher number means higher precidence. 
func getPrecidence(opr interface{}) (typ int, ok bool) {
	ok = false
	typ = -1
	switch opr {
	case "*":
		typ = MULT
		ok = true
	case "/":
		typ = DIVIDE
		ok = true
	case "+":
		typ = PLUS
		ok = true
	case "-":
		typ = MINUS
		ok = true
	case "**":
		typ = EXPONENT
		ok = true
	case ")":			//currently not used, set typ to RPAREN and ok to true to enable
		typ = -1
		ok = false
	case "(":			//currently not used, set typ to LPAREN and ok to true to enable
		typ = -1
		ok = false
	}
	return typ, ok
}
func evaluate(opr string, nums ...float64) (float64, bool) {
	var out float64
	switch opr {
	case "*", "mult":
		out = multiply(nums...)
	case "/", "div":
		out = divide(nums...)
	case "+", "add":
		out = add(nums...)
	case "-", "sub":
		out = subtract(nums...)
	case "**", "exp":
		if len(nums) != 2 {
			log.Println("[Evaluate], more than two arguments provided to exponential function. Exiting")
			return math.NaN(), false
		}
		out = exponent(nums[0], nums[1])
	case "avg":
		out = average(nums...)
	case "max":
		out,_ = maximum(nums...)
	case "min":
		out,_ = minimum(nums...)
	case "floor":
		if len(nums) != 1 {
			log.Println("[Evaluate], more than one argument provided to floor function. Exiting")
			return math.NaN(), false
		}
		out = floor(nums[0])
	case "ceil":
		if len(nums) != 1 {
			log.Println("[Evaluate], more than one argument provided to ceil function. Exiting")
			return math.NaN(), false
		}
		out = ceil(nums[0])
	case "round":
		if len(nums) != 1 {
			log.Println("[Evaluate], more than one argument provided to round function. Exiting")
			return math.NaN(), false
		}
		out = round(nums[0])
	default:
		return math.NaN(), false
	}
	return out, true
}
// performs stack evaluation on post-fix expression from shuntingYard
func pfEval(q Queue) interface{} {
	var pfs Stack //This stack will contain numbers from q
	// Dequeue elements from the queue, pushing onto the stack if a number and poping twice, evaluating, and pushing result if operator. 
	for ;; {
		ele, ok := q.Dequeue()
		if !ok {
			break
		}
		switch ele.(type) {
		case float64 :
			pfs.Push(ele.(float64))
		case int:
			tmp := ele.(int)
			pfs.Push(float64(tmp)) //all of the math funcitons work on floats
		default: //Otherwise we have an operator that is a string. No need to check type here since if it got enqueued earlier it was confirmed to be either a float64 or string
			var first interface{} //Pop returns an interface. 
			var second interface{}
			var ok bool
			if second, ok = pfs.Pop(); !ok {
				log.Println("[pfEval] Error, unexpected empty stack during evaluation")
				return nil
			}
			if first, ok = pfs.Pop(); !ok {
				log.Println("[pfEval] Error, unexpected empty stack during evaluation")
				return nil
			}
			if out, ok := evaluate(ele.(string), first.(float64), second.(float64)); ok { //operator, left operand, right operand.
				pfs.Push(out) 
			} else {
				log.Println("[pfEval] error evaluating expression. Operator:", ele, "left operand:", first, "right operand:", second)
			}
		}
	}
	//Pop result from stack
	if ret, ok := pfs.Pop(); ok {
		if pfs.IsEmpty() {
			return ret
		} else {
			log.Println("[pfEval] Error, stack not empty when expected")
		}
	} else {
		log.Println("[pfEval] Error, unexpected empty stack while retrieving result")
	}
	return nil
}
// implements the shunting yard algorithm and post-fix stack evaluator. 
// https://en.wikipedia.org/wiki/Shunting_yard_algorithm
func shuntingYard(eSlice []interface{}) interface{} {
	var s Stack //Place to put operators that will reverse their execution
	var q Queue //Where the post-fix expression will live and be parsed from. 
	for _, elem := range eSlice {
		switch elem.(type) {
		case string: //operators are strings
			//enqueue operator, taking into account precidence
			nextOprPrec, ok := getPrecidence(elem)
			if !ok {
				log.Println("[shuntingYard] could not parse operator", elem)
				return nil //string in elem was not a valid operator. 
			}
			if s.IsEmpty() {
				s.Push(elem)
				continue
			} else {
				stackOpr := (s)[(len(s)-1)]
				stackOprPrec, _ := getPrecidence(stackOpr) //don't need to check if ok. If it was previously pushed it is already confirmed to be a valid operator
				if nextOprPrec >= stackOprPrec {
					s.Push(elem)
				} else {
					stackOpr, _ = s.Pop() //don't need to check if ok, since we already know s is not empty. 
					q.Enqueue(stackOpr)
					s.Push(elem)
				}
				continue
			}
		case int:
			q.Enqueue(float64(elem.(int))) //values always get enqueued
			continue
		case float64:
			q.Enqueue(elem)
		default:
			log.Println("[shuntingYard] unexpected type found in expression. Element:", elem, "Type:", reflect.ValueOf(elem).Kind()) //TODO GB: better to use printf and send %T
		}
	}
	//Pop remaining operators off the stack and enqueue them. 
	for ;; {
		if s.IsEmpty(){
			break
		}
		if ele, ok := s.Pop(); ok {
			q.Enqueue(ele)
		} else {
			log.Println("[shuntingYard] Unable to pop from stack. Current stack:", s)
			return nil
		}
	} 
	//evaluate the post-fix expression now contained in the Queue 
	ret := pfEval(q)
	if ret == nil {
		log.Println("[shuntingYard] unable to process expression:", eSlice)
	}
	return ret
}
//recursive portion of collapseConfig(). Allows for nested maps (e.g. SOC measurement structure)
func collapseConfigMap(cfgMap map[string]interface{}) interface{} {
	for k,v := range cfgMap {
		if _,ok := v.(map[string]interface{}); !ok {
			log.Println("[collapseConfigMap] expected nested map, got instead",reflect.ValueOf(v).Kind(), "key", k)
			continue
		}
		var value interface{}
		var ok bool
		var exp interface{}
		if value, ok = v.(map[string]interface{})["default"]; !ok { // we must go deeper
			cfgMap[k] = collapseConfigMap(v.(map[string]interface{}))
			continue
		}
		if exp, ok = v.(map[string]interface{})["expression"]; (!ok || exp == nil) {
			cfgMap[k] = value
			continue
		}
		if len(exp.([]interface{})) == 1{
			cfgMap[k] = exp.([]interface{})[0]
		} else {
			cfgMap[k] = exp
		}
	}
	return cfgMap
}
// Updates configMap from {"variable": {"value": val, "expression": [exp]}} structure to {"variable": exp} or {"variable": val} depending on whether or not there is an expression found. 
func collapseConfig() {
	for _,v := range configMap { //k: asset type, v: slice of assets
		if _,ok := v.([]interface{}); !ok {
			continue //we're looking for the asset objects, which contain slices as their value. Other objects like updateRate should be skipped here. 
		}
		for i := range v.([]interface{}) { //slice of assets
			astMap := v.([]interface{})[i].(map[string]interface{}) //so we don't have to keep making this ugly type assertion later.
			for varname, val := range astMap { 
				if _, ok := val.(map[string]interface{}); !ok {
					//log.Println("[collapseConfig] Expected map objects at top level of asset configs. Received instead:", valKind, "asset type:", k, "slice element number:", i)
					continue
				}
				var value interface{}
				var exp interface{}
				var ok bool
				if value,ok = val.(map[string]interface{})["default"]; !ok { //if the "value" field is not found at this level, then we have a nested map and need to unpack. 
					val = collapseConfigMap(val.(map[string]interface{}))
					continue
				}
				if exp, ok = val.(map[string]interface{})["expression"]; (!ok || exp == nil) { // no expression found, so take default value
					astMap[varname] = value
					continue
				}
				//expressions are all slices at this point. If expression contains a single value then unpack and assign to asset variable directly.
				if len(exp.([]interface{})) == 1{
					astMap[varname] = exp.([]interface{})[0]
				} else {
					astMap[varname] = exp
				}
			}
		}
	}
	return
}
// Replaces expressions in configMap with expression evaluated in expressions. 
// This is called after evaluateExpressions() and all related functions to find, replace, and evaluate variables and expressions have run
// TODO GB: this is needed because the expected initial shallow copy from configMap to expressions currently does not work as intended
// after the expression evaluation step, some evaluated expressions do not appear in configMap, so replace them all manually here.
// once that issue is fixed then this entire step can be skipped and this function is written as such. 
// Update 1/10/2024: now this function also detects nil entries in expMap and deletes the corresponding expression in cfgMap so the default will be picked. 
func replaceExpressions(cfgMap map[string]interface{}, expMap map[string]map[string][]interface{}) {
	// fmt.Println("Trying to print expMap")
	// printMap(expMap)
	for k,v := range cfgMap { //k: asset type, v: slice of assets
		if _, ok := v.([]interface{}); !ok {
			continue //we're looking for the asset objects, which contain slices as their value. Other objects like updateRate should be skipped here. 
		}
		for i := range v.([]interface{}) { //slice of assets
			astMap := v.([]interface{})[i].(map[string]interface{}) //so we don't have to keep making this ugly type assertion later. 
			var astID string
			var ok bool
			if astID, ok = astMap["id"].(string); !ok {
				log.Println("[replaceExpressions] Could not find id in slice element",i, "of object category", k)
				continue
			}
			var varMap map[string][]interface{}
			if varMap, ok = expMap[astID]; !ok { //look for asset id in expression map. If found, then look for matching variables in both cfg and exp, then replace expressions if found.
				continue
			}
			for variable, exp := range varMap {
				// fmt.Println(exp)
				var astVar map[string]interface{}
				if astVar, ok = astMap[variable].(map[string]interface{}); !ok {
					continue
				}
				badExp := false
				//if the parsed expression returned NaN (not a number) for any reason, then don't use that expression and instead use the variable's default value from the psm_dflt.json
				for _, expcheck := range exp { //exp can be a slice of length > 1. If any slice member is NaN, then the whole expression should be tossed
					var tmp float64
					var success bool
					if tmp, success = expcheck.(float64); !success {
						continue
					}
					// fmt.Println(tmp)
					if math.IsNaN(tmp) || math.IsInf(tmp,0) {
						log.Println("WARNING: expression not parsed properly for asset", astID, "Variable:", variable)
						log.Println("Deleting expression",astVar["expression"], " from astVar", astVar)
						delete(astVar, "expression") // delete from configMap
						delete(varMap, variable) // delete from expressions Map
						// fmt.Println("astVar after delete", astVar)
						badExp = true
						break
					}
				}
				if !badExp {
					astVar["expression"] = exp	//equivalent to configMap[k].([]interface{})[i].(map[string]interface{})[variable].(map[string]interface{})["expression"] = exp 
				}
			}
		}
	}
}

func parseExpressionFunc(funcMap map[string]interface{}) (interface{}) {
	//var out interface{}
	//var args []float64
	for k,v := range funcMap {
		switch v.(type) {
		//unpack function arguments
		case []interface{}: 
			var acc []float64
			var sliceHandler func ([]interface{}) interface{}
			var sliceUnpack func([]interface{})
			sliceHandler = func(slc []interface{}) interface{} {
				for i,ele := range slc {
					switch ele.(type) {
					case map[string]interface{}:
						slc[i] = parseExpressionFunc(ele.(map[string]interface{}))
					case []interface{}:
						slc[i] = sliceHandler(ele.([]interface{}))
					case string: //encountering a string means that the slice we're looking at is an expression.
						return parseExpression(slc)
					}
				}
				return slc
			}
			v = sliceHandler(v.([]interface{}))
			//in the case we have multiple slice arguments to a function, need to agregate them into a single flat slice
			sliceUnpack = func(slc []interface{}) {
				for _,ele := range slc {
					if _,ok := ele.([]interface{}); ok {
						sliceUnpack(ele.([]interface{}))
					} else {
						if _,ok := ele.(float64); !ok{
							ele = math.NaN()
						}
						acc = append(acc, ele.(float64))
					}
				}
			}
			sliceUnpack(v.([]interface{}))
			out, ok := evaluate(cleanString(k), acc...); 
			if !ok {
				log.Println("Unable to parse function", k, "with arguments", acc)
			}
			return out
		default:
			log.Printf("Unexpected data type for function arguments. Expected []interface, got %T\n", v)
			return nil
		}
	}
	// for k,v := range funcMap {
	// 	vKind := reflect.ValueOf(v).Kind()

	// 	if vKind == reflect.Slice {
	// 		for _, ele := range v.([]interface{}) {
	// 			//eleKind := reflect.ValueOf(ele).Kind()
	// 			if _,ok := ele.([]interface{}); ok { //unpack nested slice potentially containing an expression
	// 				v = parseExpression(v.([]interface{}))
	// 				break 
	// 			}
	// 		}
	// 		//now unpack resulting slice into args to pass to evaluate later
	// 		//can only do this after parse expression, and only know to do the parse expression if a nested slice is detected
	// 		vKind := reflect.ValueOf(v).Kind()
	// 		if vKind == reflect.Slice {
	// 			for _, elem := range v.([]interface{}) {
	// 				elemKind := reflect.ValueOf(elem).Kind()
	// 				if elemKind == reflect.Float64 {
	// 					args = append(args, elem.(float64))
	// 				} else if elemKind == reflect.Int {
	// 					args = append(args, float64(elem.(int)))
	// 				} else if elem == nil {
	// 					args = append(args, math.NaN())
	// 				}
	// 			} 				
	// 		} else if vKind == reflect.Float64 {
	// 			args = append(args,v.(float64))
	// 		} else if vKind == reflect.Int {
	// 			args = append(args,float64(v.(int)))
	// 		} else if v == nil {
	// 			args = append(args, math.NaN())
	// 		} else {
	// 			log.Print("[parseExpressionFunc] unable to parse function", funcMap)
	// 			return nil
	// 		} 

	// 		//handle functions inside of functions
	// 	} else if vKind == reflect.Map {
	// 		v = parseExpressionFunc(v.(map[string]interface{}))
	// 	} else if vKind == reflect.Int {
	// 		v = float64(v.(int))
	// 	} else if vKind != reflect.Float64 {
	// 		log.Println("[parseExpressionFunc] Unexpected data type encountered in unpacking map. args:", v, "datatype:", vKind)
	// 	}
	// 	//now that we've unpacked internal expressions and functions, evaluate outer funciton. 
	// 	var ok bool
	// 	key := cleanString(k)
	// 	if out, ok = evaluate(key,args...); ok {
	// 		return out
	// 	} else {
	// 		log.Println("[parseExpressionFunc] Unable to parse function", funcMap)
	// 	}
	// }
	return nil
}

// like parseExpression, except handles multidimensional expressions. 
func parseExpressionSlice(eSlice []interface{}) ([]interface{}) {
	for i := range eSlice {
		//if reflect.ValueOf(eSlice[i]).Kind() == reflect.Slice{
		if _,ok := eSlice[i].([]interface{}); ok {
			out := parseExpression(eSlice[i].([]interface{})) //we checked this before calling this function. 
		    eSlice[i] = out
		}
	}
	eSlice = deNest(eSlice)
	return eSlice
}
// parseExpressionSlice unpacks and understands the nested slice eSlice to provide a flat expression to the shunting yard algorithm.
// Parentheses are handled at this level by recursion since they are represented by nested slices in the expressions map. 
// functions are parsed separately and their results returned back here. 
func parseExpression(eSlice []interface{}) (interface{}) {
	isArry := true
	isExp := false
	var out interface{}
	for i := range eSlice {
		//this loop examines the expression in eSlice and determines what it contains and what format is expected for the return value. 
		iKind := reflect.ValueOf(eSlice[i]).Kind()
		//a slice of all slices is meant to be a two dimensional value.
		if iKind == reflect.Map {
			eSlice[i] = parseExpressionFunc(eSlice[i].(map[string]interface{}))
			isArry = false
			continue
		} else if len(eSlice) == 1 && iKind != reflect.Slice{ //if we're evaluating an expression that is not a map and is only length one then it is a single value and should be returned to the calling function
			return eSlice[0]
		}
		if iKind == reflect.Slice {
			eSlice[i] = parseExpression(eSlice[i].([]interface{}))
		} else if iKind == reflect.Float64 || iKind == reflect.Int {
			continue
		} else {
			isArry = false
		}
		if iKind == reflect.String { // if we encounter a string, then it's an operator and we know that eSlice contains an expression at this level. 
			isExp = true
		}
	}
	if isArry && len(eSlice) > 1{ 
		out = parseExpressionSlice(eSlice)
	} else { //this properly handles nested expressions. 
		if isExp{
			var tmp []interface{}
			out = shuntingYard(eSlice)
			tmp = append(tmp, out)
			eSlice = tmp
		} else {
			out = eSlice
		}

	}
	if reflect.ValueOf(out).Kind() == reflect.Slice {
		out = deNest(out.([]interface{}))
	}
	return out
}
// deNest() flattens a nested value. Expects a single value or single slice of values nested somewhere in a slice of slices.
func deNest(nest []interface{}) []interface{} {
	var found bool = false
	var out []interface {}
	for i := range nest {
		itemKind := reflect.ValueOf(nest[i]).Kind() 
		if itemKind == reflect.Slice {
			if found {
				out = nil
				log.Println("[deNest] Expected only a single value, received instead: ",nest)
				break
			}
			out = deNest(nest[i].([]interface{}))
			found = true
		} else {
			out = append(out,nest[i])
		}
	}
	return out
}

func parse() {
	for asset_id,asset := range expressions { 
		for variable, exp := range asset {
			tmpExp := deepCopySlice(exp) //for logging if something goes pear-shaped
			var tmp []interface{}
			out := parseExpression(exp)
			if out == nil {
				log.Println("[parse] Unable to parse expression from asset:", asset_id, "variable:", variable, "expression:", tmpExp)
			}
			//In cases of nested expressions, resulting value may be nested within a slice. 
			if _,ok := out.([]interface{}); ok {
				expressions[asset_id][variable] = deNest(out.([]interface{}))
			} else {
				tmp = append(tmp, out)
				expressions[asset_id][variable] = tmp
			}

		}
	}
}

// findVarSpecial() looks for 'special' variables used in configuration e.g. updateRate (dT), tickRate, etc. Any variable
// that is not referenced to an asset's relative or self gets parsed here. 
func findVarSpecial(varsplit []string, asset_id string, depth int) (interface{}) {
	if len(varsplit) > 1 {
		log.Println("[findVarSpecial] deep addressing of special variables currently not supported. Variable", varsplit, "asset id:", asset_id)
		return nil
	}
	if varsplit[0] == "dt" || varsplit[0] == "updaterate" {
		if dt != nil {
			return dt //we've already found it so just return it back
		}
		var ok bool
		if dt,ok = configMap["updateRate"]; ok{
			return dt
		}
		if dt == nil {
			log.Println("[findVarSpecial] Could not find updaterate in configurations. Configurations referencing this will take defaults")
		}
		return dt
	}
	log.Println("[findVarSpecial unable to find special variable", varsplit, "from asset", asset_id,"in configurations. Referenced calibrations will use default")
	return nil
}
// findAssetRecurse() looks through the pwrTree to find an asset with id == asset_id, then finds the correct relatives
// indicated by relationship.
func findAssetRecurse(asset_id string, tree treeCfgNode, parent treeCfgNode, relationship string) ([]treeCfgNode) {
	res := make([]treeCfgNode, 0)
	if tree.ID == asset_id {
		if relationship == "this" {
			res = append(res, tree)
		} else if relationship == "child" {
			for i := range tree.Children {
				res = append(res, tree.Children[i])
			}
		} else if relationship == "parent" {
			res = append(res, parent)
		}
		return res
	} else {
		for i := range tree.Children {
			ret := findAssetRecurse(asset_id, tree.Children[i], tree, relationship)
			for j := range ret {
				if ret[j].ID != "" {
					res = append(res, ret[j])
				}
			}
		}
	}
	return res
}

//accepts a split variable containing e.g. ["this", "plim"] or a special variable. Looks through pwrTree to understand asset familiar relationships,
//And calls varHandler again if needed if encountering another $variable when trying to unpack.
func findVar(varsplit []string, asset_id string, depth int) (interface{}) {
	//var relatives []treeCfgNode Will contain a reference to the asset's parent, children, or self if needed. If parent or self(this) then len(relative) will always be 1.
	relatives := findAssetRecurse(asset_id, pwrTree, treeCfgNode{}, varsplit[0]) //find the assets referenced by the $variable (parents children or this asset itself).
	variables := make([]interface{}, len(relatives))
	if len(varsplit) < 2 {
		//if we have this or parent without a . then it's a misconfiguration
		if varsplit[0] != "child" {
			log.Println("[findVar] Unexpected end of variable string. Expected variable designation with 'this' or 'parent'. Received:", varsplit)
			return nil
		}
		//otherwise $child returns number of children. 
		return len(relatives)
	}
	for i := range relatives {
		assetType := cleanString(relatives[i].Asset_type)
		asts, ok := configMap[assetType]
		if !ok{
			log.Println("[findVar] could not find asset type", assetType, "in configuration map. Parsed variable was", varsplit, "in asset", asset_id)
			return nil
		}
		assets := asts.([]interface{})
		//assets are a slice of a given asset type, e.g. slice of all PCSes. 
		for j := range assets { //from configMap
			id, ok := assets[j].(map[string]interface{})["id"]
			if !ok {
				continue
			}
			if id == relatives[i].ID { //TODO GB: continues below here probably should be breaks?
				//find var name from varsplit[1]. Later handle if expression is found here. 
				var val interface{} = nil
				var varName string = ""
				for k,v := range assets[j].(map[string]interface{}) {
					cleank := cleanString(k)
					if cleank == varsplit[1] {
						val = v
						varName = k
						break
					}
				}
				//this is a two-for-one. Will only pass if val is a map, meaning the block above was able to find the variable and the datatype was correct for next steps 
				if reflect.ValueOf(val).Kind() != reflect.Map { 
					log.Println("[findVar] could not find referenced variable", varsplit, "for asset",asset_id,". Referenced relative asset ID:", relatives[i].ID, "This may be expected.")
					continue
				}
				valmap := val.(map[string]interface{}) 
				if _, okay := valmap["expression"]; !okay {
					value, success := valmap["default"]
					if !success {
						if verbose {
							log.Println("[findVar] Could not find value field of variable", varsplit[1], "from asset", relatives[i].ID,"Please check configs. Using default for this configuration")
						}
						continue
					} else {
						variables[i] = value
					}
					continue
				} else {
					//at this point, we've referenced a variable that itself is an expression. Attempt to replace variables in that expression
					//then assign the referenced expression to the variable. Later when this expression is parsed it will reflect correctly
					//wherever referenced due to the shallow copy. 
					//var exp interface{}
					exp := expressions[relatives[i].ID][varName]
					res := varHandler(exp, relatives[i].ID, depth+1) //we're going deeper
					if reflect.ValueOf(res).Kind() != reflect.Slice {
						continue //TODO GB: logging. 
					}
					result := res.([]interface{})
					if len(result) < 2 {
						variables[i] = result[0]
					} else {
						variables[i] = result
					}
				}
			}
		}
	}
	//flatten variables if returning a single value. Makes later processing easier. 
	//variables = deNest(variables)
	//TODO GB: Should this be done on return from this function instead? There are other cases to care about too e.g. many 2-dimentional variables passed to a function need to have their slices combined. 
	// fmt.Println("Variables:", variables)
	if len(variables) == 0{
		return nil
	}
	if len(variables) < 2 {
		return variables[0]
	} else {
		return variables
	}
	return variables
}
// varHandler() receives an expression to the variable argument, parses through it looking for $variables,
// cleans them and splits them into a varslice and calls findVar with that slice
// and information about the asset and its relative to findVar()
// Detects circular references by incrementing the depth argument whenever encountering a variable referenced by the current variable
func varHandler(variable interface{}, asset_id string, depth int) (interface{}) {
	if depth >= MAX_DEPTH {
		log.Print("[varHandler] Potential circular reference identified in asset :", asset_id, ": variable :", variable)
		log.Fatal("[varHandler] Maximum variable parse depth reached. Unrecoverable fault, exiting. ")
	}
	varKind := reflect.ValueOf(variable).Kind()
	//Variables can be a slice of expressions, so unpack those here without increasing depth since this wasn't a moment where we are following a variable to another asset. 
	if varKind == reflect.Slice{
		for i := range variable.([]interface{}) {
			variable.([]interface{})[i] = varHandler(variable.([]interface{})[i], asset_id, depth)
		}
		return variable
	//Functions likely have variables, so parse those next.	
	} else if varKind == reflect.Map {
		for k,v := range variable.(map[string]interface{}) {
			if reflect.ValueOf(v).Kind() != reflect.Slice {
				log.Println("[varHandler] encountered function without associated slice. Value is", v, "Function called is", k, "from asset", asset_id)
				return nil
			}
			value := v.([]interface{})
			slice := make([]interface{}, len(value))
			for iter := range value {
				slice[iter] = varHandler(value[iter], asset_id, depth)
			}
			variable.(map[string]interface{})[k] = slice
			return variable
		}
	//finally if we're not a slice or a function, then we expect to be a string or a number. If we're a number then just return, otherwise parse variable in string. 	
	} else if varKind != reflect.String {
		return variable
	}
	varstring := cleanString(variable.(string))
	if !strings.HasPrefix(varstring,"$") {
		return variable
	}
	//varstring either contains some $this.thing, $parent.thing, $child.thing, $child, or $special. 
	split := strings.Split(strings.Trim(varstring, "$"), ".")
	if len(split) < 1 {
		log.Print("[varHandler] Identified variable", variable, "from asset", asset_id, "could not be parsed. Default value will be used")
		return nil
	}
	if split[0] == "this" || split[0] == "child" || split[0] == "parent" {
		variable = findVar(split, asset_id, depth)
	} else {
		variable = findVarSpecial(split, asset_id, depth)
	}
	return variable
}
//replaceVars iterates through expressions map to replace variables denoted by $ with a value that represents the variable
//then parses expressions at the appropriate level. 
//TODO GB: consider flattening structure, replacing slices with "(", ")", and include functions in precidence chart - or consider them as parens
func replaceVars() {
	for asset_id,vars := range expressions {
		for var_name, expression := range vars {
			for i, entry := range expression {
				if _,ok := entry.([]interface{}); ok {
					for j := range entry.([]interface{}) {
						expressions[asset_id][var_name][i].([]interface{})[j] = varHandler(expressions[asset_id][var_name][i].([]interface{})[j], asset_id, 0)
					}
				} else {
					expressions[asset_id][var_name][i] = varHandler(expressions[asset_id][var_name][i], asset_id, 0)
				}
			}
		}
	}
}

// Finds expressions potentially containing variables in configMap
func findExpressions() (map[string]map[string][]interface{}) {
	//top level is map of slices, extract that. 
	for k,v := range configMap {
        //some configurations live at the top level of configMap and should be skipped here. 
		if _,ok := v.([]interface{}); ok {
			for _,ast := range configMap[k].([]interface{}) {
				if reflect.ValueOf(ast).Kind() != reflect.Map {
					continue //TODO GB: logging
				}
				id, ok := ast.(map[string]interface{})["id"];
				if !ok {
					continue //TODO GB: logging
				}
				astKind := reflect.ValueOf(ast).Kind()
				if astKind != reflect.Map {
					log.Println("[extractExpressions] unexpected data type encountered. Expected Map, recieved",astKind )
				}
				asset := ast.(map[string]interface{})
				expressions[id.(string)]=make(map[string][]interface{})
				var tmp []interface{}
				for ky,v := range asset {
					if reflect.ValueOf(v).Kind() == reflect.Map {
						if exp,ok := v.(map[string]interface{})["expression"]; ok { //TODO GB: this forces expression to be lowercase with no whitespace. Previous commits had a better solution. 
							expKind := reflect.ValueOf(exp).Kind()
							if expKind == reflect.Slice{
								tmp = exp.([]interface{})
							} else {
								log.Println("[findExpressions] expected expression to be a slice, got instead", expKind)
								continue
							}
							expressions[id.(string)][ky]=tmp
						} else {
							continue
						}
					}
				}				
			} 
		}
	}
	return expressions
}
// this is the main() for expression evaluation. 
func evaluateExpressions() {
	dt = nil
	expressions = make(map[string]map[string][]interface{})
	expressions = findExpressions()
	if verbose {
		fmt.Println("The following expressions were found:")
		for k,v := range expressions{
			fmt.Println(k)
			for vbl, val := range v{
				fmt.Print("  ")
				fmt.Println(vbl, ":", val)
			}
		}
	}
	replaceVars()
	if verbose {
		fmt.Println("Expressions after variable replacement:")
		for k,v := range expressions{
			fmt.Println(k)
			for vbl, val := range v{
				fmt.Print("  ")
				fmt.Println(vbl, ":", val)
			}
		}
	}
	parse()
	if verbose {
		fmt.Println("Expressions after expression parsing:")
		for k,v := range expressions{
			fmt.Println(k)
			for vbl, val := range v{
				fmt.Print("  ")
				fmt.Println(vbl, ":", val)
			}
		}
	}
	return
}