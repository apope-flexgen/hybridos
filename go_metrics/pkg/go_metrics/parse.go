package go_metrics

import (
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"reflect"
	"strings"
)

/**
 * This function parses a string expression into an *Expression. An Expression
 * contains a string representation of an arithmetic expression, the variable names (in a list),
 * and a pointer to the root of an Abstract Syntax Tree that represents the expression.
 *
 * This expression can then be evaluated later (e.g. at run time) after values
 * have been assigned to each of the variables.
 *
 */
func Parse(expression string) (*Expression, error) {
	// parse the expression into an AST
	if strings.Contains(expression, "@") {
		expression = strings.ReplaceAll(expression, "@", ".")
	}
	tree, err := parser.ParseExpr(expression)

	if err != nil {
		return &Expression{
			String:     expression,
			Vars:       []string{},
			Ast:        nil,
			ResultType: NIL,
		}, err
	}

	// break the expression into nodes (as small as we can go)
	// and spit out the variables in the expression
	vars, resultType, err := extract(tree)

	return &Expression{
		String:     expression,
		Vars:       vars,
		Ast:        tree,
		ResultType: resultType,
	}, err
}

/**
* This is a helper function that returns an array of the variables
* that are present in an expression.
 */
func extract(node ast.Node) (vars []string, resultType DataType, err error) {
	switch node := node.(type) {
	case *ast.Ident: // variables
		vars = []string{node.Name}
		if node.Name == "true" || node.Name == "false" || node.Name == "nil" {
			resultType = BOOL
		} else if _, ok := InputScope[node.Name]; ok {
			resultType = BOOL
			for _, input := range InputScope[node.Name] {
				resultType = getResultType(resultType, input.tag)
			}
		} else if _, ok := FilterScope[node.Name]; ok {
			resultType = BOOL

			for _, inputName := range FilterScope[node.Name] {
				if _, ok := InputScope[inputName]; ok {
					for _, input := range InputScope[inputName] {
						resultType = getResultType(resultType, input.tag)
					}
				}
			}
		} else if _, ok := allPossibleAttributes[node.Name]; ok {
			resultType = BOOL
		} else if node.Name == "value" {
			resultType = BOOL
		} else {
			return []string{}, NIL, fmt.Errorf("cannot find variable %v in inputs or filters", node.Name)
		}
	case *ast.CallExpr: //function identifier
		vars, resultType, err = extractFunc(node)
	case *ast.BinaryExpr: // like X + Y
		vars, resultType, err = extractBinary(node)
	case *ast.ParenExpr: // nested functions...I think
		vars, resultType, err = extract(node.X)
	case *ast.UnaryExpr: // like !X
		vars, resultType, err = extractUnary(node)
	case *ast.BasicLit:
		switch node.Kind {
		case token.INT:
			resultType = INT
		case token.FLOAT:
			resultType = FLOAT
		case token.STRING:
			resultType = STRING
		case token.CHAR:
			err = fmt.Errorf("unhandled character as token in basic literal expression")
		default: // imaginary numbers
			err = fmt.Errorf("unhandled imaginary number in basic literal expression")
		}
	case *ast.SelectorExpr:
		vars = []string{node.X.(*ast.Ident).Name + "@" + node.Sel.Name}
		resultType = BOOL
	default:
		err = fmt.Errorf("unsupported node %+v (type %+v)", node, reflect.TypeOf(node))
	}

	return vars, resultType, err
}

