package go_metrics

import (
	"fmt"
	"log"
	"os"
	"strings"
	"testing"

	simdjson "github.com/minio/simdjson-go"
)

func TestGenerateScope(t *testing.T) {
	MetricsConfig.Inputs = map[string]Input{
		"bool1":   {Type: "bool", Attributes: []string{"enabled", "scale"}, AttributesMap: map[string]string{"enabled": "bool1@enabled", "scale": "bool1@scale"}},
		"bool2":   {Type: "bool"},
		"uint1":   {Type: "uint", Attributes: []string{"puppy", "kitty"}, AttributesMap: map[string]string{"puppy": "uint1@puppy", "kitty": "uint1@kitty"}},
		"uint2":   {Uri: "some_uri", Type: "uint"},
		"int1":    {Type: "int"},
		"int2":    {Type: "int", Internal: true},
		"float1":  {Type: "float"},
		"float2":  {Type: "float"},
		"string1": {Type: "string", Value: Union{tag: STRING, s: ""}},
		"string2": {Type: "string", Value: Union{tag: STRING, s: "I love puppies!!"}},
	}

	expectedScope := map[string][]Input{
		"bool1": {
			{
				Name:          "bool1",
				Type:          "bool",
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "bool1@enabled", "scale": "bool1@scale"},
			},
		},
		"bool2": {
			{
				Name: "bool2",
				Type: "bool",
			},
		},
		"uint1": {
			{
				Name:          "uint1",
				Type:          "uint",
				Attributes:    []string{"puppy", "kitty"},
				AttributesMap: map[string]string{"puppy": "uint1@puppy", "kitty": "uint1@kitty"},
			},
		},
		"uint2": {
			{
				Uri:  "some_uri",
				Name: "uint2",
				Type: "uint",
			},
		},
		"int1": {
			{
				Name: "int1",
				Type: "int",
			},
		},
		"int2": {
			{
				Name:     "int2",
				Type:     "int",
				Internal: true,
			},
		},
		"float1": {
			{
				Name: "float1",
				Type: "float",
			},
		},
		"float2": {
			{
				Name: "float2",
				Type: "float",
			},
		},
		"string1": {
			{
				Name:  "string1",
				Type:  "string",
				Value: Union{tag: STRING, s: ""},
			},
		},
		"string2": {
			{
				Name:  "string2",
				Type:  "string",
				Value: Union{tag: STRING, s: "I love puppies!!"},
			},
		},
		"bool1@enabled": {
			{
				Name: "enabled",
			},
		},
		"bool1@scale": {
			{
				Name: "scale",
			},
		},
		"uint1@puppy": {
			{
				Name: "puppy",
			},
		},
		"uint1@kitty": {
			{
				Name: "kitty",
			},
		},
	}

	generateScope()

	if len(InputScope) != len(expectedScope) {
		t.Errorf("%s: output scope length %v not equal to expected length %v\n", "generateScope()", len(InputScope), len(expectedScope))
	} else {
		for key, inputArr := range InputScope {
			if expectedInputArr, ok := expectedScope[key]; !ok {
				t.Errorf("%s: output scope contains an extra key %s not found in expected output scope\n", "generateScope()", key)
			} else {
				for i, input := range inputArr {
					if input != expectedInputArr[i].Value {
						t.Errorf("%s: scope input value %v does not match expected %v\n", key, input, expectedInputArr[i].Value)
					}
				}
			}
		}
		for key := range expectedScope {
			if _, ok := InputScope[key]; !ok {
				t.Errorf("%s: output scope is missing key %s\n", "generateScope()", key)
			}
		}
	}
}

func TestDeleteAtIndex(t *testing.T) {
	var jalist = []JsonAccessor{
		{
			Key:   "templates",
			JType: simdjson.TypeArray,
		},
		{
			Index: 2,
			JType: simdjson.TypeObject,
		},
		{
			Key:   "to",
			JType: simdjson.TypeInt,
		},
	}
	jalist = delete_at_index(jalist, 1)
	if jalist[0].Key != "templates" || jalist[0].JType != simdjson.TypeArray {
		t.Errorf("delete_at_index(): Json Accessor List element 0 is not what expected after deleting element from middle!")
	}
	if jalist[1].Key != "to" || jalist[1].JType != simdjson.TypeInt {
		t.Errorf("delete_at_index(): Json Accessor List element 1 is not what expected after deleting element from middle! jalist[1] is %v", jalist[1])
	}
	jalist = delete_at_index(jalist, 1)
	if jalist[0].Key != "templates" || jalist[0].JType != simdjson.TypeArray {
		t.Errorf("delete_at_index(): Json Accessor List element 0 is not what expected after deleting element from end!")
	}
	if len(jalist) != 1 {
		t.Errorf("delete_at_index(): Json Accessor List is not what expected after deleting element from end!")
	}
	jalist = delete_at_index(jalist, 1)
	if len(jalist) != 1 {
		t.Errorf("delete_at_index(): Json Accessor List is not what expected after trying to delete nonexistant element!")
	}
	jalist = delete_at_index(jalist, -1)
	if len(jalist) != 1 {
		t.Errorf("delete_at_index(): Json Accessor List is not what expected after trying to delete negative index element!")
	}
}

func compareMetricsFile(testName string, struct1, struct2 MetricsFile) (string, bool) {
	outputErr := ""
	matches := true
	for key, val := range struct1.Meta {
		if struct2.Meta[key] != val {
			if matches {
				outputErr = fmt.Sprintf("%s:\t", testName)
			}
			outputErr += fmt.Sprintf("struct1.Meta[%s] (%v) != struct2.Meta[%s] (%v)\n", key, val, key, struct2.Meta[key])
			matches = false
		}
	}
	if len(struct1.Templates) != len(struct2.Templates) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Templates) (%d) != len(struct2.Templates) (%d)\n", len(struct1.Templates), len(struct2.Templates))
		matches = false
	} else {
		for i, temps1 := range struct1.Templates {
			if len(temps1.List) != len(struct2.Templates[i].List) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				outputErr += fmt.Sprintf("len(struct1.Templates[%d].List) (%d) != len(struct2.Templates[%d].List) (%d)\n", i, len(temps1.List), i, len(struct2.Templates[i].List))
				matches = false
			} else {
				for j, temp1 := range temps1.List {
					if struct2.Templates[i].List[j] != temp1 {
						if matches {
							outputErr = fmt.Sprintf("%s:\t", testName)
						}
						outputErr += fmt.Sprintf("struct1.Templates[%d].List[%d] (%v) != struct2.Templates[%d].List[%d] (%v)\n", i, j, temp1, i, j, struct2.Templates[i].List[j])
						matches = false
					}
				}
			}
		}
	}
	if len(struct1.Inputs) != len(struct2.Inputs) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Inputs) (%d) != len(struct2.Inputs) (%d)\n", len(struct1.Inputs), len(struct2.Inputs))
		matches = false
	} else {
		for key, input1 := range struct1.Inputs {
			if !compareInputs(input1, struct2.Inputs[key]) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				outputErr += fmt.Sprintf("struct1.Inputs[%s] != struct2.Inputs[%s]\n", key, key)
				matches = false
			}
		}
	}
	// TODO: Compare Attributes
	if len(struct1.Filters) != len(struct2.Filters) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Filters) (%d) != len(struct2.Filters) (%d)\n", len(struct1.Filters), len(struct2.Filters))
		matches = false
	} else {
		for key, filter1 := range struct1.Filters {
			if !compareFilters(filter1, struct2.Filters[key]) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				outputErr += fmt.Sprintf("struct1.Filters[%s] != struct2.Filters[%s]\n", key, key)
				matches = false
			}
		}
	}
	if len(struct1.Outputs) != len(struct2.Outputs) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Outputs) (%d) != len(struct2.Outputs) (%d)\n", len(struct1.Outputs), len(struct2.Outputs))
		fmt.Println(struct1.Outputs)
		matches = false
	} else {
		for key, output1 := range struct1.Outputs {
			if !compareOutputs(output1, struct2.Outputs[key]) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				outputErr += fmt.Sprintf("struct1.Outputs[%s] != struct2.Outputs[%s]\n", key, key)
				fmt.Println(output1)
				fmt.Println(struct2.Outputs[key])
				matches = false
			}
		}
	}
	if len(struct1.Metrics) != len(struct2.Metrics) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Metrics) (%d) != len(struct2.Metrics) (%d)\n", len(struct1.Metrics), len(struct2.Metrics))
		matches = false
	} else {
		for i, metric1 := range struct1.Metrics {
			if !compareMetrics(metric1, struct2.Metrics[i]) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				fmt.Println(metric1)
				fmt.Println(struct2.Metrics[i])
				outputErr += fmt.Sprintf("struct1.Metrics[%d] != struct2.Metrics[%d]\n", i, i)
				matches = false
			}
		}
	}
	if len(struct1.Echo) != len(struct2.Echo) {
		if matches {
			outputErr = fmt.Sprintf("%s:\t", testName)
		}
		outputErr += fmt.Sprintf("len(struct1.Echo) (%d) != len(struct2.Echo) (%d)\n", len(struct1.Echo), len(struct2.Echo))
		matches = false
	} else {
		for i, echo1 := range struct1.Echo {
			if !compareEcho(echo1, struct2.Echo[i]) {
				if matches {
					outputErr = fmt.Sprintf("%s:\t", testName)
				}
				outputErr += fmt.Sprintf("struct1.Echo[%d] != struct2.Echo[%d]\n", i, i)
				matches = false
			}
		}
	}
	return outputErr, matches
}

func compareInputs(input1, input2 Input) bool {
	if input1.Uri != input2.Uri ||
		input1.Internal != input2.Internal ||
		input1.Type != input2.Type ||
		input1.Name != input2.Name ||
		input1.Value != input2.Value ||
		len(input1.Attributes) != len(input2.Attributes) ||
		len(input1.AttributesMap) != len(input2.AttributesMap) {
		return false
	}
	for i, attribute1 := range input1.Attributes {
		if input2.Attributes[i] != attribute1 ||
			input1.AttributesMap[attribute1] != input2.AttributesMap[attribute1] {
			return false
		}
	}
	return true
}

func compareAttributes(att1, att2 Attribute) bool {
	return att1 == att2
}

func compareFilters(filter1, filter2 interface{}) bool {
	switch filter1.(type) {
	case string:
		return filter1 == filter2
	case []interface{}:
		filter2_interface, ok := filter2.([]interface{})
		if !ok {
			return false
		}
		for i, val := range filter1.([]interface{}) {
			if val != filter2_interface[i] {
				return false
			}
		}
		return true
	case []string:
		filter2_interface, ok := filter2.([]string)
		if !ok {
			return false
		}
		for i, val := range filter1.([]string) {
			if val != filter2_interface[i] {
				return false
			}
		}
		return true
	default:
		return false
	}
}

func compareOutputs(output1, output2 Output) bool {
	if output1.Name != output2.Name ||
		output1.Uri != output2.Uri ||
		output1.Value != output2.Value ||
		output1.PublishRate != output2.PublishRate ||
		len(output1.Flags) != len(output2.Flags) ||
		len(output1.Enum) != len(output2.Enum) ||
		len(output1.Bitfield) != len(output2.Bitfield) {
		return false
	}

	for i, flag1 := range output1.Flags {
		if output2.Flags[i] != flag1 {
			fmt.Println("here1")
			return false
		}
	}

	for i, enum1 := range output1.Enum {
		if output2.Enum[i] != enum1 {
			fmt.Println("here2")
			return false
		}
	}

	for i, bitfield1 := range output1.Bitfield {
		if output2.Bitfield[i] != bitfield1 {
			fmt.Println("here3")
			return false
		}
	}

	for key, attribute1 := range output1.Attributes {
		if output2.Attributes[key] != attribute1 {
			return false
		}
	}

	for key, attribute1 := range output1.AttributesMap {
		if output2.AttributesMap[key] != attribute1 {
			return false
		}
	}

	return true
}

func compareMetrics(metric1, metric2 MetricsObject) bool {
	if metric1.Type != metric2.Type ||
		metric1.InternalOutput != metric2.InternalOutput ||
		metric1.Expression != metric2.Expression ||
		len(metric1.State) != len(metric2.State) ||
		len(metric1.Outputs) != len(metric2.Outputs) {
		return false
	}
	for i, output1 := range metric1.Outputs {
		if metric2.Outputs[i] != output1 {
			return false
		}
	}

	for key, vals1 := range metric1.State {
		if len(metric2.State[key]) != len(vals1) {
			return false
		}
		for i, val1 := range vals1 {
			if metric2.State[key][i] != val1 {
				return false
			}
		}
	}

	return compareExpression(metric1.ParsedExpression, metric2.ParsedExpression)
}

func compareExpression(exp1, exp2 Expression) bool {
	if exp1.String != exp2.String ||
		exp1.ResultType != exp2.ResultType ||
		exp1.IsRegex != exp2.IsRegex ||
		exp1.IsTypeFilter != exp2.IsTypeFilter ||
		len(exp1.Vars) != len(exp2.Vars) {
		return false
	}

	for i, var1 := range exp1.Vars {
		if exp2.Vars[i] != var1 {
			return false
		}
	}
	return true
}

