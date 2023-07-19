package go_metrics

import (
	"testing"
)

type EvalFilterTest struct {
	inputExpression string
	expectedOutput  []string
	errorExpected   bool
}

var EvalFilterTests = []EvalFilterTest{
	EvalFilterTest{
		inputExpression: "Attribute(enabled == true)",
		expectedOutput:  []string{"input1", "input2"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(enabled == false)",
		expectedOutput:  []string{"input4"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(enabled == nottrueorfalse)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(nonexistant == true)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: `Attribute(scale == "string")`,
		expectedOutput:  []string{},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale > 2)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale > 2.0)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale < 2)",
		expectedOutput:  []string{"input2", "input4"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale >= 2)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale <= 2)",
		expectedOutput:  []string{"input2", "input4"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale == 2)",
		expectedOutput:  []string{},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale != 2)",
		expectedOutput:  []string{"input1", "input2", "input4"},
		errorExpected:   false,
	},
	EvalFilterTest{
		inputExpression: "Attribute(2 != scale)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "NotAFunction(scale <= 2)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "scale + 2",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale + 2)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale < '2')",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale < 2i )",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale < 3.7976931348623157e+308)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	EvalFilterTest{
		inputExpression: "Attribute(scale < 999999999999999999999999)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
}

func TestEvalFilter(t *testing.T) {
	Scope = map[string][]Input{
		"input1": []Input{
			Input{
				Type:          "bool",
				Value:         Union{tag: BOOL, b: true},
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "input1@enabled", "scale": "input1@scale"},
			},
		},
		"input2": []Input{
			Input{
				Type:          "bool",
				Value:         Union{tag: BOOL, b: true},
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "input2@enabled", "scale": "input2@scale"},
			},
		},
		"input4": []Input{
			Input{
				Type:          "bool",
				Value:         Union{tag: BOOL, b: true},
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "input4@enabled", "scale": "input4@scale"},
			},
		},
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
		"input1@scale": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 5},
			},
		},
		"input2@scale": []Input{
			Input{
				Type:  "float",
				Value: Union{tag: FLOAT, f: 0.001},
			},
		},
		"input4@scale": []Input{},
	}
	allPossibleAttributes = make(map[string][]string, 0)
	allPossibleAttributes["enabled"] = []string{"input1@enabled", "input2@enabled", "input4@enabled"}
	allPossibleAttributes["scale"] = []string{"input1@scale", "input2@scale", "input4@scale"}
	MetricsConfig.Attributes = map[string]Attribute{
		"input1@enabled": Attribute{
			InputVar: "input1",
			Value:    Union{tag: BOOL, b: true},
		},
		"input2@enabled": Attribute{
			InputVar: "input2",
			Value:    Union{tag: BOOL, b: true},
		},
		"input4@enabled": Attribute{
			InputVar: "input4",
			Value:    Union{tag: BOOL, b: false},
		},
		"input1@scale": Attribute{
			InputVar: "input1",
			Value:    Union{tag: FLOAT, f: 5},
		},
		"input2@scale": Attribute{
			InputVar: "input2",
			Value:    Union{tag: FLOAT, f: 0.001},
		},
		"input4@scale": Attribute{
			InputVar: "input4",
			Value:    Union{},
		},
	}
	inputs := make([]string, 0)
	for key, _ := range Scope {
		inputs = append(inputs, key)
	}
	var output []string
	var err error
	for _, test := range EvalFilterTests {
		exp, _ := Parse(test.inputExpression)
		output, err = EvaluateDynamicFilter(&(exp.Ast), inputs)
		if err == nil && test.errorExpected {
			t.Errorf("%s: no error when there should have been\n", test.inputExpression)
			continue
		} else if err != nil && !test.errorExpected {
			t.Errorf("%s: got an err when there should not have been an error; error was: %v\n", test.inputExpression, err)
			continue
		}

		if len(output) != len(test.expectedOutput) {
			t.Errorf("%s: output var %v not equal to expected var %v\n", test.inputExpression, output, test.expectedOutput)
		} else {
			for i, _ := range output {
				if output[i] != test.expectedOutput[i] {
					t.Errorf("%s: output var %v not equal to expected var %v\n", test.inputExpression, output, test.expectedOutput)
				}
			}
		}
	}
}
