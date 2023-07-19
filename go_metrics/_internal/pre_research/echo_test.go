package research

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"strings"

	"time"

	"github.com/flexgen-power/echo/pkg/config"
	"github.com/flexgen-power/echo/pkg/metrics"

	"github.com/buger/jsonparser"
	"github.com/pkg/profile"
)

// Variables used for command line flags
var mode string
var scfg string
var prof string
var outputpath string
var ipaddress string

// We dont need these structs anymore
// Structures for echo json and processMap structure
type Config struct {
	Outputs []Output `json:"outputs"`
}

type Output struct {
	Uri         string                 `json:"uri"`
	PublishRate float64                `json:"publishRate"`
	Heartbeat   string                 `json:"heartbeat,omitempty"`
	Inputs      []Input                `json:"inputs"`
	Echo        map[string]interface{} `json:"echo"`
	Metrics     []metrics.Metrics      `json:"metrics"`
}

type Input struct {
	Uri       string                 `json:"uri"`
	Registers map[string]interface{} `json:"registers"`
}

type Direction struct { //Direction struct used for if values need to go to twins or echo registers
	Forward  map[string]interface{}
	Backward map[string]interface{}
}

type Link struct { //Link struct used to store uri and register of input registers (mainly twins)
	Uri      string
	Register string
}

var cfg Config
var f fims.Fims
var fimsReceive chan fims.FimsMsg
var outputTicker *time.Ticker
var pubTicker = make(chan string)
var firsttime bool = true
var uris []string

var inputMap map[string]map[string]interface{}
var outputMap map[string]interface{}
var processMap map[string]Direction
var metricsMap map[string][]metrics.Metrics
var start = time.Now()

func main() {
	log.SetPrefix("ECHO_TEST: ")
	cfgpath := os.Args[len(os.Args)-1]

	// Handle inital flag inputs for cli commands
	flag.StringVar(&mode, "mode", "", "set mode: [modbus | dnp3]")
	flag.StringVar(&scfg, "c", "", "client config file path for server echo generation only")
	flag.StringVar(&outputpath, "output", "", "this is the file path where the server and echo file will be going")
	flag.StringVar(&ipaddress, "ip", "0.0.0.0", "address used in the server files fo r client server connection")
	flag.StringVar(&prof, "prof", "", "This is for profiling only contact Claire to get permission")
	flag.Parse()

	if prof == "cpu" { //profiling argument, only works if flags are in front of config file
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	}

	// // Handle execution for runtime or strictly file generation
	// if scfg != "" {

	// 	// Generate a server.json, and a echo.json
	// 	// Generate our files, retrieve our client data
	// 	c, err := config.GenerateClientAndServerFile(mode, scfg, outputpath, ipaddress)
	// 	if err != nil {
	// 		log.Fatalf("error generating client server file: %v", err)
	// 	}

	// 	// Generate an echo json file for the given client
	// 	if err := c.GenerateEchoJSONFile(scfg, outputpath); err != nil {
	// 		log.Fatalf("error generating echo json file: %v", err)
	// 	}
	// } else {
	//Set up functions to initialize some maps
	setupConfig(cfgpath)
	setupFims()
	// setupInputMap()
	// setupProcessMap()
	// setupOutputMap()
	// setupMetrics()

	for _, out := range cfg.Outputs {
		if out.PublishRate == 0 { //if no publishRate, then default will be 100 milliseconds
			out.PublishRate = 10
		}
		//go publishRate(out.Uri, out.PublishRate, pubTicker) //Multiple Publish rate function
	}

	for { //i := 0; i < 10000; i++ { //testing profiling
		select {
		case msg := <-fimsReceive:
			if msg.Uri != "" {
				//Separating out the Uri for map traversal
				base := strings.Replace(msg.Uri, "/"+msg.Frags[len(msg.Frags)-1], "", 1)
				source := msg.Frags[len(msg.Frags)-1]
				if msg.Method == "pub" { // publish from an input device, most common use case
					addToInput(msg)
				} else if msg.Method == "get" { //Getting from the map of all the outputs
					if outputMap[base] != nil { //Same thing as msg.Frags >= 3
						f.Send(fims.FimsMsg{
							Method: "set",
							Uri:    msg.Replyto,
							Body:   outputMap[base].(map[string]interface{})[source],
						})
					} else if outputMap[msg.Uri] != nil { //Same thing as msg.Frags == 2
						f.Send(fims.FimsMsg{
							Method: "set",
							Uri:    msg.Replyto,
							Body:   outputMap[msg.Uri],
						})
					}
				} else if msg.Method == "set" {

					// if msg.Nfrags >= 3 {
					// 	processMapTraversal(base, source, msg.Body) //same thing as msg.Frags == 3
					// }

					// if outputMap[msg.Uri] != nil { //same thing as msg.Frags == 2
					// 	for k, v := range msg.Body.(map[string]interface{}) { //<- This is loop of msg.Body
					// 		processMapTraversal(msg.Uri, k, v)
					// 	}
					// }

					//addMetrics()
				}
			} else {
				log.Println("The uri is nil please check your fims message")
			}
		case uri := <-pubTicker: //Channel for multi publish rates
			if !f.Connected() {
				log.Fatal("No fims connection")
			}
			f.Send(fims.FimsMsg{
				Method: "pub",
				Uri:    uri,
				Body:   outputMap[uri],
			})
			inputMap[uri] = outputMap[uri].(map[string]interface{}) //Add all outputs into input map
			//case <-outputTicker.C:
			//	addToOutput() //constantly updating and publishing the output uris
		}
		if len(uris) == len(inputMap) && len(cfg.Outputs) == len(outputMap) && firsttime {
			//addMetrics()
			firsttime = false
		}
	}
	//	}
}