// get the arguments given to a function as a []string
func extractFunc(node *ast.CallExpr) ([]string, DataType, error) {
	var resultType DataType
	stringArr := make([]string, 0)
	tags := make([]DataType, 0)
	var tmp []string
	var tag DataType
	var err error
	for _, expr := range node.Args {
		tmp, tag, err = extract(ast.Node(expr))
		if err != nil {
			return stringArr, NIL, err
		}
		stringArr = append(stringArr, tmp...)
		tags = append(tags, tag)
	}
	id, ok := node.Fun.(*ast.Ident)
	if ok {
		switch strings.ToLower(id.Name) {
		case "attribute":
			resultType = BOOL
			for _, varName := range stringArr {
				for _, attribute := range allPossibleAttributes[varName] {
					attributeInput := InputScope[attribute]
					if len(attributeInput) > 0 {
						resultType = getResultType(resultType, attributeInput[0].tag)
					}
				}
			}
		case "value":
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
			}
		case "sum", "add":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
				}
			}
		case "subtract", "sub", "minus":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for subtract function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					err = fmt.Errorf("cannot subtract strings")
					return stringArr, NIL, err
				}
			}
		case "mult", "multiply", "product":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						err = fmt.Errorf("cannot multiply strings")
						return stringArr, NIL, err
					}
				}
			}
		case "scale", "divide", "quo", "quotient", "div":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for divide function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot divide strings")
				}
			}
		case "mod", "modulus", "modulo":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for mod function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot take the mod of strings")
				}
				if tag == FLOAT {
					return stringArr, NIL, fmt.Errorf("cannot take the mod of floats")
				}
				if tag == BOOL {
					return stringArr, NIL, fmt.Errorf("cannot take the mod of bools")
				}
			}
		case "bitwiseand", "bitwise_and":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise and of strings")
					}
					if tag == FLOAT {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise and of floats")
					}
				}
			}
		case "bitwiseor", "bitwise_or":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise or of strings")
					}
					if tag == FLOAT {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise or of floats")
					}
				}
			}
		case "bitwisexor", "bitwise_xor":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise xor of strings")
					}
					if tag == FLOAT {
						return stringArr, NIL, fmt.Errorf("cannot take the bitwise xor of floats")
					}
				}
			}
		case "leftshift", "left_shift", "lsh":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for left shift function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot take the left shift of strings")
				}
				if tag == FLOAT {
					return stringArr, NIL, fmt.Errorf("cannot take the left shift of floats")
				}
				if tag == BOOL {
					return stringArr, NIL, fmt.Errorf("cannot take the left shift of bools")
				}
			}
		case "rightshift", "right_shift", "rsh":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for right shift function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot take the right shift of strings")
				}
				if tag == FLOAT {
					return stringArr, NIL, fmt.Errorf("cannot take the right shift of floats")
				}
				if tag == BOOL {
					return stringArr, NIL, fmt.Errorf("cannot take the right shift of bools")
				}
			}
		case "bitwiseandnot", "bitwise_and_not", "bitwise_andnot":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for bitwise and not function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot take the bitwise and not of strings")
				}
				if tag == FLOAT {
					return stringArr, NIL, fmt.Errorf("cannot take the bitwise and not of floats")
				}
			}
		case "and":
			resultType = BOOL
		case "or":
			resultType = BOOL
		case "not":
			resultType = BOOL
		case "eq", "equ", "equal", "equalto", "isequal", "equal_to", "is_equal":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for Equal function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "neq", "nequ", "nequal", "notequalto", "notequal", "not_equal_to", "not_equal":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for NotEqual function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "lt", "lessthan", "less_than":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for LessThan function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "gt", "greaterthan", "greater_than":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for GreaterThan function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "lte", "lessthanorequal", "less_than_or_equal":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for LessThanOrEqual function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "gte", "greaterthanorequal", "greater_than_or_equal":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for GreaterThanOrEqual function; need at least 2", len(node.Args))
			}
			resultType = BOOL
		case "root", "rt":
			resultType = FLOAT
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for root function; need exactly 2", len(node.Args))
			}
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in root function")
				}
			}
		case "pow", "power", "exp":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for power function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in power function")
				}
			}
		case "max", "maximum":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the max function on strings")
					}
				}
			}
		case "min", "minimum":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the min function on strings")
					}
				}
			}
		case "avg", "average", "mean":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the average function on strings")
					}
				}
			}
			if resultType == BOOL {
				resultType = NIL
				err = fmt.Errorf("cannot use the average function on bools")
			}
		case "floor":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the floor function on strings")
					}
				}
			}
		case "ceil", "ceiling", "ciel", "cieling": //because people are bad at spelling
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the ceilings function on strings")
					}
				}
			}
		case "sqrt":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the sqrt function on strings")
					}
				}
			}
		case "pct", "pctOf", "percentof", "pct_of", "percent_of", "percent":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for percent function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in percent function")
				}
			}
		case "fdiv", "floordiv", "fdivide", "floordivide", "floor_divide":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for floor division function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in floor division function")
				}
			}
		case "abs", "absolutevalue", "absolute_value":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the absolute value function on strings")
					}
				}
			}
		case "round":
			if len(tags) > 0 {
				resultType = tags[0]
				for _, tag := range tags {
					resultType = getResultType(resultType, tag)
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use the round function on strings")
					}
				}
			}
		case "bool", "boolean":
			resultType = BOOL
		case "int", "integer":
			resultType = INT
		case "uint", "unsigned int", "unsigned integer":
			resultType = UINT
		case "float":
			resultType = FLOAT
		case "string", "str", "tostring":
			resultType = STRING
		case "integrate":
			if len(node.Args) < 1 || len(node.Args) > 4 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for integrate function; need at least 1 and at most 4 (input, timescale, minuteReset, minuteOffset)", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in integrate function")
				}
			}
		case "currenttimemilliseconds":
			if len(node.Args) != 0 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for currentTimeMilliseconds function; expected 0", len(node.Args))
			}
			resultType = INT
		case "time":
			if len(node.Args) != 0 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for time function; expected 0", len(node.Args))
			}
			resultType = INT
		case "millisecondssince":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for millisecondsSince function; expected 1", len(node.Args))
			}
			resultType = tags[0]
			if resultType == STRING {
				return stringArr, NIL, fmt.Errorf("cannot use strings in millisecondsSince function")
			}
		case "millisecondstorfc3339":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for millisecondsToRFC3339 function; expected 1", len(node.Args))
			}
			resultType = tags[0]
			if resultType == STRING {
				return stringArr, NIL, fmt.Errorf("cannot use strings in millisecondsToRFC3339 function")
			}
			resultType = STRING
		case "rfc3339":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for RFC3339 function; expected 1", len(node.Args))
			}
			resultType = tags[0]
			if resultType == STRING {
				return stringArr, NIL, fmt.Errorf("cannot use strings in RFC3339 function")
			}
			resultType = STRING
		case "srff":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for SRFF function; need exactly 2", len(node.Args))
			}
			resultType = tags[0]
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in SRFF function")
				}
			}
			resultType = BOOL
		case "rss":
			if len(tags) > 0 {
				for _, tag := range tags {
					if tag == STRING {
						return stringArr, NIL, fmt.Errorf("cannot use strings in RSS function")
					}
				}
				resultType = FLOAT
			}
		case "selectn":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for selectN function; need at least 2", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument of selectN function must be numeric")
			}
			resultType = tags[1]
			for _, tag := range tags[1:] {
				resultType = getResultType(resultType, tag)
			}
		case "enum":
			if len(node.Args) < 3 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for enum function; need at least 3", len(node.Args))
			}
			if (len(node.Args)-1)%2 != 0 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for enum function; need 2n+1", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument of enum function must be numeric")
			}
			resultType = tags[2]
			for i := 4; i < len(tags); i += 2 {
				resultType = getResultType(resultType, tag)
			}
		case "selectorn":
			if len(node.Args) < 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for selectN function; need at least 1", len(node.Args))
			}
			resultType = INT
		case "pulse":
			if len(node.Args) != 3 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for pulse function; need exactly 3", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument (trigger) of pulse function must be bool")
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (reset) of pulse function must be bool")
			}
			if tags[2] == STRING {
				return stringArr, NIL, fmt.Errorf("third argument (timeout) of pulse function must be numeric")
			}
			resultType = BOOL
		case "if", "ifthen", "ifelse", "ifthenelse", "if_then", "if_else", "if_then_else":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for %v function; need at least 2", len(node.Args), id.Name)
			}
			resultType = tags[1]
			for _, tag := range tags[1:] {
				resultType = getResultType(resultType, tag)
			}
		case "compare", "compareor", "compareand":
			if len(node.Args) < 3 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for %v function; need at least 3", len(node.Args), id.Name)
			}
			if operator, ok := node.Args[0].(*ast.BasicLit); !ok {
				return stringArr, NIL, fmt.Errorf("first argument of %v function must be a comparison operator (as a string)", id.Name)
			} else {
				if operator.Kind != token.STRING {
					return stringArr, NIL, fmt.Errorf("first argument of %v function must be a comparison operator (as a string)", id.Name)
				} else {
					tmp := strings.ReplaceAll(operator.Value, "\"", "")
					switch tmp {
					case "==", "!=", "<", ">", "<=", ">=":
						resultType = BOOL
					default:
						return stringArr, NIL, fmt.Errorf("unrecognized comparison operator %v for %v", tmp, id.Name)
					}
				}
			}
		case "maxovertimescale":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for MaxOverTimescale function; need exactly 2", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument (input) of MaxOverTimescale cannot be string")
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (timescale) of MaxOverTimescale function must be int")
			}
			resultType = tags[0]
		case "minovertimescale":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for MinOverTimescale function; need exactly 2", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument (input) of MinOverTimescale cannot be string")
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (timescale) of MinOverTimescale function must be int")
			}
			resultType = tags[0]
		case "avgovertimescale":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for AvgOverTimescale function; need exactly 2", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument (input) of AvgOverTimescale cannot be string")
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (timescale) of AvgOverTimescale function must be int")
			}
			resultType = tags[0]
		case "sumovertimescale":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for SumOverTimescale function; need exactly 2", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("first argument (input) of SumOverTimescale cannot be string")
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (timescale) of SumOverTimescale function must be int")
			}
			resultType = tags[0]
		case "valuechanged":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for ValueChanged function; need exactly 1", len(node.Args))
			}
			resultType = BOOL
		case "valuechangedovertimescale":
			if len(node.Args) != 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for ValueChangedOverTimescale function; need exactly 2", len(node.Args))
			}
			if tags[1] == STRING {
				return stringArr, NIL, fmt.Errorf("second argument (timescale) of SumOverTimescale function must be int")
			}
			resultType = BOOL
		case "quadtosigned":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for QuadToSigned function; need exactly 1", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("argument to QuadToSigned cannot be string")
			}
			resultType = FLOAT
		case "signedtoquad":
			if len(node.Args) != 1 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for SignedToQuad function; need exactly 1", len(node.Args))
			}
			if tags[0] == STRING {
				return stringArr, NIL, fmt.Errorf("argument to SignedToQuad cannot be string")
			}
			resultType = FLOAT
		case "runtime":
			if len(node.Args) < 3 || len(node.Args) > 7 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for Runtime function; need at least 3 and at most 7 (chargeEnergy, dischargeEnergy, powerOutput, gain, upperLimit, minPower, defaultPower)", len(node.Args))
			}
			for _, tag := range tags {
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in Runtime function")
				}
			}
			resultType = FLOAT
		case "unicompare":
			if len(node.Args) < 1 || len(node.Args) > 3 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for Unicompare function; need at least 1 and at most 3 (base, compare, balance)", len(node.Args))
			}
			for _, tag := range tags {
				if tag == STRING {
					return stringArr, NIL, fmt.Errorf("cannot use strings in Unicompare function")
				}
			}
			resultType = FLOAT
		case "count":
			resultType = UINT
		case "combinebits":
			resultType = UINT
		case "in":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for In function; need at least 2 arguments (the value to check for and the list of arguments to look through)", len(node.Args))
			}
			resultType = BOOL
		case "indexfilter":
			if len(node.Args) < 2 {
				return stringArr, NIL, fmt.Errorf("received %v arguments for IndexFilter function; need at least 2 arguments (the condition and the result)", len(node.Args))
			}
			resultType = BOOL
			for _, tag := range tags {
				resultType = getResultType(resultType, tag)
			}
		default:
			return stringArr, NIL, fmt.Errorf("unrecognized function %v", id.Name)
		}
		return stringArr, resultType, err
	} else {
		return stringArr, NIL, fmt.Errorf("could not parse function name")
	}

}

