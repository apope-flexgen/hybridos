package research

// test plan
// 12_24_2022
//
// get fims running done
// need to multi thread json marshall
// but note that fims decodes the object. Do we want that probably but we need multi threading
// use geturi to work out how to handle input options
// create setvar  getvar
// handle "value"
// complete merge in progress
// put flags to work on merge
// setup locks pubs are threaded
// trigger metrics
// work out sparse pubs
// sets == push metrics
// gets/pubs == pull metrics
// templates

// setvalue story
// we'll keep the ids seperate to cut down the complexity
// set /myuri/myspace id myobj 1234
// if /myuri/myspace:myobj does not exist
//    set /myuri/myspace:myobj.value 1234
// if /myuri/myspace:myobj does  exist
//    set /myuri/myspace:myobj.value 1234
// set /myuri/myspace:myobj.foo 3456
// if /myuri/myspace:myobj.value does exist
// set /myuri/myspace:myobj.foo 3456

import (
	"encoding/json"
	"fims"
	"fmt"
	"log"
	"os"
	"os/exec"
	"time"
)

var f fims.Fims
var fimsReceive chan fims.FimsMsg
var uris []string
var outputTicker *time.Ticker
var pubTicker = make(chan string)
var pubs = 0
var sets = 0
var gets = 0
var baseTime time.Time

type defUri struct {
	buff   []byte
	uri    string
	format string
	flags  int
}

func SetBaseTime() {
	baseTime = time.Now()
}

func GetTimeDoubleNs() float64 {
	elapsed := time.Since(baseTime)
	ef := float64(elapsed)
	//	fmt.Println("  base :", baseTime, " elapsed:", elapsed, " ef [", ef, "]\n")
	//	fmt.Printf(" ef  type [%T] val [%f]\n", ef, ef)

	return ef

}
func setVarParam(vmap map[string]interface{}, flags int, uri, id, param string, value interface{}) (interface{}, error) {
	// decode the uri, set up flags
	//vmap := GetVmapUri(vmapin, uri)
	// switch *vmapin.(type) {
	// case map[string]interface{}:
	// 	break
	// default:
	// 	*vmapin = make(map[string]interface{})
	// }
	//vmap := *vmapin
	//	vmapin = &vmap

	myvar := vmap[uri]
	if myvar == nil {
		vmap[uri] = make(map[string]interface{})
	}
	myvar = vmap[uri]
	if myvar == nil {
		myvar.(map[string]interface{})[uri] = make(map[string]interface{})
		myvar = myvar.(map[string]interface{})[uri]
	}
	myvar = myvar.(map[string]interface{})[id]
	if myvar == nil {
		myvar = vmap[uri]
		myvar.(map[string]interface{})[id] = make(map[string]interface{})
		myvar = myvar.(map[string]interface{})[id]
	}

	// TODO lock uri
	myvar.(map[string]interface{})[param] = value
	mymets, myok := myvar.(map[string]interface{})["metrics"]
	fmt.Println("  Metrics :", mymets, " myok:", myok, "\n")

	// TODO unlock uri
	// TODO do we have any metrics ??
	// If so and we are allowed to process metrics then run away.

	//fmt.Print(" myvar   #1 :", myvar, "\n")
	//fmt.Print(" vmap #2 :", vmap, "\n")
	// find the base uri , create one  if the noexpand flag is not set
	// find the variable  (by ID ) , create one if the noexpand flag is not set, or return , we are not interested in this uri in this message.
	// writelock the base uri
	// set the  param in the object which must be a `map[string]interface{}`
	// see if the variable has any metrics
	// write unlock component, run them if the metrics operation is enabled
	return value, nil
}

func setVar(vmap map[string]interface{}, flags int, uri, id string, value interface{}) (interface{}, error) {
	return setVarParam(vmap, flags, uri, id, "value", value)
}

