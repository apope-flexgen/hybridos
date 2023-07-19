package main

import (
	"encoding/json"
	"fims"
	"fmt"
	"io/ioutil"
	"math/rand"
	"os"
	"time"
)

// var uriArr = [15]string{"ess", "bms", "pcs", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l"} //uris that are subscribed to
// var uriArr = [1]string{"/echo_test_1"}
var uriArr = [6]string{"/echo_test_1", "/echo_test_2", "/echo_test_3", "/echo_test_4", "/echo_test_5", "/echo_test_6"} //uris that are subscribed to

// var fimsMethodArr = [1]string{"set"}
// var fimsMethodArr = [1]string{"get"}
var fimsMethodArr = [3]string{"set", "get", "pub"}

// var fimsMethodArr = [5]string{"set", "get", "post", "del", "pub"}
// var fimsMethodArr = [6]string{"set", "get", "post", "del", "pub", "sub"}

var clientUri = "echo_publish"

var small = "config/small.json"
var medium = "config/medium.json"
var large = "config/large.json"

func createRandomFimsMsgs(numSims int, numMsgs int) [][]fims.FimsMsg {

	msgs := make([][]fims.FimsMsg, numSims)

	// fmt.Println("msgs length is", len(msgs))

	// var data interface{}

	var data []string

	jsonFile, err := os.Open(small)
	if err != nil {
		fmt.Println(err)
	}
	byteData, _ := ioutil.ReadAll(jsonFile)
	json.Unmarshal(byteData, &data)

	// data := 1

	for a := 0; a < numSims; a++ {
		row := make([]fims.FimsMsg, numMsgs)

		// fmt.Printf("\n row %v length is %v \n ", a, len(row))

		if numSims > 2 {
			if a == (numSims / 3) {
				jsonFile, err = os.Open(medium)
				if err != nil {
					fmt.Println(err)
				}
				byteData, _ = ioutil.ReadAll(jsonFile)
				json.Unmarshal(byteData, &data)
			} else if a == ((2 * numSims) / 3) {
				jsonFile, err = os.Open(large)
				if err != nil {
					fmt.Println(err)
				}
				byteData, _ = ioutil.ReadAll(jsonFile)
				json.Unmarshal(byteData, &data)
			}
		}

		for b := 0; b < numMsgs; b++ {

			//Get random uri
			rand.Seed(time.Now().UnixNano())
			index := 0
			largestArrIndex := len(uriArr) - 1
			if largestArrIndex != 0 {
				index = rand.Intn(len(uriArr) - 1)
			}
			randUri := uriArr[index]

			//Get random FimsMsg Method
			rand.Seed(time.Now().UnixNano())
			index = 0
			largestArrIndex = len(fimsMethodArr) - 1
			if largestArrIndex != 0 {
				index = rand.Intn(len(fimsMethodArr) - 1)
			}
			randFimsMethod := fimsMethodArr[index]

			//Data within fims message
			// s := 1
			s := data

			fimsMsg := fims.FimsMsg{}

			switch { //switch statement that changes what the contents of the fims message are based on randomized fims method
			case randFimsMethod == "set":
				rand.Seed(time.Now().UnixNano())
				num := rand.Intn(1)

				switch { //switch statement that allows for random fims sets that either have replyTo or don't
				case num == 0: //fims set without reply to
					fimsMsg = fims.FimsMsg{
						Method: randFimsMethod,
						Uri:    randUri,
						Body:   s,
					}
				case num == 1: //fims set with reply to
					fimsMsg = fims.FimsMsg{
						Method:  randFimsMethod,
						Uri:     randUri,
						Body:    s,
						Replyto: clientUri,
					}
				}

			case randFimsMethod == "get":
				fimsMsg = fims.FimsMsg{
					Method:  randFimsMethod,
					Uri:     randUri,
					Replyto: clientUri,
				}
			case randFimsMethod == "post":
				fimsMsg = fims.FimsMsg{
					Method: randFimsMethod,
					Uri:    randUri,
					Body:   s,
				}
			case randFimsMethod == "del":
				fimsMsg = fims.FimsMsg{
					Method: randFimsMethod,
					Uri:    randUri,
					Body:   s,
				}
			case randFimsMethod == "pub":
				fimsMsg = fims.FimsMsg{
					Method: randFimsMethod,
					Uri:    randUri,
					Body:   s,
				}
			case randFimsMethod == "sub":
				fimsMsg = fims.FimsMsg{
					Method: randFimsMethod,
					Uri:    randUri,
					Body:   s,
				}
			}
			// row = append(row, fimsMsg)
			row[b] = fimsMsg

			// fmt.Printf("The size of fimsMsg is %v", unsafe.Sizeof(fimsMsg))

		}
		// msgs = append(msgs, row)
		msgs[a] = row

		// fmt.Printf("The size of row is %v", unsafe.Sizeof(row))

	}
	// fmt.Printf("The size of msgs is %v", unsafe.Sizeof(msgs))

	// for x := 0; x < len(msgs); x++ {
	// 	row := msgs[x]
	// 	for y := 0; y < len(row); y++ {
	// 		fmt.Printf("\n\n Map:%v Message#:%v    Data: %v \n\n", x, y, row[y])
	// 	}
	// }

	jsonFile.Close()

	return msgs
}

func publishPreCreatedMsgs(fimsMsgs []fims.FimsMsg, f fims.Fims) {

	// fmt.Println("Got into publishPreCreatedMsgs")
	count := 1

	for _, msg := range fimsMsgs {
		// fmt.Println("Before fims send for message ", count)
		// fmt.Println("msg is ", msg)
		f.Send(msg)
		// time.Sleep(5 * time.Millisecond)
		// fmt.Println("After fims send for message ", count)

		// if (count % 100) == 0 {
		// 	time.Sleep(100 * time.Millisecond)
		// }

		count++
	}
}
