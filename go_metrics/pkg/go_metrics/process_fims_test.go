package go_metrics

import (
	"fmt"
	// simdjson "github.com/minio/simdjson-go"
	// "log"
	// "os"
	// "strings"
	"fims"
	"strings"
	"testing"
)

func TestHandleDecodedMetricsInputValue(t *testing.T) {
	MetricsConfig.Inputs = map[string]Input{
		"input1": Input{
			Name:  "input1",
			Value: Union{tag: STRING, s: ""},
		},
	}
	InputScope = map[string][]Union{
		"input1": []Union{{tag: STRING, s: ""}},
	}
	inputToFilterExpression = map[string][]string{
		"input1": []string{"filter1"},
	}
	inputToMetricsExpression = map[string][]int{
		"filter1": []int{0},
		"input1":  []int{1},
	}
	expressionNeedsEval = map[int]bool{
		0: false,
		1: false,
	}
	elementValue = "some string"
	handleDecodedMetricsInputValue("input1")
	expectedInput1 := Input{Name: "input1", Value: Union{tag: STRING, s: ""}}
	expectedScopeInput1 := Union{tag: STRING, s: "some string"}
	if !compareInputs(MetricsConfig.Inputs["input1"], expectedInput1) {
		t.Errorf("MetricsConfig.Input[%s] not as expected after handleDecodedMetricsInputValue(%s)", "input1", "input1")
	}
	if InputScope["input1"][0] != expectedScopeInput1 {
		t.Errorf("scope[%s] not as expected after handleDecodedMetricsInputValue(%s)", "input1", "input1")
	}
	if expressionNeedsEval[0] != true {
		t.Errorf("expressionNeedsEval[%d] not as expected after handleDecodedMetricsInputValue(%s)", 0, "input1")
	}
	if expressionNeedsEval[1] != true {
		t.Errorf("expressionNeedsEval[%d] not as expected after handleDecodedMetricsInputValue(%s)", 1, "input1")
	}
}

func TestHandleDecodedMetricsAttributeValue(t *testing.T) {
	MetricsConfig.Inputs = map[string]Input{
		"input1": Input{
			Name:          "input1",
			Value:         Union{tag: STRING, s: ""},
			Attributes:    []string{"enabled"},
			AttributesMap: map[string]string{"enabled": "input1@enabled"},
		},
	}
	InputScope = map[string][]Union{
		"input1": []Union{{tag: STRING, s: ""}},
		"input1@enabled": []Union{{tag: BOOL, b: false}},
	}
	inputToFilterExpression = map[string][]string{
		"input1":         []string{"filter1"},
		"input1@enabled": []string{"filter2"},
	}
	inputToMetricsExpression = map[string][]int{
		"filter1":        []int{0},
		"input1":         []int{1},
		"filter2":        []int{2},
		"input1@enabled": []int{3},
	}
	expressionNeedsEval = map[int]bool{
		0: false,
		1: false,
		2: false,
		3: false,
	}
	elementValue = true
	handleDecodedMetricsAttributeValue("input1", "input1@enabled")
	expectedInput1 := Input{Name: "input1", Value: Union{tag: STRING, s: ""}, Attributes: []string{"enabled"}, AttributesMap: map[string]string{"enabled": "input1@enabled"}}
	expectedAttribute1 := Input{Value: Union{tag: BOOL, b: true}}
	if !compareInputs(MetricsConfig.Inputs["input1"], expectedInput1) {
		t.Errorf("MetricsConfig.Input[%s] not as expected after handleDecodedMetricsAttributeValue(%s, %s)", "input1", "input1", "input1@enabled")
	}
	if InputScope["input1"][0] != expectedInput1.Value {
		t.Errorf("scope[%s] not as expected after handleDecodedMetricsAttributeValue(%s, %s)", "input1", "input1", "input1@enabled")
	}
	if InputScope["input1@enabled"][0] != expectedAttribute1.Value {
		t.Errorf("scope[%s] not as expected after handleDecodedMetricsAttributeValue(%s, %s)", "input1@enabled", "input1", "input1@enabled")
	}
	for key, val := range expressionNeedsEval {
		if !val && key != 1 {
			t.Errorf("expressionNeedsEval[%d] not as expected after handleDecodedMetricsInputValue(%s, %s)", key, "input1", "input1@enabled")
		} else if val && key == 1 {
			t.Errorf("expressionNeedsEval[%d] not as expected after handleDecodedMetricsInputValue(%s, %s)", key, "input1", "input1@enabled")
		}
	}
}

