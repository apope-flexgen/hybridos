package go_metrics

import (
	"fmt"
	"math"
	"math/bits"
	"time"
)

// One thing to note for ALL of these functions:
// FLOAT > INT > UINT > BOOL
// For example, INT + UINT --> INT
// For example, FLOAT + BOOL --> FLOAT
// To maintain "safe" operations later on, most functions return NIL unions if there's an error.

// add strings or numbers
func Add(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	switch resultType {
	case STRING:
		resultString := ""
		for _, arg := range args {
			resultString = resultString + unionValueToString(&arg)
		}
		return Union{tag: STRING, s: resultString}, nil
	case BOOL:
		for _, arg := range args { // no need to cast types here because bool gets overruled by everything else
			if arg.b {
				return Union{tag: BOOL, b: true}, nil
			}
		}
		return Union{tag: BOOL, b: false}, nil
	case UINT:
		sum := uint64(0)
		for _, arg := range args {
			castUnionType(&arg, UINT) // converting bools to uint so no error can happen
			if sum > 0 && arg.ui > math.MaxUint64-sum {
				return Union{}, fmt.Errorf("overflow error when attempting to add numbers")
			}
			sum += arg.ui
		}
		return Union{tag: UINT, ui: sum}, nil
	case INT:
		sum := int64(0)
		for _, arg := range args {
			err := castUnionType(&arg, INT)
			if err != nil {
				return Union{}, err
			}
			if sum > 0 && arg.i > math.MaxInt64-sum {
				return Union{}, fmt.Errorf("overflow error when attempting to add numbers")
			} else if sum < 0 && arg.i < math.MinInt64-sum {
				return Union{}, fmt.Errorf("overflow error when attempting to add negative numbers")
			}
			sum += arg.i
		}
		return Union{tag: INT, i: sum}, nil
	case FLOAT:
		sum := float64(0)
		for _, arg := range args {
			castUnionType(&arg, FLOAT)
			if sum > 0 && arg.f > math.MaxFloat64-sum {
				return Union{}, fmt.Errorf("overflow error when attempting to add numbers")
			} else if sum < 0 && arg.f < -math.MaxFloat64-sum {
				return Union{}, fmt.Errorf("overflow error when attempting to add negative numbers")
			}
			sum += arg.f
		}
		return Union{tag: FLOAT, f: sum}, nil
	}
	return Union{}, fmt.Errorf("cannot add nil values")
}

// subtract arg2 from arg1
func Sub(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	switch resultType {
	case STRING:
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot subtract strings")
	case BOOL:
		return Union{tag: BOOL, b: arg1.b != arg2.b}, nil
	case UINT:
		castUnionType(&arg1, UINT)
		castUnionType(&arg2, UINT)

		if arg2.ui > 0 && arg1.ui < arg2.ui {
			if arg2.ui-arg1.ui > uint64(math.MaxInt64) {
				return Union{}, fmt.Errorf("overflow error when attempting to subtract numbers")
			}
			return Union{tag: INT, i: -int64(arg2.ui - arg1.ui)}, nil
		} else {
			return Union{tag: UINT, ui: arg1.ui - arg2.ui}, nil
		}
	case INT:
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}

		if arg2.i > 0 && arg1.i < math.MinInt64+arg2.i {
			return Union{}, fmt.Errorf("overflow error when attempting to subtract numbers")
		} else if arg2.i < 0 && arg1.i > math.MaxInt64+arg2.i {
			return Union{}, fmt.Errorf("overflow error when attempting to subtract numbers")
		}
		return Union{tag: INT, i: arg1.i - arg2.i}, nil
	case FLOAT:
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)

		if arg2.f > 0 && arg1.f < -math.MaxFloat64+arg2.f {
			return Union{}, fmt.Errorf("overflow error when attempting to subtract numbers")
		} else if arg2.f < 0 && arg1.f > math.MaxFloat64+arg2.f {
			return Union{}, fmt.Errorf("overflow error when attempting to subtract numbers")
		}
		return Union{tag: FLOAT, f: arg1.f - arg2.f}, nil
	}
	return Union{}, fmt.Errorf("cannot subtract nil values")
}

// multiply numbers
func Mult(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	switch resultType {
	case STRING:
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot multiply strings")
	case BOOL:
		for _, arg := range args { // no need to cast types here because bool gets overruled by everything else
			if !arg.b {
				return Union{tag: BOOL, b: false}, nil
			}
		}
		return Union{tag: BOOL, b: true}, nil
	case UINT:
		prod := uint64(1)
		for _, arg := range args {
			castUnionType(&arg, UINT)
			if arg.ui > 1 && prod != 0 && arg.ui > math.MaxUint64/prod {
				return Union{}, fmt.Errorf("overflow error when attempting to multiply numbers")
			}
			prod *= arg.ui
		}
		return Union{tag: UINT, ui: prod}, nil
	case INT:
		prod := int64(1)
		var err error
		for _, arg := range args {
			err = castUnionType(&arg, INT)
			if err != nil {
				return Union{}, err
			}
			if math.Abs(float64(arg.i)) > 1.0 && prod != 0 && math.Abs(float64(arg.i)) > math.Abs(float64(math.MaxInt64/prod)) {
				return Union{}, fmt.Errorf("overflow error when attempting to multiply numbers")
			}
			prod *= arg.i
		}
		return Union{tag: INT, i: prod}, nil
	case FLOAT:
		prod := 1.0
		for _, arg := range args {
			castUnionType(&arg, FLOAT)
			if math.Abs(arg.f) > 1 && prod != 0 && math.Abs(arg.f) > math.Abs(math.MaxFloat64/prod) {
				return Union{}, fmt.Errorf("overflow error when attempting to multiply numbers")
			}
			prod *= arg.f
		}
		return Union{tag: FLOAT, f: prod}, nil
	}

	return Union{}, fmt.Errorf("cannot multiply nil values")
}

