package go_metrics

import (
	"fmt"
	"go/ast"
	"go/token"
	"reflect"
	"sort"
	"strconv"
	"strings"
)

// Looks at a node in an AST and determines the type of the node.
// Based on the type, it calls an auxiliary function to do the actual evaluation.
func EvaluateDynamicFilter(node *ast.Node, inputs []string) (values []string, err error) {
	switch (*node).(type) {
	case *ast.Ident:
		values, err = evaluateFilterIdent((*node).(*ast.Ident), inputs)
	case *ast.CallExpr:
		values, err = evaluateFilterCallExpr((*node).(*ast.CallExpr), inputs)
	case *ast.BinaryExpr:
		values, err = evaluateFilterBinary((*node).(*ast.BinaryExpr), inputs)
	case *ast.ParenExpr:
		newNode := ast.Node((*node).(*ast.ParenExpr).X)
		values, err = EvaluateDynamicFilter(&newNode, inputs)
	default:
		err = fmt.Errorf("unsupported node %+v (type %+v)", *node, reflect.TypeOf(*node))
	}

	return values, err
}

// pull a variable out of the map of inputs
func evaluateFilterIdent(node *ast.Ident, inputList []string) ([]string, error) {
	outputs := make([]string, 0)
	if stringInSlice(inputList, node.Name) {
		if _, ok := FilterScope[node.Name]; ok {
			outputs = append(outputs, FilterScope[node.Name]...)
		} else if _, ok := InputScope[node.Name]; ok {
			outputs = append(outputs, node.Name)
		}
	} else if node.Name == "value" {
		for _, inputName := range inputList {
			if _, ok := FilterScope[inputName]; ok {
				outputs = append(outputs, FilterScope[inputName]...)
			} else if _, ok := InputScope[inputName]; ok {
				outputs = append(outputs, inputName)
			}
		}
	} else {
		for _, inputName := range inputList {
			if strings.Contains(inputName, "@") {
				attrList := strings.Split(inputName, "@")
				if len(attrList) > 1 && attrList[1] == node.Name {
					if _, ok := FilterScope[inputName]; ok {
						outputs = append(outputs, FilterScope[inputName]...)
					} else if _, ok := InputScope[inputName]; ok {
						outputs = append(outputs, inputName)
					}
				}
			}
		}
	}
	sort.Strings(outputs)

	return outputs, nil
}

// evaluate the result of a binary operation
func evaluateFilterBinary(node *ast.BinaryExpr, inputList []string) ([]string, error) {
	newNodeL := ast.Node(node.X)
	lValues, err := EvaluateDynamicFilter(&newNodeL, inputList) // check if we can evaluate the left side into variables first
	var lValueBasicLit Union
	if len(lValues) == 0 || err != nil { // if not variables, then it's a basic lit
		lValues = []string{}
		lValueBasicLit, err = evaluateFilterBasicLit(&newNodeL) // evaluate the left side before combining with the right
		if err != nil {                                         // if not basic lit or variable, then we failed
			return []string{}, err
		}
	}

	newNodeR := ast.Node(node.Y)
	rValues, err := EvaluateDynamicFilter(&newNodeR, inputList) // check if we can evaluate the right side into variables first
	var rValueBasicLit Union
	if len(rValues) == 0 || err != nil { // if not variables, then it's a basic lit
		rValues = []string{}
		rValueBasicLit, err = evaluateFilterBasicLit(&newNodeR) // evaluate the right side before combining with the left
		if err != nil {                                         // if not basic lit or variable, then we failed
			return []string{}, err
		}
	}

	outputs := make([]string, 0)
	if len(lValues) > 0 && len(rValues) == 0 {
		for _, input := range lValues {
			intermediateOutputs, err := evaluateFilterBinaryComparison(node, input, rValueBasicLit)
			if err != nil {
				return outputs, err
			} else {
				if len(intermediateOutputs) > 0 {
					outputs = append(outputs, intermediateOutputs...)
				} else {
					outputs = append(outputs, "nil")
				}
			}
		}
	} else if len(rValues) > 0 && len(lValues) == 0 {
		for _, input := range rValues {
			intermediateOutputs, err := evaluateFilterBinaryComparison(node, input, lValueBasicLit)
			if err != nil {
				return outputs, err
			} else {
				if len(intermediateOutputs) > 0 {
					outputs = append(outputs, intermediateOutputs...)
				} else {
					outputs = append(outputs, "nil")
				}
			}
		}
	} else if len(lValues) == len(rValues) {
		for i := range lValues {
			expression, err := Parse(lValues[i] + node.Op.String() + rValues[i])
			if err != nil {
				return outputs, err
			}

			// evaluate like a normal expression
			intermediateOutputs, _ := Evaluate(expression, nil)

			// make sure all values are truthy
			intermediateOutputs, err = Bool(intermediateOutputs...)
			if err != nil {
				return outputs, err
			}

			// convert to string so that the output agrees with what is expected
			for _, output := range intermediateOutputs {
				if output.b {
					outputs = append(outputs, "true")
				} else {
					outputs = append(outputs, "false")
				}
			}
		}
	}

	return outputs, err
}

