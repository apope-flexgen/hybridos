package main

import (
	"fims"
	"fmt"
	"log"
	"reflect"
	"time"
	"regexp"
	"strings"
	"os"
)

var usage string = 
`
TWINS
FlexGen power system integration plant model. 

::Usage
	/path/to/twins /path/to/config [OPT Arguments]

	Twins will DEFAULT TO DBI LOAD BEHAVIOR unless provided a -f flag or if dbi load fails.

::Arguments
	-h, --help, -man, usage:
		Print this message

	-u, --uri "collection":
		Run TWINS with configuration read from a dbi collection. The following files are expected:
			twins_tree.json 
			twins_dflt.json
			twins_*_updt.json (optional, see confluence documentation)
		if "collection" is specified, twins will attempt to get files from /dbi/collection/
		otherwise the collection will default to /dbi/twins
		Cannot be used with -f
		If neither -u nor -f are provided then twins will first default to "-u" behavior
			if unable to connect to dbi, or if empty documents are received for the twins_tree or twins_dflt, then "-f" behavior will be attempted
	
	-f, --file "/path/to/config"
		Runs TWINS with configurations stored in a twins.json at the filepath provided
		If no filepath provided, twins will default to searching for a filepath in the second command line argument (legacy behavior)
			If no twins.json found at this path or if second argument is not a valid filepath, twins will finally look in the current working directory

	-v, --verbose "comma,separated,options":
		Turn on extra logging for start up and during run
		If options provided, will turn on logging for specified program section, asset, asset type, etc
		See confluence for details. VERBOSE OPTIONS NOT YET ENABLED, FUNCTIONALITY COMING SOON!
`

const (
	// Constants for dbi read success or failure, to be sent over configync channel
	CONFIG_RECEIVE_SUCCESS	int = 1
	CONFIG_RECEIVE_FAILURE	int = -1
	MAX_DBI_LOAD_ATTEMPT 	int = 5
)
var fimsReceive chan fims.FimsMsg
var t0 time.Time
var stateTicker, pubTicker *time.Ticker
var stateUpdate chan float64
var configsync chan int //channel to synchronize configs coming from dbi. blocks config flow until configs are received and processed. 
var f fims.Fims
var config cfg
var root treeNode
var assets map[string]asset
var cmds map[string]asset
var fimsMap map[string]interface{}
var configMap map[string]interface{}
var updateMap map[string]interface{}
var treeMap map[string]interface{}
var configDocs map[string]bool 		//holds the name of the configuration documents in the /dbi/twins collection. 
var updtDocs []string			//holds the name of the twins_*_updt.json documents (subset of configDogs)
var collection string = "twins" //dbi collection. default is /dbi/twins, but can be overwritten with --uri ["collection"] flag
var cpath string
var pwrTree treeCfgNode
var verbose bool
var fromFile bool
var fromDBI bool
var verboseFlags []string

func handleFlags() {
	log.SetPrefix("Twins: ")
	var help bool
	var loadURI string = ""
	if len(os.Args) > 1 {
		for i,arg := range os.Args[1:] { //don't use the first arg, it's the program name. 
			switch arg {
			case "-f":
				fromFile = true
			case "--file":
				fromFile = true
				if len(os.Args) > i+2 {
					cpath = os.Args[i+2]
				} else {
					log.Print("--file flag present without filepath. Defaulting to searching for config path in second command line argument argument. Run this program with --help for details.")
				}
			case "-v":
				verbose = true
			case "--verbose":
				verbose = true
				if len(os.Args) > i+2 {
					verboseFlags = strings.Split(os.Args[i+2],",")
				}
			case "-u":
				fromDBI = true
			case "--uri":
				fromDBI = true
				if len(os.Args) > i+2 {
					loadURI = os.Args[i+2]
				} else {
					log.Print("--uri flag present without dbi collection uri. Resulting to default load path. Run this program with --help for details.")
				}
			case "-h", "--help", "-man", "usage":
				help = true
			}
		}
	} 
	if help {
		log.Fatalf(usage)
	}
	if fromDBI && fromFile {
		log.Fatalf("File and URI load flags present. These are mutually exclusive and cannot be run together. Run this program with --help for details. Exiting...")
	}
	if loadURI != "" {
		parts := strings.Split(loadURI, "/")
		if len(parts) == 1 {
			collection = parts[0]
		} else {
			for _,part := range parts {
				cpart := cleanString(part)
				if cpart == "" || cpart == "dbi" {
					continue
				}
				collection = part
			}
		}
	}
}