// divide arg1 by arg2
func Div(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	switch resultType {
	case STRING:
		return Union{}, fmt.Errorf("cannot divide strings")
	case BOOL:
		if !arg2.b {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		return Union{tag: BOOL, b: arg1.b}, nil
	case UINT:
		castUnionType(&arg1, UINT)
		castUnionType(&arg2, UINT)
		if arg2.ui == 0 {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		return Union{tag: UINT, ui: arg1.ui / arg2.ui}, nil
	case INT:
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}
		if arg2.i == 0 {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		return Union{tag: INT, i: arg1.i / arg2.i}, nil
	case FLOAT:
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		if arg2.f == 0 {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		if math.Abs(arg2.f) < 1 && math.Abs(arg1.f) > math.Abs(math.MaxFloat64*arg2.f) {
			return Union{}, fmt.Errorf("overflow error when attempting to divide numbers")
		}
		return Union{tag: FLOAT, f: arg1.f / arg2.f}, nil
	}
	return Union{}, fmt.Errorf("cannot divide nil values")
}

// find arg1 mod arg2
func Mod(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	var div Union
	div.tag = resultType
	if resultType == STRING {
		return Union{}, fmt.Errorf("cannot take modulus of strings")
	} else if resultType == FLOAT {
		return Union{}, fmt.Errorf("cannot take modulus of floats")
	} else if resultType == BOOL {
		return Union{}, fmt.Errorf("cannot take modulus of bools")
	} else if resultType == INT {
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}
		if arg2.i == 0 {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		div.i = arg1.i % arg2.i
	} else {
		castUnionType(&arg1, UINT)
		castUnionType(&arg2, UINT)
		if arg2.ui == 0 {
			return Union{}, fmt.Errorf("attempted to divide by 0")
		}
		div.ui = arg1.ui % arg2.ui
	}
	return div, nil
}

// & for uint, int, or bool
func BitwiseAnd(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	} else if len(args) == 1 {
		return args[0], nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take Bitwise And of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take Bitwise And of floats")
	}
	result := args[0]
	err := castUnionType(&result, resultType)
	if err != nil {
		return Union{}, err
	}
	for i := 1; i < len(args); i++ {
		err = castUnionType(&args[i], resultType)
		if err != nil {
			return Union{}, err
		}
		if resultType == INT {
			result.i = result.i & args[i].i
		} else if resultType == UINT {
			result.ui = result.ui & args[i].ui
		} else {
			result.b = result.b && args[i].b
		}
	}
	return result, nil
}

// | for uint, int, or bool
func BitwiseOr(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	} else if len(args) == 1 {
		return args[0], nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take Bitwise Or of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take Bitwise Or of floats")
	}
	result := args[0]
	err := castUnionType(&result, resultType)
	if err != nil {
		return Union{}, err
	}
	for i := 1; i < len(args); i++ {
		err = castUnionType(&args[i], resultType)
		if err != nil {
			return Union{}, err
		}
		if resultType == INT {
			result.i = result.i | args[i].i
		} else if resultType == UINT {
			result.ui = result.ui | args[i].ui
		} else {
			result.b = result.b || args[i].b
		}
	}
	return result, nil
}

// ^ for uint, int, or bool
func BitwiseXor(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	} else if len(args) == 1 {
		return args[0], nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take Bitwise Xor of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take Bitwise Xor of floats")
	}
	result := args[0]
	err := castUnionType(&result, resultType)
	if err != nil {
		return Union{}, err
	}
	for i := 1; i < len(args); i++ {
		err = castUnionType(&args[i], resultType)
		if err != nil {
			return Union{}, err
		}
		if resultType == INT {
			result.i = result.i ^ args[i].i
		} else if resultType == UINT {
			result.ui = result.ui ^ args[i].ui
		} else {
			result.b = result.b != args[i].b
		}
	}
	return result, nil
}

// << for int or uint
func LeftShift(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	var result Union
	result.tag = resultType
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take left shift of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take left shift of floats")
	} else if resultType == BOOL {
		return Union{tag: BOOL, b: false}, fmt.Errorf("cannot take left shift of two bools")
	} else if resultType == INT {
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		if arg1.i == 0 {
			return Union{tag: INT, i: 0}, nil
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}
		if arg2.i < 0 {
			return Union{}, fmt.Errorf("cannot perform left shift when right operand is negative")
		} else if arg2.i == 0 {
			return arg1, nil
		} else if arg2.i > 62 && arg1.i != -1 {
			return Union{}, fmt.Errorf("cannot perform left shift when right operand is greater than 63")
		} else if arg1.i == math.MinInt64 {
			return Union{}, fmt.Errorf("performing left shift will overflow int64")
		} else if arg1.i == -1 && arg2.i == 63 {
			return Union{tag: INT, i: -1 << 63}, nil
		}
		msb := -1
		L := 0
		R := 63
		mid := 31
		for L <= R {
			mid = (L + R) / 2
			if arg1.i >= 0 && (1<<mid) > arg1.i {
				msb = mid - 1
				R = mid - 1
			} else if arg1.i < 0 && (1<<mid) > -arg1.i {
				msb = mid - 1
				R = mid - 1
			} else {
				L = mid + 1
			}
		}
		if arg2.i+int64(msb) > int64(62) {
			return Union{}, fmt.Errorf("performing left shift will overflow int64")
		}
		result.i = arg1.i << arg2.i
	} else {
		castUnionType(&arg1, UINT)
		if arg1.ui == 0 {
			return Union{tag: UINT, ui: 0}, nil
		}
		castUnionType(&arg2, UINT)
		if arg2.ui > 63 {
			return Union{}, fmt.Errorf("cannot perform left shift when right operand is greater than 63")
		} else if arg2.ui+uint64(63-bits.LeadingZeros64(arg1.ui)) > 63 {
			return Union{}, fmt.Errorf("performing left shift will overflow int64")
		}
		result.ui = arg1.ui << arg2.ui
	}

	return result, nil
}

// >> for int or uint
func RightShift(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	var result Union
	result.tag = resultType
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take right shift of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take right shift of floats")
	} else if resultType == BOOL {
		return Union{tag: BOOL, b: false}, fmt.Errorf("cannot take right shift of two bools")
	} else if resultType == INT {
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}
		if arg2.i < 0 {
			return Union{}, fmt.Errorf("cannot perform right shift when right operand is negative")
		}
		result.i = arg1.i >> arg2.i
	} else {
		castUnionType(&arg1, UINT)
		castUnionType(&arg2, UINT)
		result.ui = arg1.ui >> arg2.ui
	}
	return result, nil
}

// &^ for uint, int, or bool
func BitwiseAndNot(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)

	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take bitwise and not of strings")
	} else if resultType == FLOAT {
		return Union{tag: FLOAT, f: 0.0}, fmt.Errorf("cannot take bitwise and not of floats")
	} else if resultType == INT {
		err := castUnionType(&arg1, INT)
		if err != nil {
			return Union{}, err
		}
		err = castUnionType(&arg2, INT)
		if err != nil {
			return Union{}, err
		}
		arg1.i = arg1.i &^ arg2.i
	} else if resultType == UINT {
		castUnionType(&arg1, UINT)
		castUnionType(&arg2, UINT)
		arg1.ui = arg1.ui &^ arg2.ui
	} else {
		arg1.b = arg1.b && (!arg2.b)
	}
	return arg1, nil
}

