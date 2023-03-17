package main

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"log"

	par "github.com/buger/jsonparser"
)

func testFims() {

	running := true

	// Fims Echo body inputs
	// var uri string
	var body map[string]interface{}

	// Go's Flag package to set up parsing for command line
	bodyFlag := flag.String("b", "", "sets body of the fims echo")
	uriFlag := flag.String("u", "", "sets uri to echo for")
	flag.Parse()
	if *bodyFlag == "-u" || *uriFlag == "-b" || *uriFlag == "" || *bodyFlag == "" { // Makes sure that there are values for the program to use
		fmt.Print("Flag -b was given but no body was written OR\n")
		fmt.Print("usage: fims_echo <options> [body]\n",
			" required option -u\n\n",
			" options:\n",
			" -u   sets the uri to echo for\n",
			" -b   sets the body of the echo")
		fmt.Print("\nIMPORTANT: No uri given to echo on.\n")
		return
	}
	byt := []byte(*bodyFlag)
	json.Unmarshal(byt, &body)
	var byteBody []byte
	var byteReg []byte
	byteBody, _, _, _ = par.Get(byt)
	// fmt.Println(string(byteBody))

	// fmt.Println(body)
	fmt.Printf("Listening on %s replying with %s\n", *uriFlag, *bodyFlag)

	var p_fims fims.Fims // The fims object

	// Connect to the Fims Echo keyword
	p_fims, err := fims.Connect("fims_echo")
	if err != nil {
		log.Printf("Connection failed: %v", err)
		p_fims.Close()
		return
	}

	// Subscribe to components and twins uris that is given in the fims_echo
	subErr := p_fims.Subscribe("/components", "/twins")
	if subErr != nil {
		log.Println("Subscription to /components and /twins failed.")
		p_fims.Close()
		return
	}

	var newMap map[string]interface{}

	for running && p_fims.Connected() {
		// Message being received from the fims_send
		msg, err := p_fims.Receive()
		if err != nil {
			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
		}

		// Makes sure that we are only dealing with gets and sets
		if msg.Replyto != "" && (msg.Method != "pub" && msg.Method != "del" && msg.Method != "post") {

			if msg.Nfrags > 0 {
				switch msg.Nfrags {
				case 2:
					// Makes sure the URI is the same as the fims send one
					if msg.Uri == *uriFlag { // Used for /components URIs
						if msg.Body == nil && msg.Method == "get" { // Simple get to relay the body back
							msg.Body = string(byteBody)
						} else if msg.Body != nil && msg.Method == "set" { // Set for a body in a non-full uri
							newMap = msg.Body.(map[string]interface{})
							for key := range newMap {
								body[key] = newMap[key]
							}
							byteBody, _ = json.Marshal(body)
							msg.Body = string(byteBody)
						}
					}
				case 3:
					if msg.Uri == *uriFlag { // Used for /assets URIs
						if msg.Body == nil && msg.Method == "get" {
							msg.Body = string(byteBody)
						} else if msg.Body != nil && msg.Method == "set" {
							newMap = msg.Body.(map[string]interface{})
							for key := range newMap {
								body[key] = newMap[key]
							}
							byteBody, _ = json.Marshal(body)
							msg.Body = string(byteBody)
						}
					} else { // Used for /components URIs
						if msg.Method == "set" { // Simple set for a full uri on a single register
							body[msg.Frags[2]] = msg.Body.(map[string]interface{})["value"]
							byteBody, _ = json.Marshal(body)
							byteReg, _, _, _ = par.Get(byteBody, msg.Frags[2])
							msg.Body = string(byteReg)
						}
						if msg.Method == "get" { // Get on a single register for a full uri
							byteReg, _, _, _ = par.Get(byteBody, msg.Frags[2])
							msg.Body = string(byteReg)
						}
					}

				case 4:
					if msg.Method == "set" { // Used for /assets URIs
						body[msg.Frags[3]] = msg.Body.(map[string]interface{})["value"]
						byteBody, _ = json.Marshal(body)
						byteReg, _, _, _ = par.Get(byteBody, msg.Frags[3])
						msg.Body = string(byteReg)
					}
					if msg.Method == "get" {
						byteReg, _, _, _ = par.Get(byteBody, msg.Frags[3])
						msg.Body = string(byteReg)
					}
				}
			}
			// Sending a fims set message to print out after the fims send
			// if uri == *uriFlag {
			_, err := p_fims.SendRaw(fims.FimsMsg{
				Method: "set",
				Uri:    msg.Replyto,
				Body:   msg.Body,
			})
			if err != nil {
				log.Printf("Could not send message. %v", err)
			}
			// }
			// for k, v := range newMap {
			// p_fims.SendRaw(fims.FimsMsg{
			// 	Method: "pub",
			// 	Uri:    fmt.Sprintf("/twins/%s", k),
			//  Body:   v,
			// })
			// }
		}
	}
}

//subscribe to twins and components
//able to get those messages

// // Changing a map into a string to set to the msg.Body and body
// func mapToStr(body map[string]interface{}, order []string) string {
// 	var b []byte
// 	buf := bytes.NewBuffer(b)
// 	buf.WriteRune('{')
// 	buf.WriteRune('\n')
// 	l := len(order)
// 	for i, key := range order {
// 		km, _ := json.MarshalIndent(key, " ", "\t")
// 		buf.Write(km)
// 		buf.WriteRune(':')
// 		vm, _ := json.MarshalIndent(body[key], " ", "\t")
// 		buf.Write(vm)
// 		if i != l-1 {
// 			buf.WriteRune(',')
// 		}
// 		buf.WriteRune('\n')
// 	}
// 	buf.WriteRune('}')
// 	return buf.String()
// }

// p_fims.Close()