func compareEcho(echo1, echo2 EchoObject) bool {
	returnVal := true
	if echo1.PublishUri != echo2.PublishUri {
		fmt.Printf("%s\n%s", echo1.PublishUri, echo2.PublishUri)
		returnVal = false
	}
	if echo1.PublishRate != echo2.PublishRate {
		fmt.Printf("%d\n%d", echo1.PublishRate, echo2.PublishRate)
		returnVal = false
	}
	if echo1.Heartbeat != echo2.Heartbeat {
		fmt.Printf("%s\n%s", echo1.Heartbeat, echo2.Heartbeat)
		returnVal = false
	}
	if echo1.Format != echo2.Format {
		fmt.Printf("%s\n%s", echo1.Format, echo2.Format)
		returnVal = false
	}
	if len(echo1.Inputs) != len(echo2.Inputs) {
		fmt.Printf("len(echo1.inputs): %d\nlen(echo2.inputs): %d", len(echo1.Inputs), len(echo2.Inputs))
		returnVal = false
	} else {
		for i, input1 := range echo1.Inputs {
			input2 := echo2.Inputs[i]
			if input1.Uri != input2.Uri {
				fmt.Printf("input.Uri [1]: %s\ninput.Uri [2]: %s\n", input1.Uri, input2.Uri)
				returnVal = false
			}
			for key, val1 := range input1.Registers {
				if input2.Registers[key] != val1 {
					fmt.Printf("input.Registers [1]: %s\ninput.Registers [2]: %s\n", val1, input2.Registers[key])
					returnVal = false
				}
			}
		}
	}
	if len(echo1.Echo) != len(echo2.Echo) {
		fmt.Printf("len(echo1.echo): %d\nlen(echo2.echo): %d", len(echo1.Echo), len(echo2.Echo))
		returnVal = false
	} else {
		for key, val1 := range echo1.Echo {
			switch val1.(type) {
			case string, int64, float64, uint64, int, bool, nil:
				if echo2.Echo[key] != val1 {
					fmt.Printf("echo1 [1]: %v\necho2 [2]: %v\n", val1, echo2.Echo[key])
					returnVal = false
				}
			case map[string]interface{}:
				if _, ok := echo2.Echo[key].(map[string]interface{}); ok {
					for mapkey, mapval := range val1.(map[string]interface{}) {
						if (echo2.Echo[key].(map[string]interface{}))[mapkey] != mapval {
							fmt.Printf("echo1 [1]: %v\necho2 [2]: %v\n", mapval, (echo2.Echo[key].(map[string]interface{}))[mapkey])
							returnVal = false
						}
					}
				} else {
					returnVal = false
					fmt.Printf("%s: register types don't match; echo1 type map[string]interface{}\n", key)
				}
			case []interface{}:
				if _, ok := echo2.Echo[key].([]interface{}); ok {
					for mapkey, mapval := range val1.([]interface{}) {
						if (echo2.Echo[key].([]interface{}))[mapkey] != mapval {
							fmt.Printf("echo1 [1]: %v\necho2 [2]: %v\n", mapval, (echo2.Echo[key].([]interface{}))[mapkey])
							returnVal = false
						}
					}
				} else {
					returnVal = false
					fmt.Printf("%s: register types don't match; echo1 type []interface{}\n", key)
				}
			default:
				if echo2.Echo[key] != val1 {
					fmt.Printf("echo1 [1]: %v\necho2 [2]: %v\n", val1, echo2.Echo[key])
					returnVal = false
				}
			}
		}
	}

	return returnVal
}

var TestTemplates = []Template{
	{List: []string{"a", "b", "c"}, Tok: "##"},
	{List: []string{"1", "2", "3"}, Tok: "!!"},
	{List: []string{"asdf", "jkl;", "asdf", "jkl;"}, Tok: "**"},
}

type TemplateInputTest struct {
	initial                       map[string]Input
	initialAllPossibleAttributes  map[string][]string
	expectedFinal                 map[string]Input
	expectedAllPossibleAttributes map[string][]string
}

var TemplateInputTestCase = TemplateInputTest{
	initial: map[string]Input{
		"replace_##_me": {
			Name:       "replace_##_me",
			Uri:        "/some/uri/to/replace/##",
			Type:       "string",
			Attributes: []string{"enabled", "attribute##"},
			AttributesMap: map[string]string{
				"enabled":     "replace_##_me@enabled",
				"attribute##": "replace_##_me@attribute##",
			},
			Value:    Union{tag: STRING, s: "replace_##_me"},
			Internal: false,
		},
		"var_name!!": {
			Name:     "var_name!!",
			Internal: true,
			Type:     "string",
			Value:    Union{tag: STRING, s: ""},
		},
		"var_name_**": {
			Name:     "var_name_**",
			Internal: true,
			Type:     "int",
			Value:    Union{tag: INT, i: 5},
		},
	},
	initialAllPossibleAttributes: map[string][]string{
		"enabled":     {"replace_##_me@enabled"},
		"attribute##": {"replace_##_me@attribute##"},
	},
	expectedFinal: map[string]Input{
		"replace_a_me": {
			Name:       "replace_a_me",
			Uri:        "/some/uri/to/replace/a",
			Type:       "string",
			Attributes: []string{"enabled", "attributea"},
			AttributesMap: map[string]string{
				"enabled":    "replace_a_me@enabled",
				"attributea": "replace_a_me@attributea",
			},
			Value:    Union{tag: STRING, s: "replace_a_me"},
			Internal: false,
		},
		"replace_b_me": {
			Name:       "replace_b_me",
			Uri:        "/some/uri/to/replace/b",
			Type:       "string",
			Attributes: []string{"enabled", "attributeb"},
			AttributesMap: map[string]string{
				"enabled":    "replace_b_me@enabled",
				"attributeb": "replace_b_me@attributeb",
			},
			Value:    Union{tag: STRING, s: "replace_b_me"},
			Internal: false,
		},
		"replace_c_me": {
			Name:       "replace_c_me",
			Uri:        "/some/uri/to/replace/c",
			Type:       "string",
			Attributes: []string{"enabled", "attributec"},
			AttributesMap: map[string]string{
				"enabled":    "replace_c_me@enabled",
				"attributec": "replace_c_me@attributec",
			},
			Value:    Union{tag: STRING, s: "replace_c_me"},
			Internal: false,
		},
		"var_name1": {
			Name:     "var_name1",
			Internal: true,
			Type:     "string",
			Value:    Union{tag: STRING, s: ""},
		},
		"var_name2": {
			Name:     "var_name2",
			Internal: true,
			Type:     "string",
			Value:    Union{tag: STRING, s: ""},
		},
		"var_name3": {
			Name:     "var_name3",
			Internal: true,
			Type:     "string",
			Value:    Union{tag: STRING, s: ""},
		},
		"var_name_asdf": {
			Name:     "var_name_asdf",
			Internal: true,
			Type:     "int",
			Value:    Union{tag: INT, i: 5},
		},
		"var_name_jkl;": {
			Name:     "var_name_jkl;",
			Internal: true,
			Type:     "int",
			Value:    Union{tag: INT, i: 5},
		},
	},
	expectedAllPossibleAttributes: map[string][]string{
		"enabled":    {"replace_a_me@enabled", "replace_b_me@enabled", "replace_c_me@enabled"},
		"attributea": {"replace_a_me@attributea"},
		"attributeb": {"replace_b_me@attributeb"},
		"attributec": {"replace_c_me@attributec"},
	},
}

func TestInputTemplating(t *testing.T) {
	//initial test: shouldn't crash
	allPossibleAttributes = nil
	handleInputsTemplates()

	// setup
	MetricsConfig.Templates = TestTemplates
	MetricsConfig.Inputs = TemplateInputTestCase.initial
	MetricsConfig.Attributes = make(map[string]Attribute, 0)
	for attribute, attributeLocs := range TemplateInputTestCase.initialAllPossibleAttributes {
		for _, attributeLoc := range attributeLocs {
			MetricsConfig.Attributes[attributeLoc] = Attribute{Value: Union{}, Name: attribute, InputVar: strings.Split(attributeLoc, "@")[0]}
		}
	}
	allPossibleAttributes = TemplateInputTestCase.initialAllPossibleAttributes

	// run test
	handleInputsTemplates()

	if len(MetricsConfig.Inputs) != len(TemplateInputTestCase.expectedFinal) {
		t.Errorf("templated input map size %d is not the expected size of %d\n", len(MetricsConfig.Inputs), len(TemplateInputTestCase.expectedFinal))
	} else {
		for key, input := range MetricsConfig.Inputs {
			if !compareInputs(input, TemplateInputTestCase.expectedFinal[key]) {
				t.Errorf("templated input %s is not as expected\n", key)
			}
		}
	}

	expectedFinalAttributes := make(map[string]Attribute, 0)
	for attribute, attributeLocs := range TemplateInputTestCase.expectedAllPossibleAttributes {
		for _, attributeLoc := range attributeLocs {
			if attribute == "enabled" {
				expectedFinalAttributes[attributeLoc] = Attribute{Value: Union{tag: BOOL, b: true}, Name: attribute, InputVar: strings.Split(attributeLoc, "@")[0]}
			} else {
				expectedFinalAttributes[attributeLoc] = Attribute{Value: Union{}, Name: attribute, InputVar: strings.Split(attributeLoc, "@")[0]}
			}
		}
	}

	if len(MetricsConfig.Attributes) != len(expectedFinalAttributes) {
		t.Errorf("templated attributes map size %d is not the expected size of %d\n", len(MetricsConfig.Attributes), len(expectedFinalAttributes))
	} else {
		for key, att := range MetricsConfig.Attributes {
			if !compareAttributes(att, expectedFinalAttributes[key]) {
				t.Errorf("templated attribute %s is not as expected\n", key)
			}
		}
	}

	if len(allPossibleAttributes) != len(TemplateInputTestCase.expectedAllPossibleAttributes) {
		t.Errorf("all possible attributes has length %d (expected length %d)\n", len(allPossibleAttributes), len(TemplateInputTestCase.expectedAllPossibleAttributes))
	} else {
		for key, val := range allPossibleAttributes {
			if listOfAtts, ok := TemplateInputTestCase.expectedAllPossibleAttributes[key]; !ok {
				t.Errorf("all possible attributes has unexpected key %s\n", key)
			} else {
				for i, att := range val {
					if att != listOfAtts[i] {
						t.Errorf("all possible attributes has unexpected attribute %s\n", att)
					}
				}
			}
		}
	}
}

type TemplateFilterTest struct {
	initial       map[string]interface{}
	expectedFinal map[string]interface{}
}

var TemplateFilterTestCase = TemplateFilterTest{
	initial: map[string]interface{}{
		"var_name!!":   "some_broken_filter | won't work",
		"filter##":     "Regex(var_name*)",
		"filter_!!_**": []string{"some", "filter", "with", "!!", "and", "**"},
		"filter_##_":   []interface{}{"some", "filter", "with", "##"},
		"filter_!!_":   []interface{}{5, "filter", "with", "##"},
		"filter_##*":   map[string]interface{}{"unknown": "filter type"},
	},
	expectedFinal: map[string]interface{}{
		"filtera":       "Regex(var_name*)",
		"filterb":       "Regex(var_name*)",
		"filterc":       "Regex(var_name*)",
		"filter_1_asdf": []string{"some", "filter", "with", "1", "and", "asdf"},
		"filter_2_asdf": []string{"some", "filter", "with", "2", "and", "asdf"},
		"filter_3_asdf": []string{"some", "filter", "with", "3", "and", "asdf"},
		"filter_1_jkl;": []string{"some", "filter", "with", "1", "and", "jkl;"},
		"filter_2_jkl;": []string{"some", "filter", "with", "2", "and", "jkl;"},
		"filter_3_jkl;": []string{"some", "filter", "with", "3", "and", "jkl;"},
		"filter_a_":     []interface{}{"some", "filter", "with", "a"},
		"filter_b_":     []interface{}{"some", "filter", "with", "b"},
		"filter_c_":     []interface{}{"some", "filter", "with", "c"},
	},
}

func TestFilterTemplating(t *testing.T) {
	MetricsConfig.Templates = TestTemplates
	MetricsConfig.Filters = TemplateFilterTestCase.initial
	handleFiltersTemplates()
	if len(MetricsConfig.Filters) != len(TemplateFilterTestCase.expectedFinal) {
		t.Errorf("templated filter map size is %d (expected %d)", len(MetricsConfig.Filters), len(TemplateFilterTestCase.expectedFinal))
	} else {
		for key, filterVal := range MetricsConfig.Filters {
			if expectedFilter, ok := TemplateFilterTestCase.expectedFinal[key]; !ok {
				t.Errorf("templated filter contains unexpected key %s", key)
			} else {
				if !compareFilters(filterVal, expectedFilter) {
					t.Errorf("templated filter %v not as expected %v", filterVal, expectedFilter)
				}
			}
		}
	}
}

type TemplateOutputTest struct {
	initial       map[string]Output
	expectedFinal map[string]Output
}

