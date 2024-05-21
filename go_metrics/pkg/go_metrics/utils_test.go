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
	{
		testDescription: "nil",
		input:           NIL,
		expectedOutput:  "nil",
	},
	{
		testDescription: "bool",
		input:           BOOL,
		expectedOutput:  "bool",
	},
	{
		testDescription: "uint",
		input:           UINT,
		expectedOutput:  "uint",
	},
	{
		testDescription: "int",
		input:           INT,
		expectedOutput:  "int",
	},
	{
		testDescription: "float",
		input:           FLOAT,
		expectedOutput:  "float",
	},
	{
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
	{
		testDescription: "nil",
		input:           NIL,
		expectedOutput:  []byte("\"nil\""),
	},
	{
		testDescription: "bool",
		input:           BOOL,
		expectedOutput:  []byte("\"bool\""),
	},
	{
		testDescription: "uint",
		input:           UINT,
		expectedOutput:  []byte("\"uint\""),
	},
	{
		testDescription: "int",
		input:           INT,
		expectedOutput:  []byte("\"int\""),
	},
	{
		testDescription: "float",
		input:           FLOAT,
		expectedOutput:  []byte("\"float\""),
	},
	{
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
			for i := range output {
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
	{
		testDescription: "input = nil",
		input:           nil,
		expectedOutput:  Union{},
	},
	{
		testDescription: "input = true",
		input:           true,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "input = false",
		input:           false,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "input = int64(0)",
		input:           int64(0),
		expectedOutput:  Union{tag: INT, i: 0},
	},
	{
		testDescription: "input = uint64(0)",
		input:           uint64(0),
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "input = int64(5)",
		input:           int64(5),
		expectedOutput:  Union{tag: INT, i: 5},
	},
	{
		testDescription: "input = uint64(5)",
		input:           uint64(5),
		expectedOutput:  Union{tag: UINT, ui: 5},
	},
	{
		testDescription: "input = int64(-5)",
		input:           int64(-5),
		expectedOutput:  Union{tag: INT, i: -5},
	},
	{
		testDescription: "input = 0",
		input:           0,
		expectedOutput:  Union{},
	},
	{
		testDescription: "input = 5",
		input:           5,
		expectedOutput:  Union{},
	},
	{
		testDescription: "input = MaxUint64",
		input:           uint64(math.MaxUint64),
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
	},
	{
		testDescription: "input = MaxInt64",
		input:           int64(math.MaxInt64),
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	{
		testDescription: "input = MaxInt64, stored as uint",
		input:           uint64(math.MaxInt64),
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
	},
	{
		testDescription: "input = MinInt64",
		input:           int64(math.MinInt64),
		expectedOutput:  Union{tag: INT, i: math.MinInt64},
	},
	{
		testDescription: "input = 5.0",
		input:           5.0,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	{
		testDescription: "input = float64(5.0)",
		input:           float64(5.0),
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	{
		testDescription: "input = math.MaxFloat64",
		input:           float64(math.MaxFloat64),
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
	},
	{
		testDescription: "input = \"\"",
		input:           "",
		expectedOutput:  Union{tag: STRING, s: ""},
	},
	{
		testDescription: "input = \"something\"",
		input:           "something",
		expectedOutput:  Union{tag: STRING, s: "something"},
	},
	{
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
	{
		testDescription: "input = nil",
		input:           Union{},
		expectedOutput:  "null",
	},
	{
		testDescription: "input = Union{tag:BOOL, b:true}",
		input:           Union{tag: BOOL, b: true},
		expectedOutput:  "true",
	},
	{
		testDescription: "input = Union{tag:BOOL, b:false}",
		input:           Union{tag: BOOL, b: false},
		expectedOutput:  "false",
	},
	{
		testDescription: "input = Union{tag:INT,i:-5}",
		input:           Union{tag: INT, i: -5},
		expectedOutput:  "-5",
	},
	{
		testDescription: "input = Union{tag:UINT,ui:5}",
		input:           Union{tag: UINT, ui: 5},
		expectedOutput:  "5",
	},
	{
		testDescription: "input = Union{tag:FLOAT,f:5.0}",
		input:           Union{tag: FLOAT, f: 5.0},
		expectedOutput:  fmt.Sprintf("%.2f", 5.0),
	},
	{
		testDescription: "input = Union{tag:FLOAT,f:5.3}",
		input:           Union{tag: FLOAT, f: 5.3},
		expectedOutput:  fmt.Sprintf("%.2f", 5.3),
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
	{
		testDescription: "input = nil",
		input:           Union{},
		expectedOutput:  false,
	},
	{
		testDescription: "input = Union{tag:BOOL, b:true}",
		input:           Union{tag: BOOL, b: true},
		expectedOutput:  false,
	},
	{
		testDescription: "input = Union{tag:BOOL, b:false}",
		input:           Union{tag: BOOL, b: false},
		expectedOutput:  false,
	},
	{
		testDescription: "input = Union{tag:INT,i:-5}",
		input:           Union{tag: INT, i: -5},
		expectedOutput:  true,
	},
	{
		testDescription: "input = Union{tag:UINT,ui:5}",
		input:           Union{tag: UINT, ui: 5},
		expectedOutput:  true,
	},
	{
		testDescription: "input = Union{tag:FLOAT,f:5.0}",
		input:           Union{tag: FLOAT, f: 5.0},
		expectedOutput:  true,
	},
}

// check if a Union is an int64, uint64, or a float
func isNumeric(union *Union) bool {
	if union.tag == INT || union.tag == UINT || union.tag == FLOAT {
		return true
	}
	return false
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
	{
		testDescription: "empty union to NIL",
		input:           Union{},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "empty union to BOOL",
		input:           Union{},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "empty union to UINT",
		input:           Union{},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	{
		testDescription: "empty union to INT",
		input:           Union{},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	{
		testDescription: "empty union to FLOAT",
		input:           Union{},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	{
		testDescription: "empty union to STRING",
		input:           Union{},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to NIL",
		input:           Union{tag: BOOL, b: false},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to BOOL",
		input:           Union{tag: BOOL, b: false},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to UINT",
		input:           Union{tag: BOOL, b: false},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to INT",
		input:           Union{tag: BOOL, b: false},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to FLOAT",
		input:           Union{tag: BOOL, b: false},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	{
		testDescription: "false bool union to STRING",
		input:           Union{tag: BOOL, b: false},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "false"},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to NIL",
		input:           Union{tag: BOOL, b: true},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to BOOL",
		input:           Union{tag: BOOL, b: true},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to UINT",
		input:           Union{tag: BOOL, b: true},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to INT",
		input:           Union{tag: BOOL, b: true},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to FLOAT",
		input:           Union{tag: BOOL, b: true},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	{
		testDescription: "true bool union to STRING",
		input:           Union{tag: BOOL, b: true},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "true"},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to NIL",
		input:           Union{tag: UINT, ui: 3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to BOOL",
		input:           Union{tag: UINT, ui: 3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to false BOOL",
		input:           Union{tag: UINT, ui: 0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to UINT",
		input:           Union{tag: UINT, ui: 3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to INT",
		input:           Union{tag: UINT, ui: 3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	{
		testDescription: "large UINT union to INT",
		input:           Union{tag: UINT, ui: math.MaxUint64},
		newType:         INT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
		errorExpected:   true,
	},
	{
		testDescription: "UINT union to FLOAT",
		input:           Union{tag: UINT, ui: 3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
		errorExpected:   false,
	},
	{
		testDescription: "large UINT union to FLOAT",
		input:           Union{tag: UINT, ui: math.MaxUint64},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxUint64)},
		errorExpected:   false,
	},
	{
		testDescription: "UINT union to STRING",
		input:           Union{tag: UINT, ui: 3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to NIL",
		input:           Union{tag: INT, i: 3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to BOOL",
		input:           Union{tag: INT, i: 3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "negative INT union to BOOL",
		input:           Union{tag: INT, i: -3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to false BOOL",
		input:           Union{tag: INT, i: 0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to UINT",
		input:           Union{tag: INT, i: 3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	{
		testDescription: "negative INT union to UINT",
		input:           Union{tag: INT, i: -3},
		newType:         UINT,
		expectedOutput:  Union{tag: INT, i: -3},
		errorExpected:   true,
	},
	{
		testDescription: "max INT union to UINT",
		input:           Union{tag: INT, i: math.MaxInt64},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to INT",
		input:           Union{tag: INT, i: 3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to FLOAT",
		input:           Union{tag: INT, i: 3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
		errorExpected:   false,
	},
	{
		testDescription: "max INT union to FLOAT",
		input:           Union{tag: INT, i: math.MaxInt64},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
		errorExpected:   false,
	},
	{
		testDescription: "INT union to STRING",
		input:           Union{tag: INT, i: 3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to NIL",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to true BOOL",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to false BOOL",
		input:           Union{tag: FLOAT, f: 0.0},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "negative FLOAT union to true BOOL",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to UINT, 3.3",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to UINT, 3.6",
		input:           Union{tag: FLOAT, f: 3.6},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	{
		testDescription: "negative FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: -3.3},
		errorExpected:   true,
	},
	{
		testDescription: "large FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
		errorExpected:   true,
	},
	{
		testDescription: "large negative FLOAT union to UINT",
		input:           Union{tag: FLOAT, f: -math.MaxFloat64},
		newType:         UINT,
		expectedOutput:  Union{tag: FLOAT, f: -math.MaxFloat64},
		errorExpected:   true,
	},
	{
		testDescription: "FLOAT union to INT, 3.3",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to INT, 3.6",
		input:           Union{tag: FLOAT, f: 3.6},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	{
		testDescription: "negative FLOAT union to INT",
		input:           Union{tag: FLOAT, f: -3.3},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -3},
		errorExpected:   false,
	},
	{
		testDescription: "large FLOAT union to INT",
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
		newType:         INT,
		expectedOutput:  Union{tag: FLOAT, f: math.MaxFloat64},
		errorExpected:   true,
	},
	{
		testDescription: "large negative FLOAT union to INT",
		input:           Union{tag: FLOAT, f: -math.MaxFloat64},
		newType:         INT,
		expectedOutput:  Union{tag: FLOAT, f: -math.MaxFloat64},
		errorExpected:   true,
	},
	{
		testDescription: "FLOAT union to FLOAT",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.3},
		errorExpected:   false,
	},
	{
		testDescription: "FLOAT union to STRING",
		input:           Union{tag: FLOAT, f: 3.3},
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 3.3)},
		errorExpected:   false,
	},
	{
		testDescription: "STRING union to NIL",
		input:           Union{tag: STRING, s: "some string"},
		newType:         NIL,
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	{
		testDescription: "non-empty STRING union to BOOL",
		input:           Union{tag: STRING, s: "some string"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "\"true\" STRING union to BOOL",
		input:           Union{tag: STRING, s: "true"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	{
		testDescription: "non-empty STRING union to false BOOL",
		input:           Union{tag: STRING, s: "false"},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "empty STRING union to BOOL",
		input:           Union{tag: STRING, s: ""},
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	{
		testDescription: "int STRING union to UINT",
		input:           Union{tag: STRING, s: "5"},
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	{
		testDescription: "negative int STRING union to UINT",
		input:           Union{tag: STRING, s: "-5"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "-5"},
		errorExpected:   true,
	},
	{
		testDescription: "float STRING union to UINT",
		input:           Union{tag: STRING, s: "5.0"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "5.0"},
		errorExpected:   true,
	},
	{
		testDescription: "random STRING union to UINT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         UINT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	{
		testDescription: "int STRING union to INT",
		input:           Union{tag: STRING, s: "5"},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	{
		testDescription: "negative int STRING union to INT",
		input:           Union{tag: STRING, s: "-5"},
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	{
		testDescription: "float STRING union to INT",
		input:           Union{tag: STRING, s: "5.0"},
		newType:         INT,
		expectedOutput:  Union{tag: STRING, s: "5.0"},
		errorExpected:   true,
	},
	{
		testDescription: "random STRING union to INT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         INT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	{
		testDescription: "int STRING union to FLOAT",
		input:           Union{tag: STRING, s: "5"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	{
		testDescription: "float STRING union to FLOAT",
		input:           Union{tag: STRING, s: "5.3"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	{
		testDescription: "negative float STRING union to FLOAT",
		input:           Union{tag: STRING, s: "-5.3"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: -5.3},
		errorExpected:   false,
	},
	{
		testDescription: "random STRING union to FLOAT",
		input:           Union{tag: STRING, s: "some string"},
		newType:         FLOAT,
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   true,
	},
	{
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
	{
		testDescription: "input = nil, nil",
		input1:          NIL,
		input2:          NIL,
		expectedOutput:  NIL,
	},
	{
		testDescription: "input = nil, bool",
		input1:          NIL,
		input2:          BOOL,
		expectedOutput:  BOOL,
	},
	{
		testDescription: "input = nil, int",
		input1:          NIL,
		input2:          INT,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = nil, uint",
		input1:          NIL,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	{
		testDescription: "input = nil, float",
		input1:          NIL,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = nil, string",
		input1:          NIL,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = bool, nil",
		input1:          BOOL,
		input2:          NIL,
		expectedOutput:  BOOL,
	},
	{
		testDescription: "input = bool, bool",
		input1:          BOOL,
		input2:          BOOL,
		expectedOutput:  BOOL,
	},
	{
		testDescription: "input = bool, int",
		input1:          BOOL,
		input2:          INT,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = bool, uint",
		input1:          BOOL,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	{
		testDescription: "input = bool, float",
		input1:          BOOL,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = bool, string",
		input1:          BOOL,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = uint, nil",
		input1:          UINT,
		input2:          NIL,
		expectedOutput:  UINT,
	},
	{
		testDescription: "input = uint, bool",
		input1:          UINT,
		input2:          BOOL,
		expectedOutput:  UINT,
	},
	{
		testDescription: "input = uint, int",
		input1:          UINT,
		input2:          INT,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = uint, uint",
		input1:          UINT,
		input2:          UINT,
		expectedOutput:  UINT,
	},
	{
		testDescription: "input = uint, float",
		input1:          UINT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = uint, string",
		input1:          UINT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = int, nil",
		input1:          INT,
		input2:          NIL,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = int, bool",
		input1:          INT,
		input2:          BOOL,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = int, int",
		input1:          INT,
		input2:          INT,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = int, uint",
		input1:          INT,
		input2:          UINT,
		expectedOutput:  INT,
	},
	{
		testDescription: "input = int, float",
		input1:          INT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = int, string",
		input1:          INT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = float, nil",
		input1:          FLOAT,
		input2:          NIL,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = float, bool",
		input1:          FLOAT,
		input2:          BOOL,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = float, int",
		input1:          FLOAT,
		input2:          INT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = float, uint",
		input1:          FLOAT,
		input2:          UINT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = float, float",
		input1:          FLOAT,
		input2:          FLOAT,
		expectedOutput:  FLOAT,
	},
	{
		testDescription: "input = float, string",
		input1:          FLOAT,
		input2:          STRING,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = string, nil",
		input1:          STRING,
		input2:          NIL,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = string, bool",
		input1:          STRING,
		input2:          BOOL,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = string, int",
		input1:          STRING,
		input2:          INT,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = string, uint",
		input1:          STRING,
		input2:          UINT,
		expectedOutput:  STRING,
	},
	{
		testDescription: "input = string, float",
		input1:          STRING,
		input2:          FLOAT,
		expectedOutput:  STRING,
	},
	{
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
	{
		testDescription: "nil to NIL",
		input:           nil,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "nil to BOOL",
		input:           nil,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "nil to UINT",
		input:           nil,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "nil to INT",
		input:           nil,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	{
		testDescription: "nil to FLOAT",
		input:           nil,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	{
		testDescription: "nil to STRING",
		input:           nil,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: ""},
	},
	{
		testDescription: "false bool to NIL",
		input:           false,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "false bool to BOOL",
		input:           false,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "false bool to UINT",
		input:           false,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "false bool to INT",
		input:           false,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	{
		testDescription: "false bool to FLOAT",
		input:           false,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	{
		testDescription: "false bool to STRING",
		input:           false,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "false"},
	},
	{
		testDescription: "true bool to NIL",
		input:           true,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "true bool to BOOL",
		input:           true,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "true bool to UINT",
		input:           true,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 1},
	},
	{
		testDescription: "true bool to INT",
		input:           true,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 1},
	},
	{
		testDescription: "true bool to FLOAT",
		input:           true,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
	},
	{
		testDescription: "true bool to STRING",
		input:           true,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "true"},
	},
	{
		testDescription: "UINT 3 to NIL",
		input:           uint64(3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "UINT 3 to BOOL",
		input:           uint64(3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "UINT 0 to false BOOL",
		input:           uint64(0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "UINT 3 to UINT",
		input:           uint64(3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	{
		testDescription: "UINT 3 to INT",
		input:           uint64(3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	{
		testDescription: "large UINT to INT",
		input:           uint64(math.MaxUint64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	{
		testDescription: "UINT to FLOAT",
		input:           uint64(3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	{
		testDescription: "large UINT to FLOAT",
		input:           uint64(math.MaxUint64),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxUint64)},
	},
	{
		testDescription: "UINT to STRING",
		input:           uint64(3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	{
		testDescription: "INT to NIL",
		input:           int64(3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "INT to BOOL",
		input:           int64(3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "negative INT to BOOL",
		input:           int64(-3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "INT to false BOOL",
		input:           int64(0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "INT to UINT",
		input:           int64(3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	{
		testDescription: "negative INT to UINT",
		input:           int64(-3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, i: 0},
	},
	{
		testDescription: "max INT to UINT",
		input:           math.MaxInt64,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxInt64},
	},
	{
		testDescription: "INT to INT",
		input:           int64(3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	{
		testDescription: "INT to FLOAT",
		input:           int64(3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	{
		testDescription: "max INT to FLOAT",
		input:           int64(math.MaxInt64),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
	},
	{
		testDescription: "INT to STRING",
		input:           int64(3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	{
		testDescription: "unspecified INT to NIL",
		input:           3,
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "unspecified INT to BOOL",
		input:           3,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "unspecified negative INT to BOOL",
		input:           -3,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "unspecified INT to false BOOL",
		input:           0,
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "unspecified INT to UINT",
		input:           3,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	{
		testDescription: "unspecified negative INT to UINT",
		input:           -3,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, i: 0},
	},
	{
		testDescription: "unspecified INT to INT",
		input:           3,
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	{
		testDescription: "unspecified INT to FLOAT",
		input:           3,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.0},
	},
	{
		testDescription: "unspecified max INT to FLOAT",
		input:           math.MaxInt64,
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: float64(math.MaxInt64)},
	},
	{
		testDescription: "unspecified INT to STRING",
		input:           3,
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: "3"},
	},
	{
		testDescription: "FLOAT to NIL",
		input:           float64(3.3),
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "FLOAT to true BOOL",
		input:           float64(3.3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "FLOAT to false BOOL",
		input:           float64(0.0),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "negative FLOAT to true BOOL",
		input:           float64(-3.3),
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "FLOAT to UINT, 3.3",
		input:           float64(3.3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	{
		testDescription: "FLOAT to UINT, 3.6",
		input:           float64(3.6),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 3},
	},
	{
		testDescription: "negative FLOAT to UINT",
		input:           float64(-3.3),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "large FLOAT to UINT",
		input:           float64(math.MaxFloat64),
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
	},
	{
		testDescription: "large negative FLOAT to UINT",
		input:           -math.MaxFloat64,
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "FLOAT to INT, 3.3",
		input:           float64(3.3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	{
		testDescription: "FLOAT to INT, 3.6",
		input:           float64(3.6),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 3},
	},
	{
		testDescription: "negative FLOAT to INT",
		input:           float64(-3.3),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -3},
	},
	{
		testDescription: "large FLOAT to INT",
		input:           float64(math.MaxFloat64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
	},
	{
		testDescription: "large negative FLOAT to INT",
		input:           float64(-math.MaxFloat64),
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: math.MinInt64},
	},
	{
		testDescription: "FLOAT to FLOAT",
		input:           float64(3.3),
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 3.3},
	},
	{
		testDescription: "FLOAT to STRING",
		input:           float64(3.3),
		newType:         STRING,
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 3.3)},
	},
	{
		testDescription: "STRING to NIL",
		input:           "some string",
		newType:         NIL,
		expectedOutput:  Union{},
	},
	{
		testDescription: "non-empty STRING to BOOL",
		input:           "some string",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "\"true\" STRING to BOOL",
		input:           "true",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: true},
	},
	{
		testDescription: "non-empty STRING to false BOOL",
		input:           "false",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "empty STRING to BOOL",
		input:           "",
		newType:         BOOL,
		expectedOutput:  Union{tag: BOOL, b: false},
	},
	{
		testDescription: "int STRING to UINT",
		input:           "5",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 5},
	},
	{
		testDescription: "negative int STRING to UINT",
		input:           "-5",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "float STRING to UINT",
		input:           "5.0",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "random STRING to UINT",
		input:           "some string",
		newType:         UINT,
		expectedOutput:  Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "int STRING to INT",
		input:           "5",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 5},
	},
	{
		testDescription: "negative int STRING to INT",
		input:           "-5",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: -5},
	},
	{
		testDescription: "float STRING to INT",
		input:           "5.0",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	{
		testDescription: "random STRING to INT",
		input:           "some string",
		newType:         INT,
		expectedOutput:  Union{tag: INT, i: 0},
	},
	{
		testDescription: "int STRING to FLOAT",
		input:           "5",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
	},
	{
		testDescription: "float STRING to FLOAT",
		input:           "5.3",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
	},
	{
		testDescription: "negative float STRING to FLOAT",
		input:           "-5.3",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: -5.3},
	},
	{
		testDescription: "random STRING to FLOAT",
		input:           "some string",
		newType:         FLOAT,
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
	},
	{
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
	{
		testDescription: "input = nil",
		expectedOutput:  nil,
		input:           Union{},
	},
	{
		testDescription: "input = true",
		expectedOutput:  true,
		input:           Union{tag: BOOL, b: true},
	},
	{
		testDescription: "input = false",
		expectedOutput:  false,
		input:           Union{tag: BOOL, b: false},
	},
	{
		testDescription: "input = int64(0)",
		expectedOutput:  int64(0),
		input:           Union{tag: INT, i: 0},
	},
	{
		testDescription: "input = uint64(0)",
		expectedOutput:  uint64(0),
		input:           Union{tag: UINT, ui: 0},
	},
	{
		testDescription: "input = int64(5)",
		expectedOutput:  int64(5),
		input:           Union{tag: INT, i: 5},
	},
	{
		testDescription: "input = uint64(5)",
		expectedOutput:  uint64(5),
		input:           Union{tag: UINT, ui: 5},
	},
	{
		testDescription: "input = int64(-5)",
		expectedOutput:  int64(-5),
		input:           Union{tag: INT, i: -5},
	},
	{
		testDescription: "input = MaxUint64",
		expectedOutput:  uint64(math.MaxUint64),
		input:           Union{tag: UINT, ui: math.MaxUint64},
	},
	{
		testDescription: "input = MaxInt64",
		expectedOutput:  int64(math.MaxInt64),
		input:           Union{tag: INT, i: math.MaxInt64},
	},
	{
		testDescription: "input = MaxInt64, stored as uint",
		expectedOutput:  uint64(math.MaxInt64),
		input:           Union{tag: UINT, ui: math.MaxInt64},
	},
	{
		testDescription: "input = MinInt64",
		expectedOutput:  int64(math.MinInt64),
		input:           Union{tag: INT, i: math.MinInt64},
	},
	{
		testDescription: "input = 5.0",
		expectedOutput:  5.0,
		input:           Union{tag: FLOAT, f: 5.0},
	},
	{
		testDescription: "input = float64(5.0)",
		expectedOutput:  float64(5.0),
		input:           Union{tag: FLOAT, f: 5.0},
	},
	{
		testDescription: "input = math.MaxFloat64",
		expectedOutput:  float64(math.MaxFloat64),
		input:           Union{tag: FLOAT, f: math.MaxFloat64},
	},
	{
		testDescription: "input = \"\"",
		expectedOutput:  "",
		input:           Union{tag: STRING, s: ""},
	},
	{
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
	{
		testDescription: "string in slice",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:  true,
	},
	{
		testDescription: "slightly different",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey."},
		expectedOutput:  false,
	},
	{
		testDescription: "not there",
		stringToLookFor: "golden monkey",
		slice:           []string{"I", "wish", "I", "was", "a", "puppy"},
		expectedOutput:  false,
	},
	{
		testDescription: "empty list",
		stringToLookFor: "golden monkey",
		slice:           []string{},
		expectedOutput:  false,
	},
	{
		testDescription: "empty string (not in list)",
		stringToLookFor: "",
		slice:           []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:  false,
	},
	{
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
	{
		testDescription: "error in slice",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkey")},
		expectedOutput:  true,
	},
	{
		testDescription: "slightly different",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkeyy")},
		expectedOutput:  false,
	},
	{
		testDescription: "not there",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("puppy")},
		expectedOutput:  false,
	},
	{
		testDescription: "empty list",
		errorToLookFor:  fmt.Errorf("golden monkey"),
		slice:           []error{},
		expectedOutput:  false,
	},
	{
		testDescription: "empty error (not in list)",
		errorToLookFor:  fmt.Errorf(""),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf("golden monkey")},
		expectedOutput:  false,
	},
	{
		testDescription: "empty error (in list)",
		errorToLookFor:  fmt.Errorf(""),
		slice:           []error{fmt.Errorf("I"), fmt.Errorf("wish"), fmt.Errorf("I"), fmt.Errorf("was"), fmt.Errorf("a"), fmt.Errorf(""), fmt.Errorf("golden monkey")},
		expectedOutput:  true,
	},
}

// why does this not exist in the standard string library yet????
func errorInSlice(s []error, str error) bool {
	stringErr := fmt.Sprintf("%v", str)
	for _, v := range s {
		if fmt.Sprintf("%v", v) == stringErr {
			return true
		}
	}
	return false
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
	{
		testDescription: "empty string",
		input:           "",
		expectedOutput:  "",
	},
	{
		testDescription: "random string",
		input:           "blah",
		expectedOutput:  "/blah",
	},
	{
		testDescription: "/",
		input:           "/",
		expectedOutput:  "/",
	},
	{
		testDescription: "/components",
		input:           "/components",
		expectedOutput:  "/components",
	},
	{
		testDescription: "/components/ess_1",
		input:           "/components/ess_1",
		expectedOutput:  "/components",
	},
	{
		testDescription: "/components/ess_1/vmax",
		input:           "/components/ess_1/vmax",
		expectedOutput:  "/components/ess_1",
	},
	{
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
	{
		testDescription: "empty string",
		input:           "",
		expectedOutput:  "",
	},
	{
		testDescription: "random string",
		input:           "blah",
		expectedOutput:  "blah",
	},
	{
		testDescription: "/",
		input:           "/",
		expectedOutput:  "",
	},
	{
		testDescription: "/components",
		input:           "/components",
		expectedOutput:  "components",
	},
	{
		testDescription: "/components/ess_1",
		input:           "/components/ess_1",
		expectedOutput:  "ess_1",
	},
	{
		testDescription: "/components/ess_1/vmax",
		input:           "/components/ess_1/vmax",
		expectedOutput:  "vmax",
	},
	{
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
	{
		testDescription: "empty list",
		input:           []string{},
		expectedOutput:  []string{},
	},
	{
		testDescription: "no duplicates",
		input:           []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	{
		testDescription: "everything duplicated once, in order",
		input:           []string{"string1", "string1", "string2", "string2", "string3", "string3", "string4", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	{
		testDescription: "everything duplicated once, repeating list twice",
		input:           []string{"string1", "string2", "string3", "string4", "string1", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	{
		testDescription: "three duplicates",
		input:           []string{"string1", "string2", "string2", "string2", "string3", "string4"},
		expectedOutput:  []string{"string1", "string2", "string3", "string4"},
	},
	{
		testDescription: "three duplicates of empty string",
		input:           []string{"string1", "", "", "", "string3", "string4"},
		expectedOutput:  []string{"string1", "", "string3", "string4"},
	},
	{
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
		for i := range test.expectedOutput {
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
	{
		testDescription: "empty list",
		input1:          []string{},
		input2:          []string{},
		expectedOutput:  true,
	},
	{
		testDescription: "no duplicates",
		input1:          []string{"string1", "string2", "string3", "string4"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  true,
	},
	{
		testDescription: "different lengths",
		input1:          []string{"string1", "string1", "string2", "string2", "string3", "string3", "string4", "string4"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  false,
	},
	{
		testDescription: "list contains empty string",
		input1:          []string{"string1", "", "string3", "string4"},
		input2:          []string{"string1", "", "string3", "string4"},
		expectedOutput:  true,
	},
	{
		testDescription: "lists in different order",
		input1:          []string{"string2", "string1", "string4", "string3"},
		input2:          []string{"string1", "string2", "string3", "string4"},
		expectedOutput:  true,
	},
	{
		testDescription: "lists have different items",
		input1:          []string{"string2", "string1", "string4", "string3"},
		input2:          []string{"string1", "oogabooga", "string3", "string4"},
		expectedOutput:  false,
	},
}

func stringListsMatch(list1, list2 []string) bool {
	if len(list1) != len(list2) {
		return false
	}
	for _, str := range list1 {
		if !stringInSlice(list2, str) {
			return false
		}
	}
	return true
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
	{
		testDescription:      "golden* matches golden monkey",
		stringToLookFor:      "golden*",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:       true,
		expectedStringOutput: "golden monkey",
	},
	{
		testDescription:      "*",
		stringToLookFor:      "*",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey."},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	{
		testDescription:      "not there",
		stringToLookFor:      "golden*",
		slice:                []string{"I", "wish", "I", "was", "a", "puppy"},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	{
		testDescription:      "empty list",
		stringToLookFor:      "golden*",
		slice:                []string{},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	{
		testDescription:      "empty list, look for *",
		stringToLookFor:      "*",
		slice:                []string{},
		expectedOutput:       false,
		expectedStringOutput: "",
	},
	{
		testDescription:      "empty string (not in list)",
		stringToLookFor:      "",
		slice:                []string{"I", "wish", "I", "was", "a", "golden monkey"},
		expectedOutput:       true,
		expectedStringOutput: "I",
	},
	{
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
