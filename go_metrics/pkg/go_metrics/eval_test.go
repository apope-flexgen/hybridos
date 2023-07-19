package go_metrics

import (
	"fmt"
	"math"
	"testing"
	"time"
)

type EvalTest struct {
	inputExpression string
	state           map[string][]Union
	expectedOutput  Union
	errorExpected   bool
}

var EvalTests = []EvalTest{
	EvalTest{
		inputExpression: "true",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "false",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "noval",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "bool1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1",
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "bool1 + bool2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 + bool1",
		expectedOutput:  Union{tag: UINT, ui: 6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1 + bool1",
		expectedOutput:  Union{tag: INT, i: 7},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 + bool1",
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 + bool2",
		expectedOutput:  Union{tag: STRING, s: "hellofalse"},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "bool1 + uint2",
		expectedOutput:  Union{tag: UINT, ui: 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 + uint2",
		expectedOutput:  Union{tag: UINT, ui: 12},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1 + uint2",
		expectedOutput:  Union{tag: INT, i: 13},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 + uint2",
		expectedOutput:  Union{tag: FLOAT, f: 12.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 + uint2",
		expectedOutput:  Union{tag: STRING, s: "hello7"},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "bool1 + int2",
		expectedOutput:  Union{tag: INT, i: 9},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 + int2",
		expectedOutput:  Union{tag: INT, i: 13},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1 + int2",
		expectedOutput:  Union{tag: INT, i: 14},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 + int2",
		expectedOutput:  Union{tag: FLOAT, f: 13.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 + int2",
		expectedOutput:  Union{tag: STRING, s: "hello8"},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "bool1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 8.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 12.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 13.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 12.6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 + float2",
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("hello%f", 7.3)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 == float2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "!bool1",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "-int1",
		expectedOutput:  Union{tag: INT, i: -6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "!nonexistant",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "^bool1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "!string1",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "-string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Add(float1,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 10.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Sub(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: -2.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Sub(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Sub(float1,float2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Mult(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 * 7.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Mult(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Div(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Div(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Div(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Mod(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Mod(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Mod(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Mod(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Mod(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseAnd(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 & 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "BitwiseAnd(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseAnd(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseOr(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 | 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "BitwiseOr(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseOr(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseXor(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseXor(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 ^ 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "BitwiseXor(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LeftShift(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 << 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "LeftShift(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LeftShift(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LeftShift(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LeftShift(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RightShift(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "RightShift(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RightShift(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RightShift(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RightShift(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseAndNot(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 &^ 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "BitwiseAndNot(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseAndNot(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "BitwiseAndNot(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "And(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Or(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Not(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Equal(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Equal(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "NotEqual(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "NotEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LessThan(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "LessThan(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "GreaterThan(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "GreaterThan(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "LessThanOrEqual(uint1,uint2,5)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "LessThanOrEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "GreaterThanOrEqual(uint2,uint1,5)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "GreaterThanOrEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Root(int1,3)",
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(6, 1.0/3.0)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Root(2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Root(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Pow(2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Pow(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Pow(int1,2)",
		expectedOutput:  Union{tag: INT, i: int64(math.Pow(6, 2))},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Max(int1,int2,uint1)",
		expectedOutput:  Union{tag: INT, i: 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Max(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Min(int1,int2,uint1)",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Min(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Avg(int1,int2,uint1)",
		expectedOutput:  Union{tag: INT, i: (5 + 6 + 8) / 3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Avg(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Avg(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Floor(float1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Floor(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Ceil(float1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: 6.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Ceil(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Sqrt(int1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(6)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Sqrt(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Pct(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3 * 100},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Pct(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Pct(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "FloorDiv(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "FloorDiv(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "FloorDiv(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Abs(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Abs(float1,float2,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Abs(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Round(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Round(float1,float2,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Round(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Bool(float1,float2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Int(float1,float2)",
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Uint(float1,float2)",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Float(int1,uint2)",
		expectedOutput:  Union{tag: FLOAT, f: 6.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "String(float1,float2)",
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 5.3)},
		errorExpected:   false,
	},
	// EvalTest{
	// 	inputExpression: "Integrate(float1)",
	// 	expectedOutput:  Union{tag: FLOAT, f: 0.0},
	// 	errorExpected:   false,
	// },
	// EvalTest{
	// 	inputExpression: "Integrate(float1,float2)",
	// 	expectedOutput:  Union{tag: FLOAT, f: 0.0},
	// 	errorExpected:   false,
	// },
	// EvalTest{
	// 	inputExpression: "Integrate()",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Integrate(string1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Integrate(string1,float2)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "IntegrateOverTimescale(float1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "IntegrateOverTimescale(float1,float2)",
	// 	expectedOutput:  Union{tag: FLOAT, f: 0.0},
	// 	errorExpected:   false,
	// },
	// EvalTest{
	// 	inputExpression: "IntegrateOverTimescale(string1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "IntegrateOverTimescale(string1,float2)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	EvalTest{
		inputExpression: "MillisecondsSince(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "MillisecondsToRFC3339()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "MillisecondsToRFC3339(int1)",
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(6).Format(time.RFC3339)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "MillisecondsToRFC3339(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RFC3339()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "RFC3339(int1)",
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(6).Format(time.RFC3339)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "RFC3339(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Srff(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Srff(float1,float2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Srff(float1,float2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Rss(float1,float2,int1)",
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(5.3*5.3 + 7.3*7.3 + 36)},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Rss(string1,string2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Rss()",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "SelectN(1,string1,int1)",
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "SelectN()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "SelectN(string1, string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "SelectorN(float1,string1,int1)",
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "SelectorN()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	// EvalTest{
	// 	inputExpression: "Pulse(bool1, bool2, int1)",
	// 	expectedOutput:  []string{"bool1", "bool2", "int1"},
	// 	errorExpected:   false,
	// },
	// EvalTest{
	// 	inputExpression: "Pulse(bool1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Pulse(string1, int1, int2)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Pulse(bool1, string1, int1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Pulse(bool1, float1, string1)",
	// 	expectedOutput:  Union{},
	// 	errorExpected:   true,
	// },
	// EvalTest{
	// 	inputExpression: "Pulse(bool1, bool2, int1)",
	// 	expectedOutput:  []string{"bool1", "bool2", "int1"},
	// 	errorExpected:   false,
	// },
	EvalTest{
		inputExpression: "IfElse(bool1, bool2, int1)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "IfElse()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "NotAFunction(bool1, bool2, int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "IfElse(Bool(nonexistant), bool2, int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Attribute(enabled == true)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Attribute(nonexistant == true)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1-float2",
		expectedOutput:  Union{tag: FLOAT, f: -2.0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1-string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1*float2",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 * 7.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1*string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1/float2",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 / string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 % int2",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 % float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "string1 % string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "bool1 % bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 & int2",
		expectedOutput:  Union{tag: INT, i: 6 & 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 & string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1 & float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 | int2",
		expectedOutput:  Union{tag: INT, i: 6 | 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 | string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1 | float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 ^ int2",
		expectedOutput:  Union{tag: INT, i: 6 ^ 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 ^ string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1 ^ float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 << int2",
		expectedOutput:  Union{tag: INT, i: 6 << 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 << string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1 << float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "bool1 << bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 >> int2",
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "string1 >> string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "float1 >> float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "bool1 >> bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 &^ int2",
		expectedOutput:  Union{tag: INT, i: 6 &^ 8},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "float1 &^ float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "string1 &^ string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "int1 && string1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "int1 || string1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 == uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 != uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 < uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 > uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 > string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 < string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 >= string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 <= string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 <= uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 >= uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Sub() >= uint2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint2 >= Sub()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "func(){uint1 == uint2}()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 == 'c'",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: `uint1 == "string"`,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 == 3.14",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "uint1 == 99999999999999999999999999999",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 == 3.7976931348623157e+308",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "uint1 == 3.14i",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: `string1[0:5]`,
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "input1@enabled || input2@enabled",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "input3@enabled",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "input4@enabled",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "multival + 5",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "5 + multival",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "!multival",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "!Sub()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "(uint1 + uint2)/float1",
		expectedOutput:  Union{tag: FLOAT, f: 12 / 5.3},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "☺",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "string1[0:5]",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestEval(t *testing.T) {
	loud := Quiet()
	defer loud()
	Scope = map[string][]Input{
		"bool1": []Input{
			Input{
				Type:  "bool",
				Value: Union{tag: BOOL, b: true},
			},
		},
		"bool2": []Input{
			Input{
				Type:  "bool",
				Value: Union{tag: BOOL, b: false},
			},
		},
		"uint1": []Input{
			Input{
				Type:  "uint",
				Value: Union{tag: UINT, ui: 5},
			},
		},
		"uint2": []Input{
			Input{
				Type:  "uint",
				Value: Union{tag: UINT, ui: 7},
			},
		},
		"int1": []Input{
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 6},
			},
		},
		"int2": []Input{
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 8},
			},
		},
		"float1": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 5.3},
			},
		},
		"float2": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 7.3},
			},
		},
		"string1": []Input{
			Input{
				Type:  "string",
				Value: Union{tag: STRING, s: "hello"},
			},
		},
		"string2": []Input{
			Input{
				Type:  "string",
				Value: Union{tag: STRING, s: ""},
			},
		},
		"noval": []Input{},
		"multival": []Input{
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 9},
			},
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 11},
			},
		},
		"input1@enabled": []Input{
			Input{
				Type:  "bool",
				Value: Union{tag: BOOL, b: true},
			},
		},
		"input2@enabled": []Input{
			Input{
				Type:  "bool",
				Value: Union{tag: BOOL, b: true},
			},
		},
		"input4@enabled": []Input{},
	}
	var output Union
	var err error
	for _, test := range EvalTests {
		exp, _ := Parse(test.inputExpression)
		output, err = Evaluate(exp, &(test.state))
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.inputExpression)
			continue
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error; error was: %v\n", test.inputExpression, err)
			continue
		}

		if output != test.expectedOutput {
			t.Errorf("%s: output var %v not equal to expected var %v\n", test.inputExpression, output, test.expectedOutput)
		}
	}
}

var TimeSensitiveTests = []EvalTest{
	EvalTest{
		inputExpression: "CurrentTimeMilliseconds()",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli()},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "CurrentTimeMilliseconds(int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "Time()",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli()},
		errorExpected:   false,
	},
	EvalTest{
		inputExpression: "Time(int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "MillisecondsSince()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	EvalTest{
		inputExpression: "MillisecondsSince(int1)",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli() - 6},
		errorExpected:   false,
	},
}

func TestTimeSensitiveEvals(t *testing.T) {
	var output Union
	var err error
	for _, test := range TimeSensitiveTests {
		t0 := time.Now()
		exp, _ := Parse(test.inputExpression)
		output, err = Evaluate(exp, &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.inputExpression, duration)
		}

		if output.tag != test.expectedOutput.tag {
			t.Errorf("%s: output %v not equal to expected %v\n", test.inputExpression, output, test.expectedOutput)
		}
		// this might be too large of a buffer...
		if math.Abs(float64(output.i-test.expectedOutput.i)) >= 10 {
			t.Errorf("%s: output %v not within 10 milliseconds of expected %v\n", test.inputExpression, output, test.expectedOutput)
		}

		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.inputExpression)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.inputExpression)
		}
	}
}
