package go_metrics

import (
	"fmt"
	"math"
	"testing"
	"time"
)

type TestCase struct {
	testDescription    string
	inputs             []Union
	state              map[string][]Union
	expectedOutput     Union
	expectedOutputList []Union
	errorExpected      bool
	wait               int64
}

var AddTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "truehello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 6},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "1hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 6},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "1hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 10.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 10.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 11.6},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: fmt.Sprintf("%f", 5.3) + "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: "hello true"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: "hello 5"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: "hello -5"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: "hello " + fmt.Sprintf("%f", 6.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: "hello world!"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 7},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 7},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-maxfloat),FLOAT(-maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: -math.MaxFloat64}, Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestAdd(t *testing.T) {
	var output Union
	var err error
	for _, test := range AddTests {
		t0 := time.Now()
		output, err = Add(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var SubTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: -4.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: -4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: -4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: -4.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: -4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: -4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: -4.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 4.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: float64(5.3) - float64(uint64(5))},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: float64(5.3) - float64(int64(5))},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: -1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(-maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-maxfloat),FLOAT(maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: -math.MaxFloat64}, Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestSub(t *testing.T) {
	var output Union
	var err error
	for _, test := range SubTests {
		t0 := time.Now()
		output, err = Sub(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var MultTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: (5.3 * 5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: (5.3 * 5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: (5.3 * 6.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(3),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 3}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 15},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestMult(t *testing.T) {
	var output Union
	var err error
	for _, test := range MultTests {
		t0 := time.Now()
		output, err = Mult(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var DivTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: FLOAT, f: float64(uint64(5)) / 1.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),UINT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),INT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),false",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 5.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 5.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(0.1)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 0.1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestDiv(t *testing.T) {
	var output Union
	var err error
	for _, test := range DivTests {
		t0 := time.Now()
		output, err = Div(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var ModTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(0)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(5),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),UINT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 2}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),INT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),false",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestMod(t *testing.T) {
	var output Union
	var err error
	for _, test := range ModTests {
		t0 := time.Now()
		output, err = Mod(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var BitwiseAndTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: (1 & -1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1 & 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(-5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: int64(1) & int64(-5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: int64(-1) & int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 1 & 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(3),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 3}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: int64(3) & int64(5) & int64(1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(MaxInt64)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: math.MinInt64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MaxInt64), UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestBitwiseAnd(t *testing.T) {
	var output Union
	var err error
	for _, test := range BitwiseAndTests {
		t0 := time.Now()
		output, err = BitwiseAnd(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var BitwiseOrTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: int64(1) | int64(-1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1 | 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) | int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) | int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) | int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(3),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 3}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: int64(3) | int64(5) | int64(1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(MaxInt64)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: math.MinInt64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MaxInt64), UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestBitwiseOr(t *testing.T) {
	var output Union
	var err error
	for _, test := range BitwiseOrTests {
		t0 := time.Now()
		output, err = BitwiseOr(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var BitwiseXorTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: int64(1) ^ int64(-1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1 ^ 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) ^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) ^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) ^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(3),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 3}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: int64(3) ^ int64(5) ^ int64(1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(MinInt64)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: math.MinInt64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MaxInt64), UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestBitwiseXor(t *testing.T) {
	var output Union
	var err error
	for _, test := range BitwiseXorTests {
		t0 := time.Now()
		output, err = BitwiseXor(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var LeftShiftTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 64},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(0)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 64},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 32},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 32},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(0),INT(MaxInt64)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: math.MaxInt64}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(63)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 63}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(62)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 62}},
		expectedOutput:  Union{tag: INT, i: 1 << 62},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),INT(62)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: INT, i: 62}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs =INT(MaxInt64-1<<62),INT(1)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64 - (1 << 62)}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: (math.MaxInt64 - (1 << 62)) << 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs =INT(MaxInt64-1<<62),INT(2)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64 - (1 << 62)}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(2),INT(61)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: INT, i: 61}},
		expectedOutput:  Union{tag: INT, i: 2 << 61},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(62)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 62}},
		expectedOutput:  Union{tag: UINT, ui: 2 << 62},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(63)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 63}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(63)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 63}},
		expectedOutput:  Union{tag: UINT, ui: 1 << 63},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(64)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(-1),INT(63)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: INT, i: 63}},
		expectedOutput:  Union{tag: INT, i: -1 << 63},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(-1),INT(64)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: INT, i: 64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MinInt64),INT(1)",
		inputs:          []Union{Union{tag: INT, i: math.MinInt64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MaxInt64),INT(0)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),UINT(0)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: UINT, ui: math.MaxUint64},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(MaxUint64),",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestLeftShift(t *testing.T) {
	var output Union
	var err error
	for _, test := range LeftShiftTests {
		t0 := time.Now()
		output, err = LeftShift(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var RightShiftTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(4),true",
		inputs:          []Union{Union{tag: UINT, ui: 4}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(4),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 4}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(2),UINT(0)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(4),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 4}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(2),true",
		inputs:          []Union{Union{tag: INT, i: 2}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(32),UINT(1)",
		inputs:          []Union{Union{tag: INT, i: 32}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: INT, i: 16},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(4),INT(1)",
		inputs:          []Union{Union{tag: INT, i: 4}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1), UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(0),INT(MaxInt64)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: math.MaxInt64}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
}

