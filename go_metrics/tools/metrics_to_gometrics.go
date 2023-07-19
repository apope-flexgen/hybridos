package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"go_metrics"
	"log"
	"os"
	"path/filepath"
)

type OldMetricsFile struct {
	PublishRate int64                  `json:"publishRate"`
	ListenRate  int64                  `json:"listenRate"`
	MetricsUri  string                 `json:"metricsUri"`
	PublishUris []OldMetricsPublishUri `json:"publishUris"`
}

type OldMetricsPublishUri struct {
	Uri     string                   `json:"uri"`
	Naked   interface{}              `json:"naked"`
	Metrics []map[string]interface{} `json:"metrics"`
}

// type OldMetricsObject struct {
// 	Id        string                 `json:"id"`
// 	Inputs    []OldMetricsIO         `json:"inputs"`
// 	Outputs   []OldMetricsIO         `json:"outputs"`
// 	Operation string                 `json:"operation"`
// 	Param     map[string]interface{} `json:"param"`
// }

type OldMetricsIO struct {
	Uri string `json:"uri"`
	Id  string `json:"id"`
}

type NewMetricsFile struct {
	Meta    map[string]interface{} `json:"meta,omitempty"`    // metadata for the file
	Inputs  map[string]NewInput    `json:"inputs,omitempty"`  // a map of variable names to the uri locations they will come from
	Filters map[string]interface{} `json:"filters,omitempty"` // a map of variable names to apply 1) run-time changes to metrics or 2) execute regular expressions on variable names
	Outputs map[string]NewOutput   `json:"outputs,omitempty"` // a map of variable names that get sent to specific uris with optional flags
	Metrics []NewMetricsObject     `json:"metrics,omitempty"` // an array of the calculations that we want to perform by default
	Echo    []NewEchoObject        `json:"echo,omitempty"`    // an array of things to echo; basically the same config as the original echo
}

// the uris that we will look for in incoming fims data
type NewInput struct {
	Uri        string      `json:"uri,omitempty"`
	Type       string      `json:"type,omitempty"`
	Attributes []string    `json:"attributes,omitempty"`
	Default    interface{} `json:"default,omitempty"`
}

// the uris that we will publish to for outgoing fims data
type NewOutput struct {
	Uri        string                 `json:"uri,omitempty"`
	Flags      []string               `json:"flags,omitempty"`
	Attributes map[string]interface{} `json:"attributes,omitempty"`
}

type NewMetricsObject struct {
	Type       go_metrics.DataType `json:"type"`                 // the default value for an output - also specifies the data type
	Outputs    []string            `json:"outputs,omitempty"`    // the output variable to publish to (e.g. "output_1" which would have been mapped to an Output struct)
	Expression string              `json:"expression,omitempty"` // an expression to evaluate
}

type NewEchoObject struct {
	PublishUri  string                 `json:"uri,omitempty"`         // the uri to publish to
	PublishRate int64                  `json:"publishRate,omitempty"` // the rate to publish at
	Heartbeat   string                 `json:"heartbeat,omitempty"`   // still unsure what this does...
	Inputs      []NewEchoInput         `json:"inputs,omitempty"`      // where to look for values
	Echo        map[string]interface{} `json:"echo,omitempty"`        // values to give to variables by default; overwritten if found in inputs
}

type NewEchoInput struct {
	Uri       string            `json:"uri,omitempty"`        // the uri to look at
	Registers map[string]string `json:"registers,omiteempty"` // a map of new variable names (published to output uri) to old variable names (found in the input uri)
}

