package go_metrics

import (
	"testing"
)

type ParseTest struct {
	equation      string
	vars          []string
	errorExpected bool
	resultType    DataType
}

var ParseTests = []ParseTest{
	ParseTest{
		equation:      "bool1",
		vars:          []string{"bool1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1",
		vars:          []string{"uint1"},
		errorExpected: false,
		resultType:    UINT,
	},
	ParseTest{
		equation:      "int1",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1",
		vars:          []string{"float1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1",
		vars:          []string{"string1"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "bool1 + bool2",
		vars:          []string{"bool1", "bool2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 + bool2",
		vars:          []string{"uint1", "bool2"},
		errorExpected: false,
		resultType:    UINT,
	},
	ParseTest{
		equation:      "int1 + bool2",
		vars:          []string{"int1", "bool2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1 + bool2",
		vars:          []string{"float1", "bool2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1 + bool2",
		vars:          []string{"string1", "bool2"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "bool1 + uint2",
		vars:          []string{"bool1", "uint2"},
		errorExpected: false,
		resultType:    UINT,
	},
	ParseTest{
		equation:      "uint1 + uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    UINT,
	},
	ParseTest{
		equation:      "int1 + uint2",
		vars:          []string{"int1", "uint2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1 + uint2",
		vars:          []string{"float1", "uint2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1 + uint2",
		vars:          []string{"string1", "uint2"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "bool1 + int2",
		vars:          []string{"bool1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "uint1 + int2",
		vars:          []string{"uint1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "int1 + int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1 + int2",
		vars:          []string{"float1", "int2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1 + int2",
		vars:          []string{"string1", "int2"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "bool1 + float2",
		vars:          []string{"bool1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "uint1 + float2",
		vars:          []string{"uint1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "int1 + float2",
		vars:          []string{"int1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "float1 + float2",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1 + float2",
		vars:          []string{"string1", "float2"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "string1 == float2",
		vars:          []string{"string1", "float2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "!bool1",
		vars:          []string{"bool1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "!nonexistant",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "^bool1",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "!string1",
		vars:          []string{"string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "-string1",
		vars:          []string{"string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Add(float1,5.0)",
		vars:          []string{"float1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Sub(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Sub(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Sub(float1,float2,int1)",
		vars:          []string{"float1", "float2", "int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Mult(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Mult(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Div(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Div(float1,float2,5.0)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Div(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Mod(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Mod(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Mod(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Mod(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Mod(bool1,bool2)",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseAnd(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "BitwiseAnd(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseAnd(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseOr(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "BitwiseOr(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseOr(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseXor(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseXor(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "BitwiseXor(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LeftShift(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "LeftShift(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LeftShift(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LeftShift(bool1,bool2)",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LeftShift(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RightShift(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "RightShift(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RightShift(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RightShift(bool1,bool2)",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RightShift(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseAndNot(int1,int2)",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "BitwiseAndNot(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseAndNot(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "BitwiseAndNot(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "And(int1,string1)",
		vars:          []string{"int1", "string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Or(int1,string1)",
		vars:          []string{"int1", "string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Not(int1,string1)",
		vars:          []string{"int1", "string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Equal(uint1,uint2)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Equal(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "NotEqual(uint1,uint2)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "NotEqual(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LessThan(uint1,uint2,5)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "LessThan(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "GreaterThan(uint1,uint2,5)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "GreaterThan(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "LessThanOrEqual(uint1,uint2,5)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "LessThanOrEqual(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "GreaterThanOrEqual(uint1,uint2,5)",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "GreaterThanOrEqual(uint1)",
		vars:          []string{"uint1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Root(int1,3)",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Root(2)",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Root(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pow(2)",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pow(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pow(int1,2)",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Max(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Max(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Min(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Min(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Avg(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Avg(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Avg(bool1,bool2)",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Floor(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Floor(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Ceil(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Ceil(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Sqrt(int1,int2,uint1)",
		vars:          []string{"int1", "int2", "uint1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Sqrt(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pct(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Pct(float1,float2,5.0)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pct(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "FloorDiv(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "FloorDiv(float1,float2,5.0)",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "FloorDiv(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Abs(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Abs(float1,float2,5.0)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Abs(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Round(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Round(float1,float2,5.0)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Round(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Bool(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Int(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Uint(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    UINT,
	},
	ParseTest{
		equation:      "Float(int1,uint2)",
		vars:          []string{"int1", "uint2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "String(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "Integrate(float1)",
		vars:          []string{"float1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Integrate(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Integrate()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Integrate(string1)",
		vars:          []string{"string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Integrate(string1,float2)",
		vars:          []string{"string1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "CurrentTimeMilliseconds()",
		vars:          []string{},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "CurrentTimeMilliseconds(int1)",
		vars:          []string{"int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Time()",
		vars:          []string{},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "Time(int1)",
		vars:          []string{"int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "MillisecondsSince()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "MillisecondsSince(int1)",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "MillisecondsSince(string1)",
		vars:          []string{"string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "MillisecondsToRFC3339()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "MillisecondsToRFC3339(int1)",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "MillisecondsToRFC3339(string1)",
		vars:          []string{"string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RFC3339()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "RFC3339(int1)",
		vars:          []string{"int1"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "RFC3339(string1)",
		vars:          []string{"string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Srff(string1,string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Srff(float1,float2)",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Srff(float1,float2,int1)",
		vars:          []string{"float1", "float2", "int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Rss(float1,float2,int1)",
		vars:          []string{"float1", "float2", "int1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "Rss(string1,string2,int1)",
		vars:          []string{"string1", "string2", "int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Rss()",
		vars:          []string{},
		errorExpected: false,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "SelectN(float1,string1,int1)",
		vars:          []string{"float1", "string1", "int1"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "SelectN()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "SelectN(string1, string2)",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "SelectorN(float1,string1,int1)",
		vars:          []string{"float1", "string1", "int1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "SelectorN()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "SelectN(float1,string1,int1)",
		vars:          []string{"float1", "string1", "int1"},
		errorExpected: false,
		resultType:    STRING,
	},
	ParseTest{
		equation:      "SelectN()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pulse(bool1, bool2, int1)",
		vars:          []string{"bool1", "bool2", "int1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Pulse(bool1)",
		vars:          []string{"bool1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pulse(string1, int1, int2)",
		vars:          []string{"string1", "int1", "int2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pulse(bool1, string1, int1)",
		vars:          []string{"bool1", "string1", "int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pulse(bool1, float1, string1)",
		vars:          []string{"bool1", "float1", "string1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Pulse(bool1, bool2, int1)",
		vars:          []string{"bool1", "bool2", "int1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "IfElse(bool1, bool2, int1)",
		vars:          []string{"bool1", "bool2", "int1"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "IfElse()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "NotAFunction(bool1, bool2, int1)",
		vars:          []string{"bool1", "bool2", "int1"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "IfElse(Bool(nonexistant), bool2, int1)",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "Attribute(enabled == true)",
		vars:          []string{"enabled", "true"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Attribute(nonexistant == true)",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1-float2",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1-string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1*float2",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1*string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1/float2",
		vars:          []string{"float1", "float2"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "string1 / string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 % int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1 % float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "string1 % string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "bool1 % bool2",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 & int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "string1 & string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1 & float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 | int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "string1 | string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1 | float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 ^ int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "string1 ^ string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1 ^ float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 << int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "string1 << string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1 << float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "bool1 << bool2",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 >> int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "string1 >> string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "float1 >> float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "bool1 >> bool2",
		vars:          []string{"bool1", "bool2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 &^ int2",
		vars:          []string{"int1", "int2"},
		errorExpected: false,
		resultType:    INT,
	},
	ParseTest{
		equation:      "float1 &^ float2",
		vars:          []string{"float1", "float2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "string1 &^ string2",
		vars:          []string{"string1", "string2"},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "int1 && string1",
		vars:          []string{"int1", "string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "int1 || string1",
		vars:          []string{"int1", "string1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 == uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 != uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 < uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 > uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 <= uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 >= uint2",
		vars:          []string{"uint1", "uint2"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "Sub() >= uint2",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "uint2 >= Sub()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "func(){uint1 == uint2}()",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "uint1 == 'c'",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      `uint1 == "string"`,
		vars:          []string{"uint1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 == 3.14",
		vars:          []string{"uint1"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "uint1 == 3.14i",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      `string1[0:5]`,
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
	ParseTest{
		equation:      "input1@enabled || input2@enabled",
		vars:          []string{"input1@enabled", "input2@enabled"},
		errorExpected: false,
		resultType:    BOOL,
	},
	ParseTest{
		equation:      "(uint1 + uint2)/float1",
		vars:          []string{"uint1", "uint2", "float1"},
		errorExpected: false,
		resultType:    FLOAT,
	},
	ParseTest{
		equation:      "☺",
		vars:          []string{},
		errorExpected: true,
		resultType:    NIL,
	},
}

func TestParse(t *testing.T) {
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
				Value: Union{tag: BOOL, b: true},
			},
		},
		"uint1": []Input{
			Input{
				Type:  "uint",
				Value: Union{tag: UINT, ui: 0},
			},
		},
		"uint2": []Input{
			Input{
				Type:  "uint",
				Value: Union{tag: UINT, ui: 0},
			},
		},
		"int1": []Input{
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 0},
			},
		},
		"int2": []Input{
			Input{
				Type:  "int",
				Value: Union{tag: INT, i: 0},
			},
		},
		"float1": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 0},
			},
		},
		"float2": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 0},
			},
		},
		"string1": []Input{
			Input{
				Type:  "string",
				Value: Union{tag: STRING, s: ""},
			},
		},
		"string2": []Input{
			Input{
				Type:  "string",
				Value: Union{tag: STRING, s: ""},
			},
		},
		"input1@enabled": []Input{
			Input{
				Type:  "bool",
				Value: Union{tag: BOOL, b: true},
			},
		},
	}
	allPossibleAttributes = make(map[string][]string, 0)
	allPossibleAttributes["enabled"] = []string{"input1@enabled"}
	var output *Expression
	output = &Expression{
		Vars: []string{},
	}
	var err error
	for _, test := range ParseTests {
		output, err = Parse(test.equation)
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.equation)
			continue
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error; error was: %v\n", test.equation, err)
			continue
		}
		if len(output.Vars) != len(test.vars) {
			t.Errorf("%s: output vars %v not equal to expected vars %v\n", test.equation, output.Vars, test.vars)
		} else {
			for i, _ := range output.Vars {
				if output.Vars[i] != test.vars[i] {
					t.Errorf("%s: output var %v not equal to expected var %v\n", test.equation, output.Vars[i], test.vars[i])
				}
			}
		}
		if output.ResultType != test.resultType {
			t.Errorf("%s: output result type %v not equal to expected %v\n", test.equation, output.ResultType, test.resultType)
		}

	}
}