// combine inputs into outputs - remapping process
func addToOutput() {
	for _, out := range cfg.Outputs {
		if out.Inputs != nil {
			for _, in := range out.Inputs {
				if processMap[out.Uri].Backward[in.Uri] != nil { // optimize out case when there are no remaps
					// range over registers in backward link section
					// delete remapped and null registers from input
					for k, v := range processMap[out.Uri].Backward[in.Uri].(map[string]interface{}) {
						if inputMap[in.Uri][v.(string)] != nil {
							outputMap[out.Uri].(map[string]interface{})[k] = inputMap[in.Uri][v.(string)]
						} else {
							outputMap[out.Uri].(map[string]interface{})[k] = 0
						}
					}
				}
			}
		}
		if out.Heartbeat != "" { // If heartbeat enabled, constantly change the time the program was started
			outputMap[out.Uri].(map[string]interface{})[out.Heartbeat] = float64(time.Since(start)) / 1000000000.0
		}
	}
}

// Process sleeps for however long the publish rate is
func publishRate(uri string, rate float64, co chan<- string) {
	for {
		time.Sleep(time.Duration(rate) * time.Millisecond)
		co <- uri
	}
}

// Add all input publishes bodies into the input map
func addToInput(msg fims.FimsMsg) {
	inputMap[msg.Uri] = msg.Body.(map[string]interface{})
}

/* Process map example
processMap: {
	"/components/ess_1": {
		forward: {
			"active_power": {
				uri: "/components/pcs_1"
				register: "p"
			},
			"active_power_setpoint": {
				uri: "/components/pcs_1"
				register: "pcmd"
			}
			...
		},
		backward: {
			"/components/pcs_1": {
				"p": "active_power",
				"pcmd": "active_power_setpoint",
				"dvoltage": null
				...
			}
		}
	},
	"/components/ess_2": {
		...
	}
}
*/

