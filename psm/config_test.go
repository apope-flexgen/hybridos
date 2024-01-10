package main

import (
	"encoding/json"
	"fmt"
	"testing"
)

var blob = []byte(`{
    "updateRate":50,
    "publishRate":200,
    "timeMultiplier":60,
    "ess":[
        {
            "id":"sungrow_ess_1",
            "cap":2500,
            "ctrlword1cfg":[
                {
                    "value": 1,
                    "controls": [
                        { "field": "oncmd", "value": true },
                        { "field": "kacclosecmd", "value": true }
                    ]
                },
                {
                    "value": 2,
                    "controls": [
                        { "field": "offcmd", "value": true },
                        { "field": "kacopencmd", "value": true }
                    ]
                }
            ],
            "statuscfg": [
                {
                    "value":4,
                    "string":"Running",
                    "field":"on"
                },
                {
                    "value":7,
                    "string":"Stopped",
                    "field":"on",
                    "invert":true
                }
            ],
            "dactive": {
                "ynom":1000,
                "xnom":60,
                "percent":0.05
            }
        }
    ]
}`)

var testConfig cfg

func TestReadConfig(t *testing.T) {
	err := json.Unmarshal(blob, &testConfig)
	if err != nil {
		t.Error("Failed to unmarshal the json:", err)
	}
	fmt.Println(testConfig.Ess[0].Dactive)
}
