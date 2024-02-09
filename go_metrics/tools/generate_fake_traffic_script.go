package main

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"math/rand"
	"os"
	"time"
	"strings"
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

func GetParentUri(str string) string {
	ind := strings.LastIndex(str, "/")
	if ind < 0 {
		if len(str) > 0 {
			return "/" + str
		}
		return str
	} else if ind == 0 {
		return str
	}
	if ind > 0 && strings.Index(str, "/") != 0 {
		return "/" + str[0:ind]
	}
	return str[0:ind]
}

/*
 * Gets one level above for URI
 * returns original URI, if no parent
 */
func GetUriElement(str string) string {
	ind := strings.LastIndex(str, "/")
	if ind < 0 {
		return str
	}
	return str[ind+1:]
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

while true
do
`
	parent_uri_map := make(map[string]map[string]interface{},0)
	for i := 0; i < 10; i++ {
	for _, input := range inputData.Inputs {
		if input.Uri != ""{
			parent_uri := GetParentUri(input.Uri)
			element := GetUriElement(input.Uri)
			if _,ok := parent_uri_map[parent_uri]; !ok {
				parent_uri_map[parent_uri] = make(map[string]interface{})
			}
			switch input.Type {
			case "string":
				parent_uri_map[parent_uri][element] = String(rand.Intn(10))
			case "float":
				parent_uri_map[parent_uri][element] = rand.Float64()*100
			case "bool":
				if math.Round(rand.Float64()) == 0 {
					parent_uri_map[parent_uri][element] = false
				} else {
					parent_uri_map[parent_uri][element] = true
				}
			default:
				if math.Round(rand.Float64()) == 0 {
					parent_uri_map[parent_uri][element] = 0
				} else {
					parent_uri_map[parent_uri][element] = 1
				}
			}
		}
	}
	for parent_uri, body := range parent_uri_map {
		new_body, _:= json.Marshal(body)
		file += fmt.Sprintf("fims_send -m pub -u %s '%s'\n", parent_uri, new_body)
	}
}
file += "done\n"
	f, err := os.Create("run_data.sh")
	f.WriteString(file)
	f.Close()
}