func getVarParam(vmap map[string]interface{}, flags int, uri, id, param string) (interface{}, error) {
	myvar, myok := vmap[uri]
	if !myok {
		fmt.Printf(" cannot find uri [%s] /n", uri)
		return nil, nil
	}

	myvar, myok = myvar.(map[string]interface{})[id]
	if !myok {
		fmt.Printf(" cannot find id [%s] /n", id)
		return nil, nil
	}
	myval, myvok := myvar.(map[string]interface{})[param]
	if !myvok {
		fmt.Printf(" cannot find param [%s] /n", param)
		return nil, nil
	}
	return myval, nil

	// decode the uri, set up flags
	// find the base uri, do not create one if missing
	// find the variable  (by ID ) , do not create one if missing
	// if metrics and metrics are enabled , run get metrics
	// writelock the base uri
	// get the param object in the variable object which must be a `map[string]interface{}`
	// writeunlock the base uri
	//	return myvar, nil
}
func getVar(vmap map[string]interface{}, flags int, uri, id string) (interface{}, error) {
	return getVarParam(vmap, flags, uri, id, "value")
}

// look for expand, metrics, full, naked,clothed, uri
// TODO look for single
// TODO look for params
// TODO look for index
func GetUri(inUri []byte, base string) *defUri {
	//TODO does this need to be deleted
	g := new(defUri)
	g.buff = inUri
	// a := bytes.Split(inUri, []byte("/"))
	// if string(a[1]) == "echo_test" {
	// 	for ii, ix := range a {
	// 		//if ii == 0 {
	// 		//	continue
	// 		//}
	// 		fmt.Printf(" ix = %d =>", ii)
	// 		fmt.Println(string(ix))
	// 	}
	// }
	ic := 0
	iterm := 0
	for ii, ix := range inUri {
		if ix == '/' && ii > 0 {
			sv := string(inUri[ic:ii])
			fmt.Printf(" ii => %d [%s] ", ii, sv)
			if sv == "/"+base {
				fmt.Printf(" base \n")
			} else if sv == "metrics" {
				fmt.Printf(" metrics \n")
			} else if sv == "expand" {
				fmt.Printf(" expand \n")
			} else if sv == "uri" {
				fmt.Printf(" uri \n")
				iterm = ii
				break
			} else {
				iterm = ic - 1
				fmt.Printf(" done \n")
				break
			}

			ic = ii + 1
		}
	}
	g.uri = string(inUri[iterm:])
	fmt.Printf(" Uri at end #1 = [%s] \n", string(inUri[iterm:]))
	fmt.Printf(" Uri at end #2 = [%s] \n", g.uri)
	return g
}

func timeTrack(start time.Time, name string) {
	elapsed := time.Since(start)
	log.Printf("%s took %s", name, elapsed)
}

func runjUnmarshal(data []byte, fx *interface{}) (err error) {
	//var f interface{}
	defer timeTrack(time.Now(), "json unmarshal")
	return json.Unmarshal(data, fx)
}

func runjMarshal(v *interface{}) (output []byte, err error) {
	defer timeTrack(time.Now(), "json marshal")
	return json.Marshal(*v)
}

func runMergeJsons(vmap interface{}, uri string, json1, json2 interface{}, flags, depth int) (json_out interface{}) {
	//var f interface{}
	defer timeTrack(time.Now(), "MergeJson")
	return MergeJsons(vmap, uri, json1, json2, flags, depth)
}

func getVmapVar(vmapin interface{}, uri, id string) (interface{}, error) {
	vmap := GetVmapUri(vmapin, uri)
	ret, reterr := vmap.(map[string]interface{})[id]
	fmt.Print("getVmapVar ->  id   [", id, "] ret [", ret, "] err [", reterr, "]\n")
	return ret, nil
}

func GetVmapUri(vmapin interface{}, uri string) interface{} {
	if vmapin == nil {
		vmapin = make(map[string]interface{})
	}
	vm, ok := vmapin.(map[string]interface{})[uri]
	fmt.Println("#1 vm :", vm, " ok :", ok)
	if !ok {
		vmapin.(map[string]interface{})[uri] = make(map[string]interface{})
	}
	vm, ok = vmapin.(map[string]interface{})[uri]
	fmt.Println("#2 vm :", vm, " ok :", ok)
	return vm
}

func setVmapVar(vmapin interface{}, uri, id string, val interface{}) (interface{}, bool) {
	vmap := GetVmapUri(vmapin, uri)
	ret, reterr := vmap.(map[string]interface{})[id]
	fmt.Print("setVmapVar ->  id   [", id, "] ret [", ret, "] err [", reterr, "]\n")
	if !reterr {
		ret.(map[string]interface{})["value"] = val

	} else {
		vmap.(map[string]interface{})[id] = make(map[string]interface{})
		vmap.(map[string]interface{})[id].(map[string]interface{})["value"] = val
	}
	return ret, reterr
}

