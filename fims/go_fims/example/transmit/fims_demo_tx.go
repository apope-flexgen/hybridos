package main

import (
	"fims"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"time"
)

var f fims.Fims

func readBody() ([]byte, int, error) {
	// Looking for twins.json
	cpath := "test.json"
	count := 1
	var err error
	if len(os.Args) < 1 {
		fmt.Printf("Test file argument [1]  not found. Usage 'go run fims_demo_tx.go <test_file> <test_count>'. Trying current working directory\n")
	}
	if len(os.Args) < 2 {
		fmt.Printf("Count argument [2]  not found. Usage 'go run fims_demo_tx.go <test_file> <test_count>'. Setting count to 1\n")
	}
	if len(os.Args) > 1 {
		cpath = os.Args[1]
	}
	s := "Unknown"
	if len(os.Args) > 2 {
		s = os.Args[2]
		count, err = strconv.Atoi(s)
	}
	if err != nil {
		fmt.Printf("Couldn't decode count %s: %s \n", s, err)
		count = 1
	}
	_, err = os.Stat(cpath)
	if err != nil {
		fmt.Printf("Test file %s err : %s \n", cpath, err)
	} else {
		fmt.Printf("Test file %s \n", cpath)
	}
	if err == nil {
		body, xerr := ioutil.ReadFile(cpath)
		if xerr != nil {
			fmt.Printf("Couldn't read the file %s: %s\n", cpath, xerr)
			err = xerr
		}
		return body, count, err
	}
	return nil, count, err
}

func init() {
	var err error
	f, err = fims.Connect("Go Program")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}
}

func main() {
	x := 0
	body, count, err := readBody()
	b := len(body)
	bs := string(body)
	defer f.Close() // This makes sure the FIMS connection gets closed no matter how the program exits
	if err != nil {
		return
	}
	start := time.Now()
	for x < count {
		f.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    "/go_test",
			Body:   bs,
		})
		x += 1
	}

	// show times
	duration := time.Since(start)
	dur := float32(float32(duration.Microseconds()) / 1000.0)
	total := (x * b)
	rate := float32(total) / (dur / 1000.0)
	fmt.Printf(" sent count %d size %d total_size %d time (mS) %f rate %f \n", x, b, total, dur, rate)
}
