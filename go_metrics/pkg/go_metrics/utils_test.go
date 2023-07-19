package go_metrics

import (
	"fmt"
	"math"
	"testing"
	"time"
)

type DataTypeStringTestCase struct {
	testDescription string
	input           DataType
	expectedOutput  string
}

var DataTypeStringTests = []DataTypeStringTestCase{
	DataTypeStringTestCase{
		testDescription: "nil",
		input:           NIL,
		expectedOutput:  "nil",
	},
	DataTypeStringTestCase{
		testDescription: "bool",
		input:           BOOL,
		expectedOutput:  "bool",
	},
	DataTypeStringTestCase{
		testDescription: "uint",
		input:           UINT,
		expectedOutput:  "uint",
	},
	DataTypeStringTestCase{
		testDescription: "int",
		input:           INT,
		expectedOutput:  "int",
	},
	DataTypeStringTestCase{
		testDescription: "float",
		input:           FLOAT,
		expectedOutput:  "float",
	},
	DataTypeStringTestCase{
		testDescription: "string",
		input:           STRING,
		expectedOutput:  "string",
	},
}

func TestDataTypeString(t *testing.T) {
	var output string
	for _, test := range DataTypeStringTests {
		t0 := time.Now()
		output = (&test.input).String()
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type DataTypeMarshalTestCase struct {
	testDescription string
	input           DataType
	expectedOutput  []byte
}

var DataTypeMarshalTests = []DataTypeMarshalTestCase{
	DataTypeMarshalTestCase{
		testDescription: "nil",
		input:           NIL,
		expectedOutput:  []byte("\"nil\""),
	},
	DataTypeMarshalTestCase{
		testDescription: "bool",
		input:           BOOL,
		expectedOutput:  []byte("\"bool\""),
	},
	DataTypeMarshalTestCase{
		testDescription: "uint",
		input:           UINT,
		expectedOutput:  []byte("\"uint\""),
	},
	DataTypeMarshalTestCase{
		testDescription: "int",
		input:           INT,
		expectedOutput:  []byte("\"int\""),
	},
	DataTypeMarshalTestCase{
		testDescription: "float",
		input:           FLOAT,
		expectedOutput:  []byte("\"float\""),
	},
	DataTypeMarshalTestCase{
		testDescription: "string",
		input:           STRING,
		expectedOutput:  []byte("\"string\""),
	},
}

func TestDataTypeMarshal(t *testing.T) {
	var output []byte
	for _, test := range DataTypeMarshalTests {
		t0 := time.Now()
		output, _ = (test.input).MarshalJSON()
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutput) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		} else {
			for i, _ := range output {
				if output[i] != test.expectedOutput[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output[i], test.expectedOutput[i])
				}
			}
		}
	}
}

type InterfaceUnionTestCase struct {
	testDescription string
	input           interface{}
	expectedOutput  Union
}

