package main

import (
	"fims"
	"fmt"
	"log"
	"reflect"
	"time"
)

var fimsReceive chan fims.FimsMsg
var t0 time.Time
var stateTicker, pubTicker *time.Ticker
var stateUpdate chan float64
var f fims.Fims
var config cfg
var root treeNode
var assets map[string]asset
var cmds map[string]asset
var fimsMap map[string]interface{}

// init() initializes the data structures and configurations for Twins.
// It makes calls to routines in config.go, dies if it can't properly
// configure, and makes everything ready for main()
func runConfig() {
	var err error
	log.SetPrefix("Twins: ")
	log.Printf("Configuring")
	readConfig(&config)
	fimsMap = make(map[string]interface{})
	root, assets, err = buildState(&config, fimsMap)
	if err != nil {
		log.Fatalf("Couldn't put together the tree: %s", err)
	}
	//fmt.Printf("%#v", root)
	f, err = fims.Connect("Twins")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server: %v", err)
	}
	fimsReceive = make(chan fims.FimsMsg)
	err = f.Subscribe("/components", "/twins")
	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	go f.ReceiveChannel(fimsReceive)
	stateUpdate = make(chan float64)
	stateTicker = time.NewTicker(time.Duration(config.UpdateRate) * time.Millisecond)
	pubTicker = time.NewTicker(time.Duration(config.PublishRate) * time.Millisecond)
	t0 = time.Now()
}

// main() consists of an infinite loop and a select statement that blocks until any
// of the following channels are ready with data:
// 1. stateTicker - this is a fixed rate timer for free running solver updates
// 2. stateUpdate - set by either the stateTicker or a manual tick message, runs the solver
// 3. fimsReceive - a FIMS message has been received and needs to be handled
// 4. pubTicker - a fixed rate timer for publishing all off the modeled components on FIMS
func main() {
	// init() executes implicitly before main()
	runConfig()
	log.Printf("Starting main loop")

	// A goroutine to handle incoming FIMS messages
	go func() {
		for {
			msg := <-fimsReceive
			// Receive inputs over FIMS, process (but don't update)
			// fmt.Println("I have received", msg.Uri, msg.Replyto, msg.Body)
			receiveFims(msg, fimsMap)
		}
	}()

	for {
		select {
		case t1 := <-stateTicker.C:
			// Ticker channels return a Time struct
			if !config.ManualTick {
				go func() {
					stateUpdate <- t1.Sub(t0).Seconds() * float64(config.TimeMultiplier)
					t0 = t1
				}()
			}
		case dt := <-stateUpdate:
			// Update internal variables of models
			// fmt.Printf("The node %v just processed, ended up with %#v\n", root.value.GetID(), root.value.Term()))
			// updateState(root, root.asset.Term(), dt)
			discoverTree(root, root.asset.Term(), dt)
			calculateTree(root, root.asset.Term(), dt)
			updateTree(root, root.asset.Term(), dt)
		case <-pubTicker.C:
			// Publish everything, maybe the maps are updated under the state ticker,
			// and simply output every field in each struct
			if !f.Connected() {
				log.Fatalf("FIMS no longer connected")
			}
			for k, v := range fimsMap {
				// if k == "bms_1" { //&& v == "status_0_value" {
				// 	if w, ok := v.(map[string]interface{}); ok {
				// 		for key, val := range w {
				// 			if key == "status_0_value" {
				// 				fmt.Println("KEY:", key, "VAL:", reflect.ValueOf(val).Elem(), "ADDRESS:", val)
				// 			}
				// 		}
				// 	}
				// }
				f.Send(fims.FimsMsg{
					Method: "pub",
					Uri:    fmt.Sprintf("/components/%s", k),
					Body:   v,
				})
			}
		}
	}
}

// discoverTree() is responsible for updating the operating modes and first pass
// voltage (v, f, ph) of all assets as it navigates down the tree, and collecting
// droop share (output p and q, droop parameters) information on the way back up
func discoverTree(node treeNode, input terminal, dt float64) (output terminal) {
	// fmt.Println("Discovering", node.asset.GetID(), input)
	childInput := node.asset.UpdateMode(input)
	collected := childInput
	for i, child := range node.children {
		if i == 0 {
			collected = terminal{}
		}
		childOutput := discoverTree(child, childInput, dt)
		collected = combineTerminals(collected, childOutput)
	}
	output = node.asset.GetLoadLines(collected, dt)
	return output
}

// calculateTree() is responsible for distributing droop shared voltage (v, f, ph) down the tree
// (which should be trivial if no grid forming assets are present), and calculating
// output (p, q) for grid following assets on the way back up
func calculateTree(node treeNode, input terminal, dt float64) (output terminal) {
	// fmt.Println("Calculating", node.asset.GetID(), input)
	childInput := node.asset.DistributeVoltage(input)
	collected := childInput
	for i, child := range node.children {
		if i == 0 {
			collected = terminal{}
		}
		childOutput := calculateTree(child, childInput, dt)
		collected = combineTerminals(collected, childOutput)
	}
	output = node.asset.CalculateState(collected, dt)
	// fmt.Println("collected", collected, "for", node.asset.GetID(), "output", output)
	return output
}

// updateTree() is responsible for distributing loads (p,q) collected in the calculateTree
// phase across grid forming assets, then recollecting asset output power (p,q) on the way
// back up
func updateTree(node treeNode, input terminal, dt float64) (output terminal) {
	childInput := node.asset.DistributeLoad(input)
	collected := childInput
	for i, child := range node.children {
		if i == 0 {
			collected = terminal{}
		}
		childOutput := updateTree(child, childInput, dt)
		collected = combineTerminals(collected, childOutput)
	}
	output = node.asset.UpdateState(collected, dt)
	return output
}