var TemplateOutputTestCase = TemplateOutputTest{
	initial: map[string]Output{
		"replace_##_me": {
			Name:        "replace_##_me",
			Uri:         "/some/uri/to/replace/##",
			Flags:       []string{"flag", "naked", "enum", "bitfield", "group##"},
			Attributes:  map[string]interface{}{"value": "no numbers", "scale": "blach##", "something##": "no numbers"},
			PublishRate: 1000,
			Enum: []EnumObject{
				{
					Value:  0,
					String: "s##ome string",
				},
				{
					Value:  1,
					String: "some string##",
				},
			},
			Bitfield: []EnumObject{
				{
					Value:  0,
					String: "some string##",
				},
				{
					Value:  1,
					String: "som##e string##",
				},
				{
					Value:  2,
					String: "##",
				},
			},
		},
		"var_name!!": {
			Name: "var_name!!",
			Uri:  "/some/other/!!/uri",
		},
		"var_name_**": {
			Name: "var_name_**",
			Uri:  "/and/another**",
		},
	},
	expectedFinal: map[string]Output{
		"replace_a_me": {
			Name:        "replace_a_me",
			Uri:         "/some/uri/to/replace/a",
			Flags:       []string{"flag", "naked", "enum", "bitfield", "groupa"},
			Attributes:  map[string]interface{}{"value": "no numbers", "scale": "blacha", "somethinga": "no numbers"},
			PublishRate: 1000,
			Enum: []EnumObject{
				{
					Value:  0,
					String: "saome string",
				},
				{
					Value:  1,
					String: "some stringa",
				},
			},
			Bitfield: []EnumObject{
				{
					Value:  0,
					String: "some stringa",
				},
				{
					Value:  1,
					String: "somae stringa",
				},
				{
					Value:  2,
					String: "a",
				},
			},
		},
		"replace_b_me": {
			Name:        "replace_b_me",
			Uri:         "/some/uri/to/replace/b",
			Flags:       []string{"flag", "naked", "enum", "bitfield", "groupb"},
			Attributes:  map[string]interface{}{"value": "no numbers", "scale": "blachb", "somethingb": "no numbers"},
			PublishRate: 1000,
			Enum: []EnumObject{
				{
					Value:  0,
					String: "sbome string",
				},
				{
					Value:  1,
					String: "some stringb",
				},
			},
			Bitfield: []EnumObject{
				{
					Value:  0,
					String: "some stringb",
				},
				{
					Value:  1,
					String: "sombe stringb",
				},
				{
					Value:  2,
					String: "b",
				},
			},
		},
		"replace_c_me": {
			Name:        "replace_c_me",
			Uri:         "/some/uri/to/replace/c",
			Flags:       []string{"flag", "naked", "enum", "bitfield", "groupc"},
			Attributes:  map[string]interface{}{"value": "no numbers", "scale": "blachc", "somethingc": "no numbers"},
			PublishRate: 1000,
			Enum: []EnumObject{
				{
					Value:  0,
					String: "scome string",
				},
				{
					Value:  1,
					String: "some stringc",
				},
			},
			Bitfield: []EnumObject{
				{
					Value:  0,
					String: "some stringc",
				},
				{
					Value:  1,
					String: "somce stringc",
				},
				{
					Value:  2,
					String: "c",
				},
			},
		},
		"var_name1": {
			Name: "var_name1",
			Uri:  "/some/other/1/uri",
		},
		"var_name2": {
			Name: "var_name2",
			Uri:  "/some/other/2/uri",
		},
		"var_name3": {
			Name: "var_name3",
			Uri:  "/some/other/3/uri",
		},
		"var_name_asdf": {
			Name: "var_name_asdf",
			Uri:  "/and/anotherasdf",
		},
		"var_name_jkl;": {
			Name: "var_name_jkl;",
			Uri:  "/and/anotherjkl;",
		},
	},
}

func TestOutputTemplating(t *testing.T) {
	// setup
	MetricsConfig.Templates = TestTemplates
	MetricsConfig.Outputs = TemplateOutputTestCase.initial

	// run test
	handleOutputsTemplates()

	if len(MetricsConfig.Outputs) != len(TemplateOutputTestCase.expectedFinal) {
		t.Errorf("templated output map size %d is not the expected size of %d\n", len(MetricsConfig.Outputs), len(TemplateOutputTestCase.expectedFinal))
	} else {
		for key, output := range MetricsConfig.Outputs {
			if !compareOutputs(output, TemplateOutputTestCase.expectedFinal[key]) {
				t.Errorf("templated outputs %s is not as expected\n", key)
				t.Errorf("%v\n", output)
				t.Errorf("%v\n", TemplateOutputTestCase.expectedFinal[key])
			}
		}
	}
}

type TemplateMetricsTest struct {
	initial       []MetricsObject
	expectedFinal []MetricsObject
}

var TemplateMetricsTestCase = TemplateMetricsTest{
	initial: []MetricsObject{
		{
			Type:           INT,
			Outputs:        []string{"output##"},
			InternalOutput: "internal_output",
			Expression:     "Abs(input)",
		},
		{
			Type:           STRING,
			Outputs:        []string{"output!!"},
			InternalOutput: "internal_output!!",
			Expression:     "Abs(input!!)",
		},
		{
			Type:           FLOAT,
			Outputs:        []string{"output**"},
			InternalOutput: "internal_output**",
			Expression:     "Abs(input**)",
		},
	},
	expectedFinal: []MetricsObject{
		{
			Type:           INT,
			Outputs:        []string{"outputa", "outputb", "outputc"},
			InternalOutput: "internal_output",
			Expression:     "Abs(input)",
		},
		{
			Type:           STRING,
			Outputs:        []string{"output1"},
			InternalOutput: "internal_output1",
			Expression:     "Abs(input1)",
		},
		{
			Type:           STRING,
			Outputs:        []string{"output2"},
			InternalOutput: "internal_output2",
			Expression:     "Abs(input2)",
		},
		{
			Type:           STRING,
			Outputs:        []string{"output3"},
			InternalOutput: "internal_output3",
			Expression:     "Abs(input3)",
		},
		{
			Type:           FLOAT,
			Outputs:        []string{"outputasdf"},
			InternalOutput: "internal_outputasdf",
			Expression:     "Abs(inputasdf)",
		},
		{
			Type:           FLOAT,
			Outputs:        []string{"outputjkl;"},
			InternalOutput: "internal_outputjkl;",
			Expression:     "Abs(inputjkl;)",
		},
		{
			Type:           FLOAT,
			Outputs:        []string{"outputasdf"},
			InternalOutput: "internal_outputasdf",
			Expression:     "Abs(inputasdf)",
		},
		{
			Type:           FLOAT,
			Outputs:        []string{"outputjkl;"},
			InternalOutput: "internal_outputjkl;",
			Expression:     "Abs(inputjkl;)",
		},
	},
}

func TestMetricsTemplating(t *testing.T) {
	// setup
	MetricsConfig.Templates = TestTemplates
	metricsList := make([]MetricsObject, 0)

	// run test
	for _, metric := range TemplateMetricsTestCase.initial {
		tmp := handleMetricsTemplates(metric)
		metricsList = append(metricsList, tmp...)
	}

	if len(metricsList) != len(TemplateMetricsTestCase.expectedFinal) {
		t.Errorf("templated metrics list size %d is not the expected size of %d\n", len(metricsList), len(TemplateMetricsTestCase.expectedFinal))
	} else {
		for i, metric1 := range metricsList {
			if !compareMetrics(metric1, TemplateMetricsTestCase.expectedFinal[i]) {
				t.Errorf("templated metric %v is not as expected\n", metric1)
			}
		}
	}
}

type TemplateEchoTest struct {
	initial       []EchoObject
	expectedFinal []EchoObject
}