// logical and. Converts everything to booleans first. Strings behave as true if len(str) > 0
func And(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{tag: BOOL, b: false}, nil
	}
	var result = Union{tag: BOOL, b: true}
	for _, arg := range args {
		castUnionType(&arg, BOOL)
		result.b = result.b && arg.b
	}
	return result, nil
}

// logical or. Converts everything to booleans first. Strings behave as true if len(str) > 0
func Or(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{tag: BOOL, b: false}, nil
	}
	var result Union
	result.tag = BOOL
	for _, arg := range args {
		castUnionType(&arg, BOOL)
		result.b = result.b || arg.b
	}
	return result, nil
}

// logical not. Converts everything to booleans first. Strings behave as true if len(str) > 0
func Not(args ...Union) ([]Union, error) {
	for i, arg := range args {
		castUnionType(&arg, BOOL)
		arg.b = !arg.b
		args[i] = arg
	}
	return args, nil
}

// checks if all unions store the same value
// treats 1.0 == 1 as true
// treats 1 == true, 2 != true
// treats "any_string" != true (even if len > 0)
func Equal(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for equality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			return Union{tag: BOOL, b: false}, nil
		} else if dataType == STRING {
			b = arg1.s == arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f == arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// checks if two unions do not store the same value
// treats 1.0 != 1 as false
func NotEqual(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for inequality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			b = true
			continue
		} else if dataType == STRING {
			b = arg1.s != arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f != arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// checks if arg1 < arg2
func LessThan(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for inequality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			return Union{}, fmt.Errorf("cannot compare string and non-string using < operator")
		} else if dataType == STRING {
			b = arg1.s < arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f < arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// checks if arg1 > arg2
func GreaterThan(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for inequality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			return Union{}, fmt.Errorf("cannot compare string and non-string using > operator")
		} else if dataType == STRING {
			b = arg1.s > arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f > arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// checks if arg1 <= arg2
func LessThanOrEqual(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for inequality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			return Union{}, fmt.Errorf("cannot compare string and non-string using <= operator")
		} else if dataType == STRING {
			b = arg1.s <= arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f <= arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// checks if arg1 >= arg2
func GreaterThanOrEqual(args ...Union) (Union, error) {
	if len(args) == 0 || len(args) == 1 {
		return Union{}, fmt.Errorf("cannot compare a single value for inequality")
	}
	var b bool
	for i, arg2 := range args[1:] {
		arg1 := args[i]
		dataType := getResultType(arg1.tag, arg2.tag)
		if dataType == STRING && (arg1.tag != STRING || arg2.tag != STRING) {
			return Union{}, fmt.Errorf("cannot compare string and non-string using >= operator")
		} else if dataType == STRING {
			b = arg1.s >= arg2.s
			if !b {
				break
			} else {
				continue
			}
		}
		castUnionType(&arg1, FLOAT)
		castUnionType(&arg2, FLOAT)
		b = arg1.f >= arg2.f
		if !b {
			break
		}
	}
	return Union{tag: BOOL, b: b}, nil
}

// evaluate the Nth root of a number
func Root(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot take a root of a string")
	}
	castUnionType(&arg1, FLOAT)
	castUnionType(&arg2, FLOAT)
	if arg2.f == 0 {
		return Union{}, fmt.Errorf("cannot take the 0th root of a number")
	}
	resultVal := math.Pow(arg1.f, 1/arg2.f)
	if math.IsInf(resultVal, 0) || math.IsNaN(resultVal) {
		return Union{}, fmt.Errorf("overflow error when finding the root of a number")
	}
	return Union{tag: FLOAT, f: resultVal}, nil
}

// raise arg1 to the power of arg2
func Pow(arg1, arg2 Union) (Union, error) {
	var resultType DataType
	resultType = getResultType(arg1.tag, arg2.tag)
	if resultType == STRING {
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot raise a string to a power")
	}
	castUnionType(&arg1, FLOAT)
	castUnionType(&arg2, FLOAT)
	resultVal := math.Pow(arg1.f, arg2.f)
	if math.IsInf(resultVal, 0) || math.IsNaN(resultVal) {
		return Union{}, fmt.Errorf("overflow error when finding the power of a number")
	}
	return castValueToUnionType(resultVal, resultType), nil
}

// find the maximum of a list of numbers
func Max(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	}
	max := math.Inf(-1)
	var indexOfMax int
	var err error
	for i, arg := range args {
		err = castUnionType(&arg, FLOAT)
		if err != nil {
			return Union{}, fmt.Errorf("cannot find the maximum of a list of strings")
		}
		if arg.f > max {
			max = arg.f
			indexOfMax = i
		}
	}
	return args[indexOfMax], nil
}

// find the minimum of a list of numbers
func Min(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	}
	min := math.Inf(1)
	var indexOfMin int
	var err error
	for i, arg := range args {
		err = castUnionType(&arg, FLOAT)
		if err != nil {
			return Union{}, fmt.Errorf("cannot find the minimum of a list of strings")
		}
		if arg.f < min {
			min = arg.f
			indexOfMin = i
		}
	}
	return args[indexOfMin], nil
}

// find the average of a list of numbers
func Avg(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	} else if len(args) == 1 {
		return args[0], nil
	}
	sum, _ := Add(args...)

	switch sum.tag {
	case STRING:
		return Union{tag: STRING, s: ""}, fmt.Errorf("cannot find the average of a list of strings")
	case FLOAT:
		avg := sum.f / float64(len(args))
		return Union{tag: FLOAT, f: avg}, nil
	case INT:
		avg := sum.i / int64(len(args))
		return Union{tag: INT, i: avg}, nil
	case UINT:
		avg := sum.ui / uint64(len(args))
		return Union{tag: UINT, ui: avg}, nil
	default:
		return Union{tag: BOOL, b: false}, fmt.Errorf("cannot find the average of a list of bools")
	}
}

// find the floor of a number
func Floor(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		if arg.tag == STRING {
			err = fmt.Errorf("cannot find the floor of a string")
		} else if arg.tag == FLOAT {
			arg.f = math.Floor(arg.f)
			args[i] = arg
		}
	}
	return args, err
}

// find the ceiling of a number
func Ceil(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		if arg.tag == STRING {
			err = fmt.Errorf("cannot find the ceiling of a string")
		} else if arg.tag == FLOAT {
			arg.f = math.Ceil(arg.f)
			args[i] = arg
		}
	}
	return args, err
}