// receiveFims() is mostly looking for 'sets' to components
// The primary interface would be through typical control registers,
// such as set points, targets, digital I/O
// The secondary interface is engine or model specific configuration,
// i.e. initialize the model, start/stop simulation time, set time step
func receiveFims(msg fims.FimsMsg, fimsMap map[string]interface{}) {
	if msg.Method == "pub" || msg.Method == "del" || msg.Method == "post" {
		return
	}
	if msg.Nfrags > 0 {
		switch msg.Frags[0] {
		case "components":
			switch msg.Nfrags {
			case 1:
				if msg.Method == "get" && msg.Replyto != "" {
					f.Send(fims.FimsMsg{
						Method: "set",
						Uri:    msg.Replyto,
						Body:   fimsMap,
					})
				}
			case 2:
				if msg.Method == "get" && msg.Replyto != "" {
					x, ok := fimsMap[msg.Frags[1]]
					if ok {
						f.Send(fims.FimsMsg{
							Method: "set",
							Uri:    msg.Replyto,
							Body:   x,
						})
					}
				}
				if msg.Method == "set" {

					// If we get the following fims message body: /components/bms_1 '{"ctrlword1":{"value":1},"ctrlword2":{"value":2}}'
					// Then, in the target URI, we'll update each field with a value
					component, ok := fimsMap[msg.Frags[1]]
					if !ok {
						return
					}
					switch t := msg.Body.(type) {
					case map[string]interface{}:
						for k, v := range t {
							field, ok := component.(map[string]interface{})[k]
							if !ok {
								continue
							}
							processSet(field, fims.FimsMsg{Body: v})
						}
					}
				}
			case 3:
				component, ok := fimsMap[msg.Frags[1]]
				if !ok {
					return
				}
				field, ok := component.(map[string]interface{})[msg.Frags[2]]
				if !ok {
					return
				}
				if msg.Method == "set" {
					processSet(field, msg)
				}
				if (msg.Method == "get" || msg.Method == "set") && msg.Replyto != "" {
					f.Send(fims.FimsMsg{
						Method: "set",
						Uri:    msg.Replyto,
						Body:   field,
					})
				}
			}
		case "twins":
			switch msg.Nfrags {
			case 1:
				if msg.Method == "get" && msg.Replyto != "" {
					f.Send(fims.FimsMsg{
						Method: "set",
						Uri:    msg.Replyto,
						Body:   config,
					})
				}
			case 2:
				switch msg.Frags[1] {
				case "tick":
					if config.ManualTick && msg.Method == "set" {
						var field interface{}
						var dt float64
						field = &dt
						processSet(field, msg)
						if msg.Replyto != "" {
							f.Send(fims.FimsMsg{
								Method: "set",
								Uri:    msg.Replyto,
								Body:   field,
							})
						}
						// Have to launch this as a goroutine since the main for structure is not ready
						// to receive, so this would block forever
						go func() {
							stateUpdate <- dt
						}()
					}
				case "init":
					if config.ManualTick && msg.Method == "set" && msg.Body == true {
						initTree(root)
						if msg.Replyto != "" {
							f.Send(fims.FimsMsg{
								Method: "set",
								Uri:    msg.Replyto,
								Body:   true,
							})
						}
					}
				}
			}
		}
	}
}

// processSet() takes `field`, which is a pointer to a struct's field of any data type,
// and set the contents of the incoming FIMS message body to it
func processSet(field interface{}, msg fims.FimsMsg) {
	var err error
	var ok bool
	var fieldToSet interface{}
	switch t := msg.Body.(type) {
	case map[string]interface{}:
		fieldToSet, ok = t["value"]
		if !ok {
			err = fmt.Errorf("Got a fims set with a multilevel body to a fully qualified URI: Expected {\"value\":<bool, string, number>}")
		}
	default:
		fieldToSet = t
	}
	if err == nil {
		if fieldToSet != nil {
			fieldv := reflect.ValueOf(field).Elem()
			fieldToSetV := reflect.ValueOf(fieldToSet)
			if fieldToSetV.Kind() == fieldv.Kind() ||
				fieldToSetV.Kind() == reflect.Float64 && fieldv.Kind() == reflect.Int {
				switch fieldv.Kind() {
				case reflect.String:
					fieldv.SetString(fieldToSet.(string))
				case reflect.Bool:
					fieldv.SetBool(fieldToSet.(bool))
				case reflect.Float64:
					fieldv.SetFloat(fieldToSet.(float64))
				case reflect.Int:
					fieldv.SetInt(int64(fieldToSet.(float64)))
				default:
					err = fmt.Errorf("%s didn't match a known type %s", field, fieldv)
				}
			} else {
				err = fmt.Errorf("mismatched field type for %s, got %s, expected %s", msg.Uri, fieldToSetV.Kind(), fieldv.Kind())
			}
		} else {
			replyto := msg.Replyto
			if replyto == "" {
				replyto = "anonymous"
			}
			err = fmt.Errorf("got a null value on a FIMS set to %s from %s", msg.Uri, replyto)
		}
	}
	if err != nil {
		log.Println("Received a set that couldn't be handled:", err)
	}
}

// initTree() is responsible for rerunning the Init functions of all assets.
// This is used during a parametric sweep to recalculate asset parameters when a new value is passed to the asset.
func initTree(node treeNode) {
	node.asset.Init()
	for _, child := range node.children {
		initTree(child)
	}
}
