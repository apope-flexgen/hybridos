package main

// #cgo LDFLAGS: -L/usr/local/lib -lgofims
// #include <stdio.h>
// #include <stdlib.h>
// #include "gofims.h"
import "C"

import (
	"encoding/json"
	"fmt"
	"log"
	"time"
	"unsafe"
	"strings"
)

var maxMessageSize = 65536
var maxSubscriptions = 64
var maxURIDepth = 64
//export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
// go run try_fims2.go

// I think we ca:
// open fims
// send a message
// receive 
// subscribe
// next we have to recv FimsMsg through the chan 


type FimsMsg struct {
	Method  string      `json:"method"`
	Uri     string      `json:"uri,omitempty"`
	Replyto string      `json:"replyto,omitempty"`
	Body    interface{} `json:"body,omitempty"`
	Nfrags  int         `json:"-"`
	Frags   []string    `json:"-"`
}

//help
type Fims struct {
	ptr       unsafe.Pointer
	connected bool
	buff      []byte
	n         int
}

type cfg struct {
	UpdateRate     float64
	PublishRate    float64
	ManualTick     bool
	TimeMultiplier float64
}

func NewFims() Fims {
	var f Fims
	f.ptr = C.FIMS_NewFims()
	return f
}

func (f *Fims) ConnectFims(name string) (ret int) {

	ret = int(C.FIMS_ConnectFims(f.ptr, C.CString(name)))
	return ret
}

func (f *Fims) SendFims(method string, uri string, replyto string, body string) (ret int) {

	ret = int(C.FIMS_SendFims(f.ptr, C.CString(method), C.CString(uri), C.CString(replyto), C.CString(body)))
	return ret
}

func (f *Fims) SendRawFims(body string, len int) (ret int) {

	ret = int(C.FIMS_SendRawFims(f.ptr, C.CString(body), C.int(len)))
	return ret
}

func (f *Fims) Free() {
	C.FIMS_DestroyFims(f.ptr)
}

func (f *Fims) SendRaw(msg FimsMsg) (int, error) {
	// logic to unpack FimsMsg into a buffer to write out
	var err error
	b, err := json.Marshal(msg)
	// fmt.Println(string(b))
	n := f.SendRawFims(string(b), 0)

	// n, err := f.conn.Write(b)
	// if err != nil {
	// 	f.connected = false
	// }
	return n, err
}

func (f *Fims) Subscribe(uris ...string) error {
	// Send a message with no uri, method: sub, body: [{"uri":uris[x],"pub_only":false}]
	// Save for FIMS upgrade
	var body []map[string]interface{}
	for _, u := range uris {
		body = append(body, map[string]interface{}{
			"uri":      u,
			"pub_only": false,
		})
	}
	msg := FimsMsg{
		Method: "sub",
		Body:   body,
	}
	// fmt.Println(uris)
	//_, err := f.SendRawFims(msg, 0)
	_, err := f.SendRaw(msg)
	// fmt.Println(n)
	// s, err := f.ReceiveRaw()
	// if s != "SUCCESS" {
	// 	err = fmt.Errorf("Didn't get the success string got: %s", s)
	// }
	return err
}

//func (foo Foo) value() int {
//	return int(C.LIB_FooValue(foo.ptr))
//}
// this was error TODO
func Connect(pName string) (Fims, int) {
	// Set up struct vars
	//connected := false
	buff := make([]byte, maxMessageSize)
	f := NewFims()
	err := f.ConnectFims(pName)
	f.connected = true
	f.buff = buff
	return f, err
}

func (f *Fims) Send(msg FimsMsg) (int, error) {
	// logic to unpack FimsMsg into a buffer to write out
	var err error
	// var bb []byte
	// bb, err = json.Marshal(msg.Body)
	// msg.Body = string(bb)
	//b, err := json.Marshal(msg)
	// fmt.Println(string(b))
	n, err := f.SendRaw(msg)

	//n, err := f.conn.Write(b)
	if err != nil {
		f.connected = false
	}
	return n, err
}

// TODO
func (f *Fims) ReceiveRaw() (string, error) {
	n, err := C.FIMS_ReceiveRawFims(f.ptr, (*C.char)(unsafe.Pointer(&f.buff[0])))
	if err != nil {
		f.connected = false
		return "", err
	}
	f.n = int(n)
	s := string(f.buff[:n-1])
	return s, err
}