var TemplateEchoTestCase = TemplateEchoTest{
	initial: []EchoObject{
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no",
					Registers: map[string]string{
						"register1": "blah",
						"register2": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register1": nil,
				"register2": nil,
				"map":       5,
				"string":    "interface",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my##/code",
			PublishRate: 1,
			Heartbeat:   "gone...I die##d",
			Format:      "ugh##hh",
			Inputs: []EchoInput{
				{
					Uri: "/n##o",
					Registers: map[string]string{
						"registe##r1": "bla##h",
						"regis##ter2": "doe##s it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"registe##r1": nil,
				"regis##ter2": nil,
				"m##ap":       5,
				"st##ring":    "interfa##ce",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no!!",
					Registers: map[string]string{
						"register!!1": "blah",
						"register!!2": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register!!1": nil,
				"register!!2": nil,
				"m##!!ap":     5,
				"str##i!!ng":  "interf!!ace",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no",
					Registers: map[string]string{
						"register1": "blah",
						"register2": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register1": nil,
				"register2": nil,
				"m**ap":     5,
				"st**ring":  "interface",
			},
		},
	},
	expectedFinal: []EchoObject{
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no",
					Registers: map[string]string{
						"register1": "blah",
						"register2": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register1": nil,
				"register2": nil,
				"map":       5,
				"string":    "interface",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no1",
					Registers: map[string]string{
						"register11": "blah",
						"register12": "does it matter?",
					},
				},
				{
					Uri: "/no2",
					Registers: map[string]string{
						"register21": "blah",
						"register22": "does it matter?",
					},
				},
				{
					Uri: "/no3",
					Registers: map[string]string{
						"register31": "blah",
						"register32": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register11": nil,
				"register12": nil,
				"register21": nil,
				"register22": nil,
				"register31": nil,
				"register32": nil,
				"ma1ap":      5,
				"ma2ap":      5,
				"ma3ap":      5,
				"strai1ng":   "interf1ace",
				"strai2ng":   "interf2ace",
				"strai3ng":   "interf3ace",
				"mb1ap":      5,
				"mb2ap":      5,
				"mb3ap":      5,
				"strbi1ng":   "interf1ace",
				"strbi2ng":   "interf2ace",
				"strbi3ng":   "interf3ace",
				"mc1ap":      5,
				"mc2ap":      5,
				"mc3ap":      5,
				"strci1ng":   "interf1ace",
				"strci2ng":   "interf2ace",
				"strci3ng":   "interf3ace",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/my/code",
			PublishRate: 1,
			Heartbeat:   "gone...I died",
			Format:      "ughhh",
			Inputs: []EchoInput{
				{
					Uri: "/no",
					Registers: map[string]string{
						"register1": "blah",
						"register2": "does it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"register1":  nil,
				"register2":  nil,
				"masdfap":    5,
				"stasdfring": "interface",
				"mjkl;ap":    5,
				"stjkl;ring": "interface",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/mya/code",
			PublishRate: 1,
			Heartbeat:   "gone...I diead",
			Format:      "ughahh",
			Inputs: []EchoInput{
				{
					Uri: "/nao",
					Registers: map[string]string{
						"registear1": "blaah",
						"regisater2": "doeas it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"registear1": nil,
				"regisater2": nil,
				"maap":       5,
				"staring":    "interfaace",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/myb/code",
			PublishRate: 1,
			Heartbeat:   "gone...I diebd",
			Format:      "ughbhh",
			Inputs: []EchoInput{
				{
					Uri: "/nbo",
					Registers: map[string]string{
						"registebr1": "blabh",
						"regisbter2": "doebs it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"registebr1": nil,
				"regisbter2": nil,
				"mbap":       5,
				"stbring":    "interfabce",
			},
		},
		{
			PublishUri:  "/omg/I/love/deleting/all/of/myc/code",
			PublishRate: 1,
			Heartbeat:   "gone...I diecd",
			Format:      "ughchh",
			Inputs: []EchoInput{
				{
					Uri: "/nco",
					Registers: map[string]string{
						"registecr1": "blach",
						"regiscter2": "doecs it matter?",
					},
				},
			},
			Echo: map[string]interface{}{
				"registecr1": nil,
				"regiscter2": nil,
				"mcap":       5,
				"stcring":    "interfacce",
			},
		},
	},
}

func TestEchoTemplating(t *testing.T) {
	// setup
	MetricsConfig.Templates = TestTemplates
	MetricsConfig.Echo = TemplateEchoTestCase.initial

	// run test
	handleEchoTemplates()
	if len(MetricsConfig.Echo) != len(TemplateEchoTestCase.expectedFinal) {
		t.Errorf("templated echo list size %d is not the expected size of %d\n", len(MetricsConfig.Echo), len(TemplateEchoTestCase.expectedFinal))
	} else {
		for i, echo1 := range MetricsConfig.Echo {
			if !compareEcho(echo1, TemplateEchoTestCase.expectedFinal[i]) {
				t.Errorf("templated echo object %v is not as expected\n", echo1)
			}
		}
	}
}

type CombineFlagsTest struct {
	outputName   string
	output       Output
	publishUris  map[string][]string
	pubUriFlags  map[string][]string
	configErrors []ErrorLocation
}

var CombineFlagsTestCase = []CombineFlagsTest{
	{
		outputName: "output1",
		output: Output{
			Name: "output1",
			Uri:  "/some/random/uri",
		},
		publishUris: map[string][]string{
			"/some/random/uri": {"output1"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri": {},
		},
	},
	{
		outputName: "output2",
		output: Output{
			Name:  "output2",
			Uri:   "/some/random/uri",
			Flags: []string{"clothed", "group1"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":         {"output1"},
			"/some/random/uri[group1]": {"output2"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":         {},
			"/some/random/uri[group1]": {"clothed", "group1"},
		},
	},
	{
		outputName: "output3",
		output: Output{
			Name:  "output3",
			Uri:   "/some/random/uri",
			Flags: []string{"clothed"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":         {"output1", "output3"},
			"/some/random/uri[group1]": {"output2"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":         {"clothed"},
			"/some/random/uri[group1]": {"clothed", "group1"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
	{
		outputName: "output4",
		output: Output{
			Name:  "output4",
			Uri:   "/some/random/uri",
			Flags: []string{"lonely"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":          {"output1", "output3"},
			"/some/random/uri[group1]":  {"output2"},
			"/some/random/uri[output4]": {"output4"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":          {"clothed"},
			"/some/random/uri[group1]":  {"clothed", "group1"},
			"/some/random/uri[output4]": {"lonely"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
	{
		outputName: "output5",
		output: Output{
			Name:  "output5",
			Uri:   "/some/random/uri",
			Flags: []string{"group2", "naked"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":          {"output1", "output3"},
			"/some/random/uri[group1]":  {"output2"},
			"/some/random/uri[output4]": {"output4"},
			"/some/random/uri[group2]":  {"output5"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":          {"clothed"},
			"/some/random/uri[group1]":  {"clothed", "group1"},
			"/some/random/uri[output4]": {"lonely"},
			"/some/random/uri[group2]":  {"group2", "naked"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
	{
		outputName: "output6",
		output: Output{
			Name:  "output6",
			Uri:   "/some/random/uri",
			Flags: []string{"group2", "naked", "clothed"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":          {"output1", "output3"},
			"/some/random/uri[group1]":  {"output2"},
			"/some/random/uri[output4]": {"output4"},
			"/some/random/uri[group2]":  {"output5", "output6"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":          {"clothed"},
			"/some/random/uri[group1]":  {"clothed", "group1"},
			"/some/random/uri[output4]": {"lonely"},
			"/some/random/uri[group2]":  {"group2", "naked", "clothed"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output6",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has format specified as both naked and clothed; defaulting to clothed", "output6"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
	{
		outputName: "output7",
		output: Output{
			Name:  "output7",
			Uri:   "/some/random/uri",
			Flags: []string{"group3", "naked"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":          {"output1", "output3"},
			"/some/random/uri[group1]":  {"output2"},
			"/some/random/uri[output4]": {"output4"},
			"/some/random/uri[group2]":  {"output5", "output6"},
			"/some/random/uri[group3]":  {"output7"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":          {"clothed"},
			"/some/random/uri[group1]":  {"clothed", "group1"},
			"/some/random/uri[output4]": {"lonely"},
			"/some/random/uri[group2]":  {"group2", "naked", "clothed"},
			"/some/random/uri[group3]":  {"group3", "naked"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output6",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has format specified as both naked and clothed; defaulting to clothed", "output6"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
	{
		outputName: "output8",
		output: Output{
			Name:  "output8",
			Uri:   "/some/random/uri",
			Flags: []string{"group3"},
		},
		publishUris: map[string][]string{
			"/some/random/uri":          {"output1", "output3"},
			"/some/random/uri[group1]":  {"output2"},
			"/some/random/uri[output4]": {"output4"},
			"/some/random/uri[group2]":  {"output5", "output6"},
			"/some/random/uri[group3]":  {"output7", "output8"},
		},
		pubUriFlags: map[string][]string{
			"/some/random/uri":          {"clothed"},
			"/some/random/uri[group1]":  {"clothed", "group1"},
			"/some/random/uri[output4]": {"lonely"},
			"/some/random/uri[group2]":  {"group2", "naked", "clothed"},
			"/some/random/uri[group3]":  {"group3", "naked"},
		},
		configErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output6",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has format specified as both naked and clothed; defaulting to clothed", "output6"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output8",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to naked (warning only)", "output8"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output '%v' has unspecified clothed/naked status; defaulting to clothed (warning only)", "output1"),
			},
		},
	},
}

func TestCombineFlags(t *testing.T) {
	MetricsConfig.Outputs = make(map[string]Output, 0)
	for _, test := range CombineFlagsTestCase {
		MetricsConfig.Outputs[test.outputName] = test.output
		configErrorLocs.ErrorLocs = make([]ErrorLocation, 0)
		combineFlags(test.outputName, &test.output)
		checkMixedFlags([]JsonAccessor{})
		if len(PublishUris) != len(test.publishUris) {
			t.Errorf("%s: publish URIs is unexpected length of %d (expected %d) after running combineFlags for output\n", test.outputName, len(PublishUris), len(test.publishUris))
		} else {
			for key, val := range PublishUris {
				if !stringListsMatch(val, test.publishUris[key]) {
					t.Errorf("%s: publish URIs for [%s] is not the same as expected %v (expected %v) after running combineFlags for output\n", test.outputName, key, PublishUris[key], test.publishUris[key])
				}
			}
		}
		if len(PubUriFlags) != len(test.pubUriFlags) {
			t.Errorf("%s: publish URIs is unexpected length of %d (expected %d) after running combineFlags for output\n", test.outputName, len(PubUriFlags), len(test.pubUriFlags))
		} else {
			for key, val := range PubUriFlags {
				if !stringListsMatch(val, test.pubUriFlags[key]) {
					t.Errorf("%s: pubUriFlags for [%s] is not the same as expected %v (expected %v) after running combineFlags for output\n", test.outputName, key, PubUriFlags[key], test.pubUriFlags[key])
				}
			}
		}
		if len(configErrorLocs.ErrorLocs) != len(test.configErrors) {
			t.Errorf("%s: error report is unexpected length of %d (expected %d) after running combineFlags for output\n", test.outputName, len(configErrorLocs.ErrorLocs), len(test.configErrors))
		} else {
			for i, errLoc := range configErrorLocs.ErrorLocs {
				matches := true
				for q, testErrLoc := range test.configErrors {
					correctErrorLoc := true
					for j, jsonAccessor := range errLoc.JsonLocation {
						if testErrLoc.JsonLocation[j] != jsonAccessor {
							correctErrorLoc = false
							break
						}
					}
					if correctErrorLoc {
						i = q
						break
					}
				}
				for j, jsonAccessor := range errLoc.JsonLocation {
					if test.configErrors[i].JsonLocation[j] != jsonAccessor || errLoc.JsonError != test.configErrors[i].JsonError {
						matches = false
						break
					}
				}
				if !matches {
					t.Errorf("%s: expected error report [%s] is not as expected (%s)\n", test.outputName, errLoc, test.configErrors[i])
				}
			}
		}
	}
}

type CheckErrorLog struct {
	inputErr            error
	currentJsonLocation []JsonAccessor
	expectedFinal       []ErrorLocation
}

var CheckErrorLogTestCase = []CheckErrorLog{
	{
		inputErr: fmt.Errorf("path not found"),
		currentJsonLocation: []JsonAccessor{
			{
				Key:   "outputs",
				JType: simdjson.TypeObject,
			},
			{
				Key:   "output1",
				JType: simdjson.TypeObject,
			},
			{
				Key:   "test_key",
				JType: simdjson.TypeString,
			},
		},
		expectedFinal: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
				},
				"key 'test_key' not found",
			},
		},
	},
	{
		inputErr: fmt.Errorf("value is not type string"),
		currentJsonLocation: []JsonAccessor{
			{
				Key:   "inputs",
				JType: simdjson.TypeObject,
			},
			{
				Key:   "input1",
				JType: simdjson.TypeObject,
			},
			{
				Key:   "test_string",
				JType: simdjson.TypeString,
			},
		},
		expectedFinal: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
				},
				"key 'test_key' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "input1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "test_string",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be type string",
			},
		},
	},
}

func TestCheckLogErrors(t *testing.T) {
	configErrorLocs.ErrorLocs = make([]ErrorLocation, 0)
	for p, test := range CheckErrorLogTestCase {
		logError(&(configErrorLocs.ErrorLocs), test.currentJsonLocation, test.inputErr)
		if len(configErrorLocs.ErrorLocs) != len(test.expectedFinal) {
			t.Errorf("%d: error report is unexpected length of %d (expected %d) after running combineFlags for output\n", p, len(configErrorLocs.ErrorLocs), len(test.expectedFinal))
		} else {
			for i, errLoc := range configErrorLocs.ErrorLocs {
				matches := true
				for q, testErrLoc := range test.expectedFinal {
					correctErrorLoc := true
					for j, jsonAccessor := range errLoc.JsonLocation {
						if testErrLoc.JsonLocation[j] != jsonAccessor {
							correctErrorLoc = false
							break
						}
					}
					if correctErrorLoc {
						i = q
						break
					}
				}
				for j, jsonAccessor := range errLoc.JsonLocation {
					if test.expectedFinal[i].JsonLocation[j] != jsonAccessor || errLoc.JsonError != test.expectedFinal[i].JsonError {
						matches = false
						break
					}
				}
				if !matches {
					t.Errorf("%d: expected error report [%s] is not as expected (%s)\n", p, errLoc, test.expectedFinal[i])
				}
			}
		}
	}
}

type CheckUnmarshalConfig struct {
	inputFileLoc          string
	expectedMetricsConfig MetricsFile
	expectedErrors        []ErrorLocation
}

var UnmarshalConfigTestCase = []CheckUnmarshalConfig{
	{ // test 1
		inputFileLoc: "../../test/configs/unmarshal/test1.json",
		expectedMetricsConfig: MetricsFile{
			Meta: map[string]interface{}{
				"publishRate": int64(2000),
			},
			Inputs: map[string]Input{
				"v1": {
					Name:          "v1",
					Uri:           "/components/feeder_52m1/v1",
					Type:          "float",
					Value:         Union{tag: FLOAT, f: 0.0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
			},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{
				"v1_times_5": {
					Name:          "v1_times_5",
					Uri:           "/some/v1/output",
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
			},
			Metrics: []MetricsObject{
				{
					Type:           FLOAT,
					Outputs:        []string{"v1_times_5"},
					InternalOutput: "",
					Expression:     "v1 * 5",
					ParsedExpression: Expression{
						String:       "v1 * 5",
						Vars:         []string{"v1"},
						ResultType:   FLOAT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: FLOAT, f: 0.0}},
					},
				},
			},
			Echo: []EchoObject{},
		},
		expectedErrors: []ErrorLocation{},
	},
	{ // test 2
		inputFileLoc: "../../test/configs/unmarshal/test2.json",
		expectedMetricsConfig: MetricsFile{
			Meta: map[string]interface{}{
				"note":        "all big fields (templates, inputs, filters, outputs, metrics, echo) are optional",
				"publishRate": int64(2000),
			},
			Templates: []Template{
				{
					Type: "sequential",
					From: int64(1),
					To:   int64(3),
					Step: int64(1),
					Tok:  "##",
					List: []string{"1", "2", "3"},
				},
				{
					Type: "list",
					Tok:  "qq",
					List: []string{"bobcat", "cheetah", "lion"},
				},
			},
			Inputs: map[string]Input{
				"var_name1": {
					Name:          "var_name1",
					Uri:           "/components/bms_74b/vnom",
					Type:          "float",
					Value:         Union{tag: FLOAT, f: 5.0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
				"var_name2": {
					Name:          "var_name2",
					Uri:           "/components/feeder_52m1/v1",
					Type:          "float",
					Value:         Union{tag: FLOAT, f: 0.0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
				"var_name3": {
					Name:          "var_name3",
					Uri:           "/components/feeder_52m1/id",
					Type:          "string",
					Value:         Union{tag: STRING, s: ""},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
				"var_name4": {
					Name:          "var_name4",
					Uri:           "/components/feeder_52u1/pmax",
					Type:          "bool",
					Value:         Union{tag: BOOL, b: false},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
				"var_name5": {
					Name:          "var_name5",
					Uri:           "/components/bms_74b/id",
					Type:          "string",
					Value:         Union{tag: STRING, s: ""},
					Attributes:    []string{"enabled", "scale"},
					AttributesMap: map[string]string{"enabled": "var_name5@enabled", "scale": "var_name5@scale"},
				},
				"intermediate_input": {
					Name:          "intermediate_input",
					Internal:      true,
					Uri:           "",
					Type:          "int",
					Value:         Union{tag: INT, i: 0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
			},
			Filters: map[string]interface{}{
				"all_vars_enabled": "regex(var_name*) | attribute(enabled == true)",
				"all_float_vars":   "regex(var_name*) | type(float)",
			},
			Outputs: map[string]Output{
				"output1": {
					Name:          "timestamp",
					Uri:           "/some/output1",
					Flags:         []string{"group2"},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: STRING, s: ""},
				},
				"output2": {
					Name:          "timestamp",
					Uri:           "/some/output2",
					Flags:         []string{"group2"},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: STRING, s: ""},
				},
				"output3": {
					Name:          "timestamp",
					Uri:           "/some/output3",
					Flags:         []string{"group2"},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: STRING, s: ""},
				},
				"level2_output": {
					Name:          "level2_output",
					Uri:           "/some/level2",
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: INT, i: 0},
				},
				"enum_output": {
					Name:          "status",
					Uri:           "/some/status/output",
					Flags:         []string{"enum"},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum: []EnumObject{
						{
							Value:  0,
							String: "Power up",
						},
						{
							Value:  1,
							String: "Initialization",
						},
						{
							Value:  10,
							String: "Off",
						},
						{
							Value:  11,
							String: "Precharge",
						},
						{
							Value:  20,
							String: "some other value",
						},
					},
					Bitfield: []EnumObject{},
					Value:    Union{tag: INT, i: 0},
				},
				"bitfield_output": {
					Name:          "status2",
					Uri:           "/some/status/output",
					Flags:         []string{"bitfield"},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield: []EnumObject{{
						Value:  0,
						String: "Power up",
					},
						{
							Value:  1,
							String: "Initialization",
						},
						{
							Value:  2,
							String: "Off",
						},
						{
							Value:  3,
							String: "Precharge",
						},
					},
					Value: Union{tag: INT, i: 0},
				},
				"output1_cheetah@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output1_bobcat@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output1_lion@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output2_cheetah@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output2_bobcat@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output2_lion@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output3_cheetah@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output3_bobcat@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
				"output3_lion@scale": {
					Flags:         []string{},
					Attributes:    map[string]interface{}{},
					AttributesMap: map[string]string{},
					Enum:          []EnumObject{},
					Bitfield:      []EnumObject{},
					Value:         Union{tag: FLOAT, f: 0.0},
				},
			},
			Metrics: []MetricsObject{
				{
					Type:           FLOAT,
					Outputs:        []string{"output1_bobcat@scale", "output1_cheetah@scale", "output1_lion@scale", "output2_bobcat@scale", "output2_cheetah@scale", "output2_lion@scale", "output3_bobcat@scale", "output3_cheetah@scale", "output3_lion@scale"},
					InternalOutput: "",
					Expression:     "If(var_name5@enabled < 5, 100, 150)",
					ParsedExpression: Expression{
						String:       "If(var_name5.enabled < 5, 100, 150)",
						Vars:         []string{"var_name5@enabled"},
						ResultType:   INT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: FLOAT, f: 0.0}},
					},
				},
				{
					Type:           STRING,
					Outputs:        []string{"output1", "output2", "output3"},
					InternalOutput: "",
					Expression:     "MillisecondsToRFC3339(Time())",
					ParsedExpression: Expression{
						String:       "MillisecondsToRFC3339(Time())",
						Vars:         []string{},
						ResultType:   STRING,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: true}},
						"value":          {{tag: STRING, s: ""}},
					},
				},
				{
					Type:           INT,
					Outputs:        []string{"enum_output"},
					InternalOutput: "",
					Expression:     "3",
					ParsedExpression: Expression{
						String:       "3",
						Vars:         []string{},
						ResultType:   INT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: INT, i: 0}},
					},
				},
				{
					Type:           INT,
					Outputs:        []string{"bitfield_output"},
					InternalOutput: "",
					Expression:     "true | false << 1 | true << 2 | true << 3",
					ParsedExpression: Expression{
						String:       "true | false << 1 | true << 2 | true << 3",
						Vars:         []string{"true", "false", "true", "true"},
						ResultType:   INT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: INT, i: 0}},
					},
				},
				{
					Type:           INT,
					Outputs:        []string{},
					InternalOutput: "intermediate_input",
					Expression:     "5",
					ParsedExpression: Expression{
						String:       "5",
						Vars:         []string{},
						ResultType:   INT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: INT, i: 0}},
					},
				},
				{
					Type:           INT,
					Outputs:        []string{"level2_output"},
					InternalOutput: "",
					Expression:     "intermediate_input*5",
					ParsedExpression: Expression{
						String:       "intermediate_input*5",
						Vars:         []string{"intermediate_input"},
						ResultType:   INT,
						IsRegex:      false,
						IsTypeFilter: false,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: INT, i: 0}},
					},
				},
			},
			Echo: []EchoObject{
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 1000,
					Format:      "naked",
					Inputs: []EchoInput{
						{
							Uri: "/components/feeder",
							Registers: map[string]string{
								"f":  "frequency",
								"p":  "active_power",
								"pf": "power_factor",
								"q":  "reactive_power",
								"v":  "voltage_l1",
								"v1": "voltage_l2",
								"v2": "voltage_l3",
								"s1": "string_uri_element",
								"b1": "bool_uri_element",
							},
						},
					},
					Echo: map[string]interface{}{
						"f":               int64(60),
						"p":               int64(100),
						"pf":              int64(0),
						"q":               int64(0),
						"v":               int64(0),
						"v1":              int64(0),
						"v2":              int64(0),
						"s1":              "some value for the string",
						"b1":              true,
						"apparent_power":  int64(0),
						"current_l1":      int64(0),
						"current_l2":      int64(0),
						"current_l3":      int64(0),
						"current_n":       int64(0),
						"kvarh_delivered": int64(0),
						"kvarh_received":  int64(0),
						"kwh_delivered":   int64(0),
						"kwh_received":    int64(0),
						"thd_i_l1":        int64(0),
						"thd_i_l2":        int64(0),
						"thd_i_l3":        int64(0),
						"thd_v_l1":        int64(0),
						"thd_v_l2":        int64(0),
						"thd_v_l3":        int64(0),
						"voltage_l1_l2":   int64(0),
						"voltage_l2_l3":   int64(0),
						"voltage_l3_l1":   int64(0),
					},
				},
			},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "expression",
						JType: simdjson.TypeString,
					},
				},
				"metrics expression produces possible result type int but gets cast to float (warning only)",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1_cheetah",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output1_cheetah"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1_bobcat",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output1_bobcat"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1_lion",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output1_lion"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output2_cheetah",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output2_cheetah"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output2_bobcat",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output2_bobcat"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output2_lion",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output2_lion"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output3_cheetah",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output3_cheetah"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output3_bobcat",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output3_bobcat"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output3_lion",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output3_lion"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "pf",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "pf"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "q",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "q"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v1",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v1"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v2",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v2"),
			},
		},
	},
	{ // test 3
		inputFileLoc: "../../test/configs/unmarshal/test3.json",
		expectedMetricsConfig: MetricsFile{
			Meta: map[string]interface{}{
				"note":        "all big fields (templates, inputs, filters, outputs, metrics, echo) are optional",
				"publishRate": int64(2000),
			},
			Templates: []Template{
				{
					From: int64(1),
					To:   int64(3),
					Step: int64(1),
					Tok:  "##",
					List: []string{"1", "2", "3"},
				},
				{
					Tok:  "qq",
					List: []string{"bobcat", "cheetah", "lion"},
				},
			},
			Inputs: map[string]Input{
				"var_name3": {
					Name:          "var_name3",
					Uri:           "/components/feeder_52m1/v1",
					Type:          "int",
					Value:         Union{tag: INT, i: 0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
			},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name1",
						JType: simdjson.TypeObject,
					},
				},
				"key 'uri' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name2",
						JType: simdjson.TypeObject,
					},
				},
				"key 'type' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name3",
						JType: simdjson.TypeObject,
					},
				},
				"duplicate input variable 'var_name3'; only considering first occurence",
			},
			{
				[]JsonAccessor{
					{
						Key:   "filters",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "all_vars_enabled",
						JType: simdjson.TypeObject,
					},
				},
				"unhandled filter expression; discarding filter",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output##_qq",
						JType: simdjson.TypeObject,
					},
				},
				"key 'uri' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output1_bobcat@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output1_cheetah@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output1_lion@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output2_bobcat@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output2_cheetah@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output2_lion@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output3_bobcat@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output3_cheetah@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output3_lion@scale"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output1"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output2"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "output3"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "enum_output"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "bitfield_output"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 4,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal_output",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("internal_output variable '%s' does not have a corresponding input config", "intermediate_input"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 4,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				fmt.Sprintf("output variable '%s' does not have a corresponding output config", "level2_output"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'uri' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'publishRate' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "pf",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "pf"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "q",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "q"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v1",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v1"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v2",
						JType: simdjson.TypeString,
					},
				},
				fmt.Sprintf("default value for echo input register '%s' was not specified and echo object does not contain field 'null_value_default' to override the null register value; setting default value of register to 0 (warning only)", "v2"),
			},
		},
	},
	{ // test 4
		inputFileLoc: "../../test/configs/unmarshal/test4.json",
		expectedMetricsConfig: MetricsFile{
			Meta: map[string]interface{}{
				"note":        "all big fields (templates, inputs, filters, outputs, metrics, echo) are optional",
				"publishRate": int64(2000),
			},
			Templates: []Template{
				{
					Tok:  "##",
					List: []string{"1", "2", "3"},
				},
				{
					Tok:  "qq",
					List: []string{"map[string:value]", "cheetah", "lion"},
				},
				{
					Tok:  "qq",
					List: []string{},
				},
				{
					Tok:  "qqq",
					List: []string{"bobcat", "cheetah", "lion"},
				},
				{
					Tok:  "pp",
					List: []string{"0", "1", "2", "3"},
				},
				{
					Tok:  "!!",
					List: []string{"0", "1", "2", "3"},
				},
				{
					Tok:  "cc",
					List: []string{"0", "1", "2", "3"},
				},
				{
					Tok:  "$!",
					List: []string{"0", "1", "2", "3"},
				},
			},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"could not identify template type; need either from/to pair or list",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
				},
				"key 'to' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
				},
				"key 'token' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "from",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 4,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "to",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "step",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 6,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 7,
						JType: simdjson.TypeObject,
					},
				},
				"key 'from' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 8,
						JType: simdjson.TypeObject,
					},
				},
				"key 'to' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 9,
						JType: simdjson.TypeObject,
					},
				},
				"key 'token' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 10,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "from",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 11,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "to",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 12,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "step",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 13,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 15,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "list",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 16,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "list",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 16,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"template 1 contains template 16's token in its entirety; note that neither template may behave as desired",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 17,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 18,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"template tokens cannot be empty strings",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 19,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"template tokens cannot contain '@' symbol; symbol is reserved for attributes",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 20,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"template 20 contains template 1's token in its entirety; note that neither template may behave as desired",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 20,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "token",
						JType: simdjson.TypeString,
					},
				},
				"template 20 contains template 2's token in its entirety; note that neither template may behave as desired",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 21,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "type",
						JType: simdjson.TypeString,
					},
				},
				"unexpected template type something invalid: need \\\"sequential\\\" or \\\"list\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 22,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "type",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 23,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "step",
						JType: simdjson.TypeInt,
					},
				},
				"cannot have template step of 0; defaulting to a step of 1",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 25,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "step",
						JType: simdjson.TypeInt,
					},
				},
				"cannot have template step of 0; defaulting to a step of 1",
			},
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 26,
						JType: simdjson.TypeObject,
					},
				},
				"key 'list' not found",
			},
		},
	},
	{ // test 5
		inputFileLoc: "../../test/configs/unmarshal/test5.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 6
		inputFileLoc: "../../test/configs/unmarshal/test6.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "templates",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 7
		inputFileLoc: "../../test/configs/unmarshal/test7.json",
		expectedMetricsConfig: MetricsFile{
			Meta: map[string]interface{}{},
			Inputs: map[string]Input{
				"var_name5": {
					Name:          "var_name5",
					Uri:           "/components/bms_74b/id",
					Type:          "string",
					Value:         Union{tag: STRING, s: ""},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
				"var_name6": {
					Name:          "var_name6",
					Uri:           "",
					Internal:      true,
					Type:          "string",
					Value:         Union{tag: STRING, s: ""},
					Attributes:    []string{"enabled", "scale"},
					AttributesMap: map[string]string{"enabled": "var_name6@enabled", "scale": "var_name6@scale"},
				},
				"var_name10": {
					Name:          "var_name10",
					Uri:           "/components/bms_74b/id",
					Type:          "uint",
					Value:         Union{tag: UINT, ui: 0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
			},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "uri",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name2",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "type",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name3",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "type",
						JType: simdjson.TypeString,
					},
				},
				"invalid data type banana specified for input; must be string, bool, float, int, or uint",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name5",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "attributes",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name6",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal",
						JType: simdjson.TypeBool,
					},
				},
				"key 'internal' is specified as true but 'uri' field contains a uri; defaulting to internally calculated value",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name7",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "uri",
						JType: simdjson.TypeString,
					},
				},
				"type \\\"[\\\" found before object was found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name8",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal",
						JType: simdjson.TypeBool,
					},
				},
				"expected value to be bool, but \\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name9",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal",
						JType: simdjson.TypeBool,
					},
				},
				"key 'internal' is specified as false but 'uri' field is empty; need one or the other",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "var_name10",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "attributes",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
		},
	},
	{ // test 8
		inputFileLoc: "../../test/configs/unmarshal/test8.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 9
		inputFileLoc: "../../test/configs/unmarshal/test9.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "filters",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 10
		inputFileLoc: "../../test/configs/unmarshal/test10.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "filters",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 11
		inputFileLoc: "../../test/configs/unmarshal/test11.json",
		expectedMetricsConfig: MetricsFile{
			Meta:    map[string]interface{}{},
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output1",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "uri",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output2",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output3",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output4",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "flags",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeString,
					},
				},
				"invalid output flag 'banana'",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output5",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "publishRate",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output6",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "attributes",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output7",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "name",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output8",
						JType: simdjson.TypeObject,
					},
				},
				"found 'enum' field, but no matching output flag",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output9",
						JType: simdjson.TypeObject,
					},
				},
				"found 'enum' flag, but no matching 'enum' field",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output10",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output11",
						JType: simdjson.TypeObject,
					},
				},
				"found 'bitfield' flag, but no matching 'bitfield' field",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output12",
						JType: simdjson.TypeObject,
					},
				},
				"found 'bitfield' field, but no matching output flag",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output13",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output14",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "publishRate",
						JType: simdjson.TypeInt,
					},
				},
				"publish rate must be greater than 0; defaulting to global publish rate",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output14",
						JType: simdjson.TypeObject,
					},
				},
				"duplicate output variable 'output14'; only considering first occurence",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "value",
						JType: simdjson.TypeInt,
					},
				},
				"path not found; using default index of 0",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"path not found; using default string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string; using default index of 1 and string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "value",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int; using default index of 2",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "enum",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string; using default string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "value",
						JType: simdjson.TypeInt,
					},
				},
				"path not found; using default index of 0",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"path not found; using default string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string; using default index of 1 and string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "value",
						JType: simdjson.TypeInt,
					},
				},
				"unable to convert type \\\" to int; using default index of 2",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "string",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string; using default string of \\\"Unknown\\\"",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "bitfield",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "value",
						JType: simdjson.TypeInt,
					},
				},
				"cannot skip values for bitfields; using default index of 3",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output2",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output2"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output3",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output3"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output4",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output4"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output5",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output5"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output6",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output6"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output7",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output7"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output8",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output8"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output9",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output9"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output10",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output10"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output11",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output11"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output12",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output12"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output13",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output13"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output14",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output14"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output15",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output15"),
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "output16",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "output16"),
			},
		},
	},
	{ // test 12
		inputFileLoc: "../../test/configs/unmarshal/test12.json",
		expectedMetricsConfig: MetricsFile{
			Inputs: map[string]Input{
				"v1": {
					Name:          "v1",
					Uri:           "/components/feeder_52m1/v1",
					Type:          "float",
					Value:         Union{tag: FLOAT, f: 0.0},
					Attributes:    []string{},
					AttributesMap: map[string]string{},
				},
			},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{
				{
					InternalOutput: "v1",
					Expression:     "5",
					ParsedExpression: Expression{
						String:     "5",
						ResultType: INT,
					},
					State: map[string][]Union{
						"alwaysEvaluate": {{tag: BOOL, b: false}},
						"value":          {{tag: INT, i: 0}},
					},
				},
			},
			Echo: []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
				},
				"output variable '5' does not have a corresponding output config",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "expression",
						JType: simdjson.TypeString,
					},
				},
				"cannot find variable banana in inputs or filters; excluding this metric from calculations",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal_output",
						JType: simdjson.TypeString,
					},
				},
				"internal_output variable '5' does not have a corresponding input config",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "expression",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal_output",
						JType: simdjson.TypeString,
					},
				},
				"internal_output variable 'v1_times_5' does not have a corresponding input config",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 4,
						JType: simdjson.TypeObject,
					},
				},
				"key 'type' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
				},
				"key 'outputs' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 6,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "outputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 6,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "expression",
						JType: simdjson.TypeString,
					},
				},
				"cannot find variable banana in inputs or filters; excluding this metric from calculations",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 7,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal_output",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type array to string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 7,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 8,
						JType: simdjson.TypeObject,
					},
				},
				"key 'expression' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 9,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "internal_output",
						JType: simdjson.TypeString,
					},
				},
				"cannot map internal_output variable to attribute",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 9,
						JType: simdjson.TypeObject,
					},
				},
				"after applying templating and variable checks, metric does not have valid internal_output or output variables; discarding metric",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "v1_times_5",
						JType: simdjson.TypeObject,
					},
				},
				fmt.Sprintf("output '%v' is never set by a metrics expression; excluding from published outputs", "v1_times_5"),
			},
		},
	},
	{ // test 13
		inputFileLoc: "../../test/configs/unmarshal/test13.json",
		expectedMetricsConfig: MetricsFile{
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 14
		inputFileLoc: "../../test/configs/unmarshal/test14.json",
		expectedMetricsConfig: MetricsFile{
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
	{ // test 15
		inputFileLoc: "../../test/configs/unmarshal/test15.json",
		expectedMetricsConfig: MetricsFile{
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo: []EchoObject{
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs: []EchoInput{
						{
							Uri:       "/components/feeder",
							Registers: map[string]string{},
						},
					},
					Echo: map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs: []EchoInput{
						{
							Uri:       "/components/feeder",
							Registers: map[string]string{},
						},
					},
					Echo: map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs: []EchoInput{
						{
							Uri:       "/components/feeder",
							Registers: map[string]string{},
						},
					},
					Echo: map[string]interface{}{},
				},
				{
					PublishUri:  "/components/sel_735",
					PublishRate: 2000,
					Inputs:      []EchoInput{},
					Echo:        map[string]interface{}{},
				},
			},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'uri' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'publishRate' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 1,
						JType: simdjson.TypeObject,
					},
				},
				"key 'publishRate' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "uri",
						JType: simdjson.TypeString,
					},
				},
				"cannot convert type object to string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 2,
						JType: simdjson.TypeObject,
					},
				},
				"key 'publishRate' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 3,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 4,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "uri",
						JType: simdjson.TypeString,
					},
				},
				"type \\\"l\\\" found before object was found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 5,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'registers' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 6,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"key 'uri' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 7,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "registers",
						JType: simdjson.TypeObject,
					},
				},
				"could not convert registers to map[string]interface{}",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 8,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "registers",
						JType: simdjson.TypeObject,
					},
				},
				"could not convert registers to map[string]string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 9,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "registers",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "f",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "source",
						JType: simdjson.TypeString,
					},
				},
				"expected value to be string",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 10,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "inputs",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "registers",
						JType: simdjson.TypeObject,
					},
					{
						Key:   "f",
						JType: simdjson.TypeObject,
					},
				},
				"key 'source' not found",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 11,
						JType: simdjson.TypeObject,
					},
					{
						Key:   "echo",
						JType: simdjson.TypeObject,
					},
				},
				"could not convert echo object to map[string]interface{}",
			},
		},
	},
	{ // test 16
		inputFileLoc: "../../test/configs/unmarshal/test16.json",
		expectedMetricsConfig: MetricsFile{
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "meta",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "inputs",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "filters",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "outputs",
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "metrics",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
				},
				"next item is not object",
			},
			{
				[]JsonAccessor{
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"found unknown key 'string' in configuration document; ignoring config element",
			},
		},
	},
	{ // test 17
		inputFileLoc: "../../test/configs/unmarshal/test17.json",
		expectedMetricsConfig: MetricsFile{
			Inputs:  map[string]Input{},
			Filters: map[string]interface{}{},
			Outputs: map[string]Output{},
			Metrics: []MetricsObject{},
			Echo:    []EchoObject{},
		},
		expectedErrors: []ErrorLocation{
			{
				[]JsonAccessor{
					{
						Key:   "echo",
						JType: simdjson.TypeArray,
					},
					{
						Index: 0,
						JType: simdjson.TypeObject,
					},
				},
				"next item is not object",
			},
		},
	},
}