// evaluate the square root of a number
// always returns a float if sqrt is possible
func Sqrt(args ...Union) ([]Union, error) {
	var err error
	var resultVal float64
	for i, arg := range args {
		if arg.tag == STRING {
			err = fmt.Errorf("cannot take the square root of a string")
		} else {
			castUnionType(&arg, FLOAT)
			resultVal = math.Sqrt(arg.f)
			if math.IsInf(resultVal, 0) || math.IsNaN(resultVal) {
				err = fmt.Errorf("overflow error when finding the power of a number")
				continue
			}
			arg.f = resultVal
			args[i] = arg
		}
	}
	return args, err
}

// finds arg1 as a percent of arg2
func Pct(arg1, arg2 Union) (Union, error) {
	resultType := getResultType(arg1.tag, arg2.tag)
	resultType = getResultType(resultType, UINT)
	if resultType == STRING {
		return Union{}, fmt.Errorf("cannot divide strings")
	}
	castUnionType(&arg1, FLOAT)
	castUnionType(&arg2, FLOAT)
	if arg2.f == 0 {
		return Union{}, fmt.Errorf("attempted to divide by 0")
	}
	if math.Abs(arg2.f) < 1 && math.Abs(arg1.f) > math.Abs(math.MaxFloat64*arg2.f) {
		return Union{}, fmt.Errorf("overflow error when attempting to divide numbers")
	}
	result, err := Mult([]Union{Union{tag: FLOAT, f: arg1.f / arg2.f}, Union{tag: UINT, ui: 100}}...)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&result, resultType)
	return result, err
}

func FloorDiv(arg1, arg2 Union) (Union, error) {
	result, err := Div(arg1, arg2)
	if err != nil {
		return result, err
	}
	result.f = math.Floor(result.f) // we don't care about doing a type conversion because if it's any other type, it's already "floored"
	return result, err
}

// evaluate the absolute value of a number
func Abs(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		switch arg.tag {
		case STRING:
			err = fmt.Errorf("cannot take the absolute value of a string")
			args[i] = Union{}
		case BOOL, UINT:
			// do nothing; already absolute
		case INT:
			if arg.i < 0 {
				if arg.i == math.MinInt64 {
					err = fmt.Errorf("cannot perform type-safe Abs(MinInt64)")
					args[i] = Union{}
				} else {
					arg.i = -arg.i
					args[i] = arg
				}
			} // else do nothing
		case FLOAT:
			arg.f = math.Abs(arg.f)
			args[i] = arg
		}
	}
	return args, err
}

// round a float to a whole number
func Round(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		if arg.tag == STRING {
			err = fmt.Errorf("cannot round a string")
		} else if arg.tag == FLOAT {
			arg.f = math.Round(arg.f)
			args[i] = arg
		}
	}
	return args, err
}

// convert union to bool-type union
func Bool(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		castUnionType(&arg, BOOL) // there can never be an error when converting to a bool
		args[i] = arg
	}
	return args, err
}

// convert union to int-type union
func Int(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		tempErr := castUnionType(&arg, INT)
		if tempErr != nil { // if there's another error, we don't want to overwrite it with nil
			err = tempErr
		}
		args[i] = arg
	}
	return args, err
}

// convert union to uint-type union
func UInt(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		tempErr := castUnionType(&arg, UINT)
		if tempErr != nil { // if there's another error, we don't want to overwrite it with nil
			err = tempErr
		}
		args[i] = arg
	}
	return args, err
}

// convert union to float-type union
func Float(args ...Union) ([]Union, error) {
	var err error
	for i, arg := range args {
		tempErr := castUnionType(&arg, FLOAT)
		if tempErr != nil { // if there's another error, we don't want to overwrite it with nil
			err = tempErr
		}
		args[i] = arg
	}
	return args, err
}

// convert union to string-type union
func String(args ...Union) ([]Union, error) {
	for i, _ := range args {
		castUnionType(&args[i], STRING)
	}
	return args, nil
}

// integrate the input over a timescale of 1 hour
func Integrate(input, timescale, minuteReset, minuteOffset Union, state *map[string][]Union) (Union, error) {
	var dataType DataType
	dataType = input.tag
	err := castUnionType(&input, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&timescale, FLOAT)
	if err != nil {
		return Union{}, err
	}
	timescale_f := timescale.f * 3600 * 1000 // timescale (in hours) * milliseconds/hr = timescale (in milliseconds)

	err = castUnionType(&minuteReset, INT)
	if err != nil {
		return Union{}, err
	}

	err = castUnionType(&minuteOffset, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for Integrate")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamp"] = []Union{Union{tag: INT, i: (time.Now()).UnixMilli()}}
		(*state)["minute"] = make([]Union, 1)
	} else if _, ok := (*state)["timestamp"]; !ok || len((*state)["timestamp"]) == 0 {
		(*state)["timestamp"] = []Union{Union{tag: INT, i: (time.Now()).UnixMilli()}}
		(*state)["minute"] = make([]Union, 1)
	}

	prevValue := (*state)["value"]
	if prevValue == nil || len(prevValue) == 0 {
		prevValue = []Union{Union{tag: FLOAT, f: 0}}
	}
	state_minute := (*state)["minute"]
	if state_minute == nil || len(state_minute) == 0 {
		state_minute = make([]Union, 1)
	}

	err = castUnionType(&prevValue[0], FLOAT)
	if err != nil {
		return Union{}, err
	}

	timestamp := (time.Now()).UnixMilli()
	minutes := int64((time.Now()).Minute())
	if oldTimestamp, ok := (*state)["timestamp"]; ok {
		delta := (input.f * float64(timestamp-oldTimestamp[0].i)) / float64(timescale_f)
		if minuteReset.i != -1 && minutes != state_minute[0].i && ((minutes-minuteOffset.i)%minuteReset.i) == 0 {
			prevValue[0].f = delta
		} else {
			prevValue[0].f += delta
		}
	} else {
		(*state)["timestamp"] = make([]Union, 1)
	}
	(*state)["timestamp"][0] = Union{tag: INT, i: timestamp}
	(*state)["minute"][0] = Union{tag: INT, i: minutes}
	err = castUnionType(&prevValue[0], dataType)
	return prevValue[0], err
}

// spit out the current time in milliseconds
func CurrentTimeMilliseconds() (Union, error) {
	return Union{tag: INT, i: time.Now().UnixMilli()}, nil
}

// calculate the milliseconds since the input time (also in milliseconds)
func MillisecondsSince(input Union) (Union, error) {
	currentTime := time.Now().UnixMilli()
	err := castUnionType(&input, INT)
	if err != nil {
		return Union{}, err
	}

	// technically, this can happen but I don't think it ever would...
	if input.i < 0 && currentTime > math.MaxInt64+input.i {
		return Union{}, fmt.Errorf("overflow error when finding milliseconds since %d", input.i)
	}
	result := Union{tag: INT, i: (currentTime - input.i)}
	return result, nil
}

