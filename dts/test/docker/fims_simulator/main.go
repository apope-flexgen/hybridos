package main

import (
	"fims"
	"log"
	"math/rand"
	"time"
)

func main() {
	fimsObj, err := fims.Connect("fims_simulator")
	if err != nil {
		log.Fatalf("Failed to connect to fims")
	}

	val := 0
	for {
		fimsObj.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    "/test",
			Body: map[string]interface{}{
				"random_walk": val,
				"bit_string": []map[string]interface{}{
					{
						"value":  0,
						"string": "Status 0",
					},
					{
						"value":  1,
						"string": "Status 1",
					},
				},
				"on_off": rand.Intn(2) == 0,
				"bagel":  "tasty",
			},
		})
		val += (100 - rand.Intn(201))

		time.Sleep(time.Second)
	}
}