type ProcessFimsTestsGlobal struct {
	inputToFilterExpression  map[string][]string
	inputToMetricsExpression map[string][]int
	uriToInputNameMap        map[string][]string
	uriToEchoObjectInputMap  map[string]map[int]int
	uriToOutputNameMap       map[string]string
	uriElements              map[string]interface{}
	allPossibleAttributes	map[string][]string
}
type ProcessFimsTest struct {
	inputMsg                    fims.FimsMsgRaw
	startingMetricsInputs       map[string]Input
	startingEcho                EchoObject
	startingScope               map[string][]Union
	startingExpressionNeedsEval map[int]bool
	endingMetricsInputs         map[string]Input
	endingScope                 map[string][]Union
	endingExpressionNeedsEval   map[int]bool
	endingEcho                  EchoObject
}

var GlobalConstants = ProcessFimsTestsGlobal{
	uriElements: map[string]interface{}{
		"": map[string]interface{}{
			"test": map[string]interface{}{
				"input1": map[string]interface{} {
					"attribute": map[string]interface{}{},
				},
				"input2": map[string]interface{} {
					
				},
			},
			"echo": map[string]interface{}{
				"test": map[string]interface{}{
					"input1": map[string]interface{} {
						"v1": map[string]interface{}{},
						"v2": map[string]interface{}{},
					},
					"input2": map[string]interface{} {
						
					},
				},
			},
		},
		"/": map[string]interface{}{
			"test": map[string]interface{}{
				"input1": map[string]interface{} {
					"attribute": map[string]interface{}{},
				},
				"input2": map[string]interface{} {
					
				},
			},
			"echo": map[string]interface{}{
				"test": map[string]interface{}{
					"input1": map[string]interface{} {
						"v1": map[string]interface{}{},
						"v2": map[string]interface{}{},
					},
					"input2": map[string]interface{} {
						
					},
				},
			},
		},
		"test": map[string]interface{}{
			"input1": map[string]interface{} {
				"attribute": map[string]interface{}{},
			},
			"input2": map[string]interface{} {
				
			},
		},
		"/test": map[string]interface{}{
			"input1": map[string]interface{} {
				"attribute": map[string]interface{}{},
			},
			"input2": map[string]interface{} {
				
			},
		},
		"/test/input1": map[string]interface{}{
			"attribute": map[string]interface{}{},
		},
		"/test/input1/attribute": map[string]interface{}{
		},
		"/test/input1@attribute": map[string]interface{}{
		},
		"/test/input2": map[string]interface{}{
		},
		"/echo": map[string]interface{}{
			"test1": map[string]interface{}{
				"input1": map[string]interface{} {
					"v1": map[string]interface{}{},
					"v2": map[string]interface{}{},
				},
				"input2": map[string]interface{} {
					
				},
			},
		},
		"echo": map[string]interface{}{
			"test1": map[string]interface{}{
				"input1": map[string]interface{} {
					"v1": map[string]interface{}{},
					"v2": map[string]interface{}{},
				},
				"input2": map[string]interface{} {
					
				},
			},
		},
		"/echo/test1": map[string]interface{}{
			"input1": map[string]interface{} {
				"v1": map[string]interface{}{},
				"v2": map[string]interface{}{},
			},
			"input2": map[string]interface{} {
			},
		},
		"/echo/test1/input1":map[string]interface{} {
			"v1": map[string]interface{}{},
			"v2": map[string]interface{}{},
		},
		"/echo/test1/input2":map[string]interface{} {
		},
		"/echo/test1/input1/v1":map[string]interface{} {
		},
		"/echo/test1/input1/v2":map[string]interface{} {
		},
	},
	inputToFilterExpression: map[string][]string{
		"input1":           []string{"filter1"},
		"input1@attribute": []string{"filter2"},
		"input2":           []string{"filter3"},
	},
	inputToMetricsExpression: map[string][]int{
		"input1":           []int{0},
		"filter1":          []int{1},
		"input1@attribute": []int{2},
		"filter2":          []int{3},
		"input2":           []int{4},
		"filter3":          []int{5},
	},
	uriToInputNameMap: map[string][]string{
		"/test/input1": []string{"input1"},
		"/test/input2": []string{"input2"},
	},
	uriToEchoObjectInputMap: map[string]map[int]int{
		"/echo/test1/input1": map[int]int{0: 0},
		"/echo/test2/input2": map[int]int{0: 1},
	},
	uriToOutputNameMap: map[string]string{},
	allPossibleAttributes: map[string][]string {
		"attribute":[]string{"input1@attribute"},
	},
}

