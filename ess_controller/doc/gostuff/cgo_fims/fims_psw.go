package main

// #cgo LDFLAGS: -L/usr/local/lib -lgofims
// #include "gofims.h"

import "C"

import (
	"fmt"
	"log"
	"time"
)

var fimsReceive chan fims.FimsMsg
var stateTicker, pubTicker *time.Ticker
var f fims.Fims

func init() {
	var err error
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}
	fimsReceive = make(chan fims.FimsMsg)
	f.Subscribe("/go_program")
	go f.ReceiveChannel(fimsReceive)
	stateTicker = time.NewTicker(100 * time.Millisecond)
	pubTicker = time.NewTicker(500 * time.Millisecond)
}

func main() {
	defer f.Close() // This makes sure the FIMS connection gets closed no matter how the program exits
	for {           // Infinite loop
		select { // Select waits on one of the channels in its case statements to have a value to process
		case <-stateTicker.C:
			// Do whatever you might do in the background, update internal state
		case msg := <-fimsReceive:
			// Receive inputs over FIMS and process
			fmt.Printf("From %v I got a %v containing %v. Best send something back to %v", msg.Uri, msg.Method, msg.Body, msg.Replyto)
			if msg.Replyto != "" {
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   "Message received",
				})
			}
		case <-pubTicker.C:
			// Time to publish
			f.Send(fims.FimsMsg{
				Method: "pub",
				Uri:    "/go_program",
				Body:   "I'm here and I'm publishing",
			})
		}
	}
}