// convert a millisecond time input to RFC3339
func MillisecondsToRFC3339(input Union) (Union, error) {
	err := castUnionType(&input, INT)
	if err != nil {
		return Union{}, err
	}
	timeObj := time.UnixMilli(input.i)
	timeStr := timeObj.Format(time.RFC3339)

	return Union{tag: STRING, s: timeStr}, nil
}

// set-reset flip-flop
func Srff(arg1, arg2 Union, state *map[string][]Union) (Union, error) {
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for SRFF")
	} else if arg1.tag == STRING || arg2.tag == STRING {
		return Union{}, fmt.Errorf("cannot use strings in SRFF")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
	}
	if _, ok := (*state)["q"]; !ok {
		(*state)["q"] = []Union{Union{tag: BOOL, b: false}}
	}

	castUnionType(&arg1, BOOL)
	castUnionType(&arg2, BOOL)

	if arg1.b {
		(*state)["q"][0] = Union{tag: BOOL, b: true}
	} else if arg2.b {
		(*state)["q"][0] = Union{tag: BOOL, b: false}
	}
	return (*state)["q"][0], nil
}

// residual sum of squares of the arguments
func Rss(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, nil
	}
	resultType := args[0].tag
	for _, arg := range args {
		resultType = getResultType(arg.tag, resultType)
	}
	if resultType == STRING {
		return Union{}, fmt.Errorf("cannot take residual sum of squares on strings")
	}
	sum := 0.0
	for _, arg := range args {
		castUnionType(&arg, FLOAT)
		if math.Abs(arg.f) > 1 && math.Abs(arg.f) > math.Abs(math.MaxFloat64/arg.f) {
			return Union{}, fmt.Errorf("overflow error when attempting to multiply numbers")
		}
		if sum > 0 && arg.f*arg.f > math.MaxFloat64-sum {
			return Union{}, fmt.Errorf("overflow error when attempting to add numbers")
		}
		sum += arg.f * arg.f
	}
	rss := math.Sqrt(sum)
	return Union{tag: FLOAT, f: rss}, nil
}

// based off of args[0], select one of the subsequent inputs
func SelectN(input Union, enumNames ...Union) (Union, error) {
	err := castUnionType(&input, UINT)
	if err != nil {
		return Union{}, err
	}
	if input.ui > uint64(len(enumNames)) || input.ui <= 0 {
		return Union{}, fmt.Errorf("invalid value %v for selectN %v", input.ui, enumNames)
	}
	return enumNames[int(input.ui)-1], nil
}

// based off of args[0], select one of the subsequent inputs
// takes the form Enum(inputVal, enumVal1, enumString1, enumVal2, enumString2, ..., enumValN, enumStringN)
// inputVal and enumVal_i should be INT unions
func Enum(input Union, enumNames ...Union) (Union, error) {
	err := castUnionType(&input, INT)
	if err != nil {
		return Union{}, err
	}
	if len(enumNames)%2 != 0 {
		return Union{}, fmt.Errorf("enum function requires 2n + 1 arguments")
	}
	val := Union{tag: BOOL, b: false}
	for i := 0; i < len(enumNames); i += 2 {
		if val, _ = Equal(input, enumNames[i]); val.b {
			return enumNames[i+1], nil
		}
	}
	return Union{tag: STRING, s: "Unknown"}, nil
}

// give the index of the first input that's true (as an int); else return -1
func SelectorN(args ...Union) (Union, error) {
	if len(args) == 0 {
		return Union{}, fmt.Errorf("selectoLetr N requires at least one argument")
	}

	for i, arg := range args {
		castUnionType(&arg, BOOL)
		if arg.b {
			return Union{tag: INT, i: int64(i + 1)}, nil
		}
	}
	return Union{tag: INT, i: -1}, nil
}

// pulse a signal on and off (true/false)
func Pulse(trigger, reset, timeout Union, state *map[string][]Union) (Union, error) {
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for Pulse")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["triggerEvent"] = make([]Union, 1)
		(*state)["timeout"] = make([]Union, 1)
	}

	value := Union{tag: BOOL, b: false}
	err := castUnionType(&trigger, BOOL)
	if err != nil { // don't think you can get here
		return Union{}, fmt.Errorf("trigger for pulse event must be bool")
	}
	err = castUnionType(&reset, BOOL)
	if err != nil { // don't think you can get here
		return Union{}, fmt.Errorf("reset for pulse event must be bool")
	}
	err = castUnionType(&timeout, INT)
	if err != nil {
		return Union{}, fmt.Errorf("timeout for pulse event must be int")
	}
	inTrigger, ok := (*state)["triggerEvent"]
	if !ok || (ok && !inTrigger[0].b) { // if we are NOT already in a trigger event...
		if reset.b { // if reset and trigger are both set at the same time, reset wins
			value.b = false
		} else if trigger.b { // ...check for trigger
			(*state)["triggerEvent"][0] = Union{tag: BOOL, b: true}                           // record that we are now in a trigger event
			(*state)["timeout"][0] = Union{tag: INT, i: (time.Now().UnixMilli() + timeout.i)} // trigger event will end after configured time period
			value.b = true                                                                    // output will be high during the trigger event
		}
	} else if reset.b {
		(*state)["triggerEvent"][0] = Union{tag: BOOL, b: false} // if we are already in a trigger event...
		value.b = false                                          // note true for reset
	} else if time.Now().UnixMilli() >= (*state)["timeout"][0].i { // ...check if the trigger event is over
		(*state)["triggerEvent"][0] = Union{tag: BOOL, b: false}
		value.b = false
	} else {
		value.b = true
	}

	return value, nil
}