func main() {

	configPath, err := filepath.Abs(os.Args[1])
	if err != nil {
		log.Fatal("Could not resolve filepath")
	}

	// read in the file and unmarshal it into a MetricsFile struct
	data, err := os.ReadFile(configPath)
	if err != nil {
		log.Fatalf("Error reading json file: %s", err)
	}

	var oldMetricsFile OldMetricsFile
	err = json.Unmarshal(data, &oldMetricsFile)
	if err != nil {
		log.Fatalf("Error unmarshalling json file: %s", err)
	}

	var newMetricsFile NewMetricsFile
	newMetricsFile.Echo = make([]NewEchoObject, 0)
	newMetricsFile.Outputs = make(map[string]NewOutput, 0)
	newMetricsFile.Inputs = make(map[string]NewInput, 0)
	newMetricsFile.Metrics = make([]NewMetricsObject, 0)

	for _, pubUri := range oldMetricsFile.PublishUris {
		for _, metric := range pubUri.Metrics {
			id, _ := metric["id"].(string)
			initialInput, hasInitialInput := metric["initialInput"]
			// input
			for _, oldInput := range metric["inputs"].([]interface{}) {
				oldInputMap := oldInput.(map[string]interface{})
				if hasInitialInput {
					dtype := ""
					switch initialInput.(type) {
					case string:
						dtype = "string"
					case float64:
						dtype = "float"
					case bool:
						dtype = "bool"
					default:
						dtype = "float"
					}
					if _, alreadyExists := newMetricsFile.Inputs[oldInputMap["id"].(string)]; alreadyExists {
						newMetricsFile.Inputs[go_metrics.GetUriElement(oldInputMap["uri"].(string))+"_"+oldInputMap["id"].(string)] = NewInput{Uri: fmt.Sprintf("%s/%s", oldInputMap["uri"].(string), oldInputMap["id"].(string)), Type: dtype, Default: initialInput}
					} else {
						newMetricsFile.Inputs[oldInputMap["id"].(string)] = NewInput{Uri: fmt.Sprintf("%s/%s", oldInputMap["uri"].(string), oldInputMap["id"].(string)), Type: dtype, Default: initialInput}
					}
				} else {
					if _, alreadyExists := newMetricsFile.Inputs[oldInputMap["id"].(string)]; alreadyExists {
						newMetricsFile.Inputs[go_metrics.GetUriElement(oldInputMap["uri"].(string))+"_"+oldInputMap["id"].(string)] = NewInput{Uri: fmt.Sprintf("%s/%s", oldInputMap["uri"].(string), oldInputMap["id"].(string)), Type: "float"}
					} else {
						newMetricsFile.Inputs[oldInputMap["id"].(string)] = NewInput{Uri: fmt.Sprintf("%s/%s", oldInputMap["uri"].(string), oldInputMap["id"].(string)), Type: "float"}
					}

				}
			}

			// output
			flags := make([]string, 0)
			naked, isString := pubUri.Naked.(string)
			if isString && naked != "true" {
				flags = append(flags, "clothed")
			}
			nakedBool, isBool := pubUri.Naked.(bool)
			if isBool && !nakedBool {
				flags = append(flags, "clothed")
			}

			attributes := make(map[string]interface{}, 0)
			for key, value := range metric {
				if key != "id" && key != "inputs" && key != "operation" && key != "param" && key != "outputs" && key != "initialValue" && key != "initialInput" {
					attributes[key] = value
				}
			}
			if len(flags) > 0 && len(attributes) > 0 {
				newMetricsFile.Outputs[id] = NewOutput{Uri: pubUri.Uri, Flags: flags, Attributes: attributes}
			} else if len(flags) > 0 {
				newMetricsFile.Outputs[id] = NewOutput{Uri: pubUri.Uri, Flags: flags}
			} else if len(attributes) > 0 {
				newMetricsFile.Outputs[id] = NewOutput{Uri: pubUri.Uri, Attributes: attributes}
			} else {
				newMetricsFile.Outputs[id] = NewOutput{Uri: pubUri.Uri}
			}

			// metrics expression hahahahaha *cry*
			newMetricsFile.Metrics = append(newMetricsFile.Metrics, translateMetricsExpression(pubUri.Uri, metric))

		}
	}
	outputFile, _ := json.MarshalIndent(newMetricsFile, "", "    ")
	outputFile = bytes.ReplaceAll(outputFile, []byte(",\n            "), []byte(", "))
	outputFile = bytes.ReplaceAll(outputFile, []byte("\n        },"), []byte(" },"))
	outputFile = bytes.ReplaceAll(outputFile, []byte("\n            "), []byte(" "))
	outputFile = bytes.ReplaceAll(outputFile, []byte("[     "), []byte("[ "))
	outputFile = bytes.ReplaceAll(outputFile, []byte("\n        }\n    }"), []byte(" }\n    }"))
	outputFile = bytes.ReplaceAll(outputFile, []byte("{\n            \"uri\":"), []byte("{ \"uri\":"))
	outputFile = bytes.ReplaceAll(outputFile, []byte(" }, {"), []byte(" },\n            {"))
	for i, _ := range outputFile[:len(outputFile)-6] {
		if string(outputFile[i:i+6]) == "\\u0026" {
			outputFile = append(outputFile[:i], outputFile[i+5:]...)
			outputFile[i] = byte('&')
		}
		if string(outputFile[i:i+6]) == "\\u003c" {
			outputFile = append(outputFile[:i], outputFile[i+5:]...)
			outputFile[i] = byte('<')
		}
		if string(outputFile[i:i+6]) == "\\u003e" {
			outputFile = append(outputFile[:i], outputFile[i+5:]...)
			outputFile[i] = byte('>')
		}
	}
	f, _ := os.OpenFile("new_metrics_file.json", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
	defer f.Close()
	f.Write(outputFile)

}

// publish uri is unused right now, but may be needed later if we have multiple metrics with the same id
// but different publish uris
func translateMetricsExpression(publishUri string, oldMetric map[string]interface{}) NewMetricsObject {
	newMetric := NewMetricsObject{}
	operation := oldMetric["operation"].(string)
	expression := ""
	inputsInterface := oldMetric["inputs"].([]interface{})
	inputs := make([]OldMetricsIO, 0)
	inputIds := make(map[string]int, len(inputsInterface))
	if _, ok := oldMetric["initialValue"]; ok {
		switch oldMetric["initialValue"].(type) {
		case string:
			newMetric.Type = go_metrics.STRING
		case float64:
			newMetric.Type = go_metrics.FLOAT
		case bool:
			newMetric.Type = go_metrics.BOOL
		default:
			newMetric.Type = go_metrics.NIL
		}
	}
	for _, inputInterface := range inputsInterface {
		inputMap := inputInterface.(map[string]interface{})
		inputId := inputMap["id"].(string)
		inputUri := inputMap["uri"].(string)
		if _, alreadyExists := inputIds[inputId]; alreadyExists {
			inputIds[go_metrics.GetUriElement(inputUri)+"_"+inputId] = 1
			inputs = append(inputs, OldMetricsIO{Uri: inputUri, Id: go_metrics.GetUriElement(inputUri) + "_" + inputId})
		} else {
			inputIds[inputId] = 1
			inputs = append(inputs, OldMetricsIO{Uri: inputUri, Id: inputId})
		}
	}
	_, hasParam := oldMetric["param"]
	var parameters map[string]interface{}
	if hasParam {
		parameters = oldMetric["param"].(map[string]interface{})
	}
	switch operation {
	case "max":
		expression = "Max("
		for _, input := range inputs[:len(inputs)-1] {
			expression += input.Id + ", "
		}
		expression += inputs[len(inputs)-1].Id + ")"
	case "min":
		expression = "Min("
		for _, input := range inputs[:len(inputs)-1] {
			expression += input.Id + ", "
		}
		expression += inputs[len(inputs)-1].Id + ")"
	case "sum":
		expression = ""
		_, hasOffset := parameters["offset"]
		if hasOffset {
			expression += parameters["offset"].(string)
		}
		_, hasOps := parameters["operations"]
		if hasOps {
			ops := parameters["operations"].(string)
			if len(ops) == len(inputs) {
				for i, c := range ops {
					if fmt.Sprintf("%c", c) == "+" {
						if i == 0 && !hasOffset {
							expression += inputs[0].Id
						} else {
							expression += " + " + inputs[i].Id
						}
					} else {
						expression += " - " + inputs[i].Id
					}
				}
			}
		} else {
			if hasOffset {
				for _, input := range inputs {
					expression += " + " + input.Id
				}
			} else {
				expression += inputs[0].Id
				for _, input := range inputs[1:] {
					expression += " + " + input.Id
				}
			}
		}
	case "add":
		for _, input := range inputs[:len(inputs)-1] {
			expression += input.Id + " + "
		}
		expression += inputs[len(inputs)-1].Id
	case "product":
		expression = ""
		_, hasOps := parameters["operations"]
		if hasOps {
			ops := parameters["operations"].(string)
			if len(ops) == len(inputs) {
				for i, c := range ops {
					if fmt.Sprintf("%c", c) == "*" {
						if i == 0 {
							expression += inputs[0].Id
						} else {
							expression += "*" + inputs[i].Id
						}
					} else {
						expression += "/" + inputs[i].Id
					}
				}
			}
		} else {
			expression += inputs[0].Id
			for _, input := range inputs[1:] {
				expression += "*" + input.Id
			}
		}
		_, hasUpper := parameters["upperLimit"]
		_, hasLower := parameters["lowerLimit"]
		_, hasGain := parameters["gain"]
		if hasLower {
			lower := parameters["lowerLimit"]
			expression = fmt.Sprintf("Max(%s, %v)", expression, lower)
		}
		if hasUpper {
			upper := parameters["upperLimit"]
			expression = fmt.Sprintf("Min(%s, %v)", expression, upper)
		}
		if hasGain {
			gain := parameters["gain"]
			expression = fmt.Sprintf("%v*%s", gain, expression)
		}
	case "average":
		expression = "Average(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += "," + input.Id
		}
		expression += ")"
	case "integrate":
		_, hasTimescale := parameters["timescale"]
		if hasTimescale {
			timescale := parameters["timescale"]
			expression = fmt.Sprintf("IntegrateOverTimescale(%s, %v)", inputs[0].Id, timescale)
		} else {
			expression = fmt.Sprintf("Integrate(%s)", inputs[0].Id)
		}
		_, hasMinuteReset := parameters["minuteReset"]
		if hasMinuteReset {
			fmt.Println("Found minute reset. New metrics cannot handle minuteResets with integrate commands. Ignoring value.")
		}
		_, hasMinuteOffset := parameters["minuteOffset"]
		if hasMinuteOffset {
			fmt.Println("Found minute offset. New metrics cannot handle minuteOffsets with integrate commands. Ignoring value.")
		}
		_, hasAbs := parameters["abs"]
		if hasAbs {
			fmt.Println("Found absolute value. New metrics cannot handle abs with integrate commands. Ignoring value.")
		}
	case "and":
		newMetric.Type = go_metrics.BOOL
		expression = "(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += " && " + input.Id
		}
		expression += ")"
	case "or":
		newMetric.Type = go_metrics.BOOL
		expression = "(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += " || " + input.Id
		}
		expression += ")"
	case "rss":
		expression = "Rss(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += "," + input.Id
		}
		expression += ")"
	case "srff":
		expression = fmt.Sprintf("Srff(%s, %s)", inputs[0].Id, inputs[1].Id)
	case "compare":
		newMetric.Type = go_metrics.BOOL
		_, hasOp := parameters["operation"]
		_, hasRef := parameters["reference"]
		var op string
		if hasOp {
			op = parameters["operation"].(string)
		} else {
			op = "eq"
		}
		switch op {
		case "eq":
			if hasRef {
				reference := parameters["reference"]
				expression = fmt.Sprintf("(%v == %s", reference, inputs[0].Id)
			} else {
				expression = "(" + inputs[0].Id
			}
			for _, input := range inputs[1:] {
				expression += " == " + input.Id
			}
			expression += ")"
		case "ne":
			if hasRef {
				reference := parameters["reference"]
				expression = fmt.Sprintf("(%v != %s", reference, inputs[0].Id)
			} else {
				expression = "(" + inputs[0].Id
			}
			for _, input := range inputs[1:] {
				expression += " != " + input.Id
			}
			expression += ")"
		case "lt":
			expression = "(" + inputs[0].Id
			for _, input := range inputs[1:] {
				expression += " < " + input.Id
			}
			if hasRef {
				reference := parameters["reference"]
				expression += fmt.Sprintf(" < %v)", reference)
			} else {
				expression += ")"
			}
		case "gt":
			expression = "(" + inputs[0].Id
			for _, input := range inputs[1:] {
				expression += " > " + input.Id
			}
			if hasRef {
				reference := parameters["reference"]
				expression += fmt.Sprintf(" > %v)", reference)
			} else {
				expression += ")"
			}
		case "lte":
			expression = "(" + inputs[0].Id
			for _, input := range inputs[1:] {
				expression += " <= " + input.Id
			}
			if hasRef {
				reference := parameters["reference"]
				expression += fmt.Sprintf(" <= %v)", reference)
			} else {
				expression += ")"
			}
		case "gte":
			expression = "(" + inputs[0].Id
			for _, input := range inputs[1:] {
				expression += " >= " + input.Id
			}
			if hasRef {
				reference := parameters["reference"]
				expression += fmt.Sprintf(" >= %v)", reference)
			} else {
				expression += ")"
			}
		}
	case "compareand":
		newMetric.Type = go_metrics.BOOL
		_, hasOp := parameters["operation"]
		var op string
		if hasOp {
			op = parameters["operation"].(string)
		} else {
			op = "eq"
		}
		reference := parameters["reference"]
		switch op {
		case "eq":
			expression = fmt.Sprintf("((%v == %s)", reference, inputs[0].Id)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%v == %s)", reference, input.Id)
			}
			expression += ")"
		case "ne":
			expression = fmt.Sprintf("((%v != %s)", reference, inputs[0].Id)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%v != %s)", reference, input.Id)
			}
			expression += ")"
		case "lt":
			expression = fmt.Sprintf("((%s < %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%s < %v)", input.Id, reference)
			}
			expression += ")"
		case "gt":
			expression = fmt.Sprintf("((%s > %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%s > %v)", input.Id, reference)
			}
			expression += ")"
		case "lte":
			expression = fmt.Sprintf("((%s <= %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%s <= %v)", input.Id, reference)
			}
			expression += ")"
		case "gte":
			expression = fmt.Sprintf("((%s >= %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" && (%s >= %v)", input.Id, reference)
			}
			expression += ")"
		}
	case "compareor":
		newMetric.Type = go_metrics.BOOL
		_, hasOp := parameters["operation"]
		var op string
		if hasOp {
			op = parameters["operation"].(string)
		} else {
			op = "eq"
		}
		reference := parameters["reference"]
		switch op {
		case "eq":
			expression = fmt.Sprintf("((%v == %s)", reference, inputs[0].Id)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%v == %s)", reference, input.Id)
			}
			expression += ")"
		case "ne":
			expression = fmt.Sprintf("((%v != %s)", reference, inputs[0].Id)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%v != %s)", reference, input.Id)
			}
			expression += ")"
		case "lt":
			expression = fmt.Sprintf("((%s < %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%s < %v)", input.Id, reference)
			}
			expression += ")"
		case "gt":
			expression = fmt.Sprintf("((%s > %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%s > %v)", input.Id, reference)
			}
			expression += ")"
		case "lte":
			expression = fmt.Sprintf("((%s <= %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%s <= %v)", input.Id, reference)
			}
			expression += ")"
		case "gte":
			expression = fmt.Sprintf("((%s >= %v)", inputs[0].Id, reference)
			for _, input := range inputs[1:] {
				expression += fmt.Sprintf(" || (%s >= %v)", input.Id, reference)
			}
			expression += ")"
		}
	case "select":
		var trueCase interface{}
		var falseCase interface{}
		_, hasTrueCase := parameters["trueCase"]
		_, hasFalseCase := parameters["falseCase"]
		if hasTrueCase {
			trueCase = parameters["trueCase"]
		} else {
			trueCase = inputs[1].Id
		}
		if hasFalseCase {
			falseCase = parameters["falseCase"]
		} else {
			if len(inputs) > 2 {
				falseCase = inputs[2].Id
			}
		}
		if falseCase != nil {
			expression = fmt.Sprintf("IfThenElse(%v, %v, %v)", inputs[0].Id, trueCase, falseCase)
		} else {
			expression = fmt.Sprintf("IfThenElse(%v, %v)", inputs[0].Id, trueCase)
		}
	case "quadtosigned":
		newMetric.Type = go_metrics.FLOAT
		expression = fmt.Sprintf("QuadToSigned(%v)", inputs[0].Id)
	case "signedtoquad":
		newMetric.Type = go_metrics.FLOAT
		expression = fmt.Sprintf("SignedToQuad(%v)", inputs[0].Id)
	case "pulse":
		var resetVal interface{}
		if len(inputs) == 2 {
			resetVal = inputs[1].Id
		} else {
			resetVal = false
		}
		if _, hasTimeout := parameters["time"]; hasTimeout {
			expression = fmt.Sprintf("Pulse(%v, %v, %v)", inputs[0].Id, resetVal, parameters["time"])
		} else {
			expression = fmt.Sprintf("Pulse(%v, %v, 0)", inputs[0].Id, resetVal)
		}

		if _, hasInvert := parameters["invert"]; hasInvert {
			expression = "Not(" + expression + ")"
		}
	case "selectn":
		expression = "SelectN(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += ", " + input.Id
		}
		expression += ")"
	case "selectorn":
		expression = "SelectorN(" + inputs[0].Id
		for _, input := range inputs[1:] {
			expression += ", " + input.Id
		}
		expression += ")"
	case "currentTimeMilliseconds":
		newMetric.Type = go_metrics.INT
		expression = "CurrentTimeMilliseconds()"
	case "compareMillisecondsToCurrentTime":
		if hasParam {
			fmt.Println("unhandled parameters for compareMillisecondsToCurrentTime")
			break
		}
		expression = "MillisecondsSince(" + inputs[0].Id + ")"
	case "millisecondsToRFC3339":
		newMetric.Type = go_metrics.STRING
		if hasParam {
			operation, hasOperation := parameters["operation"]
			if hasOperation && operation != "zulu" {
				fmt.Println("unhandled parameters for millisecondsToRFC3339")
				break
			}
		}
		expression = "MillisecondsToRFC3339(" + inputs[0].Id + ")"
	case "echo":
		expression = fmt.Sprintf("%v", inputs[0].Id)
	default:
		fmt.Printf("unhandled function: %v\n", operation)
	}
	newMetric.Expression = expression

	id := oldMetric["id"].(string)
	newMetric.Outputs = []string{id}
	return newMetric
}
