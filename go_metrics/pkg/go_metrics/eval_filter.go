package go_metrics

import (
	"fmt"
	"go/ast"
	"go/token"
	"reflect"
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
	default:
		err = fmt.Errorf("unsupported node %+v (type %+v)", *node, reflect.TypeOf(*node))
	}

	return values, err
}

// pull a variable out of the map of inputs
func evaluateFilterIdent(node *ast.Ident, inputList []string) ([]string, error) {
	outputs := make([]string, 0)
	if stringInSlice(inputList, node.Name) {
		for _, input := range Scope[node.Name] {
			outputs = append(outputs, input.Name)
		}
	}

	return inputList, nil
}

// evaluate the result of a binary operation
func evaluateFilterBinary(node *ast.BinaryExpr, inputList []string) ([]string, error) {
	newNodeL := ast.Node(node.X)
	lValues, err := EvaluateDynamicFilter(&newNodeL, inputList) // evaluate the left side before combining with the right
	if err != nil {
		return []string{}, err
	}
	if len(lValues) == 0 {
		return []string{}, fmt.Errorf("left operand of binary expression has length 0")
	}

	expr := ast.Node(node.Y)
	rValue, err := evaluateFilterBasicLit(&expr) // evaluate the right side before combining with the left
	if err != nil {
		return []string{}, err
	}

	// I'm lazy so I'm just using the functions to perform binary operations between left and right operands
	outputs := make([]string, 0)
	for _, input := range lValues {
		lValue := Union{}
		scopeMutex.RLock()
		if _, ok := Scope[input]; ok && len(Scope[input]) > 0 {
			lValue = Scope[input][0].Value
		}
		scopeMutex.RUnlock()
		value := Union{}
		switch node.Op {
		case token.EQL:
			value, err = Equal(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		case token.LSS:
			value, err = LessThan(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		case token.GTR:
			value, err = GreaterThan(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		case token.NEQ:
			value, err = NotEqual(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		case token.LEQ:
			value, err = LessThanOrEqual(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		case token.GEQ:
			value, err = GreaterThanOrEqual(lValue, rValue)
			if err != nil {
				continue
			}
			if value.b {
				outputs = append(outputs, input)
			}
		default:
			err = fmt.Errorf("unsupported binary operation: %s", node.Op)
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
						outputs = append(outputs, MetricsConfig.Attributes[attribute].InputVar)
					}
				} else {
					return []string{}, fmt.Errorf("attribute %s does not exist in Scope", attributeName)
				}
				return outputs, nil
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
				s:   node.Value,
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
