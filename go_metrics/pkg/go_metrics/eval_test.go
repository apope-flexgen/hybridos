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
	{
		inputExpression: "true",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "false",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "noval",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		inputExpression: "bool1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	{
		inputExpression: "int1",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	{
		inputExpression: "float1",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1",
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	{
		inputExpression: "bool1 + bool2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 + bool1",
		expectedOutput:  Union{tag: UINT, ui: 6},
		errorExpected:   false,
	},
	{
		inputExpression: "int1 + bool1",
		expectedOutput:  Union{tag: INT, i: 7},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 + bool1",
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 + bool2",
		expectedOutput:  Union{tag: STRING, s: "hellofalse"},
		errorExpected:   false,
	},
	{
		inputExpression: "bool1 + uint2",
		expectedOutput:  Union{tag: UINT, ui: 8},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 + uint2",
		expectedOutput:  Union{tag: UINT, ui: 12},
		errorExpected:   false,
	},
	{
		inputExpression: "int1 + uint2",
		expectedOutput:  Union{tag: INT, i: 13},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 + uint2",
		expectedOutput:  Union{tag: FLOAT, f: 12.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 + uint2",
		expectedOutput:  Union{tag: STRING, s: "hello7"},
		errorExpected:   false,
	},
	{
		inputExpression: "bool1 + int2",
		expectedOutput:  Union{tag: INT, i: 9},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 + int2",
		expectedOutput:  Union{tag: INT, i: 13},
		errorExpected:   false,
	},
	{
		inputExpression: "int1 + int2",
		expectedOutput:  Union{tag: INT, i: 14},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 + int2",
		expectedOutput:  Union{tag: FLOAT, f: 13.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 + int2",
		expectedOutput:  Union{tag: STRING, s: "hello8"},
		errorExpected:   false,
	},
	{
		inputExpression: "bool1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 8.3},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 12.3},
		errorExpected:   false,
	},
	{
		inputExpression: "int1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 13.3},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 + float2",
		expectedOutput:  Union{tag: FLOAT, f: 12.6},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 + float2",
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("hello%f", 7.3)},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 == float2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "!bool1",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "-int1",
		expectedOutput:  Union{tag: INT, i: -6},
		errorExpected:   false,
	},
	{
		inputExpression: "!nonexistant",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "^bool1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "!string1",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "-string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Add(float1,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 10.3},
		errorExpected:   false,
	},
	{
		inputExpression: "Sub(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: -2.0},
		errorExpected:   false,
	},
	{
		inputExpression: "Sub(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Sub(float1,float2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Mult(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 * 7.3},
		errorExpected:   false,
	},
	{
		inputExpression: "Mult(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Div(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3},
		errorExpected:   false,
	},
	{
		inputExpression: "Div(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Div(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Mod(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	{
		inputExpression: "Mod(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Mod(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Mod(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Mod(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseAnd(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 & 8},
		errorExpected:   false,
	},
	{
		inputExpression: "BitwiseAnd(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseAnd(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseOr(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 | 8},
		errorExpected:   false,
	},
	{
		inputExpression: "BitwiseOr(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseOr(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseXor(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseXor(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 ^ 8},
		errorExpected:   false,
	},
	{
		inputExpression: "BitwiseXor(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LeftShift(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 << 8},
		errorExpected:   false,
	},
	{
		inputExpression: "LeftShift(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LeftShift(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LeftShift(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LeftShift(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RightShift(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	{
		inputExpression: "RightShift(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RightShift(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RightShift(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RightShift(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseAndNot(int1,int2)",
		expectedOutput:  Union{tag: INT, i: 6 &^ 8},
		errorExpected:   false,
	},
	{
		inputExpression: "BitwiseAndNot(int1,int2,uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseAndNot(float1,float2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "BitwiseAndNot(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "And(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "Or(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "Not(int1,string1)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "Equal(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "Equal(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "NotEqual(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "NotEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LessThan(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "LessThan(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "GreaterThan(uint1,uint2)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "GreaterThan(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "LessThanOrEqual(uint1,uint2,5)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "LessThanOrEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "GreaterThanOrEqual(uint2,uint1,5)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "GreaterThanOrEqual(uint1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Root(int1,3)",
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(6, 1.0/3.0)},
		errorExpected:   false,
	},
	{
		inputExpression: "Root(2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Root(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Pow(2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Pow(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Pow(int1,2)",
		expectedOutput:  Union{tag: INT, i: int64(math.Pow(6, 2))},
		errorExpected:   false,
	},
	{
		inputExpression: "Max(int1,int2,uint1)",
		expectedOutput:  Union{tag: INT, i: 8},
		errorExpected:   false,
	},
	{
		inputExpression: "Max(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Min(int1,int2,uint1)",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	{
		inputExpression: "Min(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Avg(int1,int2,uint1)",
		expectedOutput:  Union{tag: INT, i: (5 + 6 + 8) / 3},
		errorExpected:   false,
	},
	{
		inputExpression: "Avg(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Avg(bool1,bool2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Floor(float1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	{
		inputExpression: "Floor(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Ceil(float1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: 6.0},
		errorExpected:   false,
	},
	{
		inputExpression: "Ceil(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Sqrt(int1,int2,uint1)",
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(6)},
		errorExpected:   false,
	},
	{
		inputExpression: "Sqrt(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Pct(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3 * 100},
		errorExpected:   false,
	},
	{
		inputExpression: "Pct(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Pct(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "FloorDiv(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	{
		inputExpression: "FloorDiv(float1,float2,5.0)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "FloorDiv(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Abs(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	{
		inputExpression: "Abs(float1,float2,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	{
		inputExpression: "Abs(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Round(float1,float2)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	{
		inputExpression: "Round(float1,float2,5.0)",
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	{
		inputExpression: "Round(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Bool(float1,float2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "Int(float1,float2)",
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	{
		inputExpression: "Uint(float1,float2)",
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	{
		inputExpression: "Float(int1,uint2)",
		expectedOutput:  Union{tag: FLOAT, f: 6.0},
		errorExpected:   false,
	},
	{
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
	{
		inputExpression: "MillisecondsSince(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "MillisecondsToRFC3339()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "MillisecondsToRFC3339(int1)",
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(6).Format(time.RFC3339)},
		errorExpected:   false,
	},
	{
		inputExpression: "MillisecondsToRFC3339(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RFC3339()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "RFC3339(int1)",
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(6).Format(time.RFC3339)},
		errorExpected:   false,
	},
	{
		inputExpression: "RFC3339(string1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Srff(string1,string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Srff(float1,float2)",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "Srff(float1,float2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Rss(float1,float2,int1)",
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(5.3*5.3 + 7.3*7.3 + 36)},
		errorExpected:   false,
	},
	{
		inputExpression: "Rss(string1,string2,int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Rss()",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		inputExpression: "SelectN(1,string1,int1)",
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	{
		inputExpression: "SelectN()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "SelectN(string1, string2)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "SelectorN(float1,string1,int1)",
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	{
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
	{
		inputExpression: "IfElse(bool1, bool2, int1)",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "IfElse()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "NotAFunction(bool1, bool2, int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "IfElse(Bool(nonexistant), bool2, int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(enabled == true)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(nonexistant == true)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1-float2",
		expectedOutput:  Union{tag: FLOAT, f: -2.0},
		errorExpected:   false,
	},
	{
		inputExpression: "string1-string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1*float2",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 * 7.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1*string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1/float2",
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 7.3},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 / string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 % int2",
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 % float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "string1 % string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "bool1 % bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 & int2",
		expectedOutput:  Union{tag: INT, i: 6 & 8},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 & string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1 & float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 | int2",
		expectedOutput:  Union{tag: INT, i: 6 | 8},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 | string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1 | float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 ^ int2",
		expectedOutput:  Union{tag: INT, i: 6 ^ 8},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 ^ string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1 ^ float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 << int2",
		expectedOutput:  Union{tag: INT, i: 6 << 8},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 << string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1 << float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "bool1 << bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 >> int2",
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	{
		inputExpression: "string1 >> string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "float1 >> float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "bool1 >> bool2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 &^ int2",
		expectedOutput:  Union{tag: INT, i: 6 &^ 8},
		errorExpected:   false,
	},
	{
		inputExpression: "float1 &^ float2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "string1 &^ string2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "int1 && string1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "int1 || string1",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 == uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 != uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 < uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 > uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 > string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 < string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 >= string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 <= string1",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 <= uint2",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 >= uint2",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "Sub() >= uint2",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint2 >= Sub()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "func(){uint1 == uint2}()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 == 'c'",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: `uint1 == "string"`,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 == 3.14",
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		inputExpression: "uint1 == 99999999999999999999999999999",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 == 3.7976931348623157e+308",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "uint1 == 3.14i",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: `string1[0:5]`,
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "input1@enabled || input2@enabled",
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		inputExpression: "input3@enabled",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "input4@enabled",
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		inputExpression: "multival + 5",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "5 + multival",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "!multival",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "!Sub()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "(uint1 + uint2)/float1",
		expectedOutput:  Union{tag: FLOAT, f: 12 / 5.3},
		errorExpected:   false,
	},
	{
		inputExpression: "☺",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "string1[0:5]",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestEval(t *testing.T) {
	loud := Quiet()
	defer loud()
	InputScope = map[string][]Union{
		"bool1":          {{tag: BOOL, b: true}},
		"bool2":          {{tag: BOOL, b: false}},
		"uint1":          {{tag: UINT, ui: 5}},
		"uint2":          {{tag: UINT, ui: 7}},
		"int1":           {{tag: INT, i: 6}},
		"int2":           {{tag: INT, i: 8}},
		"float1":         {{tag: FLOAT, f: 5.3}},
		"float2":         {{tag: FLOAT, f: 7.3}},
		"string1":        {{tag: STRING, s: "hello"}},
		"string2":        {{tag: STRING, s: ""}},
		"noval":          {},
		"multival":       {{tag: INT, i: 9}, {tag: INT, i: 11}},
		"input1@enabled": {{tag: BOOL, b: true}},
		"input2@enabled": {{tag: BOOL, b: true}},
		"input4@enabled": {},
	}
	var output []Union
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

		if output[0] != test.expectedOutput {
			t.Errorf("%s: output var %v not equal to expected var %v\n", test.inputExpression, output[0], test.expectedOutput)
		}
	}
}

var TimeSensitiveTests = []EvalTest{
	{
		inputExpression: "CurrentTimeMilliseconds()",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli()},
		errorExpected:   false,
	},
	{
		inputExpression: "CurrentTimeMilliseconds(int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "Time()",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli()},
		errorExpected:   false,
	},
	{
		inputExpression: "Time(int1)",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "MillisecondsSince()",
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	{
		inputExpression: "MillisecondsSince(int1)",
		expectedOutput:  Union{tag: INT, i: time.Now().UnixMilli() - 6},
		errorExpected:   false,
	},
}

func TestTimeSensitiveEvals(t *testing.T) {
	var output []Union
	var err error
	for _, test := range TimeSensitiveTests {
		t0 := time.Now()
		exp, _ := Parse(test.inputExpression)
		output, err = Evaluate(exp, &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.inputExpression, duration)
		}

		if output[0].tag != test.expectedOutput.tag {
			t.Errorf("%s: output %v not equal to expected %v\n", test.inputExpression, output[0], test.expectedOutput)
		}
		// this might be too large of a buffer...
		if math.Abs(float64(output[0].i-test.expectedOutput.i)) >= 10 {
			t.Errorf("%s: output %v not within 10 milliseconds of expected %v\n", test.inputExpression, output[0], test.expectedOutput)
		}

		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.inputExpression)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.inputExpression)
		}
	}
}