var GetUnionFromValueTests = []InterfaceUnionTestCase{
	InterfaceUnionTestCase{
		testDescription: "input = nil",
		input:           nil,
		expectedOutput:  Union{},
	},
	InterfaceUnionTestCase{
		testDescription: "input = true",
		input:           true,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	InterfaceUnionTestCase{
		testDescription: "input = false",
		input:           false,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	InterfaceUnionTestCase{
		testDescription: "input = int64(0)",
		input:           int64(0),
		expectedOutput:  Union{tag: INT, i: 0},
	},
	InterfaceUnionTestCase{
		testDescription: "input = uint64(0)",
		input:           uint64(0),
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	InterfaceUnionTestCase{
		testDescription: "input = int64(5)",
		input:           int64(5),
		expectedOutput:  Union{tag: INT, i: 5},
	},
	InterfaceUnionTestCase{
		testDescription: "input = uint64(5)",
		input:           uint64(5),
		expectedOutput:  Union{tag: UINT, ui: 5},
	},
	InterfaceUnionTestCase{
		testDescription: "input = int64(-5)",
		input:           int64(-5),
		expectedOutput:  Union{tag: INT, i: -5},
	},
	InterfaceUnionTestCase{
		testDescription: "input = 0",
		input:           0,
		expectedOutput:  Union{},
	},
	InterfaceUnionTestCase{
		testDescription: "input = 5",
		input:           5,
		expectedOutput:  Union{},
	},
	InterfaceUnionTestCase{
		testDescription: "input = MaxUint64",
		input:           uint64(math.MaxUint64),
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
	},
	InterfaceUnionTestCase{
		testDescription: "input = MaxInt64",
		input:           int64(math.MaxInt64),
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	InterfaceUnionTestCase{
		testDescription: "input = MaxInt64, stored as uint",
		input:           uint64(math.MaxInt64),
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
	},
	InterfaceUnionTestCase{
		testDescription: "input = MinInt64",
		input:           int64(math.MinInt64),
		expectedOutput:  Union{tag: INT, i: math.MinInt64},
	},
	InterfaceUnionTestCase{
		testDescription: "input = 5.0",
		input:           5.0,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	InterfaceUnionTestCase{
		testDescription: "input = float64(5.0)",
		input:           float64(5.0),
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	InterfaceUnionTestCase{
		testDescription: "input = math.MaxFloat64",
		input:           float64(math.MaxFloat64),
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
	},
	InterfaceUnionTestCase{
		testDescription: "input = \"\"",
		input:           "",
		expectedOutput:  Union{tag: STRING, s: ""},
	},
	InterfaceUnionTestCase{
		testDescription: "input = \"something\"",
		input:           "something",
		expectedOutput:  Union{tag: STRING, s: "something"},
	},
	InterfaceUnionTestCase{
		testDescription: "input = map[string]interface{}",
		input:           map[string]interface{}{"key": "value", "key2": 5, "key3": 5.0},
		expectedOutput:  Union{},
	},
}

func TestGetUnionFromValue(t *testing.T) {
	var output Union
	for _, test := range GetUnionFromValueTests {
		t0 := time.Now()
		output = getUnionFromValue(test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type UnionToStringTestCase struct {
	testDescription string
	input           Union
	expectedOutput  string
}

var UnionToStringTests = []UnionToStringTestCase{
	UnionToStringTestCase{
		testDescription: "input = nil",
		input:           Union{},
		expectedOutput:  "null",
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:BOOL, b:true}",
		input:           Union{tag: BOOL, b: true},
		expectedOutput:  "true",
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:BOOL, b:false}",
		input:           Union{tag: BOOL, b: false},
		expectedOutput:  "false",
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:INT,i:-5}",
		input:           Union{tag: INT, i: -5},
		expectedOutput:  "-5",
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:UINT,ui:5}",
		input:           Union{tag: UINT, ui: 5},
		expectedOutput:  "5",
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:FLOAT,f:5.0}",
		input:           Union{tag: FLOAT, f: 5.0},
		expectedOutput:  fmt.Sprintf("%f", 5.0),
	},
	UnionToStringTestCase{
		testDescription: "input = Union{tag:FLOAT,f:5.3}",
		input:           Union{tag: FLOAT, f: 5.3},
		expectedOutput:  fmt.Sprintf("%f", 5.3),
	},
}

func TestUnionToString(t *testing.T) {
	var output string
	for _, test := range UnionToStringTests {
		t0 := time.Now()
		output = unionValueToString(&test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type IsNumericTestCase struct {
	testDescription string
	input           Union
	expectedOutput  bool
}

var IsNumericTests = []IsNumericTestCase{
	IsNumericTestCase{
		testDescription: "input = nil",
		input:           Union{},
		expectedOutput:  false,
	},
	IsNumericTestCase{
		testDescription: "input = Union{tag:BOOL, b:true}",
		input:           Union{tag: BOOL, b: true},
		expectedOutput:  false,
	},
	IsNumericTestCase{
		testDescription: "input = Union{tag:BOOL, b:false}",
		input:           Union{tag: BOOL, b: false},
		expectedOutput:  false,
	},
	IsNumericTestCase{
		testDescription: "input = Union{tag:INT,i:-5}",
		input:           Union{tag: INT, i: -5},
		expectedOutput:  true,
	},
	IsNumericTestCase{
		testDescription: "input = Union{tag:UINT,ui:5}",
		input:           Union{tag: UINT, ui: 5},
		expectedOutput:  true,
	},
	IsNumericTestCase{
		testDescription: "input = Union{tag:FLOAT,f:5.0}",
		input:           Union{tag: FLOAT, f: 5.0},
		expectedOutput:  true,
	},
}

func TestIsNumeric(t *testing.T) {
	var output bool
	for _, test := range IsNumericTests {
		t0 := time.Now()
		output = isNumeric(&test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type CastUnionTypeTestCase struct {
	testDescription string
	input           Union
	newType         DataType
	expectedOutput  Union
	errorExpected   bool
}

var CastUnionTypeTests = []CastUnionTypeTestCase{
	CastUnionTypeTestCase{
		testDescription: "empty union to NIL",
		input:           Union{},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty union to BOOL",
		input:           Union{},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty union to UINT",
		input:           Union{},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty union to INT",
		input:           Union{},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty union to FLOAT",
		input:           Union{},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty union to STRING",
		input:           Union{},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to NIL",
		input:           Union{tag: BOOL, b: false},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to BOOL",
		input:           Union{tag: BOOL, b: false},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to UINT",
		input:           Union{tag: BOOL, b: false},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to INT",
		input:           Union{tag: BOOL, b: false},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to FLOAT",
		input:           Union{tag: BOOL, b: false},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "false bool union to STRING",
		input:           Union{tag: BOOL, b: false},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "false"},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to NIL",
		input:           Union{tag: BOOL, b: true},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to BOOL",
		input:           Union{tag: BOOL, b: true},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to UINT",
		input:           Union{tag: BOOL, b: true},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to INT",
		input:           Union{tag: BOOL, b: true},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to FLOAT",
		input:           Union{tag: BOOL, b: true},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "true bool union to STRING",
		input:           Union{tag: BOOL, b: true},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "true"},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to NIL",
		input:           Union{tag: UINT, ui: 3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to BOOL",
		input:           Union{tag: UINT, ui: 3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to false BOOL",
		input:           Union{tag: UINT, ui: 0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to UINT",
		input:           Union{tag: UINT, ui: 3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to INT",
		input:           Union{tag: UINT, ui: 3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "large UINT union to INT",
		input:           Union{tag: UINT, ui: math.MaxUint64},
		newType:         INT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to FLOAT",
		input:           Union{tag: UINT, ui: 3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "large UINT union to FLOAT",
		input:           Union{tag: UINT, ui: math.MaxUint64},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxUint64)},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "UINT union to STRING",
		input:           Union{tag: UINT, ui: 3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to NIL",
		input:           Union{tag: INT, i: 3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to BOOL",
		input:           Union{tag: INT, i: 3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative INT union to BOOL",
		input:           Union{tag: INT, i: -3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to false BOOL",
		input:           Union{tag: INT, i: 0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to UINT",
		input:           Union{tag: INT, i: 3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative INT union to UINT",
		input:           Union{tag: INT, i: -3},
		newType:         UINT,
		expectedOutput:  Union{tag: INT, i: -3},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "max INT union to UINT",
		input:           Union{tag: INT, i: math.MaxInt64},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to INT",
		input:           Union{tag: INT, i: 3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to FLOAT",
		input:           Union{tag: INT, i: 3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "max INT union to FLOAT",
		input:           Union{tag: INT, i: math.MaxInt64},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "INT union to STRING",
		input:           Union{tag: INT, i: 3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to NIL",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to true BOOL",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to false BOOL",
		input:           Union{tag: FLOAT, f: 0.0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative FLOAT union to true BOOL",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to UINT, 3.3",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to UINT, 3.6",
		input:           Union{tag: FLOAT, f: 3.6},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: -3.3},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "large FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "large negative FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: -math.MaxFloat64},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: -math.MaxFloat64},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to INT, 3.3",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to INT, 3.6",
		input:           Union{tag: FLOAT, f: 3.6},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative FLOAT union to INT",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "large FLOAT union to INT",
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
		newType:         INT,
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "large negative FLOAT union to INT",
		input:           Union{tag: FLOAT, f: -math.MaxFloat64},
		newType:         INT,
		expectedOutput:  Union{tag: FLOAT, f: -math.MaxFloat64},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to FLOAT",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "FLOAT union to STRING",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 3.3)},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "STRING union to NIL",
		input:           Union{tag: STRING, s: "some string"},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "non-empty STRING union to BOOL",
		input:           Union{tag: STRING, s: "some string"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "\"true\" STRING union to BOOL",
		input:           Union{tag: STRING, s: "true"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "non-empty STRING union to false BOOL",
		input:           Union{tag: STRING, s: "false"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "empty STRING union to BOOL",
		input:           Union{tag: STRING, s: ""},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "int STRING union to UINT",
		input:           Union{tag: STRING, s: "5"},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative int STRING union to UINT",
		input:           Union{tag: STRING, s: "-5"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "-5"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "float STRING union to UINT",
		input:           Union{tag: STRING, s: "5.0"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "5.0"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "random STRING union to UINT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "int STRING union to INT",
		input:           Union{tag: STRING, s: "5"},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative int STRING union to INT",
		input:           Union{tag: STRING, s: "-5"},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "float STRING union to INT",
		input:           Union{tag: STRING, s: "5.0"},
		newType:         INT,
		expectedOutput:  Union{tag: STRING, s: "5.0"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "random STRING union to INT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         INT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "int STRING union to FLOAT",
		input:           Union{tag: STRING, s: "5"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "float STRING union to FLOAT",
		input:           Union{tag: STRING, s: "5.3"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "negative float STRING union to FLOAT",
		input:           Union{tag: STRING, s: "-5.3"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: -5.3},
		errorExpected:   false,
	},
	CastUnionTypeTestCase{
		testDescription: "random STRING union to FLOAT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	CastUnionTypeTestCase{
		testDescription: "STRING union to STRING",
		input:           Union{tag: STRING, s: "some string"},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   false,
	},
}

func TestCastUnionType(t *testing.T) {
	var err error
	for _, test := range CastUnionTypeTests {
		t0 := time.Now()
		err = castUnionType(&test.input, test.newType)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if test.input != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, test.input, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

type GetResultTypeTestCase struct {
	testDescription string
	input1          DataType
	input2          DataType
	expectedOutput  DataType
}

var GetResultTypeTests = []GetResultTypeTestCase{
	GetResultTypeTestCase{
		testDescription: "input = nil, nil",
		input1:          NIL,
		input2:          NIL,
		expectedOutput:  NIL,
	},
	GetResultTypeTestCase{
		testDescription: "input = nil, bool",
		input1:          NIL,
		input2:          BOOL,
		expectedOutput:  BOOL,
	},
	GetResultTypeTestCase{
		testDescription: "input = nil, int",
		input1:          NIL,
		input2:          INT,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = nil, uint",
		input1:          NIL,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	GetResultTypeTestCase{
		testDescription: "input = nil, float",
		input1:          NIL,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = nil, string",
		input1:          NIL,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, nil",
		input1:          BOOL,
		input2:          NIL,
		expectedOutput:  BOOL,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, bool",
		input1:          BOOL,
		input2:          BOOL,
		expectedOutput:  BOOL,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, int",
		input1:          BOOL,
		input2:          INT,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, uint",
		input1:          BOOL,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, float",
		input1:          BOOL,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = bool, string",
		input1:          BOOL,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, nil",
		input1:          UINT,
		input2:          NIL,
		expectedOutput:  UINT,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, bool",
		input1:          UINT,
		input2:          BOOL,
		expectedOutput:  UINT,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, int",
		input1:          UINT,
		input2:          INT,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, uint",
		input1:          UINT,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, float",
		input1:          UINT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = uint, string",
		input1:          UINT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, nil",
		input1:          INT,
		input2:          NIL,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, bool",
		input1:          INT,
		input2:          BOOL,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, int",
		input1:          INT,
		input2:          INT,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, uint",
		input1:          INT,
		input2:          UINT,
		expectedOutput:  INT,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, float",
		input1:          INT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = int, string",
		input1:          INT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, nil",
		input1:          FLOAT,
		input2:          NIL,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, bool",
		input1:          FLOAT,
		input2:          BOOL,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, int",
		input1:          FLOAT,
		input2:          INT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, uint",
		input1:          FLOAT,
		input2:          UINT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, float",
		input1:          FLOAT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	GetResultTypeTestCase{
		testDescription: "input = float, string",
		input1:          FLOAT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, nil",
		input1:          STRING,
		input2:          NIL,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, bool",
		input1:          STRING,
		input2:          BOOL,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, int",
		input1:          STRING,
		input2:          INT,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, uint",
		input1:          STRING,
		input2:          UINT,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, float",
		input1:          STRING,
		input2:          FLOAT,
		expectedOutput:  STRING,
	},
	GetResultTypeTestCase{
		testDescription: "input = string, string",
		input1:          STRING,
		input2:          STRING,
		expectedOutput:  STRING,
	},
}

func TestGetResultType(t *testing.T) {
	var output DataType
	for _, test := range GetResultTypeTests {
		t0 := time.Now()
		output = getResultType(test.input1, test.input2)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type CastValueToUnionTypeTestCase struct {
	testDescription string
	input           interface{}
	newType         DataType
	expectedOutput  Union
}

var CastValueToUnionTypeTests = []CastValueToUnionTypeTestCase{
	CastValueToUnionTypeTestCase{
		testDescription: "nil to NIL",
		input:           nil,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "nil to BOOL",
		input:           nil,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "nil to UINT",
		input:           nil,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "nil to INT",
		input:           nil,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "nil to FLOAT",
		input:           nil,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "nil to STRING",
		input:           nil,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: ""},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to NIL",
		input:           false,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to BOOL",
		input:           false,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to UINT",
		input:           false,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to INT",
		input:           false,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to FLOAT",
		input:           false,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "false bool to STRING",
		input:           false,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "false"},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to NIL",
		input:           true,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to BOOL",
		input:           true,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to UINT",
		input:           true,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 1},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to INT",
		input:           true,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 1},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to FLOAT",
		input:           true,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "true bool to STRING",
		input:           true,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "true"},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT 3 to NIL",
		input:           uint64(3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT 3 to BOOL",
		input:           uint64(3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT 0 to false BOOL",
		input:           uint64(0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT 3 to UINT",
		input:           uint64(3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT 3 to INT",
		input:           uint64(3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large UINT to INT",
		input:           uint64(math.MaxUint64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT to FLOAT",
		input:           uint64(3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large UINT to FLOAT",
		input:           uint64(math.MaxUint64),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxUint64)},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "UINT to STRING",
		input:           uint64(3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to NIL",
		input:           int64(3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to BOOL",
		input:           int64(3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative INT to BOOL",
		input:           int64(-3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to false BOOL",
		input:           int64(0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to UINT",
		input:           int64(3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative INT to UINT",
		input:           int64(-3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "max INT to UINT",
		input:           math.MaxInt64,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to INT",
		input:           int64(3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to FLOAT",
		input:           int64(3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "max INT to FLOAT",
		input:           int64(math.MaxInt64),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "INT to STRING",
		input:           int64(3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to NIL",
		input:           3,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to BOOL",
		input:           3,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified negative INT to BOOL",
		input:           -3,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to false BOOL",
		input:           0,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to UINT",
		input:           3,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified negative INT to UINT",
		input:           -3,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to INT",
		input:           3,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to FLOAT",
		input:           3,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified max INT to FLOAT",
		input:           math.MaxInt64,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "unspecified INT to STRING",
		input:           3,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to NIL",
		input:           float64(3.3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to true BOOL",
		input:           float64(3.3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to false BOOL",
		input:           float64(0.0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative FLOAT to true BOOL",
		input:           float64(-3.3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to UINT, 3.3",
		input:           float64(3.3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to UINT, 3.6",
		input:           float64(3.6),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative FLOAT to UINT",
		input:           float64(-3.3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large FLOAT to UINT",
		input:           float64(math.MaxFloat64),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large negative FLOAT to UINT",
		input:           -math.MaxFloat64,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to INT, 3.3",
		input:           float64(3.3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to INT, 3.6",
		input:           float64(3.6),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative FLOAT to INT",
		input:           float64(-3.3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large FLOAT to INT",
		input:           float64(math.MaxFloat64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "large negative FLOAT to INT",
		input:           float64(-math.MaxFloat64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MinInt64},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to FLOAT",
		input:           float64(3.3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "FLOAT to STRING",
		input:           float64(3.3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 3.3)},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "STRING to NIL",
		input:           "some string",
		newType:         NIL,
		expectedOutput:  Union{},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "non-empty STRING to BOOL",
		input:           "some string",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "\"true\" STRING to BOOL",
		input:           "true",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "non-empty STRING to false BOOL",
		input:           "false",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "empty STRING to BOOL",
		input:           "",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "int STRING to UINT",
		input:           "5",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 5},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative int STRING to UINT",
		input:           "-5",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "float STRING to UINT",
		input:           "5.0",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "random STRING to UINT",
		input:           "some string",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "int STRING to INT",
		input:           "5",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 5},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative int STRING to INT",
		input:           "-5",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -5},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "float STRING to INT",
		input:           "5.0",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "random STRING to INT",
		input:           "some string",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "int STRING to FLOAT",
		input:           "5",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "float STRING to FLOAT",
		input:           "5.3",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "negative float STRING to FLOAT",
		input:           "-5.3",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: -5.3},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "random STRING to FLOAT",
		input:           "some string",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	CastValueToUnionTypeTestCase{
		testDescription: "STRING to STRING",
		input:           "some string",
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "some string"},
	},
}

func TestCastValueToUnionType(t *testing.T) {
	var output Union
	for _, test := range CastValueToUnionTypeTests {
		t0 := time.Now()
		output = castValueToUnionType(test.input, test.newType)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, test.input, test.expectedOutput)
		}
	}
}

type UnionInterfaceTestCase struct {
	testDescription string
	expectedOutput  interface{}
	input           Union
}

var GetValueFromUnionTests = []UnionInterfaceTestCase{
	UnionInterfaceTestCase{
		testDescription: "input = nil",
		expectedOutput:  nil,
		input:           Union{},
	},
	UnionInterfaceTestCase{
		testDescription: "input = true",
		expectedOutput:  true,
		input:           Union{tag: BOOL, b: true},
	},
	UnionInterfaceTestCase{
		testDescription: "input = false",
		expectedOutput:  false,
		input:           Union{tag: BOOL, b: false},
	},
	UnionInterfaceTestCase{
		testDescription: "input = int64(0)",
		expectedOutput:  int64(0),
		input:           Union{tag: INT, i: 0},
	},
	UnionInterfaceTestCase{
		testDescription: "input = uint64(0)",
		expectedOutput:  uint64(0),
		input:           Union{tag: UINT, ui: 0},
	},
	UnionInterfaceTestCase{
		testDescription: "input = int64(5)",
		expectedOutput:  int64(5),
		input:           Union{tag: INT, i: 5},
	},
	UnionInterfaceTestCase{
		testDescription: "input = uint64(5)",
		expectedOutput:  uint64(5),
		input:           Union{tag: UINT, ui: 5},
	},
	UnionInterfaceTestCase{
		testDescription: "input = int64(-5)",
		expectedOutput:  int64(-5),
		input:           Union{tag: INT, i: -5},
	},
	UnionInterfaceTestCase{
		testDescription: "input = MaxUint64",
		expectedOutput:  uint64(math.MaxUint64),
		input:           Union{tag: UINT, ui: math.MaxUint64},
	},
	UnionInterfaceTestCase{
		testDescription: "input = MaxInt64",
		expectedOutput:  int64(math.MaxInt64),
		input:           Union{tag: INT, i: math.MaxInt64},
	},
	UnionInterfaceTestCase{
		testDescription: "input = MaxInt64, stored as uint",
		expectedOutput:  uint64(math.MaxInt64),
		input:           Union{tag: UINT, ui: math.MaxInt64},
	},
	UnionInterfaceTestCase{
		testDescription: "input = MinInt64",
		expectedOutput:  int64(math.MinInt64),
		input:           Union{tag: INT, i: math.MinInt64},
	},
	UnionInterfaceTestCase{
		testDescription: "input = 5.0",
		expectedOutput:  5.0,
		input:           Union{tag: FLOAT, f: 5.0},
	},
	UnionInterfaceTestCase{
		testDescription: "input = float64(5.0)",
		expectedOutput:  float64(5.0),
		input:           Union{tag: FLOAT, f: 5.0},
	},
	UnionInterfaceTestCase{
		testDescription: "input = math.MaxFloat64",
		expectedOutput:  float64(math.MaxFloat64),
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
	},
	UnionInterfaceTestCase{
		testDescription: "input = \"\"",
		expectedOutput:  "",
		input:           Union{tag: STRING, s: ""},
	},
	UnionInterfaceTestCase{
		testDescription: "input = \"something\"",
		expectedOutput:  "something",
		input:           Union{tag: STRING, s: "something"},
	},
}

func TestGetValueFromUnion(t *testing.T) {
	var output interface{}
	for _, test := range GetValueFromUnionTests {
		t0 := time.Now()
		output = getValueFromUnion(&test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type StringInSliceTestCase struct {
	testDescription string
	stringToLookFor string
	slice           []string
	expectedOutput  bool
}

var StringInSliceTests = []StringInSliceTestCase{
	StringInSliceTestCase{
		testDescription: "string in slice",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:  true,
	},
	StringInSliceTestCase{
		testDescription: "slightly different",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey."},
		expectedOutput:  false,
	},
	StringInSliceTestCase{
		testDescription: "not there",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "puppy"},
		expectedOutput:  false,
	},
	StringInSliceTestCase{
		testDescription: "empty list",
		stringToLookFor: "golden monkey",
		slice:           []string{},
		expectedOutput:  false,
	},
	StringInSliceTestCase{
		testDescription: "empty string (not in list)",
		stringToLookFor: "",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:  false,
	},
	StringInSliceTestCase{
		testDescription: "empty string (in list)",
		stringToLookFor: "",
		slice:           []string{"I", "wish", "I", "was", "a", "", "golden monkey"},
		expectedOutput:  true,
	},
}

func TestStringInSlice(t *testing.T) {
	var output bool
	for _, test := range StringInSliceTests {
		t0 := time.Now()
		output = stringInSlice(test.slice, test.stringToLookFor)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type ErrorInSliceTestCase struct {
	testDescription string
	errorToLookFor  error
	slice           []error
	expectedOutput  bool
}

var ErrorInSliceTests = []ErrorInSliceTestCase{
	ErrorInSliceTestCase{
		testDescription: "error in slice",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkey")},
		expectedOutput:  true,
	},
	ErrorInSliceTestCase{
		testDescription: "slightly different",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkey.")},
		expectedOutput:  false,
	},
	ErrorInSliceTestCase{
		testDescription: "not there",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("puppy")},
		expectedOutput:  false,
	},
	ErrorInSliceTestCase{
		testDescription: "empty list",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{},
		expectedOutput:  false,
	},
	ErrorInSliceTestCase{
		testDescription: "empty error (not in list)",
		errorToLookFor:  fmt.Errorf(""),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkey")},
		expectedOutput:  false,
	},
	ErrorInSliceTestCase{
		testDescription: "empty error (in list)",
		errorToLookFor:  fmt.Errorf(""),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf(""), fmt.Errorf("golden monkey")},
		expectedOutput:  true,
	},
}

func TestErrorInSlice(t *testing.T) {
	var output bool
	for _, test := range ErrorInSliceTests {
		t0 := time.Now()
		output = errorInSlice(test.slice, test.errorToLookFor)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type GetParentUriTestCase struct {
	testDescription string
	input           string
	expectedOutput  string
}

var GetParentUriTests = []GetParentUriTestCase{
	GetParentUriTestCase{
		testDescription: "empty string",
		input:           "",
		expectedOutput:  "",
	},
	GetParentUriTestCase{
		testDescription: "random string",
		input:           "blah",
		expectedOutput:  "/blah",
	},
	GetParentUriTestCase{
		testDescription: "/",
		input:           "/",
		expectedOutput:  "/",
	},
	GetParentUriTestCase{
		testDescription: "/components",
		input:           "/components",
		expectedOutput:  "/components",
	},
	GetParentUriTestCase{
		testDescription: "/components/ess_1",
		input:           "/components/ess_1",
		expectedOutput:  "/components",
	},
	GetParentUriTestCase{
		testDescription: "/components/ess_1/vmax",
		input:           "/components/ess_1/vmax",
		expectedOutput:  "/components/ess_1",
	},
	GetParentUriTestCase{
		testDescription: "components/ess_1/vmax",
		input:           "components/ess_1/vmax",
		expectedOutput:  "/components/ess_1",
	},
}

func TestGetParentUri(t *testing.T) {
	var output string
	for _, test := range GetParentUriTests {
		t0 := time.Now()
		output = GetParentUri(test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

var GetUriElementTests = []GetParentUriTestCase{
	GetParentUriTestCase{
		testDescription: "empty string",
		input:           "",
		expectedOutput:  "",
	},
	GetParentUriTestCase{
		testDescription: "random string",
		input:           "blah",
		expectedOutput:  "blah",
	},
	GetParentUriTestCase{
		testDescription: "/",
		input:           "/",
		expectedOutput:  "",
	},
	GetParentUriTestCase{
		testDescription: "/components",
		input:           "/components",
		expectedOutput:  "components",
	},
	GetParentUriTestCase{
		testDescription: "/components/ess_1",
		input:           "/components/ess_1",
		expectedOutput:  "ess_1",
	},
	GetParentUriTestCase{
		testDescription: "/components/ess_1/vmax",
		input:           "/components/ess_1/vmax",
		expectedOutput:  "vmax",
	},
	GetParentUriTestCase{
		testDescription: "components/ess_1/vmax",
		input:           "components/ess_1/vmax",
		expectedOutput:  "vmax",
	},
}

func TestGetUriElement(t *testing.T) {
	var output string
	for _, test := range GetUriElementTests {
		t0 := time.Now()
		output = GetUriElement(test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
	}
}

type RemoveDuplicateValuesTestCase struct {
	testDescription string
	input           []string
	expectedOutput  []string
}

var RemoveDuplicateValuesTests = []RemoveDuplicateValuesTestCase{
	RemoveDuplicateValuesTestCase{
		testDescription: "empty list",
		input:           []string{},
		expectedOutput:  []string{},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "no duplicates",
		input:           []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "everything duplicated once, in order",
		input:           []string{"string1", "string1", "string2", "string2", "string3", "string3", "string4", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "everything duplicated once, repeating list twice",
		input:           []string{"string1", "string2", "string3", "string4", "string1", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "three duplicates",
		input:           []string{"string1", "string2", "string2", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "three duplicates of empty string",
		input:           []string{"string1", "", "", "", "string3", "string4"},
		expectedOutput:  []string{"string1", "", "string3", "string4"},
	},
	RemoveDuplicateValuesTestCase{
		testDescription: "everything is duplicate",
		input:           []string{"string1", "string1", "string1", "string1", "string1"},
		expectedOutput:  []string{"string1"},
	},
}

func TestRemoveDuplicateValues(t *testing.T) {
	var output []string
	for _, test := range RemoveDuplicateValuesTests {
		t0 := time.Now()
		output = removeDuplicateValues(test.input)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutput) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		for i, _ := range test.expectedOutput {
			if output[i] != test.expectedOutput[i] {
				t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
				break
			}
		}
	}
}

type StringListsMatchTestCase struct {
	testDescription string
	input1          []string
	input2          []string
	expectedOutput  bool
}

var StringListsMatchTests = []StringListsMatchTestCase{
	StringListsMatchTestCase{
		testDescription: "empty list",
		input1:          []string{},
		input2:          []string{},
		expectedOutput:  true,
	},
	StringListsMatchTestCase{
		testDescription: "no duplicates",
		input1:          []string{"string1", "string2", "string3", "string4"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  true,
	},
	StringListsMatchTestCase{
		testDescription: "different lengths",
		input1:          []string{"string1", "string1", "string2", "string2", "string3", "string3", "string4", "string4"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  false,
	},
	StringListsMatchTestCase{
		testDescription: "list contains empty string",
		input1:          []string{"string1", "", "string3", "string4"},
		input2:          []string{"string1", "", "string3", "string4"},
		expectedOutput:  true,
	},
	StringListsMatchTestCase{
		testDescription: "lists in different order",
		input1:          []string{"string2", "string1", "string4", "string3"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  true,
	},
	StringListsMatchTestCase{
		testDescription: "lists have different items",
		input1:          []string{"string2", "string1", "string4", "string3"},
		input2:          []string{"string1", "oogabooga", "string3", "string4"},
		expectedOutput:  false,
	},
}

func TestStringListsMatch(t *testing.T) {
	var output bool
	for _, test := range StringListsMatchTests {
		t0 := time.Now()
		output = stringListsMatch(test.input1, test.input2)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}

	}
}

type RegexStringInSliceTestCase struct {
	testDescription      string
	stringToLookFor      string
	slice                []string
	expectedOutput       bool
	expectedStringOutput string
}

var RegexStringInSliceTests = []RegexStringInSliceTestCase{
	RegexStringInSliceTestCase{
		testDescription:      "golden* matches golden monkey",
		stringToLookFor:      "golden*",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:       true,
		expectedStringOutput: "golden monkey",
	},
	RegexStringInSliceTestCase{
		testDescription:      "*",
		stringToLookFor:      "*",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey."},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	RegexStringInSliceTestCase{
		testDescription:      "not there",
		stringToLookFor:      "golden*",
		slice:                []string{"I", "wish", "I", "was", "a", "puppy"},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	RegexStringInSliceTestCase{
		testDescription:      "empty list",
		stringToLookFor:      "golden*",
		slice:                []string{},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	RegexStringInSliceTestCase{
		testDescription:      "empty list, look for *",
		stringToLookFor:      "*",
		slice:                []string{},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	RegexStringInSliceTestCase{
		testDescription:      "empty string (not in list)",
		stringToLookFor:      "",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:       true,
		expectedStringOutput: "I",
	},
	RegexStringInSliceTestCase{
		testDescription:      "empty string (in list)",
		stringToLookFor:      "",
		slice:                []string{"I", "wish", "I", "was", "a", "", "golden monkey"},
		expectedOutput:       true,
		expectedStringOutput: "I",
	},
}

func TestRegexStringInSlice(t *testing.T) {
	var output bool
	var outputStr string
	for _, test := range RegexStringInSliceTests {
		t0 := time.Now()
		outputStr, output = regexStringInSlice(test.slice, test.stringToLookFor)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if outputStr != test.expectedStringOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, outputStr, test.expectedStringOutput)
		}
	}
}