// Making Forward and Backward Link maps in the process Map
func setupProcessMap() {
	processMap = make(map[string]Direction)
	var dirt Direction

	// iterate over outputs
	for _, n := range cfg.Outputs {
		dirt.Forward = make(map[string]interface{})
		dirt.Backward = make(map[string]interface{})
		processMap[n.Uri] = dirt // key (n.Uri) - "/components/ess_1"

		// insert link registers
		for _, m := range n.Inputs {
			for k, v := range m.Registers { // k - "active_power", v (remap) - "p"
				if v != nil { // do not add if remap is 'null' - hide unwanted registers
					var temp Link

					temp.Uri = m.Uri           // "/components/pcs_1"
					temp.Register = v.(string) // "p"

					// forward links are used to support "set" forwarding to an input
					processMap[n.Uri].Forward[k] = temp
				}
			}

			// backward links are used to avoid duplicated values in output publish
			processMap[n.Uri].Backward[m.Uri] = m.Registers
		}
	}

	// debug printing
	newBytes, _ := json.MarshalIndent(processMap, "", "\t")
	fmt.Println(string(newBytes))
}

// create map to hold output publishes - combining occurs in addToOutput
func setupOutputMap() {
	outputMap = make(map[string]interface{})
	for _, n := range cfg.Outputs {
		outputMap[n.Uri] = make(map[string]interface{})
		if n.Echo != nil {
			outputMap[n.Uri] = n.Echo // add echo registers immediately
		}
	}

	// create timer channel for limit rate process
	outputTicker = time.NewTicker(time.Duration(100) * time.Millisecond)
}

// create map to hold input fims publishes
func setupInputMap() {
	inputMap = make(map[string]map[string]interface{})
}

// Connect and subscribe using fims
func setupFims() {
	var err error

	// subscribe to current process PID for config updates
	// pid := os.Getpid()
	// uris = append(uris, strconv.Itoa(pid))

	// subscribe to all inputs and outputs
	for _, output := range cfg.Outputs {
		for _, input := range output.Inputs {
			if !findString(input.Uri, uris) {
				uris = append(uris, input.Uri)
			}
		}
		if !findString(output.Uri, uris) {
			uris = append(uris, output.Uri)
		}
	}
	// connect to fims
	f, err = fims.Connect("echo")
	if err != nil {
		log.Fatal("Unable to connect to FIMS server")
	}

	// subscribe on fims with slice of uris using variadic args
	err = f.Subscribe(uris...)
	if err != nil {
		log.Fatal("Unable to subscribe")
	}

	// launch go routine for receive channel
	fimsReceive = make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)
}

// Read the echo.json and make sure there are no duplicates in it
func setupConfig(path string) {
	configJSON, err := ioutil.ReadFile(path)
	if err != nil {
		log.Fatalf("could not read config file: ", err)
	}
	outBytes, _, _, _ := jsonparser.Get(configJSON, "outputs")
	err = json.Unmarshal(outBytes, &cfg.Outputs) // simple unmarshal is sufficient as data structure matches json
	if err != nil {
		log.Fatalf("failed to unmarshal config file: ", err)
	}

	// debug printing
	fmt.Println(cfg.Outputs[0].Uri)
	fmt.Println(cfg.Outputs[0].PublishRate)

	// Checking for duplications in the inputs remap and echo portion
	for _, m := range cfg.Outputs { //loop through outputs
		var regslice []string        //create new slice for each output
		for _, n := range m.Inputs { //loop through inputs
			if n.Uri == m.Uri {
				log.Fatalf("THERE CAN BE NO DUPLICATES URIS, Output: %s == Input: %s", m.Uri, n.Uri)
			}
			for r := range n.Registers { //loop though registers
				if r != "" { // make sure there is something
					if !findString(r, regslice) { // Check if remap name already exists
						regslice = append(regslice, r) //Add to regslice
					} else {
						log.Fatalf("THERE CAN BE NO DUPLICATES IN INPUT REGISTERS", r)
					}
				}
			}
		}
		//Same things as above
		for e := range m.Echo {
			if !findString(e, regslice) {
				regslice = append(regslice, e)
			} else {
				log.Fatalf("THERE CAN BE NO DUPLICATES IN ECHO REGISTERS ", e)
			}
		}
	}
}