// get the left and right sides of an expression
// evaluate them to their most basic form, then give back their variables
func extractBinary(node *ast.BinaryExpr) ([]string, DataType, error) {

	var vars []string

	switch node.Op {
	case token.ADD, token.SUB, token.MUL, token.QUO, token.REM,
		token.AND, token.OR, token.XOR, token.SHL, token.SHR, token.AND_NOT, token.LAND, token.LOR,
		token.EQL, token.LSS, token.GTR, token.NEQ, token.LEQ, token.GEQ:
		break
	default:
		return vars, NIL, fmt.Errorf("unsupported binary operation: %s", node.Op) // I don't think it's technically possible to get here...
	}

	lVars, lResultType, err := extract(node.X)

	if err != nil {
		return vars, NIL, err
	}

	rVars, rResultType, err := extract(node.Y)

	if err != nil {
		return vars, NIL, err
	}

	vars = append(lVars, rVars...)

	if lResultType == STRING || rResultType == STRING {
		if !(node.Op == token.ADD || node.Op == token.EQL || node.Op == token.LSS || node.Op == token.GTR || node.Op == token.NEQ || node.Op == token.LEQ || node.Op == token.GEQ || node.Op == token.LAND || node.Op == token.LOR) {
			return vars, NIL, fmt.Errorf("cannot perform binary operation %v on strings", node.Op)
		}
	}

	var resultType DataType
	switch node.Op {
	case token.ADD:
		resultType = getResultType(lResultType, rResultType)
	case token.SUB, token.MUL, token.QUO:
		resultType = getResultType(lResultType, rResultType)
	case token.REM:
		resultType = getResultType(lResultType, rResultType)
		if resultType == BOOL {
			return vars, NIL, fmt.Errorf("cannot take the mod of bools")
		}
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the mod of floats")
		}
	case token.AND:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the bitwise and of floats")
		}
	case token.OR:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the bitwise or of floats")
		}
	case token.XOR:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the bitwise xor of floats")
		}
	case token.SHL:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the left shift of floats")
		}
		if resultType == BOOL {
			return vars, NIL, fmt.Errorf("cannot take the left shift of bools")
		}
	case token.SHR:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take the right shift of floats")
		}
		if resultType == BOOL {
			return vars, NIL, fmt.Errorf("cannot take the right shift of bools")
		}
	case token.AND_NOT:
		resultType = getResultType(lResultType, rResultType)
		if resultType == FLOAT {
			return vars, NIL, fmt.Errorf("cannot take &^ of floats")
		}
	case token.LAND, token.LOR, token.EQL, token.NEQ, token.LSS, token.GTR, token.LEQ, token.GEQ:
		resultType = BOOL
	}

	return vars, resultType, err
}

// extract the operand of a unary expression and extract
// the variables from the operand
func extractUnary(node *ast.UnaryExpr) ([]string, DataType, error) {

	var vars []string
	var resultType DataType
	var err error

	switch node.Op {
	case token.NOT, token.SUB:
		break
	default:
		return vars, NIL, fmt.Errorf("unsupported unary operation: %s", node.Op)
	}

	vars, resultType, err = extract(node.X)
	if resultType == STRING && node.Op != token.NOT {
		return vars, NIL, fmt.Errorf("cannot perform unary operation %v on string value", node.Op)
	}
	if node.Op == token.NOT && resultType != NIL {
		resultType = BOOL
	}

	return vars, resultType, err
}
