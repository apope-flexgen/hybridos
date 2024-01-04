package main

import (
	"fims"
	"time"
)

// periodically publish on fims
func main() {
	fimsObj, _ := fims.Connect("periodic_pub")
	var counter uint16 = 0
	for {
		fimsObj.SendPub("/ftd_test", map[string]interface{}{"counter": counter})
		counter++
		time.Sleep(1 * time.Millisecond)
	}
}