// func (f *Fims) Receive() (FimsMsg, error) {
// 	n, err := f.conn.Read(f.buff)
// 	var msg FimsMsg
// 	if err != nil {
// 		f.connected = false
// 		return FimsMsg{}, err
// 	}
// 	// logic to unpack the buffer into msg
// 	msg, err = Decode(f.buff[:n])
// 	if err != nil {
// 		return FimsMsg{Uri: msg.Uri}, err
// 	}
// 	msg.Nfrags, msg.Frags = GetFrags(msg.Uri)
// 	// fmt.Printf("Got %v bytes %s\n", n, f.buff[:n])
// 	return msg, nil
// }

// func (f *Fims) ReceiveChannel(c chan<- FimsMsg) {
// 	for f.connected == true {
// 		msg, err := f.Receive()
// 		if err != nil {
// 			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
// 		}
// 		// fmt.Printf("about to put the msg on the channel %v\n", msg)
// 		c <- msg
// 	}
// }

// TODO get back the errors
func (f *Fims) Close() {
	C.FIMS_Close(f.ptr)
	f.connected = false
	return
}

func (f *Fims) Connected() bool {
	return f.connected
}

//var fimsReceive chan fims.FimsMsg
var t0 time.Time
var stateTicker, pubTicker *time.Ticker
var stateUpdate chan float64

//var f fims.Fims
var config cfg
var fimsReceive chan FimsMsg
var foo Fims
//var root treeNode
//var assets map[string]asset
//var cmds map[string]asset
//var fimsMap map[string]interface{}
// msg.go
func Decode(b []byte) (FimsMsg, error) {
	var msg FimsMsg
	err := json.Unmarshal(b, &msg)
	if err != nil {
			return FimsMsg{}, err
	}
	var body interface{}
	if msg.Body != nil {
			err = json.Unmarshal([]byte(msg.Body.(string)), &body)
			if err == nil {
					msg.Body = body
			} else {
					return FimsMsg{Uri: msg.Uri}, err
			}
	}
	return msg, nil
}

func GetFrags(s string) (int, []string) {
	// Expecting something like /components/ess1 or /components/ess1/voltage_ac
	// String split will have empty strings before a leading / or after a trailing /
	frags := strings.Split(s, "/")
	start, end, n := 0, len(frags), len(frags)
	if end == 0 {
			return 0, []string{}
	}
	if frags[0] == "" {
			if end == 1 {
					return 0, []string{}
			}
			start = 1
			n--
	}
	if frags[end-1] == "" {
			end--
			n--
	}
	return n, frags[start:end]
}

func (f *Fims) Receive() (FimsMsg, error) {
	n, err := C.FIMS_ReceiveRawFims(f.ptr, (*C.char)(unsafe.Pointer(&f.buff[0])))
	fmt.Printf("Got %v bytes %s\n", n, f.buff[:n])
	fmt.Printf("Got %v bytes 1: %x n-2 %x n: %x\n", n, f.buff[0], f.buff[n-2],f.buff[n])

	var msg FimsMsg
	if err != nil {
		f.connected = false
		return FimsMsg{}, err
	}
	// logic to unpack the buffer into msg
	if f.buff[n-1] == 0 {
		n = n-1
	}
	msg, err = Decode(f.buff[:n])
	if err != nil {
		return FimsMsg{Uri: msg.Uri}, err
	}
	msg.Nfrags, msg.Frags = GetFrags(msg.Uri)
	return msg, nil
}

func (f *Fims) ReceiveChannel(c chan<- FimsMsg) {
	for f.connected == true {
		msg, err := f.Receive()
		if err != nil {
			log.Printf("Had an error while receiving on %s: %s\n", msg.Uri, err)
		}
		// fmt.Printf("about to put the msg on the channel %v\n", msg)
		c <- msg
	}
}

func runConfig() {
	//var err error
	log.SetPrefix("Twins: ")
	log.Printf("Configuring")
	config.UpdateRate = 500.0
	config.PublishRate = 2000.0
	config.ManualTick = false
	config.TimeMultiplier = 1.0

	//readConfig(&config)
	//fimsMap = make(map[string]interface{})
	// root, assets, err = buildState(&config, fimsMap)
	// if err != nil {
	// 	log.Fatalf("Couldn't put together the tree: %s", err)
	// }
	// //fmt.Printf("%#v", root)
	// f, err = fims.Connect("Twins")
	// if err != nil {
	// 	log.Fatal("Unable to connect to FIMS server")
	// }
	//fimsReceive = make(chan FimsMsg)
	// err = f.Subscribe("/components", "/twins")
	// if err != nil {
	// 	log.Fatal("Unable to subscribe")
	// }
	//go f.ReceiveChannel(fimsReceive)
	stateUpdate = make(chan float64)
	stateTicker = time.NewTicker(time.Duration(config.UpdateRate) * time.Millisecond)
	pubTicker = time.NewTicker(time.Duration(config.PublishRate) * time.Millisecond)
	t0 = time.Now()
}

