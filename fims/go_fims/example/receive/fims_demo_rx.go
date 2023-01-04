package main

import (
	"fims"
	"fmt"
	"log"
	"time"
)

var fimsReceive chan fims.FimsMsg

var f fims.Fims
var err error

func init() {
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	fimsReceive = make(chan fims.FimsMsg)
	f.Subscribe("/go_program")
	go f.ReceiveChannel(fimsReceive)
}

func main() {
	start := time.Now()
	for {
		msg := <-fimsReceive
		if msg.Uri == "/go_program/end" {
			break
		}
		fmt.Printf("From [%s] I got a [%s] containing [%v]. Best send something back to [%s]\n", msg.Uri, msg.Method, msg.Body, msg.Replyto)
		if msg.Replyto != "" {
			f.Send(fims.FimsMsg{
				Method:      "set",
				Uri:         msg.Replyto,
				ProcessName: "mySetTest",
				Body:        msg.Body,
			})
		}
	}

	// show times
	duration := time.Since(start)
	dur := float32(float32(duration.Microseconds()) / 1000.0)
	fmt.Printf(" rx time (mS) %f \n", dur)
}