func TestRightShift(t *testing.T) {
	var output Union
	var err error
	for _, test := range RightShiftTests {
		t0 := time.Now()
		output, err = RightShift(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var BitwiseAndNotTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: int64(1) &^ int64(-1)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1 &^ 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) &^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) &^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: int64(1) &^ int64(5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(MaxInt64)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: math.MinInt64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(MaxInt64), UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: math.MaxInt64}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestBitwiseAndNot(t *testing.T) {
	var output Union
	var err error
	for _, test := range BitwiseAndNotTests {
		t0 := time.Now()
		output, err = BitwiseAndNot(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var AndTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int(0)",
		inputs:          []Union{Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint64(0)",
		inputs:          []Union{Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float(0)",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(-5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestAnd(t *testing.T) {
	var output Union
	var err error
	for _, test := range AndTests {
		t0 := time.Now()
		output, err = And(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var OrTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int(0)",
		inputs:          []Union{Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint64(0)",
		inputs:          []Union{Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float(0)",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(-5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(0),INT(0),false, float(0)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestOr(t *testing.T) {
	var output Union
	var err error
	for _, test := range OrTests {
		t0 := time.Now()
		output, err = Or(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var NotTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 0",
		inputs:             []Union{},
		expectedOutputList: []Union{},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (true)",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (false)",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int",
		inputs:             []Union{Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int(0)",
		inputs:             []Union{Union{tag: INT, i: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint",
		inputs:             []Union{Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint64(0)",
		inputs:             []Union{Union{tag: UINT, ui: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float(0)",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: ""}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,true",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,false",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,INT(0)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),true",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(0),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 0}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),INT(-5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),true",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(0),true",
		inputs:             []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:             []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(0),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 0}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(0),STRING(\"\")",
		inputs:             []Union{Union{tag: INT, i: 0}, Union{tag: STRING, s: ""}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(0.0),false",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(0),FLOAT(6.3)",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: FLOAT, f: 6.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(0),FLOAT(0)",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: FLOAT, f: 0.0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",true",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = UINT(1),INT(0),true",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = UINT(0),INT(0),true",
		inputs:             []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 4, inputs = UINT(0),INT(0),false, float(0)",
		inputs:             []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
}

func TestNot(t *testing.T) {
	var output []Union
	var err error
	for _, test := range NotTests {
		t0 := time.Now()
		output, err = Not(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := 0; i < len(test.expectedOutputList); i += 1 {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output[i], test.expectedOutputList[i])
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var EqualTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(0),INT(0),false, float(0)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
}

func TestEqual(t *testing.T) {
	var output Union
	var err error
	for _, test := range EqualTests {
		t0 := time.Now()
		output, err = Equal(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var NotEqualTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(0),INT(0),false, float(0)",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestNotEqual(t *testing.T) {
	var output Union
	var err error
	for _, test := range NotEqualTests {
		t0 := time.Now()
		output, err = NotEqual(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var LessThanTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(10),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 10}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-1.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: -1.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 6}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 6}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(6.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 6.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hellox\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hellox"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(0),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = INT(-1),UINT(0),true",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(5),INT(6),float(7.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 7.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(7),INT(6),float(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 7}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestLessThan(t *testing.T) {
	var output Union
	var err error
	for _, test := range LessThanTests {
		t0 := time.Now()
		output, err = LessThan(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var LessThanOrEqualTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(10),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 10}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-1.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: -1.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 6}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 6}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(6.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 6.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hellox\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hellox"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(0),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = INT(-1),UINT(0),true",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(5),INT(6),float(7.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 7.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(7),INT(6),float(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 7}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestLessThanOrEqual(t *testing.T) {
	var output Union
	var err error
	for _, test := range LessThanOrEqualTests {
		t0 := time.Now()
		output, err = LessThanOrEqual(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var GreaterThanTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(10),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 10}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-1.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: -1.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 6}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 6}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(6.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 6.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hellox\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hellox"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(0),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = INT(-1),UINT(0),true",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(5),INT(6),float(7.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 7.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(7),INT(6),float(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 7}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
}

func TestGreaterThan(t *testing.T) {
	var output Union
	var err error
	for _, test := range GreaterThanTests {
		t0 := time.Now()
		output, err = GreaterThan(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var GreaterThanOrEqualTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(0.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: ""}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(0),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(1)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(10),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 10}, Union{tag: FLOAT, f: 5.0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-1.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: -1.0}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 6}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(6)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 6}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(6.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 6.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"true\",true",
		inputs:          []Union{Union{tag: STRING, s: "true"}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1\",1",
		inputs:          []Union{Union{tag: STRING, s: "1"}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"1.0\",1.0",
		inputs:          []Union{Union{tag: STRING, s: fmt.Sprintf("%v", 1.0)}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hellox\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hellox"}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hellox\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hellox"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(0),INT(0),true",
		inputs:          []Union{Union{tag: UINT, ui: 0}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = INT(-1),UINT(0),true",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 0}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(5),INT(6),float(7.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 7.3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, inputs = UINT(7),INT(6),float(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 7}, Union{tag: INT, i: 6}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
}

func TestGreaterThanOrEqual(t *testing.T) {
	var output Union
	var err error
	for _, test := range GreaterThanOrEqualTests {
		t0 := time.Now()
		output, err = GreaterThanOrEqual(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var MaxTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(4.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 4.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(4.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 4.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(5),INT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(5),INT(1),true, float(-10)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: -10.0}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
}

var RootTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 1.0/5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(25),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 25}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(25, 0.5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(25),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 25}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(25, 0.5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 1.0/5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 25}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(25, 0.5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 25}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(25, 0.5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 1/5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 1/5.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 1/5.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 1/6.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(MaxUint64)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(minNonZeroFloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: math.SmallestNonzeroFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(smallestfloat),FLOAT(-maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.SmallestNonzeroFloat64}, Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
}

func TestRoot(t *testing.T) {
	var output Union
	var err error
	for _, test := range RootTests {
		t0 := time.Now()
		output, err = Root(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var PowTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(25),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 25}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: UINT, ui: 625},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(25),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 25}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 625},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(25),UINT(2)",
		inputs:          []Union{Union{tag: INT, i: 25}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: INT, i: 625},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(25),INT(2)",
		inputs:          []Union{Union{tag: INT, i: 25}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 625},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(1, 5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 5.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 5.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Pow(5.3, 6.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(MaxUint64), INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: math.MaxInt64},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(maxFloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestPow(t *testing.T) {
	var output Union
	var err error
	for _, test := range PowTests {
		t0 := time.Now()
		output, err = Pow(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

func TestMax(t *testing.T) {
	var output Union
	var err error
	for _, test := range MaxTests {
		t0 := time.Now()
		output, err = Max(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var MinTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(0.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 0.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),false",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 0.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(4.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 4.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 4.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(4.3),INT(3)",
		inputs:          []Union{Union{tag: FLOAT, f: 4.3}, Union{tag: INT, i: 3}},
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(2.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 2.3}},
		expectedOutput:  Union{tag: FLOAT, f: 2.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(5),INT(3),true",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(5),INT(1),true, float(-10)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: -10.0}},
		expectedOutput:  Union{tag: FLOAT, f: -10.0},
		errorExpected:   false,
	},
}

func TestMin(t *testing.T) {
	var output Union
	var err error
	for _, test := range MinTests {
		t0 := time.Now()
		output, err = Min(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var AvgTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: INT, i: -5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 6.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 10.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 10.3 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 11.6 / 2.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{tag: STRING, s: ""},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 7 / 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 7 / 3},
		errorExpected:   false,
	},
}

func TestAvg(t *testing.T) {
	var output Union
	var err error
	for _, test := range AvgTests {
		t0 := time.Now()
		output, err = Avg(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var FloorTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 0",
		inputs:             []Union{},
		expectedOutputList: []Union{},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (true)",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (false)",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int",
		inputs:             []Union{Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: INT, i: -5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint",
		inputs:             []Union{Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,true",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,false",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,FLOAT(5.7)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),true",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),FLOAT(5.7)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),true",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),FLOAT(5.7)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),INT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),FLOAT(6.7)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: FLOAT, f: 6.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",true",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",FLOAT(6.7)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.7}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = FLOAT(6.7),INT(5),true",
		inputs:             []Union{Union{tag: FLOAT, f: 6.7}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
}

func TestFloor(t *testing.T) {
	var output []Union
	var err error
	for _, test := range FloorTests {
		t0 := time.Now()
		output, err = Floor(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var CeilTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 0",
		inputs:             []Union{},
		expectedOutputList: []Union{},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (true)",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (false)",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int",
		inputs:             []Union{Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: INT, i: -5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint",
		inputs:             []Union{Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,true",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,false",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),true",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),true",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.7)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",true",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",FLOAT(5.7)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = FLOAT(5.7),INT(5),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
}

func TestCeil(t *testing.T) {
	var output []Union
	var err error
	for _, test := range CeilTests {
		t0 := time.Now()
		output, err = Ceil(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var SqrtTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 0",
		inputs:             []Union{},
		expectedOutputList: []Union{},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (true)",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (false)",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int",
		inputs:             []Union{Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint",
		inputs:             []Union{Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.7)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,true",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1}, Union{tag: FLOAT, f: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,false",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 0}, Union{tag: FLOAT, f: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(5)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 0}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,INT(5)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,FLOAT(5.7)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(1)}, Union{tag: FLOAT, f: math.Sqrt(5.7)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(1)}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(5),true",
		inputs:             []Union{Union{tag: UINT, ui: 5}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5)}, Union{tag: FLOAT, f: math.Sqrt(1)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(6),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 6}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(6),INT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 6}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(6),FLOAT(5.7)",
		inputs:             []Union{Union{tag: UINT, ui: 6}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(5.7)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(6),STRING(\"hello\")",
		inputs:             []Union{Union{tag: UINT, ui: 6}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(6),true",
		inputs:             []Union{Union{tag: INT, i: 6}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(1)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(6),UINT(5)",
		inputs:             []Union{Union{tag: INT, i: 6}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:             []Union{Union{tag: INT, i: 6}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),FLOAT(5.7)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(1)}, Union{tag: FLOAT, f: math.Sqrt(5.7)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.7)}, Union{tag: FLOAT, f: math.Sqrt(1)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),UINT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.7)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),INT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.7)}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.7),FLOAT(6.7)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: FLOAT, f: 6.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.7)}, Union{tag: FLOAT, f: math.Sqrt(6.7)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(5.3)}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",true",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 1}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",INT(5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: math.Sqrt(5)}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",FLOAT(6.7)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.7}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: math.Sqrt(6.7)}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = FLOAT(6.7),INT(5),true",
		inputs:             []Union{Union{tag: FLOAT, f: 6.7}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.Sqrt(6.7)}, Union{tag: FLOAT, f: math.Sqrt(5)}, Union{tag: FLOAT, f: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(-5)",
		inputs:             []Union{Union{tag: FLOAT, f: -5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: -5}},
		errorExpected:      true,
	},
}

func TestSqrt(t *testing.T) {
	var output []Union
	var err error
	for _, test := range SqrtTests {
		t0 := time.Now()
		output, err = Sqrt(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range test.expectedOutputList {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output[i], test.expectedOutputList[i])
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var PctTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: -100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 20},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: UINT, ui: 250},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 20},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 250},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: FLOAT, f: float64(uint64(5)) / 1.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 20},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),UINT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: INT, i: 250},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 20},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),INT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 250},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0 / 5.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),false",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 5.0 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 5.0 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3 / 6.3 * 100},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: FLOAT, f: math.MaxUint64 * 100.0},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(0.1)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 0.1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(1)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestPct(t *testing.T) {
	var output Union
	var err error
	for _, test := range PctTests {
		t0 := time.Now()
		output, err = Pct(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var FloorDivTests = []TestCase{
	TestCase{
		testDescription: "inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),UINT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),INT(2)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(5),FLOAT(1.3)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.3}},
		expectedOutput:  Union{tag: FLOAT, f: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),false",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),UINT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: INT, i: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(5),INT(2)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 2}},
		expectedOutput:  Union{tag: INT, i: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),false",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(6.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 6.3}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "inputs = UINT(MaxUint64),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(0.1)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 0.1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestFloorDiv(t *testing.T) {
	var output Union
	var err error
	for _, test := range FloorDivTests {
		t0 := time.Now()
		output, err = FloorDiv(test.inputs[0], test.inputs[1])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var AbsTests = []TestCase{
	TestCase{
		testDescription:    "empty union",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "large UINT union",
		inputs:             []Union{Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutputList: []Union{Union{tag: UINT, ui: math.MaxUint64}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union",
		inputs:             []Union{Union{tag: INT, i: -3}},
		expectedOutputList: []Union{Union{tag: INT, i: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "MinInt64 INT union",
		inputs:             []Union{Union{tag: INT, i: math.MinInt64}},
		expectedOutputList: []Union{Union{}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "FLOAT union -3.3",
		inputs:             []Union{Union{tag: FLOAT, f: -3.3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 3.3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "STRING",
		inputs:             []Union{Union{tag: STRING, s: "5"}},
		expectedOutputList: []Union{Union{}},
		errorExpected:      true,
	},
}

func TestAbs(t *testing.T) {
	var output []Union
	var err error
	for _, test := range AbsTests {
		t0 := time.Now()
		output, err = Abs(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var RoundTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 0",
		inputs:             []Union{},
		expectedOutputList: []Union{},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (true)",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = bool (false)",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = int",
		inputs:             []Union{Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: INT, i: -5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = uint",
		inputs:             []Union{Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = float",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 1, input = string",
		inputs:             []Union{Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,true",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,false",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:             []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),true",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),true",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:             []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.7)",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:             []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: STRING, s: "hello"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",true",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello\",FLOAT(5.7)",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 5.7}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.0}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:             []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, inputs = FLOAT(5.7),INT(5),true",
		inputs:             []Union{Union{tag: FLOAT, f: 5.7}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 6.0}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
}

func TestRound(t *testing.T) {
	var output []Union
	var err error
	for _, test := range RoundTests {
		t0 := time.Now()
		output, err = Round(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var BoolTests = []TestCase{
	TestCase{
		testDescription:    "empty union to BOOL",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union to BOOL",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union to BOOL",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to BOOL",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to false BOOL",
		inputs:             []Union{Union{tag: UINT, ui: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union to BOOL",
		inputs:             []Union{Union{tag: INT, i: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative INT union to BOOL",
		inputs:             []Union{Union{tag: INT, i: -3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union to false BOOL",
		inputs:             []Union{Union{tag: INT, i: 0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to true BOOL",
		inputs:             []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to false BOOL",
		inputs:             []Union{Union{tag: FLOAT, f: 0.0}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative FLOAT union to true BOOL",
		inputs:             []Union{Union{tag: FLOAT, f: -3.3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "non-empty STRING union to BOOL",
		inputs:             []Union{Union{tag: STRING, s: "some string"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "\"true\" STRING union to BOOL",
		inputs:             []Union{Union{tag: STRING, s: "true"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "non-empty STRING union to false BOOL",
		inputs:             []Union{Union{tag: STRING, s: "false"}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "empty STRING union to BOOL",
		inputs:             []Union{Union{tag: STRING, s: ""}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
}

func TestBool(t *testing.T) {
	var output []Union
	var err error
	for _, test := range BoolTests {
		t0 := time.Now()
		output, err = Bool(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var IntTests = []TestCase{
	TestCase{
		testDescription:    "empty union to INT",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{tag: INT, i: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union to INT",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: INT, i: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union to INT",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: INT, i: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to INT",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: INT, i: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "large UINT union to INT",
		inputs:             []Union{Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutputList: []Union{Union{tag: UINT, ui: math.MaxUint64}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "INT union to INT",
		inputs:             []Union{Union{tag: INT, i: 3}},
		expectedOutputList: []Union{Union{tag: INT, i: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to INT, 3.3",
		inputs:             []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutputList: []Union{Union{tag: INT, i: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to INT, 3.6",
		inputs:             []Union{Union{tag: FLOAT, f: 3.6}},
		expectedOutputList: []Union{Union{tag: INT, i: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative FLOAT union to INT",
		inputs:             []Union{Union{tag: FLOAT, f: -3.3}},
		expectedOutputList: []Union{Union{tag: INT, i: -3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "large FLOAT union to INT",
		inputs:             []Union{Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.MaxFloat64}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "large negative FLOAT union to INT",
		inputs:             []Union{Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: -math.MaxFloat64}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "int STRING union to INT",
		inputs:             []Union{Union{tag: STRING, s: "5"}},
		expectedOutputList: []Union{Union{tag: INT, i: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative int STRING union to INT",
		inputs:             []Union{Union{tag: STRING, s: "-5"}},
		expectedOutputList: []Union{Union{tag: INT, i: -5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "float STRING union to INT",
		inputs:             []Union{Union{tag: STRING, s: "5.0"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "5.0"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "random STRING union to INT",
		inputs:             []Union{Union{tag: STRING, s: "some string"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "some string"}},
		errorExpected:      true,
	},
}

func TestInt(t *testing.T) {
	var output []Union
	var err error
	for _, test := range IntTests {
		t0 := time.Now()
		output, err = Int(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var UIntTests = []TestCase{
	TestCase{
		testDescription:    "empty union to UINT",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union to UINT",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union to UINT",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 1}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to UINT",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union to UINT",
		inputs:             []Union{Union{tag: INT, i: 3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative INT union to UINT",
		inputs:             []Union{Union{tag: INT, i: -3}},
		expectedOutputList: []Union{Union{tag: INT, i: -3}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "max INT union to UINT",
		inputs:             []Union{Union{tag: INT, i: math.MaxInt64}},
		expectedOutputList: []Union{Union{tag: UINT, ui: math.MaxInt64}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to UINT, 3.3",
		inputs:             []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to UINT, 3.6",
		inputs:             []Union{Union{tag: FLOAT, f: 3.6}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative FLOAT union to UINT",
		inputs:             []Union{Union{tag: FLOAT, f: -3.3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: -3.3}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "large FLOAT union to UINT",
		inputs:             []Union{Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: math.MaxFloat64}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "large negative FLOAT union to UINT",
		inputs:             []Union{Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: -math.MaxFloat64}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "int STRING union to UINT",
		inputs:             []Union{Union{tag: STRING, s: "5"}},
		expectedOutputList: []Union{Union{tag: UINT, ui: 5}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative int STRING union to UINT",
		inputs:             []Union{Union{tag: STRING, s: "-5"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "-5"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "float STRING union to UINT",
		inputs:             []Union{Union{tag: STRING, s: "5.0"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "5.0"}},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "random STRING union to UINT",
		inputs:             []Union{Union{tag: STRING, s: "some string"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "some string"}},
		errorExpected:      true,
	},
}

func TestUInt(t *testing.T) {
	var output []Union
	var err error
	for _, test := range UIntTests {
		t0 := time.Now()
		output, err = UInt(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var FloatTests = []TestCase{
	TestCase{
		testDescription:    "empty union to FLOAT",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 0.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union to FLOAT",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 0.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union to FLOAT",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 1.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to FLOAT",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 3.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "large UINT union to FLOAT",
		inputs:             []Union{Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: float64(math.MaxUint64)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union to FLOAT",
		inputs:             []Union{Union{tag: INT, i: 3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 3.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "max INT union to FLOAT",
		inputs:             []Union{Union{tag: INT, i: math.MaxInt64}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: float64(math.MaxInt64)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to FLOAT",
		inputs:             []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 3.3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "int STRING union to FLOAT",
		inputs:             []Union{Union{tag: STRING, s: "5"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.0}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "float STRING union to FLOAT",
		inputs:             []Union{Union{tag: STRING, s: "5.3"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: 5.3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "negative float STRING union to FLOAT",
		inputs:             []Union{Union{tag: STRING, s: "-5.3"}},
		expectedOutputList: []Union{Union{tag: FLOAT, f: -5.3}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "random STRING union to FLOAT",
		inputs:             []Union{Union{tag: STRING, s: "some string"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "some string"}},
		errorExpected:      true,
	},
}

func TestFloat(t *testing.T) {
	var output []Union
	var err error
	for _, test := range FloatTests {
		t0 := time.Now()
		output, err = Float(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var StringTests = []TestCase{
	TestCase{
		testDescription:    "empty union to STRING",
		inputs:             []Union{Union{}},
		expectedOutputList: []Union{Union{tag: STRING, s: ""}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "false bool union to STRING",
		inputs:             []Union{Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: STRING, s: "false"}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "true bool union to STRING",
		inputs:             []Union{Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: STRING, s: "true"}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "UINT union to STRING",
		inputs:             []Union{Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: STRING, s: "3"}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "INT union to STRING",
		inputs:             []Union{Union{tag: INT, i: 3}},
		expectedOutputList: []Union{Union{tag: STRING, s: "3"}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "FLOAT union to STRING",
		inputs:             []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutputList: []Union{Union{tag: STRING, s: fmt.Sprintf("%f", 3.3)}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "STRING union to STRING",
		inputs:             []Union{Union{tag: STRING, s: "some string"}},
		expectedOutputList: []Union{Union{tag: STRING, s: "some string"}},
		errorExpected:      false,
	},
}

func TestString(t *testing.T) {
	var output []Union
	var err error
	for _, test := range StringTests {
		t0 := time.Now()
		output, err = String(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

// Not sure how to test Integrate
// These are the tests that don't depend on time
var IntegrateTests = []TestCase{
	TestCase{
		testDescription: "large UINT union",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: FLOAT, f: 1.0}, Union{tag: UINT, ui: math.MaxUint64}, Union{tag: INT, i: 0}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "STRING union",
		inputs:          []Union{Union{tag: STRING, s: "blah"}, Union{tag: FLOAT, f: 1.0}, Union{tag: INT, i: -1}, Union{tag: INT, i: 0}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

// There is probably a better way to test MillisecondsSince...
func TestIntegrate(t *testing.T) {
	var output Union
	var err error
	for _, test := range IntegrateTests {
		t0 := time.Now()
		output, err = Integrate(test.inputs[0], test.inputs[1], test.inputs[2], test.inputs[3], &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}

		if output.tag != test.expectedOutput.tag {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	state := make(map[string][]Union, 0)
	output, _ = Integrate(Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 1.0 / (3600 * 1000)}, Union{tag: INT, i: -1}, Union{tag: INT, i: 0}, &state)
	state["value"] = []Union{output}
	time.Sleep(100 * time.Millisecond)
	output, err = Integrate(Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 1.0 / (3600 * 1000)}, Union{tag: INT, i: -1}, Union{tag: INT, i: 0}, &state)
	if output.tag != INT {
		t.Errorf("%s: output %v not equal to expected %v\n", "Integrate", output, Union{tag: INT, i: 100})
	}
	if output.i > 101 {
		t.Errorf("Integrate was off by more than 1")
	}
	if err != nil {
		t.Errorf("%s: got an err when there should not have been an error\n", "Integrate")
	}
}

// There is possibly a better way to test CurrentTimeMilliseconds...
func TestCurrentTimeMilliseconds(t *testing.T) {
	var output Union
	var err error
	t0 := time.Now()
	output, err = CurrentTimeMilliseconds()
	if output.i-t0.UnixMilli() > 1 {
		t.Errorf("CurrentTimeMilliseconds took longer than 1 millisecond to return a result\n")
	}
	if err != nil {
		t.Errorf("CurrentTimeMilliseconds returned an error when it should not have\n")
	}
}

// These are the tests that don't depend on time
var MillisecondsSinceTests = []TestCase{
	TestCase{
		testDescription: "large UINT union",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "large negative FLOAT union to INT",
		inputs:          []Union{Union{tag: FLOAT, f: float64(math.MinInt64 + int64(100000))}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "float STRING union to INT",
		inputs:          []Union{Union{tag: STRING, s: "5.0"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "random STRING union to INT",
		inputs:          []Union{Union{tag: STRING, s: "some string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

// There is probably a better way to test MillisecondsSince...
func TestMillisecondsSince(t *testing.T) {
	var output Union
	var err error
	for _, test := range MillisecondsSinceTests {
		t0 := time.Now()
		output, err = MillisecondsSince(test.inputs[0])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}

		if output.tag != test.expectedOutput.tag {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	t0 := time.Now()
	time.Sleep(100 * time.Millisecond)
	output, err = MillisecondsSince(Union{tag: INT, i: t0.UnixMilli()})
	if output.tag != INT {
		t.Errorf("%s: output %v not equal to expected %v\n", "MillisecondsSince", output, Union{tag: INT, i: 100})
	}
	if output.i > 101 {
		t.Errorf("MillisecondsSince was off by more than 1 millisecond")
	}
	if err != nil {
		t.Errorf("%s: got an err when there should not have been an error\n", "MillisecondsSince")
	}
}

var MillisecondsToRFC3339Tests = []TestCase{
	TestCase{
		testDescription: "empty union",
		inputs:          []Union{Union{}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(0).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "false bool union",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(0).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "true bool union",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(1).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "UINT union",
		inputs:          []Union{Union{tag: UINT, ui: uint64(time.Now().UnixMilli())}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(time.Now().UnixMilli()).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "large UINT union",
		inputs:          []Union{Union{tag: UINT, ui: math.MaxUint64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "INT union",
		inputs:          []Union{Union{tag: INT, i: time.Now().UnixMilli()}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(time.Now().UnixMilli()).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "INT union",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(-5).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "FLOAT union to INT",
		inputs:          []Union{Union{tag: FLOAT, f: float64(time.Now().UnixMilli())}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(time.Now().UnixMilli()).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "negative FLOAT union to INT",
		inputs:          []Union{Union{tag: FLOAT, f: -5.5}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(-5).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "int STRING union to INT",
		inputs:          []Union{Union{tag: STRING, s: "5"}},
		expectedOutput:  Union{tag: STRING, s: time.UnixMilli(5).Format(time.RFC3339)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "random STRING union to INT",
		inputs:          []Union{Union{tag: STRING, s: "some string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestMillisecondsToRFC4449(t *testing.T) {
	var output Union
	var err error
	for _, test := range MillisecondsToRFC3339Tests {
		t0 := time.Now()
		output, err = MillisecondsToRFC3339(test.inputs[0])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}

		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}

		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var SrffTests = []TestCase{
	TestCase{
		testDescription: "empty unions, state true",
		inputs:          []Union{Union{}, Union{}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: true}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "empty unions, state false",
		inputs:          []Union{Union{}, Union{}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: false}}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "both inputs false, state true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: true}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "input 1 true, state true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: true}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "input 1 true, state false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: false}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "input 1 false, input 2 true, state true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: true}}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "input 1 false, input 2 true, state false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: false}}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "both inputs true, state true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: true}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "both inputs false, state false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		state:           map[string][]Union{"q": []Union{Union{tag: BOOL, b: false}}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "state nil",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		state:           nil,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "string input",
		inputs:          []Union{Union{tag: STRING, s: "some string"}, Union{tag: BOOL, b: false}},
		state:           nil,
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	// additional test case below for state pointer nil
}

func TestSrff(t *testing.T) {
	var output Union
	var err error
	for _, test := range SrffTests {
		t0 := time.Now()
		output, err = Srff(test.inputs[0], test.inputs[1], &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = Srff(SrffTests[1].inputs[0], SrffTests[1].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var RssTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: 5.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: FLOAT, f: 5.4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(2.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: FLOAT, f: 0.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(2.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: FLOAT, f: 1.0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(2.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(1 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(2.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(26)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(26)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(1 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(2.0)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(26)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(26)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(1 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),true",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(1 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(25 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(25 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(6.3*6.3 + 5.3*5.3)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",true",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",UINT(5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",INT(-5)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: INT, i: -5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",FLOAT(6.3)",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: FLOAT, f: 6.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello \",\"world!\"",
		inputs:          []Union{Union{tag: STRING, s: "hello "}, Union{tag: STRING, s: "world!"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(27)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,true,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{tag: FLOAT, f: math.Sqrt(27)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(SQRT(maxfloat)),FLOAT(SQRT(maxfloat))",
		inputs:          []Union{Union{tag: FLOAT, f: math.Sqrt(math.MaxFloat64)}, Union{tag: FLOAT, f: math.Sqrt(math.MaxFloat64)}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(maxfloat),FLOAT(maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(-maxfloat),FLOAT(-maxfloat)",
		inputs:          []Union{Union{tag: FLOAT, f: -math.MaxFloat64}, Union{tag: FLOAT, f: -math.MaxFloat64}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestRss(t *testing.T) {
	var output Union
	var err error
	for _, test := range RssTests {
		t0 := time.Now()
		output, err = Rss(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var SelectNTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = false,true,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,false,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = false,false,true",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(5)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = 1,UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = 2,UINT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,INT(5)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 5}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = false,INT(5),INT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,INT(5),INT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 5}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: INT, i: 5},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,FLOAT(5.3),FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{tag: FLOAT, f: 5.3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = false,FLOAT(5.3),FLOAT(1.0)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 1.0}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,STRING(\"hello\"),STRING(\"world\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "world"}},
		expectedOutput:  Union{tag: STRING, s: "hello"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,STRING(\"hello\"),STRING(\"world\")",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "world"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = MaxFloat64,FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestSelectN(t *testing.T) {
	var output Union
	var err error
	for _, test := range SelectNTests {
		t0 := time.Now()
		output, err = SelectN(test.inputs[0], test.inputs[1:]...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var EnumTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = false,true,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: STRING, s: "Unknown"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = true,true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = 5,5,'some string'",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}, Union{tag: STRING, s: "some string"}},
		expectedOutput:  Union{tag: STRING, s: "some string"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 9, inputs = 5,5,'some string'",
		inputs:          []Union{Union{tag: INT, i: 3}, Union{tag: INT, i: 1}, Union{tag: STRING, s: "some string1"}, Union{tag: INT, i: 2}, Union{tag: STRING, s: "some string2"}, Union{tag: INT, i: 3}, Union{tag: STRING, s: "some string3"}, Union{tag: INT, i: 4}, Union{tag: STRING, s: "some string4"}},
		expectedOutput:  Union{tag: STRING, s: "some string3"},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = MaxFloat64,FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestEnum(t *testing.T) {
	var output Union
	var err error
	for _, test := range EnumTests {
		t0 := time.Now()
		output, err = Enum(test.inputs[0], test.inputs[1:]...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var SelectorNTests = []TestCase{
	TestCase{
		testDescription: "no input",
		inputs:          []Union{},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "empty union to BOOL",
		inputs:          []Union{Union{}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "false bool union to BOOL",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "true bool union to BOOL",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "UINT union to BOOL",
		inputs:          []Union{Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "UINT union to false BOOL",
		inputs:          []Union{Union{tag: UINT, ui: 0}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "INT union to BOOL",
		inputs:          []Union{Union{tag: INT, i: 3}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "negative INT union to BOOL",
		inputs:          []Union{Union{tag: INT, i: -3}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "INT union to false BOOL",
		inputs:          []Union{Union{tag: INT, i: 0}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "FLOAT union to true BOOL",
		inputs:          []Union{Union{tag: FLOAT, f: 3.3}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "FLOAT union to false BOOL",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "negative FLOAT union to true BOOL",
		inputs:          []Union{Union{tag: FLOAT, f: -3.3}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "non-empty STRING union to BOOL",
		inputs:          []Union{Union{tag: STRING, s: "some string"}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "\"true\" STRING union to BOOL",
		inputs:          []Union{Union{tag: STRING, s: "true"}},
		expectedOutput:  Union{tag: INT, i: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "non-empty STRING union to false BOOL",
		inputs:          []Union{Union{tag: STRING, s: "false"}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "empty STRING union to BOOL",
		inputs:          []Union{Union{tag: STRING, s: ""}},
		expectedOutput:  Union{tag: INT, i: -1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "empty STRING union to BOOL",
		inputs:          []Union{Union{tag: STRING, s: ""}, Union{tag: INT, i: 0}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: -3.3}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: INT, i: 4},
		errorExpected:   false,
	},
}

func TestSelectorN(t *testing.T) {
	var output Union
	var err error
	for _, test := range SelectorNTests {
		t0 := time.Now()
		output, err = SelectorN(test.inputs...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var PulseTests = []TestCase{
	TestCase{
		testDescription: "trigger true, wait, trigger true (after timeout)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}},
		state:           nil,
		wait:            1100,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "trigger true, wait, trigger false (after timeout)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}},
		state:           nil,
		wait:            1500,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "trigger true, wait, trigger false (before timeout)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}},
		state:           nil,
		wait:            800,
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "trigger true & reset true, wait, trigger true & reset true (before timeout)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 1000}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 1000}},
		state:           nil,
		wait:            800,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "trigger true & reset false, wait, trigger false & reset true (before timeout)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 1000}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 1000}},
		state:           nil,
		wait:            800,
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "large float timeout",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}, Union{tag: FLOAT, f: math.MaxFloat64}, Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: math.MaxFloat64}},
		state:           nil,
		wait:            100,
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	// additional test case below for state pointer nil
}

func TestPulse(t *testing.T) {
	var output Union
	var err error
	for _, test := range PulseTests {
		t0 := time.Now()
		Pulse(test.inputs[0], test.inputs[1], test.inputs[2], &(test.state))
		time.Sleep(time.Duration(test.wait) * time.Millisecond)
		output, err = Pulse(test.inputs[3], test.inputs[4], test.inputs[5], &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = Pulse(PulseTests[0].inputs[0], PulseTests[0].inputs[1], PulseTests[0].inputs[2], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var CompareTests = []TestCase{
	TestCase{
		testDescription:    "len(inputs) = 3, operator not string",
		inputs:             []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, ==, true, true",
		inputs:             []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, ==, true, false",
		inputs:             []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, ==, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, ==, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, !=, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, !=, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <, 3, 2",
		inputs:             []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >, 3, 2",
		inputs:             []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <=, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <=, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <=, 3, 2",
		inputs:             []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >=, 3, 5",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >=, 3, 3",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutputList: []Union{Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >=, 3, 2",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) = 4, >=, 3, 2,5",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutputList: []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: true}},
		errorExpected:      false,
	},
	TestCase{
		testDescription:    "len(inputs) =3, ~, 3, 2",
		inputs:             []Union{Union{tag: STRING, s: "~"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <, 3, 'string'",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) =3, >, 3, 'string'",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, <=, 3, 'string'",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
	TestCase{
		testDescription:    "len(inputs) = 3, >=, 3, 'string'",
		inputs:             []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutputList: []Union{},
		errorExpected:      true,
	},
}

func TestCompare(t *testing.T) {
	var output []Union
	var err error
	for _, test := range CompareTests {
		t0 := time.Now()
		output, err = Compare(test.inputs[0], test.inputs[1], test.inputs[2:]...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if len(output) != len(test.expectedOutputList) {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
		} else {
			for i := range output {
				if output[i] != test.expectedOutputList[i] {
					t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutputList)
				}
			}
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var CompareOrTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 3, operator not string",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, true, true",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, true, false",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, !=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, !=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, >=, 3, 2,5",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) =3, ~, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "~"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) =3, >, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestCompareOr(t *testing.T) {
	var output Union
	var err error
	for _, test := range CompareOrTests {
		t0 := time.Now()
		output, err = CompareOr(test.inputs[0], test.inputs[1], test.inputs[2:]...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var CompareAndTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 3, operator not string",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, true, true",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, true, false",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ==, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "=="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, !=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, !=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "!="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 5",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 3",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 3}},
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, >=, 3, 2, 5",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, >, 3, 2, 5",
		inputs:          []Union{Union{tag: STRING, s: ">"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, <, 3, 2,5",
		inputs:          []Union{Union{tag: STRING, s: "<"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 4, <=, 3, 2,5",
		inputs:          []Union{Union{tag: STRING, s: "<="}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}, Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, ~, 3, 2",
		inputs:          []Union{Union{tag: STRING, s: "~"}, Union{tag: UINT, ui: 3}, Union{tag: UINT, ui: 2}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, <=, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
	TestCase{
		testDescription: "len(inputs) = 3, >=, 3, 'string'",
		inputs:          []Union{Union{tag: STRING, s: ">="}, Union{tag: UINT, ui: 3}, Union{tag: STRING, s: "string"}},
		expectedOutput:  Union{},
		errorExpected:   true,
	},
}

func TestCompareAnd(t *testing.T) {
	var output Union
	var err error
	for _, test := range CompareAndTests {
		t0 := time.Now()
		output, err = CompareAnd(test.inputs[0], test.inputs[1], test.inputs[2:]...)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var MaxOverTimescaleTestCases = []TestCase{
	TestCase{
		testDescription: "NIL union",
		inputs:          []Union{Union{tag: NIL, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "STRING union",
		inputs:          []Union{Union{tag: STRING, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "all false unions",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one true union",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one true union with timeout",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "max with uints",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 101}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 101},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "max with uints with timeout",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 101}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "max with ints",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 310}, Union{tag: INT, i: 101}, Union{tag: INT, i: 310}, Union{tag: INT, i: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 101},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "max with ints with timeout",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 101}, Union{tag: INT, i: 100}, Union{tag: INT, i: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "max with floats",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 101}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 101},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "max with floats with timeout",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 101}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
}

func TestMaxOverTimescale(t *testing.T) {
	var output Union
	var err error
	for _, test := range MaxOverTimescaleTestCases {
		t0 := time.Now()
		for i := 0; i < len(test.inputs); i += 2 {
			output, err = MaxOverTimescale(test.inputs[i], test.inputs[i+1], &(test.state))
			time.Sleep(time.Duration(test.wait) * time.Millisecond)
		}
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = MaxOverTimescale(MaxOverTimescaleTestCases[0].inputs[0], MaxOverTimescaleTestCases[0].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var MinOverTimescaleTestCases = []TestCase{
	TestCase{
		testDescription: "NIL union",
		inputs:          []Union{Union{tag: NIL, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "STRING union",
		inputs:          []Union{Union{tag: STRING, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "all false unions",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one false union",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one false union with timeout",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "min with uints",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 30},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "min with uints with timeout",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "min with ints",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 310}, Union{tag: INT, i: 30}, Union{tag: INT, i: 310}, Union{tag: INT, i: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 30},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "min with ints with timeout",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 30}, Union{tag: INT, i: 100}, Union{tag: INT, i: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "min with floats",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 30},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "min with floats with timeout",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
}

func TestMinOverTimescale(t *testing.T) {
	var output Union
	var err error
	for _, test := range MinOverTimescaleTestCases {
		t0 := time.Now()
		for i := 0; i < len(test.inputs); i += 2 {
			output, err = MinOverTimescale(test.inputs[i], test.inputs[i+1], &(test.state))
			time.Sleep(time.Duration(test.wait) * time.Millisecond)
		}
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = MinOverTimescale(MaxOverTimescaleTestCases[0].inputs[0], MaxOverTimescaleTestCases[0].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var AvgOverTimescaleTestCases = []TestCase{
	TestCase{
		testDescription: "NIL union",
		inputs:          []Union{Union{tag: NIL, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "STRING union",
		inputs:          []Union{Union{tag: STRING, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "all bool unions",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "avg with uints",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 76},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "avg with uints with timeout",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "avg with ints",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 310}, Union{tag: INT, i: 30}, Union{tag: INT, i: 310}, Union{tag: INT, i: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 76},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "avg with ints with timeout",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 30}, Union{tag: INT, i: 100}, Union{tag: INT, i: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "avg with floats",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: (100.0 + 30.0 + 99.0) / 3.0},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "avg with floats with timeout",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 99.0},
		errorExpected:   false,
		wait:            int64(110),
	},
}

func TestAvgOverTimescale(t *testing.T) {
	var output Union
	var err error
	for _, test := range AvgOverTimescaleTestCases {
		t0 := time.Now()
		for i := 0; i < len(test.inputs); i += 2 {
			output, err = AvgOverTimescale(test.inputs[i], test.inputs[i+1], &(test.state))
			time.Sleep(time.Duration(test.wait) * time.Millisecond)
		}
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = AvgOverTimescale(AvgOverTimescaleTestCases[0].inputs[0], AvgOverTimescaleTestCases[0].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var SumOverTimescaleTestCases = []TestCase{
	TestCase{
		testDescription: "NIL union",
		inputs:          []Union{Union{tag: NIL, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "STRING union",
		inputs:          []Union{Union{tag: STRING, s: "false"}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{},
		errorExpected:   true,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "all bool unions",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "sum with uints",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 229},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "sum with uints with timeout",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 30}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: UINT, ui: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "sum with ints",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 310}, Union{tag: INT, i: 30}, Union{tag: INT, i: 310}, Union{tag: INT, i: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 229},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "sum with ints with timeout",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 30}, Union{tag: INT, i: 100}, Union{tag: INT, i: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: INT, i: 99},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "sum with floats",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 229},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "sum with floats with timeout",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 30}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 99}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: FLOAT, f: 99.0},
		errorExpected:   false,
		wait:            int64(110),
	},
}

func TestSumOverTimescale(t *testing.T) {
	var output Union
	var err error
	for _, test := range SumOverTimescaleTestCases {
		t0 := time.Now()
		for i := 0; i < len(test.inputs); i += 2 {
			output, err = SumOverTimescale(test.inputs[i], test.inputs[i+1], &(test.state))
			time.Sleep(time.Duration(test.wait) * time.Millisecond)
		}
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = AvgOverTimescale(SumOverTimescaleTestCases[0].inputs[0], SumOverTimescaleTestCases[0].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var ValueChangedOverTimescaleTestCases = []TestCase{
	TestCase{
		testDescription: "all false unions",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one true union",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 310}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "one true union with timeout",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: true}, Union{tag: INT, i: 100}, Union{tag: BOOL, b: false}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "uint changed",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 101}, Union{tag: INT, i: 310}, Union{tag: UINT, ui: 100}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "uint with timeout",
		inputs:          []Union{Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 101}, Union{tag: INT, i: 100}, Union{tag: UINT, ui: 100}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "int changed",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 310}, Union{tag: INT, i: 101}, Union{tag: INT, i: 310}, Union{tag: INT, i: 100}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "int with timeout",
		inputs:          []Union{Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 101}, Union{tag: INT, i: 100}, Union{tag: INT, i: 100}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(110),
	},
	TestCase{
		testDescription: "float changed",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 101}, Union{tag: INT, i: 310}, Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 310}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
		wait:            int64(100),
	},
	TestCase{
		testDescription: "float changed with timeout",
		inputs:          []Union{Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 101}, Union{tag: INT, i: 100}, Union{tag: FLOAT, f: 100}, Union{tag: INT, i: 100}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
		wait:            int64(110),
	},
}

func TestValueChangedOverTimescale(t *testing.T) {
	var output Union
	var err error
	for _, test := range ValueChangedOverTimescaleTestCases {
		t0 := time.Now()
		for i := 0; i < len(test.inputs); i += 2 {
			output, err = ValueChangedOverTimescale(test.inputs[i], test.inputs[i+1], &(test.state))
			time.Sleep(time.Duration(test.wait) * time.Millisecond)
		}
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}

	// state pointer nil - expect an error
	t0 := time.Now()
	output, err = ValueChangedOverTimescale(MaxOverTimescaleTestCases[0].inputs[0], MaxOverTimescaleTestCases[0].inputs[1], nil)
	duration := time.Since(t0)
	if testing.Verbose() {
		fmt.Printf("%-50s\t%5d ns\n", "state pointer nil", duration)
	}
	if (output != Union{}) {
		t.Errorf("%s: output %v not equal to expected %v\n", "state pointer nil", output, Union{})
	}
	if err == nil {
		t.Errorf("%s: no error when there should have been\n", "state pointer nil")
	}
}

var ValueChangedTests = []TestCase{
	TestCase{
		testDescription: "bool no change",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "bool change",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "bool-ui change",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "ui-bool change",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(5),UINT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 5}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 1}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(5)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: -5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 5.3}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),FLOAT(1.0)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: FLOAT, f: 1.0}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: STRING, s: "hello"}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),true",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),true",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(0),true",
		inputs:          []Union{Union{tag: INT, i: 0}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(-1),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: -1}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),UINT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: INT, i: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),INT(5)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: INT, i: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),FLOAT(5.3)",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: FLOAT, f: 5.3}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(5),FLOAT(5.0)",
		inputs:          []Union{Union{tag: INT, i: 5}, Union{tag: FLOAT, f: 5.0}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = INT(1),STRING(\"hello\")",
		inputs:          []Union{Union{tag: INT, i: 1}, Union{tag: STRING, s: "hello"}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(1.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 1.0}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),true",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: true}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(0.0),false",
		inputs:          []Union{Union{tag: FLOAT, f: 0.0}, Union{tag: BOOL, b: false}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),UINT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: UINT, ui: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.0),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.0}, Union{tag: INT, i: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),INT(5)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: INT, i: 5}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(6.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 6.3}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),FLOAT(5.3)",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: FLOAT, f: 5.3}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = FLOAT(5.3),STRING(\"hello\")",
		inputs:          []Union{Union{tag: FLOAT, f: 5.3}, Union{tag: STRING, s: "hello"}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = \"hello\",\"hello\"",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		state:           make(map[string][]Union, 0),
		expectedOutput:  Union{tag: BOOL, b: false},
		errorExpected:   false,
	},
}

func TestValueChanged(t *testing.T) {
	var output Union
	var err error
	for _, test := range ValueChangedTests {
		t0 := time.Now()
		ValueChanged(test.inputs[0], &(test.state))
		output, err = ValueChanged(test.inputs[1], &(test.state))
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var CountTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{tag:UINT, ui:0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui:1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput: Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
}

func TestCount(t *testing.T) {
	var output Union
	var err error
	for _, test := range CountTests {
		t0 := time.Now()
		output, err = Count(test.inputs)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}

var CombineBitsTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 0",
		inputs:          []Union{},
		expectedOutput:  Union{tag:UINT, ui:0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui:2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (false)",
		inputs:          []Union{Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: UINT, ui: 1},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = int",
		inputs:          []Union{Union{tag: INT, i: -5}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = uint",
		inputs:          []Union{Union{tag: UINT, ui: 5}},
		expectedOutput:  Union{tag: UINT, ui: uint64(1<<5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = float",
		inputs:          []Union{Union{tag: FLOAT, f: 5.4}},
		expectedOutput:  Union{tag: UINT, ui: uint64(1<<5)},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 1, input = string",
		inputs:          []Union{Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: UINT, ui: 0},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: UINT, ui: 4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 4},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: UINT, ui: 3},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,INT(-1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: INT, i: -1}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,FLOAT(5.3)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: FLOAT, f: 5.3}},
		expectedOutput:  Union{tag: UINT, ui: uint64(2 + (1<<5))},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,STRING(\"hello\")",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: UINT, ui: 2},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput: Union{tag: UINT, ui: uint64(4+(1<<5))},
		errorExpected:   false,
	},
}

func TestCombineBits(t *testing.T) {
	var output Union
	var err error
	for _, test := range CombineBitsTests {
		t0 := time.Now()
		output, err = CombineBits(test.inputs)
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}


var InTests = []TestCase{
	TestCase{
		testDescription: "len(inputs) = 1, input = bool (true)",
		inputs:          []Union{Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,true",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: true}},
		expectedOutput:  Union{tag: BOOL, b:true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = false,false",
		inputs:          []Union{Union{tag: BOOL, b: false}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b:true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,false",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: BOOL, b: false}},
		expectedOutput:  Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = true,UINT(1)",
		inputs:          []Union{Union{tag: BOOL, b: true}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui:1}, Union{tag: UINT, ui: 1}},
		expectedOutput:  Union{tag: BOOL, b:true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = UINT(1),INT(1)",
		inputs:          []Union{Union{tag: UINT, ui:1}, Union{tag: INT, i: 1}},
		expectedOutput:  Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = STRING(\"hello\"),STRING(\"goodbye\")",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "goodbye"}},
		expectedOutput:  Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 2, inputs = STRING(\"hello\"),STRING(\"hello\")",
		inputs:          []Union{Union{tag: STRING, s: "hello"}, Union{tag: STRING, s: "hello"}},
		expectedOutput:  Union{tag: BOOL, b:true},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),true",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: BOOL, b: true}},
		expectedOutput: Union{tag: BOOL, b:false},
		errorExpected:   false,
	},
	TestCase{
		testDescription: "len(inputs) = 3, inputs = UINT(1),INT(5),UINT(1)",
		inputs:          []Union{Union{tag: UINT, ui: 1}, Union{tag: INT, i: 5}, Union{tag: UINT, ui: 1}},
		expectedOutput: Union{tag: BOOL, b:true},
		errorExpected:   false,
	},
}

func TestIn(t *testing.T) {
	var output Union
	var err error
	
	for _, test := range InTests {
		t0 := time.Now()
		output, err = In(test.inputs[0], test.inputs[1:])
		duration := time.Since(t0)
		if testing.Verbose() {
			fmt.Printf("%-50s\t%5d ns\n", test.testDescription, duration)
		}
		if output != test.expectedOutput {
			t.Errorf("%s: output %v not equal to expected %v\n", test.testDescription, output, test.expectedOutput)
		}
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.testDescription)
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error\n", test.testDescription)
		}
	}
}