func setupMetrics() {
	metricsMap = make(map[string][]metrics.Metrics)
	for _, out := range cfg.Outputs {
		metricsMap[out.Uri] = out.Metrics
	}
}

// Substitute for a contain function for slices
func findString(target string, list []string) bool {
	for _, a := range list {
		if a == target {
			return true
		}
	}
	return false
}

func addMetrics() {
	for u, m := range metricsMap { //u = i.e. /components/ess_1, m = u's metric body
		for _, n := range m { //n = each metric object
			strList := strings.Split(n.Uri, "/")
			base := strings.Replace(n.Uri, "/"+strList[len(strList)-1], "", 1)
			source := strList[len(strList)-1]
			var value interface{}
			// for _, op := range n.Operation {
			switch n.Operation[0] {
			case "add", "subtract", "multiply", "divide":
				value = n.Basic(inputMap)
			case "and", "or", "not":
				value = n.Logic(inputMap)
			case "forward":
				for _, reg := range n.Inputs {
					uri := reg["uri"].(string)
					regi := reg["id"].(string)
					if processMap[u].Backward[uri] != nil {
						var cheat string
						for k, v := range processMap[u].Backward[uri].(map[string]interface{}) {
							if v == regi {
								cheat = k
								break
							}
						}
						if processMap[u].Backward[uri].(map[string]interface{})[cheat] != nil {
							value = inputMap[uri][regi]
							// fmt.Print("FIRST\n")
							processMapTraversal(base, source, value)
						}
					} else if processMap[uri].Forward[regi] != nil {
						uri2 := processMap[uri].Forward[regi].(Link).Uri //Extract the Uri and register for the Forward link
						regi2 := processMap[uri].Forward[regi].(Link).Register
						value = inputMap[uri2][regi2]
						// fmt.Print("SECOND\n")
						processMapTraversal(base, source, value)
						// fmt.Println(outputMap[reg["uri"].(string)].(map[string]interface{})[reg["id"].(string)])
					} else {
						value = outputMap[uri].(map[string]interface{})[regi]
						// fmt.Print("THIRD\n")
						processMapTraversal(base, source, value)
					}
				}
			}

			if value != nil && base == u {
				outputMap[base].(map[string]interface{})[source] = value
			} else if value != nil {
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    n.Uri, //Send the change to Twins
					Body:   value,
				})
				if inputMap[base][source] != nil { //In case twins isnt running
					inputMap[base][source] = value
				}
			}
		}
	}
}

func processMapTraversal(base string, source string, value interface{}) {
	if outputMap[base] != nil { //Same thing as msg.Frags >= 3
		// fmt.Printf("here %s %s\n", base, source)
		if processMap[base].Forward[source] != nil { //If Register that is being changed is from an input Uri
			// fmt.Printf("here %s %s\n", base, source)
			uri := processMap[base].Forward[source].(Link).Uri //Extract the Uri and register for the Forward link
			reg := processMap[base].Forward[source].(Link).Register
			combine := uri + "/" + reg //Put them together for sending a fims send

			outputMap[base].(map[string]interface{})[source] = value

			/*
				if inputMap[uri][reg] != nil { //In case twins isnt running
					inputMap[uri][reg] = value //inout Map is updated
				}
			*/

			if value != nil {
				f.Send(fims.FimsMsg{
					Method: "set",
					Uri:    combine, //Send the change to Twins
					Body:   value,
				})
			}
		} else if outputMap[base].(map[string]interface{})[source] != nil { //if register changed is only an echo register
			outputMap[base].(map[string]interface{})[source] = value
		}
	} else {
		if inputMap[base][source] != nil { //In case twins isnt running
			inputMap[base][source] = value //inout Map is updated
		}
	}
}