func TestUnmarshalConfig(t *testing.T) {
	loud := Quiet()
	defer loud()
	for _, test := range UnmarshalConfigTestCase {
		configErrorLocs.ErrorLocs = []ErrorLocation{}
		MetricsConfig = MetricsFile{}
		allPossibleAttributes = nil
		data, err := os.ReadFile(test.inputFileLoc)
		if err != nil {
			t.Errorf("Error reading input json config file: %s", err)
		}
		UnmarshalConfig(data)
		if errMsg, pass := compareMetricsFile(test.inputFileLoc, MetricsConfig, test.expectedMetricsConfig); !pass {
			t.Errorf("%s", errMsg)
		}
		if len(configErrorLocs.ErrorLocs) != len(test.expectedErrors) {
			t.Errorf("%s: error report is unexpected length of %d (expected %d) after running UnmarshalConfig for output\n", test.inputFileLoc, len(configErrorLocs.ErrorLocs), len(test.expectedErrors))
		} else {
			for _, errLoc := range configErrorLocs.ErrorLocs {
				matchIdx := -1
				allLocMatches := []int{}
				for q, testErrLoc := range test.expectedErrors {
					correctErrorLoc := true
					if len(testErrLoc.JsonLocation) == len(errLoc.JsonLocation) {
						for j, jsonAccessor := range errLoc.JsonLocation {
							if testErrLoc.JsonLocation[j] != jsonAccessor {
								correctErrorLoc = false
								break
							}
						}

						if correctErrorLoc {
							allLocMatches = append(allLocMatches, q)
						}
					}
				}
				for _, i := range allLocMatches {
					for j, jsonAccessor := range errLoc.JsonLocation {
						if test.expectedErrors[i].JsonLocation[j] == jsonAccessor && errLoc.JsonError == test.expectedErrors[i].JsonError {
							matchIdx = i
							break
						}
					}

				}
				if len(allLocMatches) == 0 {
					t.Errorf("%s: could not find matching json location for %v", test.inputFileLoc, errLoc)
				} else if matchIdx == -1 {
					t.Errorf("%s: expected error report [%s] is not as expected (%s)\n", test.inputFileLoc, errLoc, test.expectedErrors[allLocMatches[0]])
				} else {
					test.expectedErrors = append(test.expectedErrors[:matchIdx], test.expectedErrors[matchIdx+1:]...)
				}
			}
		}
	}
}

