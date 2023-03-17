package metrics

import (
	"encoding/json"
	"fmt"
	"log"
	"reflect"
)

type Map map[string]interface{}
type Maps map[string]map[string]interface{}

type DirectionM struct { //Direction struct used for if values need to go to twins or echo registers
	Forward  map[string]interface{}
	Backward map[string]interface{}
}

type LinkM struct { //Link struct used to store uri and register of input registers (mainly twins)
	Uri      string
	Register string
}

type bitbody struct {
	Position []int    `json:"position"`
	String   []string `json:"string"`
	Invert   []bool   `json:"invertMask"`
}

type Metrics struct {
	Uri       string                   `json:"uri"`
	Operation []string                 `json:"operation"`
	Inputs    []map[string]interface{} `json:"inputs"`
	Param     map[string]interface{}   `json:"param,omitempty"`
	Value     interface{}              `json:"value,omitempty"`
	Operate   string                   `json:"operate,omitempty"`
}

func (info Metrics) Basic(infoMap Maps) float64 {
	var value interface{}
	var final float64

	for i, reg := range info.Inputs {
		value = infoMap[reg["uri"].(string)][reg["id"].(string)]
		if value != nil && i != 0 {
			switch info.Operation[i-1] {
			case "add":
				final += value.(float64)
				// fmt.Printf("This is the number: %f\n", infoMap[reg["uri"].(string)][reg["id"].(string)])
			case "subtract":
				final -= value.(float64)
				// fmt.Printf("This is the number: %f\n", infoMap[reg["uri"].(string)][reg["id"].(string)])
			case "multiply":
				if reflect.TypeOf(value) == reflect.TypeOf(true) {
					if value == true {
						final = final * 1
					} else {
						final = final * 0
					}
				} else {
					final = final * value.(float64)
					// fmt.Printf("This is the number: %f\n", infoMap[reg["uri"].(string)][reg["id"].(string)])
				}
			case "divide":
				if reflect.TypeOf(value) == reflect.TypeOf(true) {
					if value == true {
						final = final / 1
					} else {
						final = 0 / final
					}
				} else {
					final = final / value.(float64)
					// fmt.Printf("This is the number: %f\n", infoMap[reg["uri"].(string)][reg["id"].(string)])
				}
			}
		} else if value != nil {
			final = value.(float64)
		}
	}
	if info.Value != nil && len(info.Operation) == len(info.Inputs) {
		switch info.Operation[len(info.Operation)-1] {
		case "add":
			final = final + info.Value.(float64)
		case "subtract":
			final = final - info.Value.(float64)
		case "multiply":
			final = final * info.Value.(float64)
		case "divide":
			final = final / info.Value.(float64)
		}
	} else if info.Value != nil {
		log.Printf("Mismatch of amount of operations and inputs+value for %s\n", info.Uri)
		final = 0
	}
	return final
}

func (info Metrics) Logic(infoMap Maps) bool {
	var boolVal bool
	var value interface{}
	var output bool
	for i, reg := range info.Inputs {
		value = infoMap[reg["uri"].(string)][reg["id"].(string)]
		if value != nil && i != 0 {
			switch info.Operation[i-1] {
			case "and":
				if value == true || value == false {
					boolVal = value.(bool)
					output = boolVal && output
				} else {
					fmt.Println("You need to use a boolean value")
				}
			case "or":
				if value == true || value == false {
					boolVal = value.(bool)
					output = boolVal || output
				} else {
					fmt.Println("You need to use a boolean value")
				}
			case "not":
				if value == true || value == false {
					//get the values of uri and invert them
					boolVal = value.(bool)
					output = !boolVal
				} else {
					fmt.Println("You need to use a boolean value")
				}
			}
		} else if value != nil {
			output = value.(bool)
		}
		if info.Value != 0 {
			switch info.Operation[len(info.Operation)-1] {
			case "and":
				output = output && info.Value.(bool)
			case "or":
				output = output || info.Value.(bool)
			}
		}
	}
	return output
}

func (info Metrics) Compare(infoMap Maps) bool {
	output := false
	var uriValue float64
	var reference float64
	for i, reg := range info.Inputs {
		value := infoMap[reg["uri"].(string)][reg["id"].(string)]
		if value != nil {
			if i == 1 {
				fmt.Println("Value wont be used since there are two uris")
				reference = value.(float64)
			} else {
				uriValue = value.(float64)
				reference = info.Value.(float64)
			}
		}
	}
	//Take URIs and compare them with eq, neq, lt, gt, lte, gte
	switch info.Operate {
	//Get the values from uris and perform action based on the operation
	case "eq":
		return uriValue == reference
	case "neq":
		return uriValue != reference
	case "lt":
		return uriValue < reference
	case "gt":
		return uriValue > reference
	case "lte":
		return uriValue <= reference
	case "gte":
		return uriValue >= reference
	}
	return output
}

func (info Metrics) Bitfield() interface{} {
	//This is the most complicated one
	var body bitbody
	bitMap := make(Map)
	bitbodyByte, _ := json.Marshal(info.Param)
	json.Unmarshal(bitbodyByte, &body)
	fmt.Println(body)
	for i, mask := range body.Invert {
		if mask {
			bitMap[fmt.Sprint(body.Position[i])] = body.String[i]
		}
	}
	return bitMap
}