//initialize the minimum set of items needed to send and recieve FIMS messages.
//Now this needs to be done first so that we can receive configurations from dbi
func initFims() {
	var err error
	configsync = make(chan int)
	fimsMap = make(map[string]interface{})
	configMap = make(map[string]interface{})
	updateMap = make(map[string]interface{})
	//fmt.Printf("%#v", root)
	f, err = fims.Connect("Twins")
	if err != nil {
		log.Fatalf("Unable to connect to FIMS server: %v", err)
	}
	fimsReceive = make(chan fims.FimsMsg)
	err = f.Subscribe("/components", "/twins")
	if err != nil {
		log.Fatal("Unable to subscribe")
	}
	go f.ReceiveChannel(fimsReceive)
}

// sends appropriate FIMS messages to arbitrate between the local config file path and the load from dbi path. 
// goroutine in main() in twins.go handles the success or failure, and then replies back on configsync channel with results.
// if successful, the configs are placed in configMap, updateMap, or treeMap as appropriate. 
func arbitrateConfig() bool {
	var numUpdt int					//number of twins_*_updt.json files to parse. 
	configDocs = make(map[string]bool)
	// if fromFile && fromDBI {
	// 	log.Fatalf("Flags received to read from file and read from DBI. Cannot arbitrate, exiting")
	// }
	if fromFile {
		return false
	}
	log.Print("Attempting to read configurations from dbi")
	//Request configuration files from dbi. 
	//Response handled in receiveConfig() function, called from fims receive goroutine spawned in main()
	//Each of these will time out after five seconds, killing the application and logging an error if so. 
	if verbose {
		log.Print("Reading document names from collection: ", collection)
	}
	//Set of config documents loaded to dbi:
	var success bool = false
	var uristr string
	for tries:=0; tries < MAX_DBI_LOAD_ATTEMPT; tries++ {
		uristr = "/dbi/" + collection + "/show_documents"
		f.Send(fims.FimsMsg {
			Method:  "get",
			Uri:     uristr,
			Replyto: "/twins/configdocs",		
		})
		select {
		case ret := <- configsync:
			if ret == CONFIG_RECEIVE_FAILURE {
				log.Println("Unable to receive document names. This could mean dbi load is still ongoing. Attempt number.", tries+1)
			} else {
				success = true
				break
			}
		case <- time.After(5 * time.Second):
			log.Println("Timeout receiving asset tree configuration. Please check that dbi and mongo processes are running. Attempt number", tries+1)
		}
	}
	if !success {
		return false
	}
	// determine number of update files to parse. This will be used for the config sync blocking later. 
	for doc,_ := range configDocs{
		match,_ := regexp.Match("^twins_[a-zA-Z]+_updt$", []byte(doc))
		if match {
			numUpdt++
			updtDocs = append(updtDocs, doc)
		}
	}
	if verbose {
		log.Println("Received", numUpdt, "update files, not including the asset tree or default configuration file")
		for _,doc := range updtDocs {
			log.Println(doc)
		}
	}
	//Power system tree:
	doc := "twins_tree"
	uristr = "/dbi/" + collection + "/" + doc
	f.Send(fims.FimsMsg {
		Method:  "get",
		Uri:     uristr,
		Replyto: "/twins/config_tree",
	})
	select {
	case ret := <- configsync:
		if ret == CONFIG_RECEIVE_FAILURE {
			log.Println("Unable to receive twins asset tree. Attempting to configure using local configuration file.")
			return false
		} else {
			configDocs[doc] = true
		}
		log.Println("Received twins asset tree")
	case <- time.After(5 * time.Second):
		log.Printf("Timeout receiving asset tree configuration. Please check that dbi and mongo processes are running. Attempting local configuration")
		return false
	}
	//default config:
	doc = "twins_dflt"
	uristr = "/dbi/" + collection + "/" + doc
	f.Send(fims.FimsMsg {
		Method:  "get",
		Uri:     uristr,
		Replyto: "/twins/config_dflt",
	})
	select {
	case ret := <- configsync:
		if ret == CONFIG_RECEIVE_FAILURE {
			log.Println("Unable to receive default configurations. Attempting to configure using local configuration file.")
			return false
		} else {
			configDocs[doc] = true
		}
		log.Println("Received default configs")
	case <- time.After(5 * time.Second):
		log.Print("Timeout receiving default configurations. Please check that dbi and mongo processes are running. Attempting local configuration")
		return false
	}
	//update config
	for _,doc := range updtDocs {
		uristr = "/dbi/" + collection + "/" + doc
		f.Send(fims.FimsMsg {
			Method:  "get",
			Uri:     uristr,
			Replyto: "/twins/config_update",
		})
		select {
		case ret := <- configsync:
			if ret == CONFIG_RECEIVE_FAILURE {
				log.Println("Unable to receive configuration overwrites. If no configuration overwrite was provided then this is expected. Continuing")
			} else {
				log.Println("Received update config", doc)
				configDocs[doc] = true
			}
		case <- time.After(5 * time.Second):
			// if the block above passed, then we should never get here except for when debugging
			// but it's good to handle this gracefully and allow the config flow to continue just in case. 
			log.Fatalf("Timeout receiving update configurations. If no configuration overwrite was provided then this may be expected. Continuing")
		}
	}
	return true
}
// init() initializes the data structures and configurations for Twins.
// It makes calls to routines in config.go, dies if it can't properly
// configure, and makes everything ready for main()
func runConfig() {
	var err error
	log.Printf("Configuring")
	dbiLoadSuccessful := arbitrateConfig() //Will return false if fromFile is true (-f or --file flag used) or if dbi load otherwise fails. 
	if dbiLoadSuccessful {
		createPwrTree() //updates the pwrTree global variable. This is used at configuration time to expand default assets and evaluate expressions
		configMap = createConfig(&pwrTree) //uses pwrTree struct as instruction to create default configuration map configMap
		updateMap = createUpdate()
		configMap = combineConfigs(configMap,updateMap)
		evaluateExpressions() //replaces $variables and evaluates "expression": [] objects in configMap
		replaceExpressions(configMap, expressions)
		collapseConfig()
		configMap["root"] = treeMap["root"] // add asset tree structure to configMap. This should always be the very last step before unmarshaling into &config struct to avoid issues with overwriting asset structure.
	} else if !fromDBI { //if -u or --uri flag were provided, then do not allow twins to attempt to find a 'twins.json'
		readConfig(&config)
	} else {
		log.Fatalf("Unable to configure twins. Please check configurations and try again (maybe try the --help flag). Exiting...")
	}
	// Send completely expanded and parsed config set to dbi so it can be grabbed and easily analyzed.
	f.Send(fims.FimsMsg {
		Method:  "set",
		Uri:     "/dbi/twins/fullconfig",
		Body: 	configMap,
	})
	createStruct() //populates the struct pointed to by &config global variable
	root, assets, err = buildState(&config, fimsMap)
	if err != nil {
		log.Fatalf("Couldn't put together the tree: %s", err)
	}
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
	handleFlags() //handle command line flags like -u, --file, etc. This is done first so --help can work without fims_sever running
	initFims()    
	// A goroutine to handle incoming FIMS messages
	go func() {
		for {
			msg := <-fimsReceive
			//fmt.Println("I have received", msg.Uri, msg.Replyto, msg.Body)
			//Special case for dbi load, will reply to these URIs
			//For now this is done here in the main recieve goroutine. If TWINS has to recieve many fims messages this may slow down, so move this outside
			//It's nice to have here so that configs can be updated during run in the future. 
			if (msg.Method == "set" && ((msg.Uri == "/twins/config_dflt") || (msg.Uri == "/twins/config_update") || (msg.Uri == "/twins/config_tree") || (msg.Uri == "/twins/configdocs"))) {
				ok := receiveConfig(msg)
				if verbose {
					fmt.Println("I have received", msg.Uri, msg.Replyto, msg.Body)
				}
				//allows configuration flow to continue and indicates success or failure to receive a legitimate configuration object. 
				//attempt to send success or failure on configsync (for initial configuration, if main() is expecting it)
				//otherwise continue so that the twins process does not block if these uris get set to during operation. 
				if ok {
					select {
					case configsync <- CONFIG_RECEIVE_SUCCESS:
					default:
					}
				} else {
					select {
					case configsync <- CONFIG_RECEIVE_FAILURE:
					default:
					}
				}
				continue
			}
			// Receive inputs over FIMS, process (but don't update)
			receiveFims(msg, fimsMap)			
		}
	}()
	runConfig()
	log.Printf("Starting main loop")
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
				case "config":
					if msg.Method == "get" && msg.Replyto != "" {
						f.Send(fims.FimsMsg {
							Method: "get",
							Uri:	"/dbi/twins/fullconfig",
							Replyto : msg.Replyto,
						})
					}
				case "docs":
					if msg.Method == "get" && msg.Replyto != "" {
						f.Send(fims.FimsMsg {
							Method: "set",
							Uri:	msg.Replyto,
							Body:   configDocs,
						})
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