func evaluateFilterBinaryComparison(node *ast.BinaryExpr, inputName string, rValue Union) ([]string, error) {
	outputs := make([]string, 0)
	var err error
	var lValue Union
	if _, ok := InputScope[inputName]; ok && len(InputScope[inputName]) > 0 {
		lValue = InputScope[inputName][0]
		value := Union{}
		switch node.Op {
		case token.EQL:
			value, err = Equal(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		case token.LSS:
			value, err = LessThan(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		case token.GTR:
			value, err = GreaterThan(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		case token.NEQ:
			value, err = NotEqual(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		case token.LEQ:
			value, err = LessThanOrEqual(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		case token.GEQ:
			value, err = GreaterThanOrEqual(lValue, rValue)
			if err != nil {
				return outputs, err
			}
			if value.b {
				outputs = append(outputs, inputName)
			}
		default:
			err = fmt.Errorf("unsupported binary operation: %s", node.Op)
		}

	} else if _, ok := FilterScope[inputName]; ok && len(FilterScope[inputName]) > 0 {
		for _, new_input := range FilterScope[inputName] {
			intermediateOutputs, err := evaluateFilterBinaryComparison(node, new_input, rValue)
			if err != nil {
				return outputs, err
			} else {
				outputs = append(outputs, intermediateOutputs...)
			}
		}
	}
	return outputs, err
}

// evaluate the result of a function call
func evaluateFilterCallExpr(node *ast.CallExpr, inputList []string) ([]string, error) {
	num_args := len(node.Args)
	if num_args == 0 {
		return []string{}, fmt.Errorf("incorrect number of arguments for function evaulation")
	}
	id, ok := node.Fun.(*ast.Ident)
	if ok {
		// attribute filters
		if strings.ToLower(id.Name) == "attribute" {
			_, ok1 := node.Args[0].(*ast.BinaryExpr)
			if ok1 {
				_, ok1 = ast.Node(node.Args[0].(*ast.BinaryExpr).X).(*ast.Ident)
			}
			if ok1 {
				attributeName := (ast.Node(node.Args[0].(*ast.BinaryExpr).X).(*ast.Ident)).Name
				outputs := make([]string, 0)
				if attributeList, ok := allPossibleAttributes[attributeName]; ok {
					inputsWithAttribute := make([]string, 0)
					for _, attr := range attributeList {
						if stringInSlice(inputList, strings.Split(attr, "@")[0]) {
							inputsWithAttribute = append(inputsWithAttribute, attr)
						}
					}
					attributeInputsFiltered, err := evaluateFilterBinary(node.Args[0].(*ast.BinaryExpr), inputsWithAttribute)
					if err != nil && len(attributeInputsFiltered) == 0 {
						return []string{}, err
					}
					for _, attribute := range attributeInputsFiltered {
						if _, ok2 := MetricsConfig.Attributes[attribute]; ok2 {
							outputs = append(outputs, MetricsConfig.Attributes[attribute].InputVar)
						}
					}
				} else {
					return []string{}, fmt.Errorf("attribute %s does not exist in Scope", attributeName)
				}
				return outputs, nil
			}
		} else if strings.ToLower(id.Name) == "value" {
			_, ok1 := node.Args[0].(*ast.BinaryExpr)
			if ok1 {
				_, ok1 = ast.Node(node.Args[0].(*ast.BinaryExpr).X).(*ast.Ident)
			}
			if ok1 {
				return evaluateFilterBinary(node.Args[0].(*ast.BinaryExpr), inputList)
			}
		} else if strings.ToLower(id.Name) == "indexfilter" {
			outputs := []string{}
			_, ok1 := node.Args[0].(*ast.BinaryExpr)
			if ok1 {
				conditionNode := node.Args[0].(ast.Node)
				indices, err := EvaluateDynamicFilter(&conditionNode, inputList)
				if err != nil {
					return []string{}, fmt.Errorf("could not evaluate index filter")
				}
				var selectionTrueList []string
				if len(node.Args) > 1 {
					trueNode := node.Args[1].(ast.Node)
					selectionTrueList, err = EvaluateDynamicFilter(&trueNode, inputList)
				}
				if len(selectionTrueList) == 0 || err != nil {
					return []string{}, fmt.Errorf("could not evaluate index filter")
				}
				var selectionFalseList []string
				if len(node.Args) > 2 {
					falseNode := node.Args[2].(ast.Node)
					selectionFalseList, _ = EvaluateDynamicFilter(&falseNode, inputList)
				}
				if len(indices) == len(selectionTrueList) {
					for i, index := range indices {
						if index == "true" {
							outputs = append(outputs, selectionTrueList[i])
						} else {
							if len(indices) == len(selectionFalseList) {
								outputs = append(outputs, selectionFalseList[i])
							}
						}
					}
					return outputs, nil
				}
			}
		}
	}
	return []string{}, fmt.Errorf("could not evaluate filter")
}

// evaluate numbers as numbers (note that these have to fall within the constraints of node.Kind
// which is why there isn't a BOOL conversion)
func evaluateFilterBasicLit(val *ast.Node) (Union, error) {
	astBasic, okBasic := (*val).(*ast.BasicLit)
	astIdent, okIdent := (*val).(*ast.Ident)
	if okBasic {
		node := astBasic
		switch node.Kind {
		case token.INT:
			value, err := strconv.ParseInt(node.Value, 10, 64)
			if err != nil {
				return Union{}, err
			}
			return Union{
				tag: INT,
				i:   value,
			}, nil
		case token.FLOAT:
			value, err := strconv.ParseFloat(node.Value, 64)
			if err != nil {
				return Union{}, err
			}
			return Union{
				tag: FLOAT,
				f:   value,
			}, nil
		case token.STRING:
			return Union{
				tag: STRING,
				s:   strings.ReplaceAll(node.Value, "\"", ""),
			}, nil
		default:
			return Union{}, fmt.Errorf("error evaluating ast.BasicLit %v", node.Value)
		}
	}
	if okIdent {
		node := astIdent

		if node.Name == "true" {
			return Union{tag: BOOL, b: true}, nil
		} else if node.Name == "false" {
			return Union{tag: BOOL, b: false}, nil
		}
	}
	return Union{}, fmt.Errorf("invalid rhs [%v] to filter function", val)

}