func TestConfigErrorOutput(t *testing.T) {
	loud := Quiet()
	defer loud()
	ConfigErrorsFile = "../../test/configs/unmarshal/error_output_test15.json"
	data, err := os.ReadFile("../../test/configs/unmarshal/test15.json")
	if err != nil {
		t.Errorf("Error reading input json config file: %s", err)
	}
	UnmarshalConfig(data)
	errorOut, err1 := os.ReadFile("../../test/configs/unmarshal/error_output_test15.json")
	if err1 != nil {
		t.Errorf("Could not open output file: %s", err1)
	}
	expectedErrorOut, err2 := os.ReadFile("../../test/configs/unmarshal/expected_error_output_test15.json")
	if err2 != nil {
		t.Errorf("Could not open expected output file: %s", err2)
	}
	if string(errorOut) != string(expectedErrorOut) {
		t.Errorf("Error log file did not match what was expected.")
	}
}

func TestGetPubTickers(t *testing.T) {
	loud := Quiet()
	defer loud()
	MetricsConfig = MetricsFile{
		Meta: map[string]interface{}{
			"note":        "all big fields (templates, inputs, filters, outputs, metrics, echo) are optional",
			"publishRate": int64(2000),
		},
		Templates: []Template{
			{
				Type: "sequential",
				From: int64(1),
				To:   int64(3),
				Step: int64(1),
				Tok:  "##",
				List: []string{"1", "2", "3"},
			},
			{
				Type: "list",
				Tok:  "qq",
				List: []string{"bobcat", "cheetah", "lion"},
			},
		},
		Inputs: map[string]Input{
			"var_name1": {
				Name:          "var_name1",
				Uri:           "/components/bms_74b/vnom",
				Type:          "float",
				Value:         Union{tag: FLOAT, f: 5.0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name2": {
				Name:          "var_name2",
				Uri:           "/components/feeder_52m1/v1",
				Type:          "float",
				Value:         Union{tag: FLOAT, f: 0.0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name3": {
				Name:          "var_name3",
				Uri:           "/components/feeder_52m1/id",
				Type:          "string",
				Value:         Union{tag: STRING, s: ""},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name4": {
				Name:          "var_name4",
				Uri:           "/components/feeder_52u1/pmax",
				Type:          "bool",
				Value:         Union{tag: BOOL, b: false},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name5": {
				Name:          "var_name5",
				Uri:           "/components/bms_74b/id",
				Type:          "string",
				Value:         Union{tag: STRING, s: ""},
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "var_name5@enabled", "scale": "var_name5@scale"},
			},
			"intermediate_input": {
				Name:          "intermediate_input",
				Internal:      true,
				Uri:           "",
				Type:          "int",
				Value:         Union{tag: INT, i: 0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
		},
		Filters: map[string]interface{}{
			"all_vars_enabled": "regex(var_name*) | attribute(enabled == true)",
			"all_float_vars":   "regex(var_name*) | type(float)",
		},
		Outputs: map[string]Output{
			"output1_bobcat": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1_cheetah": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1_lion": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_bobcat": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_cheetah": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_lion": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_bobcat": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_cheetah": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_lion": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale":   float64(1000),
					"units":   "deg C",
					"ui_type": "none",
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1": {
				Name:          "timestamp",
				Uri:           "/some/output1",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"output2": {
				Name:          "timestamp",
				Uri:           "/some/output2",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"output3": {
				Name:          "timestamp",
				Uri:           "/some/output3",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"level2_output": {
				Uri:           "/some/level2",
				PublishRate:   600,
				Flags:         []string{"lonely"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: INT, i: 0},
			},
			"enum_output": {
				Name:          "status",
				Uri:           "/some/status/output",
				Flags:         []string{"enum"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum: []EnumObject{
					{
						Value:  0,
						String: "Power up",
					},
					{
						Value:  1,
						String: "Initialization",
					},
					{
						Value:  10,
						String: "Off",
					},
					{
						Value:  11,
						String: "Precharge",
					},
					{
						Value:  20,
						String: "some other value",
					},
				},
				Bitfield: []EnumObject{},
				Value:    Union{tag: INT, i: 0},
			},
			"bitfield_output": {
				Name:          "status2",
				Uri:           "/some/status/output",
				Flags:         []string{"bitfield"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield: []EnumObject{{
					Value:  0,
					String: "Power up",
				},
					{
						Value:  1,
						String: "Initialization",
					},
					{
						Value:  2,
						String: "Off",
					},
					{
						Value:  3,
						String: "Precharge",
					},
				},
				Value: Union{tag: INT, i: 0},
			},
			"output1_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output1_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output1_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
		},
		Metrics: []MetricsObject{
			{
				Type:           FLOAT,
				Outputs:        []string{"output1_bobcat@scale", "output1_cheetah@scale", "output1_lion@scale", "output2_bobcat@scale", "output2_cheetah@scale", "output2_lion@scale", "output3_bobcat@scale", "output3_cheetah@scale", "output3_lion@scale"},
				InternalOutput: "",
				Expression:     "If(var_name5@enabled < 5, 100, 150)",
				ParsedExpression: Expression{
					String:       "If(var_name5.enabled < 5, 100, 150)",
					Vars:         []string{"var_name5@enabled"},
					ResultType:   INT,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: false}},
					"value":          {{tag: FLOAT, f: 0.0}},
				},
			},
			{
				Type:           STRING,
				Outputs:        []string{"output1", "output2", "output3"},
				InternalOutput: "",
				Expression:     "MillisecondsToRFC3339(Time())",
				ParsedExpression: Expression{
					String:       "MillisecondsToRFC3339(Time())",
					Vars:         []string{},
					ResultType:   STRING,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: true}},
					"value":          {{tag: STRING, s: ""}},
				},
			},
			{
				Type:           INT,
				Outputs:        []string{"enum_output"},
				InternalOutput: "",
				Expression:     "3",
				ParsedExpression: Expression{
					String:       "3",
					Vars:         []string{},
					ResultType:   INT,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: false}},
					"value":          {{tag: INT, i: 0}},
				},
			},
			{
				Type:           INT,
				Outputs:        []string{"bitfield_output"},
				InternalOutput: "",
				Expression:     "true | false << 1 | true << 2 | true << 3",
				ParsedExpression: Expression{
					String:       "true | false << 1 | true << 2 | true << 3",
					Vars:         []string{"true", "false", "true", "true"},
					ResultType:   INT,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: false}},
					"value":          {{tag: INT, i: 0}},
				},
			},
			{
				Type:           INT,
				Outputs:        []string{},
				InternalOutput: "intermediate_input",
				Expression:     "5",
				ParsedExpression: Expression{
					String:       "5",
					Vars:         []string{},
					ResultType:   INT,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: false}},
					"value":          {{tag: INT, i: 0}},
				},
			},
			{
				Type:           INT,
				Outputs:        []string{"level2_output"},
				InternalOutput: "",
				Expression:     "intermediate_input*5",
				ParsedExpression: Expression{
					String:       "intermediate_input*5",
					Vars:         []string{"intermediate_input"},
					ResultType:   INT,
					IsRegex:      false,
					IsTypeFilter: false,
				},
				State: map[string][]Union{
					"alwaysEvaluate": {{tag: BOOL, b: false}},
					"value":          {{tag: INT, i: 0}},
				},
			},
		},
		Echo: []EchoObject{
			{
				PublishUri:  "/components/sel_735",
				PublishRate: 1000,
				Format:      "naked",
				Inputs: []EchoInput{
					{
						Uri: "/components/feeder",
						Registers: map[string]string{
							"f":  "frequency",
							"p":  "active_power",
							"pf": "power_factor",
							"q":  "reactive_power",
							"v":  "voltage_l1",
							"v1": "voltage_l2",
							"v2": "voltage_l3",
							"s1": "string_uri_element",
							"b1": "bool_uri_element",
						},
					},
				},
				Echo: map[string]interface{}{
					"f":               int64(60),
					"p":               int64(100),
					"pf":              nil,
					"q":               nil,
					"v":               nil,
					"v1":              nil,
					"v2":              nil,
					"s1":              "some value for the string",
					"b1":              true,
					"apparent_power":  int64(0),
					"current_l1":      int64(0),
					"current_l2":      int64(0),
					"current_l3":      int64(0),
					"current_n":       int64(0),
					"kvarh_delivered": int64(0),
					"kvarh_received":  int64(0),
					"kwh_delivered":   int64(0),
					"kwh_received":    int64(0),
					"thd_i_l1":        int64(0),
					"thd_i_l2":        int64(0),
					"thd_i_l3":        int64(0),
					"thd_v_l1":        int64(0),
					"thd_v_l2":        int64(0),
					"thd_v_l3":        int64(0),
					"voltage_l1_l2":   int64(0),
					"voltage_l2_l3":   int64(0),
					"voltage_l3_l1":   int64(0),
				},
			},
		},
	}
	testPubTickers := map[string]int{
		"/some/output1[group1]":       -1,
		"/some/output2[group1]":       -1,
		"/some/output3[group1]":       -1,
		"/some/output1[group2]":       0,
		"/some/output2[group2]":       0,
		"/some/output3[group2]":       0,
		"/some/level2[level2_output]": -1,
		"/some/status/output":         0,
	}
	GetPubTickers()
	if len(tickers) != 5 {
		t.Errorf("Expected %d tickers but got %d\n", 5, len(tickers))
	}
	if len(testPubTickers) != len(pubTickers) {
		t.Errorf("Expected len(pubTickers) == %d  but got len(pubTickers) == %d\n", len(testPubTickers), len(pubTickers))
	}
	for key, val := range pubTickers {
		if testVal, ok := testPubTickers[key]; !ok {
			t.Errorf("Unexpected key %s in pubTickers\n", key)
		} else if testVal != 0 && val == 0 {
			t.Errorf("pubTicker[%s] received global pubTicker but should not have\n", key)
		} else if testVal == 0 && val != 0 {
			t.Errorf("pubTicker[%s] received its own pubTicker but should have gotten global pub ticker\n", key)
		}

	}
	for key := range testPubTickers {
		if _, ok := pubTickers[key]; !ok {
			t.Errorf("Missing key %s in pubTickers\n", key)
		}
	}
	if len(metricsMutex) != 6 {
		t.Errorf("Expected %d metrics mutexes but got %d", 6, len(metricsMutex))
	}
	MetricsConfig = MetricsFile{
		Meta: map[string]interface{}{
			"publishRate": float64(0.1),
		},
	}
	testPubTickers = map[string]int{}
	GetPubTickers()
	if len(tickers) != 1 {
		t.Errorf("Expected %d tickers but got %d\n", 1, len(tickers))
	}
	if len(testPubTickers) != len(pubTickers) {
		t.Errorf("Expected len(pubTickers) == %d  but got len(pubTickers) == %d\n", len(testPubTickers), len(pubTickers))
	}
	if len(metricsMutex) != 0 {
		t.Errorf("Expected %d metrics mutexes but got %d", 0, len(metricsMutex))
	}
	MetricsConfig = MetricsFile{
		Meta: map[string]interface{}{
			"publishRate": int64(-1),
		},
	}
	testPubTickers = map[string]int{}
	GetPubTickers()
	if len(tickers) != 1 {
		t.Errorf("Expected %d tickers but got %d\n", 1, len(tickers))
	}
	if len(testPubTickers) != len(pubTickers) {
		t.Errorf("Expected len(pubTickers) == %d  but got len(pubTickers) == %d\n", len(testPubTickers), len(pubTickers))
	}
	if len(metricsMutex) != 0 {
		t.Errorf("Expected %d metrics mutexes but got %d", 0, len(metricsMutex))
	}
	MetricsConfig = MetricsFile{
		Meta: map[string]interface{}{
			"publishRate": "some string",
		},
	}
	testPubTickers = map[string]int{}
	GetPubTickers()
	if len(tickers) != 1 {
		t.Errorf("Expected %d tickers but got %d\n", 1, len(tickers))
	}
	if len(testPubTickers) != len(pubTickers) {
		t.Errorf("Expected len(pubTickers) == %d  but got len(pubTickers) == %d\n", len(testPubTickers), len(pubTickers))
	}
	if len(metricsMutex) != 0 {
		t.Errorf("Expected %d metrics mutexes but got %d", 0, len(metricsMutex))
	}
	MetricsConfig = MetricsFile{}
	testPubTickers = map[string]int{}
	GetPubTickers()
	if len(tickers) != 1 {
		t.Errorf("Expected %d tickers but got %d\n", 1, len(tickers))
	}
	if len(testPubTickers) != len(pubTickers) {
		t.Errorf("Expected len(pubTickers) == %d  but got len(pubTickers) == %d\n", len(testPubTickers), len(pubTickers))
	}
	if len(metricsMutex) != 0 {
		t.Errorf("Expected %d metrics mutexes but got %d", 0, len(metricsMutex))
	}
}