func main() {
	runConfig()
	//foo := NewFims()
	//foo.ConnectFims("/gofims")
	foo, _ := Connect("gofims")
	fimsReceive = make(chan FimsMsg)
	// err = f.Subscribe("/components", "/twins")
	// if err != nil {
	// 	log.Fatal("Unable to subscribe")
	// }
	//foo.SendRawFims("{\"method\":\"sub\",\"body\":[{ \"uri\":\"/go/fims/test\",\"pub_only\":false}]}", 0)
	foo.Subscribe("/myfoo", "/my/go/test", "/twins", "/components")
	foo.ReceiveRaw()
	fmt.Printf("#1 Got %v bytes %s\n", foo.n, foo.buff[:foo.n])

	go foo.ReceiveChannel(fimsReceive)

	foo.SendFims("pub", "/my/go/test", "", "{ \"hello\":\"its a beautiful day\"}")
	//	defer foo.Free() // The Go analog to C++'s RAII
	//s, n :=
	//foo.ReceiveRaw()
	//fmt.Printf("#2 Got %v bytes %s\n", foo.n, foo.buff[:foo.n])
	defer foo.Close() // The Go analog to C++'s RAII
	log.Printf("Starting main loop")
	for {
		select {
		case t1 := <-stateTicker.C:
			log.Printf("Starting stateTicker")

			// Ticker channels return a Time struct
			if !config.ManualTick {
				go func() {
					stateUpdate <- t1.Sub(t0).Seconds() * float64(config.TimeMultiplier)
					t0 = t1
				}()
			}
		case msg := <-fimsReceive:
 			// Receive inputs over FIMS, process (but don't update)
 			fmt.Println("I have received", msg.Uri, msg.Replyto, msg.Body)	
			//receiveFims(msg, fimsMap)
		case dt := <-stateUpdate:
			// Update internal variables of models
			fmt.Printf("stateUpdate node  just processed .. %f\n", dt)
			// 			// updateState(root, root.asset.Term(), dt)
			// 		//discoverTree(root, root.asset.Term(), dt)
			// 		//	calculateTree(root, root.asset.Term(), dt)
			// 		//updateTree(root, root.asset.Term(), dt)
		case <-pubTicker.C:
			foo.SendFims("set", "/my/go/test", "", "{ \"hello\":\"its a beautiful day\"}")
			fmt.Printf("pubTicker node  just processed .. \n")
			// 		// Publish everything, maybe the maps are updated under the state ticker,
			// 		// and simply output every field in each struct

		}
	}

	//fmt.Println("[go] fims ok")
}

// log.Printf("Starting main loop")
// for {
// 	select {
// 	case t1 := <-stateTicker.C:
// 		// Ticker channels return a Time struct
// 		if !config.ManualTick {
// 			go func() {
// 				stateUpdate <- t1.Sub(t0).Seconds() * float64(config.TimeMultiplier)
// 				t0 = t1
// 			}()
// 		}
// 	case dt := <-stateUpdate:
// 		// Update internal variables of models
// 		// fmt.Printf("The node %v just processed, ended up with %#v\n", root.value.GetID(), root.value.Term()))
// 		// updateState(root, root.asset.Term(), dt)
// 		discoverTree(root, root.asset.Term(), dt)
// 		calculateTree(root, root.asset.Term(), dt)
// 		updateTree(root, root.asset.Term(), dt)
// 	case msg := <-fimsReceive:
// 		// Receive inputs over FIMS, process (but don't update)
// 		// fmt.Println("I have received", msg.Uri, msg.Replyto, msg.Body)
// 		receiveFims(msg, fimsMap)
// 	case <-pubTicker.C:
// 		// Publish everything, maybe the maps are updated under the state ticker,
// 		// and simply output every field in each struct
// 		if !f.Connected() {
// 			log.Fatalf("FIMS no longer connected")
// 		}
// 		for k, v := range fimsMap {
// 			f.Send(fims.FimsMsg{
// 				Method: "pub",
// 				Uri:    fmt.Sprintf("/components/%s", k),
// 				Body:   v,
// 			})
// 		}
// 	}
// }
// }