// run the Echo Metrics
// we have a list of inputs , may only need one
// we have an optional list of outputs may need none
// map the input to key item
// key[value] == input
// outputs[value] == input

func runMetricsEcho(vmap, key, iok interface{}) {

	val := key.(map[string]interface{})["initialInput"]
	inputs, inputsok := iok.(map[string]interface{})["inputs"]
	fmt.Print("Echo ->  inputs  [", inputs, "] inputsok [", inputsok, "]\n")
	if inputsok {
		switch inputs.(type) {
		case []interface{}:
			for inputix, input := range inputs.([]interface{}) {
				fmt.Print("Echo ->  inputix  [", inputix, "] input [", input, "]\n")
				uri := input.(map[string]interface{})["uri"]
				id := input.(map[string]interface{})["id"]

				fmt.Print("       Echo ->  uri  [", uri, "] id [", id, "]\n")
				op, operr := getVmapVar(vmap, uri.(string), id.(string))
				fmt.Print("             Echo ->  op  [", op, "] operr [", operr, "]\n")
				fmt.Print("                      key  [", key, "]\n")
				if operr == nil {
					val = op
				}
			}
		}
	}
	key.(map[string]interface{})["value"] = val
	outputs, outputsok := iok.(map[string]interface{})["outputs"]
	if outputsok {
		switch outputs.(type) {
		case []interface{}:
			for _, opaok := range outputs.([]interface{}) {
				uri := opaok.(map[string]interface{})["uri"]
				id := opaok.(map[string]interface{})["id"]

				fmt.Print("       Echo outputs ->  uri  [", uri, "] id [", id, "]\n")
				//op, operr :=
				setVmapVar(vmap, uri.(string), id.(string), val)
			}
		}
	}
}

func runMetricsSelect(vmap, key, iok interface{}) {
	im, imok := iok.(map[string]interface{})["inputs"]
	fmt.Print("Select -> im  [", im, "] imok [", imok, "]\n")
}

// TODO put the metrics functions into a map.
func runMetrics(vmap, key, vkey interface{}) {
	defer timeTrack(time.Now(), "Running Metrics")
	fmt.Print(" Metrics for  [", key, "] \n")
	switch vkey.(type) {
	case []interface{}:
		for id, iok := range vkey.([]interface{}) {
			fmt.Print(" id  [", id, "] iok [", iok, "]")
			im, imok := iok.(map[string]interface{})["operation"]
			fmt.Print(" im  [", im, "] imok [", imok, "]\n")
			switch im {
			case "echo":
				fmt.Printf(" run metrics ==> Echo \n")
				runMetricsEcho(vmap, key, iok)
			case "select":
				fmt.Printf(" run metrics ==> Select \n")
				runMetricsSelect(vmap, key, iok)

			default:
				fmt.Printf(" undefined metrics \n")

			}

		}
	default:
		fmt.Printf(" vkey type %T\n", vkey)
	}
}

// merge jsons
// the uri tells us where to start
// we have to unpack the uri and then find the base vmap
// when we unpack the flags tell us if we can create new entries in the uris or vmap area.
// the uri is really an initial vmap entry
// uris can be "locked" using read/write access locks
//
// json1 is merged into json2
//