func Compare(operator, reference Union, args ...Union) ([]Union, error) {
	var err error
	if operator.tag != STRING {
		return []Union{}, fmt.Errorf("comparison operator for Compare must be entered as a string")
	}
	switch operator.s {
	case "==":
		for i, arg := range args {
			args[i], err = Equal(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	case "!=":
		for i, arg := range args {
			args[i], err = NotEqual(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	case "<":
		for i, arg := range args {
			args[i], err = LessThan(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	case ">":
		for i, arg := range args {
			args[i], err = GreaterThan(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	case "<=":
		for i, arg := range args {
			args[i], err = LessThanOrEqual(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	case ">=":
		for i, arg := range args {
			args[i], err = GreaterThanOrEqual(arg, reference)
			if err != nil {
				return []Union{}, err
			}
		}
	default:
		return []Union{}, fmt.Errorf("unrecognized comparison operator %s", operator.s)
	}
	return args, nil
}

func CompareOr(operator, reference Union, args ...Union) (Union, error) {
	var err error
	if operator.tag != STRING {
		return Union{}, fmt.Errorf("comparison operator for CompareOr must be entered as a string")
	}
	switch operator.s {
	case "==":
		for i, arg := range args {
			args[i], err = Equal(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	case "!=":
		for i, arg := range args {
			args[i], err = NotEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	case "<":
		for i, arg := range args {
			args[i], err = LessThan(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	case ">":
		for i, arg := range args {
			args[i], err = GreaterThan(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	case "<=":
		for i, arg := range args {
			args[i], err = LessThanOrEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	case ">=":
		for i, arg := range args {
			args[i], err = GreaterThanOrEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if args[i].b {
				break
			}
		}
	default:
		return Union{}, fmt.Errorf("unrecognized comparison operator %s", operator.s)
	}
	return Or(args...)
}

func CompareAnd(operator, reference Union, args ...Union) (Union, error) {
	var err error
	if operator.tag != STRING {
		return Union{}, fmt.Errorf("comparison operator for CompareAnd must be entered as a string")
	}
	switch operator.s {
	case "==":
		for i, arg := range args {
			args[i], err = Equal(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	case "!=":
		for i, arg := range args {
			args[i], err = NotEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	case "<":
		for i, arg := range args {
			args[i], err = LessThan(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	case ">":
		for i, arg := range args {
			args[i], err = GreaterThan(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	case "<=":
		for i, arg := range args {
			args[i], err = LessThanOrEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	case ">=":
		for i, arg := range args {
			args[i], err = GreaterThanOrEqual(arg, reference)
			if err != nil {
				return Union{}, err
			}
			if !args[i].b {
				break
			}
		}
	default:
		return Union{}, fmt.Errorf("unrecognized comparison operator %s", operator.s)
	}
	return And(args...)
}

func MaxOverTimescale(input, timescale Union, state *map[string][]Union) (Union, error) {
	var err error
	if input.tag == STRING || input.tag == NIL {
		return Union{}, fmt.Errorf("cannot find the maximum value of a set of strings")
	}
	err = castUnionType(&timescale, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for MaxOverTimescale")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
		(*state)["indexOfMax"] = []Union{Union{tag: UINT, ui: 0}}
	} else if _, ok := (*state)["timestamps"]; !ok || len((*state)["timestamps"]) == 0 {
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
		(*state)["indexOfMax"] = []Union{Union{tag: UINT, ui: 0}}
	}

	index := int((*state)["currentIndex"][0].ui)
	max := (*state)["values"][int((*state)["indexOfMax"][0].ui)]
	currentTime := time.Now().UnixMilli()

	// remove older data points
	indexToTruncate := -1
	for i, timestamp := range (*state)["timestamps"][0:index] {
		if timestamp.i != 0 && timestamp.i < currentTime-timescale.i {
			indexToTruncate = i
		} else {
			break
		}
	}
	(*state)["timestamps"] = (*state)["timestamps"][indexToTruncate+1:]
	(*state)["values"] = (*state)["values"][indexToTruncate+1:]

	// if the old maximum was truncated, find the new maximum
	if indexToTruncate >= 0 {
		(*state)["currentIndex"][0].ui = uint64(index - indexToTruncate - 1)
		if (*state)["indexOfMax"][0].ui >= uint64(indexToTruncate) && (*state)["indexOfMax"][0].ui > 0 {
			(*state)["indexOfMax"][0].ui = (*state)["indexOfMax"][0].ui - uint64(indexToTruncate+1)
		} else {
			(*state)["indexOfMax"][0].ui = 0
			switch input.tag {
			case BOOL:
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.b {
						max = (*state)["values"][i]
						(*state)["indexOfMax"][0].ui = uint64(i)
					}
				}
			case UINT:
				max = Union{tag: UINT, ui: 0}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.ui >= max.ui {
						max = (*state)["values"][i]
						(*state)["indexOfMax"][0].ui = uint64(i)
					}
				}
			case INT:
				max = Union{tag: INT, i: math.MinInt64}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.i >= max.i {
						max = value
						(*state)["indexOfMax"][0].ui = uint64(i)
					}
				}
			case FLOAT:
				max = Union{tag: FLOAT, f: -math.MaxInt64}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.f >= max.f {
						max = value
						(*state)["indexOfMax"][0].ui = uint64(i)
					}
				}
			}

		}
	}
	index = int((*state)["currentIndex"][0].ui)

	// add the current data point to the history
	(*state)["timestamps"][index].tag = INT
	(*state)["timestamps"][index].i = currentTime
	(*state)["values"][index] = input

	// compare it to the max
	switch input.tag {
	case BOOL:
		if input.b {
			(*state)["indexOfMax"][0].ui = uint64(index)
		}
	case UINT:
		if input.ui >= max.ui {
			(*state)["indexOfMax"][0].ui = uint64(index)
		}
	case INT:
		if input.i >= max.i {
			(*state)["indexOfMax"][0].ui = uint64(index)
		}
	case FLOAT:
		if input.f >= max.f {
			(*state)["indexOfMax"][0].ui = uint64(index)
		}
	}
	(*state)["currentIndex"][0].ui += 1
	if (*state)["currentIndex"][0].ui > uint64(len((*state)["timestamps"])) {
		(*state)["timestamps"] = append((*state)["timestamps"], Union{})
		(*state)["values"] = append((*state)["values"], Union{})
	}
	return (*state)["values"][int((*state)["indexOfMax"][0].ui)], nil
}

func MinOverTimescale(input, timescale Union, state *map[string][]Union) (Union, error) {
	var err error
	if input.tag == STRING || input.tag == NIL {
		return Union{}, fmt.Errorf("cannot find the minimum value of a set of strings")
	}
	err = castUnionType(&timescale, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for MinOverTimescale")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
		(*state)["indexOfMin"] = []Union{Union{tag: UINT, ui: 0}}
	} else if _, ok := (*state)["timestamps"]; !ok || len((*state)["timestamps"]) == 0 {
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
		(*state)["indexOfMin"] = []Union{Union{tag: UINT, ui: 0}}
	}

	index := int((*state)["currentIndex"][0].ui)
	min := (*state)["values"][int((*state)["indexOfMin"][0].ui)]
	currentTime := time.Now().UnixMilli()

	// remove older data points
	indexToTruncate := -1
	for i, timestamp := range (*state)["timestamps"][0 : index+1] {
		if timestamp.i != 0 && timestamp.i < currentTime-timescale.i {
			indexToTruncate = i
		} else {
			break
		}
	}
	(*state)["timestamps"] = (*state)["timestamps"][indexToTruncate+1:]
	(*state)["values"] = (*state)["values"][indexToTruncate+1:]

	// if the old minimum was truncated, find the new minimum
	if indexToTruncate >= 0 {
		(*state)["currentIndex"][0].ui = uint64(index - indexToTruncate - 1)
		if (*state)["indexOfMin"][0].ui >= uint64(indexToTruncate) && (*state)["indexOfMin"][0].ui > 0 {
			(*state)["indexOfMin"][0].ui = (*state)["indexOfMin"][0].ui - uint64(indexToTruncate+1)
		} else {
			(*state)["indexOfMin"][0].ui = 0
			switch input.tag {
			case BOOL:
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if !value.b {
						min = (*state)["values"][i]
						(*state)["indexOfMin"][0].ui = uint64(i)
					}
				}
			case UINT:
				min = Union{tag: UINT, ui: math.MaxUint64}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.ui <= min.ui {
						min = (*state)["values"][i]
						(*state)["indexOfMin"][0].ui = uint64(i)
					}
				}
			case INT:
				min = Union{tag: INT, i: math.MaxInt64}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.i <= min.i {
						min = value
						(*state)["indexOfMin"][0].ui = uint64(i)
					}
				}
			case FLOAT:
				min = Union{tag: FLOAT, f: math.MaxFloat64}
				for i, value := range (*state)["values"][0 : index-indexToTruncate] {
					if value.f <= min.f {
						min = value
						(*state)["indexOfMin"][0].ui = uint64(i)
					}
				}
			}

		}
	}
	index = int((*state)["currentIndex"][0].ui)

	// add the current data point to the history
	(*state)["timestamps"][index].tag = INT
	(*state)["timestamps"][index].i = currentTime
	(*state)["values"][index] = input

	// compare it to the max
	switch input.tag {
	case BOOL:
		if !input.b {
			(*state)["indexOfMin"][0].ui = uint64(index)
		}
	case UINT:
		if input.ui <= min.ui {
			(*state)["indexOfMin"][0].ui = uint64(index)
		}
	case INT:
		if input.i <= min.i {
			(*state)["indexOfMin"][0].ui = uint64(index)
		}
	case FLOAT:
		if input.f <= min.f {
			(*state)["indexOfMin"][0].ui = uint64(index)
		}
	}
	(*state)["currentIndex"][0].ui += 1
	if (*state)["currentIndex"][0].ui > uint64(len((*state)["timestamps"])) {
		(*state)["timestamps"] = append((*state)["timestamps"], Union{})
		(*state)["values"] = append((*state)["values"], Union{})
	}
	return (*state)["values"][int((*state)["indexOfMin"][0].ui)], nil
}

func AvgOverTimescale(input, timescale Union, state *map[string][]Union) (Union, error) {
	var err error
	if input.tag == STRING {
		return Union{}, fmt.Errorf("cannot find the average value of a set of strings")
	} else if input.tag == BOOL {
		return Union{}, fmt.Errorf("cannot find the average value of a set of bools")
	} else if input.tag == NIL {
		return Union{}, fmt.Errorf("cannot find the average value of a set of nil values")
	}
	err = castUnionType(&timescale, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for AvgOverTimescale")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	} else if _, ok := (*state)["timestamps"]; !ok || len((*state)["timestamps"]) == 0 {
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	}

	index := int((*state)["currentIndex"][0].ui)
	currentTime := time.Now().UnixMilli()

	// remove older data points
	indexToTruncate := -1
	for i, timestamp := range (*state)["timestamps"][0 : index+1] {
		if timestamp.i != 0 && timestamp.i < currentTime-timescale.i {
			indexToTruncate = i
		} else {
			break
		}
	}
	(*state)["timestamps"] = (*state)["timestamps"][indexToTruncate+1:]
	(*state)["values"] = (*state)["values"][indexToTruncate+1:]

	// if anything was truncated
	if indexToTruncate >= 0 {
		(*state)["currentIndex"][0].ui = uint64(index - indexToTruncate - 1)
	}
	index = int((*state)["currentIndex"][0].ui)

	// add the current data point to the history
	(*state)["timestamps"][index].tag = INT
	(*state)["timestamps"][index].i = currentTime
	(*state)["values"][index] = input

	avg, err := Avg((*state)["values"][0 : index+1]...)
	(*state)["currentIndex"][0].ui += 1
	if (*state)["currentIndex"][0].ui > uint64(len((*state)["timestamps"])) {
		(*state)["timestamps"] = append((*state)["timestamps"], Union{})
		(*state)["values"] = append((*state)["values"], Union{})
	}
	return avg, err
}

func SumOverTimescale(input, timescale Union, state *map[string][]Union) (Union, error) {
	var err error
	if input.tag == STRING || input.tag == NIL {
		return Union{}, fmt.Errorf("cannot find the sum of a set of strings")
	}
	err = castUnionType(&timescale, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for SumOverTimescale")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	} else if _, ok := (*state)["timestamps"]; !ok || len((*state)["timestamps"]) == 0 {
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	}

	index := int((*state)["currentIndex"][0].ui)
	currentTime := time.Now().UnixMilli()

	// remove older data points
	indexToTruncate := -1
	for i, timestamp := range (*state)["timestamps"][0 : index+1] {
		if timestamp.i != 0 && timestamp.i < currentTime-timescale.i {
			indexToTruncate = i
		} else {
			break
		}
	}
	(*state)["timestamps"] = (*state)["timestamps"][indexToTruncate+1:]
	(*state)["values"] = (*state)["values"][indexToTruncate+1:]

	// if anything was truncated
	if indexToTruncate >= 0 {
		(*state)["currentIndex"][0].ui = uint64(index - indexToTruncate - 1)
	}
	index = int((*state)["currentIndex"][0].ui)

	// add the current data point to the history
	(*state)["timestamps"][index].tag = INT
	(*state)["timestamps"][index].i = currentTime
	(*state)["values"][index] = input

	sum, err := Add((*state)["values"][0 : index+1]...)
	(*state)["currentIndex"][0].ui += 1
	if (*state)["currentIndex"][0].ui > uint64(len((*state)["timestamps"])) {
		(*state)["timestamps"] = append((*state)["timestamps"], Union{})
		(*state)["values"] = append((*state)["values"], Union{})
	}
	return sum, err
}

func ValueChanged(input Union, state *map[string][]Union) (Union, error) {
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for ValueChanged")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["previousValue"] = []Union{input}
	} else if _, ok := (*state)["previousValue"]; !ok || len((*state)["previousValue"]) == 0 {
		(*state)["previousValue"] = []Union{input}
	} else if input != (*state)["previousValue"][0] {
		(*state)["previousValue"][0] = input
		return Union{tag: BOOL, b: true}, nil
	}
	return Union{tag: BOOL, b: false}, nil
}

func ValueChangedOverTimescale(input, timescale Union, state *map[string][]Union) (Union, error) {
	var err error
	err = castUnionType(&timescale, INT)
	if err != nil {
		return Union{}, err
	}
	if state == nil {
		return Union{}, fmt.Errorf("do not have a defined state for ValueChangedOverTimescale")
	} else if *state == nil {
		*state = make(map[string][]Union, 0)
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	} else if _, ok := (*state)["timestamps"]; !ok || len((*state)["timestamps"]) == 0 {
		(*state)["timestamps"] = make([]Union, timescale.i)
		(*state)["values"] = make([]Union, timescale.i)
		(*state)["currentIndex"] = []Union{Union{tag: UINT, ui: 0}}
	}

	index := int((*state)["currentIndex"][0].ui)
	currentTime := time.Now().UnixMilli()

	// remove older data points
	indexToTruncate := -1
	for i, timestamp := range (*state)["timestamps"][0 : index+1] {
		if timestamp.i != 0 && timestamp.i < currentTime-timescale.i {
			indexToTruncate = i
		} else {
			break
		}
	}
	(*state)["timestamps"] = (*state)["timestamps"][indexToTruncate+1:]
	(*state)["values"] = (*state)["values"][indexToTruncate+1:]

	// if anything was truncated
	if indexToTruncate >= 0 {
		(*state)["currentIndex"][0].ui = uint64(index - indexToTruncate - 1)
	}
	index = int((*state)["currentIndex"][0].ui)

	// add the current data point to the history
	(*state)["timestamps"][index].tag = INT
	(*state)["timestamps"][index].i = currentTime
	(*state)["values"][index] = input
	(*state)["currentIndex"][0].ui += 1

	// add more space if we've run out (though we really shouldn't)
	if (*state)["currentIndex"][0].ui > uint64(len((*state)["timestamps"])) {
		(*state)["timestamps"] = append((*state)["timestamps"], Union{})
		(*state)["values"] = append((*state)["values"], Union{})
	}
	// check if value has changed
	for i, _ := range (*state)["values"][0:index] {
		if (*state)["values"][i] != input {
			return Union{tag: BOOL, b: true}, nil
		}
	}
	return Union{tag: BOOL, b: false}, nil
}

func QuadToSigned(arg Union) (Union, error) {
	if arg.tag == STRING || arg.tag == NIL {
		return Union{}, fmt.Errorf("cannot convert string to signed power factor")
	}
	castUnionType(&arg, FLOAT)
	if arg.f < 0 || arg.f >= 4 {
		return Union{tag: FLOAT, f: 0}, nil
	} else if arg.f < 1 {
		return Union{tag: FLOAT, f: arg.f}, nil
	} else if arg.f < 2 {
		return Union{tag: FLOAT, f: -(arg.f - 1)}, nil
	} else if arg.f < 3 {
		return Union{tag: FLOAT, f: arg.f - 2}, nil
	} else {
		return Union{tag: FLOAT, f: -(arg.f - 3)}, nil
	}

}

func SignedToQuad(arg Union) (Union, error) {
	if arg.tag == STRING || arg.tag == NIL {
		return Union{}, fmt.Errorf("cannot convert string to signed power factor")
	}
	castUnionType(&arg, FLOAT)
	if arg.f <= -1 {
		return Union{tag: FLOAT, f: -1}, nil
	} else if arg.f < 0 {
		return Union{tag: FLOAT, f: 1 - arg.f}, nil
	} else if arg.f < 1 {
		return Union{tag: FLOAT, f: arg.f}, nil
	} else {
		return Union{tag: FLOAT, f: 1}, nil
	}
}

func Runtime(chargeEnergy, dischargeEnergy, powerOutput, gain, upperLimit, minP, defaultP Union) (Union, error) {
	err := castUnionType(&chargeEnergy, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&dischargeEnergy, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&powerOutput, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&gain, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&upperLimit, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&minP, FLOAT)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&defaultP, FLOAT)
	if err != nil {
		return Union{}, err
	}

	if powerOutput.f == 0 {
		return upperLimit, nil
	} else if math.Abs(powerOutput.f) <= minP.f {
		if defaultP.f == 0 {
			return upperLimit, nil
		}
		return Union{tag: FLOAT, f: math.Min((gain.f*dischargeEnergy.f)/defaultP.f, upperLimit.f)}, nil
	} else if powerOutput.f > 0 {
		return Union{tag: FLOAT, f: math.Min((gain.f*dischargeEnergy.f)/powerOutput.f, upperLimit.f)}, nil
	}
	return Union{tag: FLOAT, f: math.Min((gain.f*chargeEnergy.f)/-powerOutput.f, upperLimit.f)}, nil
}

func Unicompare(base, compare, balance Union) (Union, error) {
	var resultType DataType
	resultType = getResultType(base.tag, compare.tag)
	if resultType == STRING {
		return Union{}, fmt.Errorf("cannot perform unicompare on strings")
	}
	err := castUnionType(&base, resultType)
	if err != nil {
		return Union{}, err
	}
	err = castUnionType(&compare, resultType)
	if err != nil {
		return Union{}, err
	}
	castUnionType(&balance, BOOL)

	switch resultType {
	case BOOL:
		if base.b == false {
			return base, nil
		} else if balance.b {
			return Union{tag: BOOL, b: base.b && !compare.b}, nil
		} else {
			return compare, nil
		}
	case UINT:
		if compare.ui > base.ui {
			if balance.b {
				return Union{tag: UINT, ui: 0}, nil
			} else {
				return base, nil
			}
		} else if balance.b {
			return Union{tag: UINT, ui: base.ui - compare.ui}, nil
		} else {
			return Union{tag: UINT, ui: compare.ui}, nil
		}
	case INT:
		if base.i < 0 {
			base.i = 0
		}
		if compare.i > base.i {
			if balance.b {
				return Union{tag: INT, i: 0}, nil
			} else {
				return base, nil
			}
		} else if balance.b {
			return Union{tag: INT, i: base.i - compare.i}, nil
		} else {
			return Union{tag: INT, i: compare.i}, nil
		}
	case FLOAT:
		if base.f < 0 {
			base.f = 0
		}
		if compare.f > base.f {
			if balance.b {
				return Union{tag: FLOAT, f: 0}, nil
			} else {
				return base, nil
			}
		} else if balance.b {
			return Union{tag: FLOAT, f: base.f - compare.f}, nil
		} else {
			return Union{tag: FLOAT, f: compare.f}, nil
		}
	default:
		return Union{}, fmt.Errorf("cannot perform unicompare on NIL unions")
	}

}
