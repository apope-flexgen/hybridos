package main

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"math/rand"
	"os"
	"time"
)

const charset = "abcdefghijklmnopqrstuvwxyz" +
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"

var seededRand *rand.Rand = rand.New(
	rand.NewSource(time.Now().UnixNano()))

type MetricsFile struct {
	Inputs map[string]Input `json:"inputs"`
}
type Input struct {
	Uri  string `json:"uri"`
	Type string `json:"type"`
}

func StringWithCharset(length int, charset string) string {
	b := make([]byte, length)
	for i := range b {
		b[i] = charset[seededRand.Intn(len(charset))]
	}
	return string(b)
}

func String(length int) string {
	return StringWithCharset(length, charset)
}

func main() {
	data, err := os.ReadFile(os.Args[1])
	if err != nil {
		log.Fatalf("Error reading json file: %s", err)
	}
	inputData := MetricsFile{}
	err = json.Unmarshal(data, &inputData)
	if err != nil {
		log.Fatalf("Error parsing json file: %s", err)
	}

	file := `#!/bin/bash
start () {
while true
do
`
	for _, input := range inputData.Inputs {
		file += fmt.Sprintf("fims_send -m set -u %s ", input.Uri)
		switch input.Type {
		case "string":
			file += "\\\"" + String(rand.Intn(10)) + "\\\"" + "\n"
		case "float":
			file += fmt.Sprintf("%f\n", rand.Float64()*100)
		case "bool":
			if math.Round(rand.Float64()) == 0 {
				file += fmt.Sprintf("false\n")
			} else {
				file += fmt.Sprintf("true\n")
			}
		default:
			file += fmt.Sprintf("%f\n", rand.Float64()*100)
		}
		file += "sleep 0.001\n"
	}
	file += "sleep 1\ndone\n}\n"

	f, err := os.Create("run_data.sh")
	f.WriteString(file)
	f.Close()
}