var ProcessFimsTests = []ProcessFimsTest{
	ProcessFimsTest{ // fims_send -m set -u /test/input1 5
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1",
			Body:   []byte("5"),
			Frags:  []string{"test", "input1"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:  "input1",
				Uri:   "/test/input1",
				Value: Union{tag: INT, i: 0},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:  "input1",
				Uri:   "/test/input1",
				Value: Union{tag: INT, i: 0},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: true,
			1: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test/input1/attribute 5
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1/attribute",
			Body:   []byte("5"),
			Frags:  []string{"test", "input1", "attribute"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{tag: FLOAT, f: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: false,
			1: true,
			2: true,
			3: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test/input1@attribute 5
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1@attribute",
			Body:   []byte("5"),
			Frags:  []string{"test", "input1@attribute"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{tag: FLOAT, f: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: false,
			1: true,
			2: true,
			3: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test/input1 '{"value":5}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1",
			Body:   []byte("{\"value\":5}"),
			Frags:  []string{"test", "input1"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:  "input1",
				Uri:   "/test/input1",
				Value: Union{tag: INT, i: 0},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:  "input1",
				Uri:   "/test/input1",
				Value: Union{tag: INT, i: 0},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: true,
			1: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test/input1 '{"value":5, "attribute":5}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1",
			Body:   []byte("{\"value\":5, \"attribute\":5}"),
			Frags:  []string{"test", "input1"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 5}},
			"input1@attribute": []Union{{tag: INT, i: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: true,
			1: true,
			2: true,
			3: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test/input1 '{"attribute":5}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test/input1",
			Body:   []byte("{\"attribute\":5}"),
			Frags:  []string{"test", "input1"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{tag: INT, i: 5}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: false,
			1: true,
			2: true,
			3: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test '{"input1": {"value":5, "attribute":5}}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test",
			Body:   []byte("{\"input1\":{\"value\":20, \"attribute\":20}}"),
			Frags:  []string{"test"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 20}},
			"input1@attribute": []Union{{tag: INT, i: 20}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: true,
			1: true,
			2: true,
			3: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /test '{"input1": 25, "input2": 30}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/test",
			Body:   []byte("{\"input1\":25, \"input2\":30}"),
			Frags:  []string{"test"},
		},
		startingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
			"input2": Input{
				Name:  "input2",
				Uri:   "/test/input2",
				Value: Union{tag: FLOAT, f: 0},
			},
		},
		startingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 0}},
			"input1@attribute": []Union{{}},
			"input2": []Union{{tag: FLOAT, f: 0}},
		},
		startingExpressionNeedsEval: map[int]bool{
			0: false,
			1: false,
			2: false,
			3: false,
			4: false,
			5: false,
		},
		endingMetricsInputs: map[string]Input{
			"input1": Input{
				Name:       "input1",
				Uri:        "/test/input1",
				Value:      Union{tag: INT, i: 0},
				Attributes: []string{"attribute"},
				AttributesMap: map[string]string{
					"attribute": "input1@attribute",
				},
			},
			"input2": Input{
				Name:  "input2",
				Uri:   "/test/input2",
				Value: Union{tag: FLOAT, f: 0},
			},
		},
		endingScope: map[string][]Union{
			"input1": []Union{{tag: INT, i: 25}},
			"input1@attribute": []Union{{}},
			"input2": []Union{{tag: FLOAT, f: 30}},
		},
		endingExpressionNeedsEval: map[int]bool{
			0: true,
			1: true,
			2: false,
			3: false,
			4: true,
			5: true,
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1/v1 5
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1/v1",
			Body:   []byte("5"),
			Frags:  []string{"echo", "test1", "input1", "v1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   float64(5),
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1 '{"v1":6, "v2": 7}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1",
			Body:   []byte("{\"v1\":6, \"v2\":7}"),
			Frags:  []string{"echo", "test1", "input1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(6),
				"voltage2":   int64(7),
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1 '{"v1":{"value":8}, "v2": {"value":9}}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1",
			Body:   []byte("{\"v1\":{\"value\":8}, \"v2\":{\"value\":9}}"),
			Frags:  []string{"echo", "test1", "input1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(8),
				"voltage2":   int64(9),
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1 '{"v1":{"value":8}, "v2": {"value":9}}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1",
			Body:   []byte("{\"v1\":{\"value\":8}, \"v2\":{\"value\":9}}"),
			Frags:  []string{"echo", "test1", "input1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "clothed",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "clothed",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(8),
				"voltage2":   int64(9),
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1 '{"v1":{"value":8}, "v2": {"value":9}}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1",
			Body:   []byte("{\"v1\":{\"value\":8}, \"v2\":{\"value\":9}}"),
			Frags:  []string{"echo", "test1", "input1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "naked",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "naked",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(8),
				"voltage2":   int64(9),
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1/v1 '{"value":33}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1/v1",
			Body:   []byte("{\"value\":33}"),
			Frags:  []string{"echo", "test1", "input1", "v1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "naked",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "naked",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(33),
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1/v1 '{"value":33}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1/v1",
			Body:   []byte("{\"value\":33}"),
			Frags:  []string{"echo", "test1", "input1", "v1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "clothed",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Format:      "clothed",
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(33),
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
	ProcessFimsTest{ // fims_send -m set -u /echo/test1/input1/v1 '{"value":33}'
		inputMsg: fims.FimsMsgRaw{
			Method: "set",
			Uri:    "/echo/test1/input1/v1",
			Body:   []byte("{\"value\":31}"),
			Frags:  []string{"echo", "test1", "input1", "v1"},
		},
		startingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   nil,
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
		endingEcho: EchoObject{
			PublishUri:  "/echo1",
			PublishRate: 2000,
			Inputs: []EchoInput{
				EchoInput{
					Uri: "/echo/test1/input1",
					Registers: map[string]string{
						"voltage1": "v1",
						"voltage2": "v2",
					},
				},
				EchoInput{
					Uri: "/echo/test2/input2",
					Registers: map[string]string{
						"frequency1": "f1",
						"frequency2": "f2",
					},
				},
			},
			Echo: map[string]interface{}{
				"voltage1":   int64(31),
				"voltage2":   nil,
				"frequency1": nil,
				"frequency2": nil,
			},
		},
	},
}

func TestProcessFims(t *testing.T) {
	for _, test := range ProcessFimsTests {
		MetricsConfig = MetricsFile{}
		testName := fmt.Sprintf("fims_send -m %s -u %s %s", strings.ToLower(test.inputMsg.Method), test.inputMsg.Uri, test.inputMsg.Body)
		UriElements = GlobalConstants.uriElements
		allPossibleAttributes = GlobalConstants.allPossibleAttributes
		MetricsConfig.Echo = []EchoObject{test.startingEcho}
		uriToEchoObjectInputMap = GlobalConstants.uriToEchoObjectInputMap
		uriToInputNameMap = GlobalConstants.uriToInputNameMap
		uriToOutputNameMap = GlobalConstants.uriToOutputNameMap
		MetricsConfig.Inputs = test.startingMetricsInputs
		InputScope = test.startingScope
		inputToFilterExpression = GlobalConstants.inputToFilterExpression
		inputToMetricsExpression = GlobalConstants.inputToMetricsExpression
		expressionNeedsEval = test.startingExpressionNeedsEval
		ProcessFims(test.inputMsg)
		for key, input := range MetricsConfig.Inputs {
			if expectedInput, ok := test.endingMetricsInputs[key]; !ok {
				t.Errorf("%s: unexpected input '%s' in MetricsConfig.Inputs", testName, key)
			} else {
				if !compareInputs(input, expectedInput) {
					t.Errorf("%s: resulting input '%s' in MetricsConfig.Inputs not as expected", testName, key)
				}
			}
		}
		for key, inputs := range InputScope {
			if expectedInputs, ok := test.endingScope[key]; !ok {
				t.Errorf("%s: unexpected input '%s' in MetricsConfig.Inputs", testName, key)
			} else {
				for i, input := range inputs {
					if input != expectedInputs[i] {
						t.Errorf("%s: resulting input '%d' in scope[%s] not as expected. Got %v. Wanted %v.", testName, i, key, input, expectedInputs[i])
					}
				}
			}
		}
		for key, val := range expressionNeedsEval {
			if val != test.endingExpressionNeedsEval[key] {
				t.Errorf("%s: expression %d needs eval [%v] not as expected [%v]", testName, key, val, test.endingExpressionNeedsEval[key])
			}
		}
		if !compareEcho(MetricsConfig.Echo[0], test.endingEcho) {
			t.Errorf("%s: echo object not as expected", testName)
		}
	}
}
