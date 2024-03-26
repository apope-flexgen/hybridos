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
	{
		inputExpression: "Attribute(enabled == true)",
		expectedOutput:  []string{"input1", "input2"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(enabled == false)",
		expectedOutput:  []string{"input4"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(enabled == nottrueorfalse)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(nonexistant == true)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: `Attribute(scale == "string")`,
		expectedOutput:  []string{},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale > 2)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale > 2.0)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale < 2)",
		expectedOutput:  []string{"input2", "input4"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale >= 2)",
		expectedOutput:  []string{"input1"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale <= 2)",
		expectedOutput:  []string{"input2", "input4"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale == 2)",
		expectedOutput:  []string{},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(scale != 2)",
		expectedOutput:  []string{"input1", "input2", "input4"},
		errorExpected:   false,
	},
	{
		inputExpression: "Attribute(2 != scale)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "NotAFunction(scale <= 2)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "scale + 2",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(scale + 2)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(scale < '2')",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(scale < 2i )",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(scale < 3.7976931348623157e+308)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
	{
		inputExpression: "Attribute(scale < 999999999999999999999999)",
		expectedOutput:  []string{},
		errorExpected:   true,
	},
}

func TestEvalFilter(t *testing.T) {
	InputScope = map[string][]Union{
		"input1":  {{tag: BOOL, b: true}},
		"input2":  {{tag: BOOL, b: true}},
		"input4":  {{tag: BOOL, b: true}},
		"bool1":   {{tag: BOOL, b: true}},
		"bool2":   {{tag: BOOL, b: false}},
		"uint1":   {{tag: UINT, ui: 5}},
		"uint2":   {{tag: UINT, ui: 7}},
		"int1":    {{tag: INT, i: 6}},
		"int2":    {{tag: INT, i: 8}},
		"float1":  {{tag: FLOAT, f: 5.3}},
		"float2":  {{tag: FLOAT, f: 7.3}},
		"string1": {{tag: STRING, s: "hello"}},
		"string2": {{tag: STRING, s: ""}},
		"noval":   {},
		"multival": {
			{tag: INT, i: 9}, {tag: INT, i: 11},
		},
		"input1@enabled": {{tag: BOOL, b: true}},
		"input2@enabled": {{tag: BOOL, b: true}},
		"input4@enabled": {{}},
		"input1@scale":   {{tag: FLOAT, f: 5}},
		"input2@scale":   {{tag: FLOAT, f: 0.001}},
		"input4@scale":   {{}},
	}
	allPossibleAttributes = make(map[string][]string, 0)
	allPossibleAttributes["enabled"] = []string{"input1@enabled", "input2@enabled", "input4@enabled"}
	allPossibleAttributes["scale"] = []string{"input1@scale", "input2@scale", "input4@scale"}
	MetricsConfig.Attributes = map[string]Attribute{
		"input1@enabled": {
			InputVar: "input1",
			Value:    Union{tag: BOOL, b: true},
		},
		"input2@enabled": {
			InputVar: "input2",
			Value:    Union{tag: BOOL, b: true},
		},
		"input4@enabled": {
			InputVar: "input4",
			Value:    Union{tag: BOOL, b: false},
		},
		"input1@scale": {
			InputVar: "input1",
			Value:    Union{tag: FLOAT, f: 5},
		},
		"input2@scale": {
			InputVar: "input2",
			Value:    Union{tag: FLOAT, f: 0.001},
		},
		"input4@scale": {
			InputVar: "input4",
			Value:    Union{},
		},
	}
	inputs := make([]string, 0)
	for key := range InputScope {
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
			for i := range output {
				if output[i] != test.expectedOutput[i] {
					t.Errorf("%s: output var %v not equal to expected var %v\n", test.inputExpression, output, test.expectedOutput)
				}
			}
		}
	}
}