func TestGetSubscribeUris(t *testing.T) {
	MetricsConfig = MetricsFile{
		Inputs: map[string]Input{
			"var_name1": {
				Name:          "var_name1",
				Uri:           "/components/bms_74b/vnom",
				Type:          "float",
				Value:         Union{tag: FLOAT, f: 5.0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name1_copy": {
				Name:          "var_name1",
				Uri:           "/components/bms_74b/vnom",
				Type:          "float",
				Value:         Union{tag: FLOAT, f: 5.0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name2": {
				Name:          "var_name2",
				Uri:           "/components/feeder_52m1/v1",
				Type:          "float",
				Value:         Union{tag: FLOAT, f: 0.0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name3": {
				Name:          "var_name3",
				Uri:           "/components/feeder_52m1/id",
				Type:          "string",
				Value:         Union{tag: STRING, s: ""},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name4": {
				Name:          "var_name4",
				Uri:           "/components/feeder_52u1/pmax",
				Type:          "bool",
				Value:         Union{tag: BOOL, b: false},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
			"var_name5": {
				Name:          "var_name5",
				Uri:           "/components/bms_74b/id",
				Type:          "string",
				Value:         Union{tag: STRING, s: ""},
				Attributes:    []string{"enabled", "scale"},
				AttributesMap: map[string]string{"enabled": "var_name5@enabled", "scale": "var_name5@scale"},
			},
			"intermediate_input": {
				Name:          "intermediate_input",
				Internal:      true,
				Uri:           "",
				Type:          "int",
				Value:         Union{tag: INT, i: 0},
				Attributes:    []string{},
				AttributesMap: map[string]string{},
			},
		},
		Filters: map[string]interface{}{
			"all_vars_enabled": "regex(var_name*) | attribute(enabled == true)",
			"all_float_vars":   "regex(var_name*) | type(float)",
		},
		Outputs: map[string]Output{
			"output1_bobcat": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1_cheetah": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1_lion": {
				Uri:         "/some/output1",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_bobcat": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_cheetah": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output2_lion": {
				Uri:         "/some/output2",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_bobcat": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_cheetah": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output3_lion": {
				Uri:         "/some/output3",
				Flags:       []string{"clothed", "group1", "clothed"},
				PublishRate: 1000,
				Attributes: map[string]interface{}{
					"scale": float64(1000),
				},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{},
			},
			"output1": {
				Name:          "timestamp",
				Uri:           "/some/output1",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"output2": {
				Name:          "timestamp",
				Uri:           "/some/output2",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"output3": {
				Name:          "timestamp",
				Uri:           "/some/output3",
				Flags:         []string{"group2"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: STRING, s: ""},
			},
			"level2_output": {
				Uri:           "/some/level2",
				PublishRate:   600,
				Flags:         []string{"lonely"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: INT, i: 0},
			},
			"enum_output": {
				Name:          "status",
				Uri:           "/some/status/output",
				Flags:         []string{"enum"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum: []EnumObject{
					{
						Value:  0,
						String: "Power up",
					},
					{
						Value:  1,
						String: "Initialization",
					},
					{
						Value:  10,
						String: "Off",
					},
					{
						Value:  11,
						String: "Precharge",
					},
					{
						Value:  20,
						String: "some other value",
					},
				},
				Bitfield: []EnumObject{},
				Value:    Union{tag: INT, i: 0},
			},
			"bitfield_output": {
				Name:          "status2",
				Uri:           "/some/status/output",
				Flags:         []string{"bitfield"},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield: []EnumObject{{
					Value:  0,
					String: "Power up",
				},
					{
						Value:  1,
						String: "Initialization",
					},
					{
						Value:  2,
						String: "Off",
					},
					{
						Value:  3,
						String: "Precharge",
					},
				},
				Value: Union{tag: INT, i: 0},
			},
			"output1_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output1_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output1_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output2_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_cheetah@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_bobcat@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
			"output3_lion@scale": {
				Flags:         []string{},
				Attributes:    map[string]interface{}{},
				AttributesMap: map[string]string{},
				Enum:          []EnumObject{},
				Bitfield:      []EnumObject{},
				Value:         Union{tag: FLOAT, f: 0.0},
			},
		},
		Metrics: []MetricsObject{},
		Echo: []EchoObject{
			{
				PublishUri:  "/components/sel_735",
				PublishRate: 1000,
				Format:      "naked",
				Inputs: []EchoInput{
					{
						Uri: "/components/feeder1",
						Registers: map[string]string{
							"f1": "frequency1",
						},
					},
					{
						Uri: "/components/feeder2",
						Registers: map[string]string{
							"f2": "frequency2",
						},
					},
				},
				Echo: map[string]interface{}{
					"f":             int64(60),
					"voltage_l3_l1": int64(0),
				},
			},
		},
	}
	expectedSubscribeUris := []string{
		"/go_metrics",
		"/components/bms_74b",
		"/components/feeder_52m1",
		"/components/feeder_52u1",
		"/components/feeder1",
		"/components/feeder2",
		"/some/output1",
		"/some/output2",
		"/some/output3",
		"/some/level2",
		"/some/status/output",
		"/components/sel_735",
	}
	expecteduriToInputNameMap := map[string][]string{
		"/components/bms_74b/vnom":     {"var_name1", "var_name1_copy"},
		"/components/feeder_52m1/v1":   {"var_name2"},
		"/components/feeder_52m1/id":   {"var_name3"},
		"/components/feeder_52u1/pmax": {"var_name4"},
		"/components/bms_74b/id":       {"var_name5"},
	}
	expecteduriToEchoObjectInputMap := map[string]map[int]int{
		"/components/feeder1": {0: 0},
		"/components/feeder2": {0: 1},
	}
	expecteduriToOutputNameMap := map[string][]string{
		"/some/output1/output1_cheetah":       {"output1_cheetah"},
		"/some/output1/output1_bobcat":        {"output1_bobcat"},
		"/some/output1/output1_lion":          {"output1_lion"},
		"/some/output2/output2_cheetah":       {"output2_cheetah"},
		"/some/output2/output2_bobcat":        {"output2_bobcat"},
		"/some/output2/output2_lion":          {"output2_lion"},
		"/some/output3/output3_cheetah":       {"output3_cheetah"},
		"/some/output3/output3_bobcat":        {"output3_bobcat"},
		"/some/output3/output3_lion":          {"output3_lion"},
		"/some/output1/timestamp":             {"output1"},
		"/some/output2/timestamp":             {"output2"},
		"/some/output3/timestamp":             {"output3"},
		"/some/level2/level2_output":          {"level2_output"},
		"/some/status/output/status":          {"enum_output"},
		"/some/status/output/status2":         {"bitfield_output"},
		"/some/output1/output1":               {"output1"},
		"/some/output2/output2":               {"output2"},
		"/some/output3/output3":               {"output3"},
		"/some/status/output/enum_output":     {"enum_output"},
		"/some/status/output/bitfield_output": {"bitfield_output"},
	}
	expectedUriElements := map[string]interface{}{
		"": map[string]interface{}{
			"components": map[string]interface{}{
				"bms_74b": map[string]interface{}{
					"vnom": map[string]interface{}{},
					"id":   map[string]interface{}{},
				},
				"feeder_52m1": map[string]interface{}{
					"v1": map[string]interface{}{},
					"id": map[string]interface{}{},
				},
				"feeder_52u1": map[string]interface{}{
					"pmax": map[string]interface{}{},
				},
				"feeder1": map[string]interface{}{
					"frequency1": map[string]interface{}{},
				},
				"feeder2": map[string]interface{}{
					"frequency2": map[string]interface{}{},
				},
				"sel_735": map[string]interface{}{
					"voltage_l3_l1": map[string]interface{}{},
					"f":             map[string]interface{}{},
					"f1":            map[string]interface{}{},
					"f2":            map[string]interface{}{},
				},
			},
			"some": map[string]interface{}{
				"output1": map[string]interface{}{
					"output1_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output1_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output1_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"output2": map[string]interface{}{
					"output2_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output2_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output2_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"output3": map[string]interface{}{
					"output3_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output3_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output3_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"level2": map[string]interface{}{
					"level2_output": map[string]interface{}{},
				},
				"status": map[string]interface{}{
					"output": map[string]interface{}{
						"status":  map[string]interface{}{},
						"status2": map[string]interface{}{},
					},
				},
			},
		},
		"/": map[string]interface{}{
			"components": map[string]interface{}{
				"bms_74b": map[string]interface{}{
					"vnom": map[string]interface{}{},
					"id":   map[string]interface{}{},
				},
				"feeder_52m1": map[string]interface{}{
					"v1": map[string]interface{}{},
					"id": map[string]interface{}{},
				},
				"feeder_52u1": map[string]interface{}{
					"pmax": map[string]interface{}{},
				},
				"feeder1": map[string]interface{}{
					"frequency1": map[string]interface{}{},
				},
				"feeder2": map[string]interface{}{
					"frequency2": map[string]interface{}{},
				},
				"sel_735": map[string]interface{}{
					"voltage_l3_l1": map[string]interface{}{},
					"f":             map[string]interface{}{},
					"f1":            map[string]interface{}{},
					"f2":            map[string]interface{}{},
				},
			},
			"some": map[string]interface{}{
				"output1": map[string]interface{}{
					"output1_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output1_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output1_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"output2": map[string]interface{}{
					"output2_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output2_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output2_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"output3": map[string]interface{}{
					"output3_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
					"output3_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
					"output3_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
					"timestamp":       map[string]interface{}{},
				},
				"level2": map[string]interface{}{
					"level2_output": map[string]interface{}{},
				},
				"status": map[string]interface{}{
					"output": map[string]interface{}{
						"status":  map[string]interface{}{},
						"status2": map[string]interface{}{},
					},
				},
			},
		},
		"/components": map[string]interface{}{
			"bms_74b": map[string]interface{}{
				"vnom": map[string]interface{}{},
				"id":   map[string]interface{}{},
			},
			"feeder_52m1": map[string]interface{}{
				"v1": map[string]interface{}{},
				"id": map[string]interface{}{},
			},
			"feeder_52u1": map[string]interface{}{
				"pmax": map[string]interface{}{},
			},
			"feeder1": map[string]interface{}{
				"frequency1": map[string]interface{}{},
			},
			"feeder2": map[string]interface{}{
				"frequency2": map[string]interface{}{},
			},
			"sel_735": map[string]interface{}{
				"voltage_l3_l1": map[string]interface{}{},
				"f":             map[string]interface{}{},
				"f1":            map[string]interface{}{},
				"f2":            map[string]interface{}{},
			},
		},
		"components": map[string]interface{}{
			"bms_74b": map[string]interface{}{
				"vnom": map[string]interface{}{},
				"id":   map[string]interface{}{},
			},
			"feeder_52m1": map[string]interface{}{
				"v1": map[string]interface{}{},
				"id": map[string]interface{}{},
			},
			"feeder_52u1": map[string]interface{}{
				"pmax": map[string]interface{}{},
			},
			"feeder1": map[string]interface{}{
				"frequency1": map[string]interface{}{},
			},
			"feeder2": map[string]interface{}{
				"frequency2": map[string]interface{}{},
			},
			"sel_735": map[string]interface{}{
				"voltage_l3_l1": map[string]interface{}{},
				"f":             map[string]interface{}{},
				"f1":            map[string]interface{}{},
				"f2":            map[string]interface{}{},
			},
		},
		"/components/sel_735": map[string]interface{}{
			"voltage_l3_l1": map[string]interface{}{},
			"f":             map[string]interface{}{},
			"f1":            map[string]interface{}{},
			"f2":            map[string]interface{}{},
		},
		"/components/sel_735/voltage_l3_l1": map[string]interface{}{},
		"/components/sel_735/f":             map[string]interface{}{},
		"/components/sel_735/f1":            map[string]interface{}{},
		"/components/sel_735/f2":            map[string]interface{}{},
		"/components/bms_74b": map[string]interface{}{
			"vnom": map[string]interface{}{},
			"id": map[string]interface{}{
				"enabled": map[string]interface{}{},
				"scale":   map[string]interface{}{},
			},
		},
		"/components/feeder_52m1": map[string]interface{}{
			"v1": map[string]interface{}{},
			"id": map[string]interface{}{},
		},
		"/components/feeder_52m1/v1": map[string]interface{}{},
		"/components/feeder_52m1/id": map[string]interface{}{},
		"/components/feeder_52u1": map[string]interface{}{
			"pmax": map[string]interface{}{},
		},
		"/components/feeder_52u1/pmax": map[string]interface{}{},
		"/components/feeder1": map[string]interface{}{
			"frequency1": map[string]interface{}{},
		},
		"/components/feeder1/frequency1": map[string]interface{}{},
		"/components/feeder2": map[string]interface{}{
			"frequency2": map[string]interface{}{},
		},
		"/components/feeder2/frequency2": map[string]interface{}{},
		"/components/bms_74b/vnom":       map[string]interface{}{},
		"/components/bms_74b/id": map[string]interface{}{
			"enabled": map[string]interface{}{},
			"scale":   map[string]interface{}{},
		},
		"/components/bms_74b/id/enabled": map[string]interface{}{},
		"/components/bms_74b/id@enabled": map[string]interface{}{},
		"/components/bms_74b/id/scale":   map[string]interface{}{},
		"/components/bms_74b/id@scale":   map[string]interface{}{},
		"/some": map[string]interface{}{
			"output1": map[string]interface{}{
				"output1_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output1_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output1_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"output2": map[string]interface{}{
				"output2_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output2_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output2_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"output3": map[string]interface{}{
				"output3_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output3_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output3_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"level2": map[string]interface{}{
				"level2_output": map[string]interface{}{},
			},
			"status": map[string]interface{}{
				"output": map[string]interface{}{
					"status":  map[string]interface{}{},
					"status2": map[string]interface{}{},
				},
			},
		},
		"some": map[string]interface{}{
			"output1": map[string]interface{}{
				"output1_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output1_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output1_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"output2": map[string]interface{}{
				"output2_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output2_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output2_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"output3": map[string]interface{}{
				"output3_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
				"output3_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
				"output3_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
				"timestamp":       map[string]interface{}{},
			},
			"level2": map[string]interface{}{
				"level2_output": map[string]interface{}{},
			},
			"status": map[string]interface{}{
				"output": map[string]interface{}{
					"status":  map[string]interface{}{},
					"status2": map[string]interface{}{},
				},
			},
		},
		"/some/level2": map[string]interface{}{
			"level2_output": map[string]interface{}{},
		},
		"/some/level2/level2_output": map[string]interface{}{},
		"/some/status": map[string]interface{}{
			"output": map[string]interface{}{
				"status":  map[string]interface{}{},
				"status2": map[string]interface{}{},
			},
		},
		"/some/status/output": map[string]interface{}{
			"status":  map[string]interface{}{},
			"status2": map[string]interface{}{},
		},
		"/some/status/output/status":  map[string]interface{}{},
		"/some/status/output/status2": map[string]interface{}{},
		"/some/output1": map[string]interface{}{
			"output1_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
			"output1_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
			"output1_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
			"timestamp":       map[string]interface{}{},
		},
		"/some/output2": map[string]interface{}{
			"output2_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
			"output2_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
			"output2_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
			"timestamp":       map[string]interface{}{},
		},
		"/some/output3": map[string]interface{}{
			"output3_cheetah": map[string]interface{}{"scale": map[string]interface{}{}},
			"output3_bobcat":  map[string]interface{}{"scale": map[string]interface{}{}},
			"output3_lion":    map[string]interface{}{"scale": map[string]interface{}{}},
			"timestamp":       map[string]interface{}{},
		},
		"/some/output1/output1_cheetah": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output1/output1_bobcat": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output1/output1_lion": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output1/timestamp": map[string]interface{}{},
		"/some/output2/output2_cheetah": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output2/output2_bobcat": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output2/output2_lion": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output2/timestamp": map[string]interface{}{},
		"/some/output3/output3_cheetah": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output3/output3_bobcat": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output3/output3_lion": map[string]interface{}{
			"scale": map[string]interface{}{},
		},
		"/some/output3/timestamp":             map[string]interface{}{},
		"/some/output1/output1_cheetah/scale": map[string]interface{}{},
		"/some/output1/output1_bobcat/scale":  map[string]interface{}{},
		"/some/output1/output1_lion/scale":    map[string]interface{}{},
		"/some/output2/output2_cheetah/scale": map[string]interface{}{},
		"/some/output2/output2_bobcat/scale":  map[string]interface{}{},
		"/some/output2/output2_lion/scale":    map[string]interface{}{},
		"/some/output3/output3_cheetah/scale": map[string]interface{}{},
		"/some/output3/output3_bobcat/scale":  map[string]interface{}{},
		"/some/output3/output3_lion/scale":    map[string]interface{}{},
	}

	GetSubscribeUris()

	if len(SubscribeUris) != len(expectedSubscribeUris) {
		t.Errorf("Expected %d subscribe URIs but only got %d\n", len(expectedSubscribeUris), len(SubscribeUris))
	}
	for _, subUri := range expectedSubscribeUris {
		if !stringInSlice(SubscribeUris, subUri) {
			t.Errorf("Expected subscribe URI [%s] but did not get\n", subUri)
		}
	}
	for _, subUri := range SubscribeUris {
		if !stringInSlice(expectedSubscribeUris, subUri) {
			t.Errorf("Got unexpected subscribe uri [%s]\n", subUri)
		}
	}
	if len(uriToInputNameMap) != len(expecteduriToInputNameMap) {
		t.Errorf("Expected %d URIs-to-input-name but only got %d\n", len(expecteduriToInputNameMap), len(uriToInputNameMap))
	}
	for subUri, inputNameList := range expecteduriToInputNameMap {
		if inputNameList2, ok := uriToInputNameMap[subUri]; !ok {
			t.Errorf("Expected subscribe URI [%s] in uriToInputNameMap but did not get\n", subUri)
		} else {
			for _, inputName := range inputNameList {
				if !stringInSlice(inputNameList2, inputName) {
					t.Errorf("Expected uri:input [%s : %s] in uriToInputNameMap but did not get\n", subUri, inputName)
				}
			}
		}
	}
	for subUri, inputNameList := range uriToInputNameMap {
		if inputNameList2, ok := expecteduriToInputNameMap[subUri]; !ok {
			t.Errorf("Unexpected subscribe URI [%s] in uriToInputNameMap\n", subUri)
		} else {
			for _, inputName := range inputNameList {
				if !stringInSlice(inputNameList2, inputName) {
					t.Errorf("Unexpected uri:input [%s : %s] in uriToInputNameMap\n", subUri, inputName)
				}
			}
		}
	}
	if len(uriToEchoObjectInputMap) != len(expecteduriToEchoObjectInputMap) {
		t.Errorf("Expected %d URIs-to-echo-object-input-name but only got %d\n", len(expecteduriToEchoObjectInputMap), len(uriToEchoObjectInputMap))
	}
	for subUri, echoInputMap := range expecteduriToEchoObjectInputMap {
		if echoInputMap2, ok := uriToEchoObjectInputMap[subUri]; !ok {
			t.Errorf("Expected subscribe URI [%s] in uriToEchoObjectInputNameMap but did not get\n", subUri)
		} else {
			for key, inputName := range echoInputMap {
				if _, ok = echoInputMap2[key]; !ok || inputName != echoInputMap2[key] {
					t.Errorf("Expected uri:echo_idx:input_idx [%s : %d : %d] in uriToEchoObjectInputNameMap but did not get\n", subUri, key, inputName)
				}
			}
		}
	}
	for subUri, echoInputMap := range uriToEchoObjectInputMap {
		if echoInputMap2, ok := expecteduriToEchoObjectInputMap[subUri]; !ok {
			t.Errorf("Unexpected subscribe URI [%s] in uriToEchoObjectInputNameMap\n", subUri)
		} else {
			for key, inputName := range echoInputMap {
				if _, ok = echoInputMap2[key]; !ok || inputName != echoInputMap2[key] {
					t.Errorf("Unexpected uri:echo_idx:input_idx [%s : %d : %d] in uriToEchoObjectInputNameMap but did not get\n", subUri, key, inputName)
				}
			}
		}
	}
	if len(uriToOutputNameMap) != len(expecteduriToOutputNameMap) {
		t.Errorf("Expected %d URIs-to-output-name but only got %d\n", len(expecteduriToOutputNameMap), len(uriToOutputNameMap))
	}
	for subUri, outputNames := range expecteduriToOutputNameMap {
		if outputNames2, ok := uriToOutputNameMap[subUri]; !ok {
			t.Errorf("Expected subscribe URI [%s] in uriToOutputNameMap but did not get\n", subUri)
		} else {
			// Ensure each individual name/output pair is present
			for _, outputName := range outputNames {
				nameFound := false
				for _, outputName2 := range outputNames2 {
					if outputName == outputName2 {
						nameFound = true
					}
				}
				if !nameFound {
					t.Errorf("Expected uri:input [%s : %s] in uriToOutputNameMap but did not get\n", subUri, outputName)
				}
			}
		}
	}
	if len(UriElements) != len(expectedUriElements) {
		t.Errorf("Expected %d uri elements but only got %d\n", len(expectedUriElements), len(UriElements))
	}
	for subUri := range expectedUriElements {
		if _, ok := UriElements[subUri]; !ok {
			t.Errorf("Expected subscribe URI [%s] in uriElements but did not get\n", subUri)
		}
	}
	for subUri := range UriElements {
		if _, ok := expectedUriElements[subUri]; !ok {
			t.Errorf("Unexpected subscribe URI [%s] in UriElements\n", subUri)
		}
	}
}

func Quiet() func() {
	null, _ := os.Open(os.DevNull)
	sout := os.Stdout
	serr := os.Stderr
	os.Stdout = null
	os.Stderr = null
	log.SetOutput(null)
	return func() {
		defer null.Close()
		os.Stdout = sout
		os.Stderr = serr
		log.SetOutput(os.Stderr)
	}
}