func MergeJsons(vmapin interface{}, uri string, json1, json2 interface{}, flags, depth int) (json_out interface{}) {
	vmap := GetVmapUri(vmapin, uri)
	iok1 := json1
	//iMap1, ok1 := iok1.(map[string]interface{})
	iMap1, _ := iok1.(map[string]interface{})
	//fmt.Print("ok1 :")
	//fmt.Println(ok1)
	if json2 == nil {
		json2 = json1
		return json2
	}
	iok2 := json2
	iMap2, _ := iok2.(map[string]interface{})
	for idx, key := range iMap1 {
		fok2 := iMap2[idx]
		//fmt.Println(fok2)
		if fok2 == nil {
			fmt.Printf("[%d] - Merge nill [%s  %s]\n", depth, idx, key)
			fmt.Println("        iMap1 idx ", iMap1[idx])
			fmt.Println("        iMap2 idx ", iMap2[idx])
			fmt.Println("        iMap1", iMap1)
			fmt.Println("        iMap2", iMap2)
			iMap2[idx] = iMap1[idx]
		} else {
			switch key.(type) {

			case map[string]interface{}:
				// look for a key value in the map

				fmt.Printf("[%d] - Merge map [%s  %f]\n", depth, idx, key)
				fmt.Println("        iMap1 ", iMap1[idx])
				fmt.Println("        iMap2 ", iMap2[idx])
				fmt.Printf("       iMap2 Type %T \n", iMap2[idx])
				if depth == 0 {
					vkey, vok := key.(map[string]interface{})["value"]
					// if vok true then we call setValue at this level.
					if vok {
						// iMap2[key] is our variable
						fmt.Printf("[%d] - value for [", depth)
						fmt.Print(key)
						fmt.Printf("] Found Value [")
						fmt.Print(vkey)
						fmt.Printf("]\n")
					}
					// if vok true and we have the metrics flag  then we call run metrics at this level.
					switch iMap2[idx].(type) {
					case map[string]interface{}:

						vkey, vok = iMap2[idx].(map[string]interface{})["metrics"]
						if vok {
							fmt.Printf("[%d] - metrics test:", depth)
							fmt.Printf(" Found Metrics [")
							fmt.Print(vkey)
							fmt.Printf("]\n")
							runMetrics(vmap, iMap2[idx], vkey)
						}
					}
				}

				// iMap2[idx] has to be a map[string]interface for this to work
				switch iMap2[idx].(type) {
				case map[string]interface{}:
					MergeJsons(vmap, uri, iMap1[idx], iMap2[idx], flags, depth+1)
				default:
					iMap2[idx] = iMap1[idx]
				}

			default:
				fmt.Printf(" Replaced [%s]\n", idx)
				iMap2[idx] = iMap1[idx]
			}
			// if key.(type) == map[string]interface{} we now recurse into
			//MergeJsons(vmap, uri, iMap1[idx], &iMap2[idx], flags, depth+1)
			//
		}
		//switch keyt :=
		switch key.(type) {
		case bool:
			fmt.Printf("Merge bool [%s  %s]\n", idx, key)
		case string:
			fmt.Printf("Merge string [%s  %s]\n", idx, key)
		case float64:
			fmt.Printf("Merge float [%s  %f]\n", idx, key)
		case map[string]interface{}:
			fmt.Printf("Merge map [%s  %f]\n", idx, key)
		default:
			fmt.Printf("Merge unknown [%s  %T]\n", idx, key)
		}
		// 				// we need to know what json1.(map[string]interface{})[idx] is
	}
	return json2
}

//var temp interface{}

func fimsSet(name, value string) {
	myval := "{\"" + name + "\":" + value + "}"
	cmd := exec.Command("/usr/local/bin/fims_send", "-mset", "-u/echo_test/foo", myval)
	err := cmd.Run()
	if err != nil {
		log.Fatal(err)
	}

}

func runTest() {
	fmt.Println("Running test")
	fimsSet("ok", "1234")
	fimsSet("myid1", "100000")
	fimsSet("ok2", "\"hello kitty\"")
	fimsSet("ok", "{\"value\":1234}")
	fimsSet("ok", "{\"test\":444}")
	fimsSet("ok", "{\"initialInput\":44400}")
	metrics1 := "{\"id\":\"metrics1\",\"operation\":\"echo\",\"inputs\":[{\"uri\":\"/some/uri\",\"id\":\"myid1\"}]}"
	metrics2 := "{\"id\":\"metrics2\",\"operation\":\"select\"}"
	fimsSet("ok", "{\"metrics\":["+metrics1+","+metrics2+"]}")
	fimsSet("ok3", "{\"value\":5000}")

	return

}
func main() {

	SetBaseTime()
	t1 := GetTimeDoubleNs()
	t2 := GetTimeDoubleNs()
	fmt.Println("t2-t1 :", t2-t1)

	var vmap interface{}
	//var base interface{}

	if len(os.Args) > 1 {
		fmt.Println("arg :", os.Args[1])
		if os.Args[1] == "test" {
			runTest()
			return
		}

	}

	fmt.Println("New Echo test")
	setupFims()
	go publishRate("/hello/pub/test", 1000, pubTicker) //Multiple Publish rate function

	g := GetUri([]byte("/echo_test/expand/metrics/hello/pub/test"), "echo_test")
	fmt.Printf(" GetUri g [%s]\n", g.uri)
	//vmap =
	GetVmapUri(vmap, g.uri)
	fmt.Println(" new vmap :[", vmap, "]\n")
	myval := 21.34
	vmapi := make(map[string]interface{})
	setVarParam(vmapi, 0, "/components/test", "tstvar", "value", myval)
	fmt.Print("setVarParam vmapi #2 ", vmapi, "\n")

	myvar, myok := getVarParam(vmapi, 0, "/components/test", "tstvar", "value")
	fmt.Print("myval #3 ", myvar, " myok :", myok, "\n")

	GetUri([]byte("/echo_test/uri/metrics/hello/pub/test"), "echo_test")

	for i := 0; i < 10000; i++ { //testing profiling
		select {
		case msg := <-fimsReceive:
			if msg.Method == "set" {
				sets++
				// we have temp and base structures
				//       map[string]interface{}

				if true {
					//err := json.Unmarshal(msg.Body.([]byte), &temp)
					//fmt.Println(msg)
					//fmt.Println(msg.Body.(map[string]interface{})["test"])
					var fx interface{}
					err := runjUnmarshal(msg.Body.([]byte), &fx)

					if err != nil {
						fmt.Println("json error", err)
						fmt.Println(string(msg.Body.([]byte)))
					}
					fmt.Println("incoming:", fx)
					vmap = runMergeJsons(vmap, msg.Uri, fx, vmap, 0, 0)
					fmt.Println("merged:", vmap)
				}

				// // if prefix == echo_test
				// read body into temp structure
				// merge object after the prefix into the base structure

				//
				// need to use geturi to pick out the base and the source
				// Separating out the Uri for map traversal
				// base := strings.Replace(msg.Uri, "/"+msg.Frags[len(msg.Frags)-1], "", 1)
				//source := msg.Frags[len(msg.Frags)-1]
			} else if msg.Method == "pub" { // publish from an input device, most common use case
				pubs++
				// read body into temp vmap
				// merge into be base vmap
				//addToInput(msg)
			} else if msg.Method == "get" && msg.Replyto != "" {
				gets++
				//var data []byte
				// todo get the Uri from vmap , skip the prefix
				data, err := runjMarshal(&vmap)
				if err != nil {
					fmt.Println(err)
					fmt.Println(vmap)
				}
				fmt.Println(string(data))
				// need to use geturi to pick out the base and the source for the get
				// unmarshall and send
				sendvmap := vmap
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    msg.Replyto,
					Body:   sendvmap,
				})
			}
		case uri := <-pubTicker: //Channel for multi publish rates
			if !f.Connected() {
				// nope we need to try and pick up the connection
				s := fmt.Sprintf("No Fims for Pub # %s", uri)
				log.Fatal(s)
			}
			// note this may be their own threads
			// find pub object from the uri
			// s := fmt.Sprintf("Hello Kitty, Pub # %d", i)
			// f.Send(fims.FimsMsg{
			// 	Method: "pub",
			// 	Uri:    uri,
			// 	Body:   s,
			// 	})
		}
	}
}

// Connect and subscribe using fims
func setupFims() {
	var err error

	// subscribe to current process PID for config updates
	// pid := os.Getpid()
	// uris = append(uris, strconv.Itoa(pid))

	// subscribe to all inputs and outputs
	//for _, output := range cfg.Outputs {
	//	for _, input := range output.Inputs {
	//		if !findString(input.Uri, uris) {
	//			uris = append(uris, input.Uri)
	//		}
	//	}
	//	if !findString(output.Uri, uris) {
	//		uris = append(uris, output.Uri)
	//	}
	//}
	uris = append(uris, "/echo_test")
	// connect to fims
	f, err = fims.Connect("echo_main")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	// subscribe on fims with slice of uris using variadic args
	err = f.Subscribe(uris...)
	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	fmt.Print("subscribed to :")
	fmt.Println(uris)

	// launch go routine for receive channel
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveRawChannel(fimsReceive)
}

func publishRate(uri string, rate float64, co chan<- string) {
	//var elapsed = 0
	var count = 0
	for {
		var tsleep = (time.Duration(rate) * time.Millisecond) //- elapsed
		time.Sleep(tsleep)
		// we may pull the data from the base vmap
		// process all the blocks
		// push data back into the base vmap
		// unmarshall and send
		s := fmt.Sprintf("Hello Kitty, Pub # %d", count)
		f.Send(fims.FimsMsg{
			Method: "pub",
			Uri:    uri,
			Body:   s,
		})
		count++
		co <- uri
	}
}
