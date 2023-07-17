package main

import (
	"encoding/json"
	"fims"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"path"
	"reflect"
	"strconv"
	"strings"
	"regexp"
)

// treeCfgNode is a struct to unmarshal the power system tree in twins.json into
type treeCfgNode struct {
	ID             string
	Asset_type     string
	Children       []treeCfgNode
}

// treeNode is a tree struct for the solver to use
type treeNode struct {
	asset    asset
	children []treeNode
}

// cfg is the struct to unmarshal all of twins.json into
type cfg struct {
	UpdateRate     uint
	PublishRate    uint
	TimeMultiplier uint
	ManualTick     bool
	Grids          []grid
	Feeds          []feed
	Ess            []ess
	Gen            []gen
	Solar          []solar
	PV			   []pv
	Loads          []load
	Transformers   []xfmr
	PCS            []pcs
	DCDC	       []dcdc
	BMS            []bms
	DCBUS		   []dcbus
	Root           treeCfgNode
}
// updt is a struct to contain necessary information for template expansion. 
// used to create a number of assets of type assetType equal to the number of names in ids with variables defined in vars. 
type updt struct {
	assetType	string 					//pcs, bms, etc
	ids			[]string				//parsed names based on templating rules
	vars 		map[string]interface{}	//variables defined in "vars" from template, excluding id. 
}
func parseUpdateMap(updtMap map[string]interface{}) map[string]interface{} {
	if _, ok := updtMap["expression"]; ok {
		return updtMap // we've encountered a map that we've already made the expression for, so don't make it again as it will compound for many like assets. 
	}
	for k,v := range updtMap {
		switch v.(type) {
		case map[string]interface{} :
			v = parseUpdateMap(v.(map[string]interface{}))

		case []interface{} :
			tmp := make(map[string]interface{})
			tmp["expression"] = v
			updtMap[k] = tmp

		default:
			tmp := make(map[string]interface{})
			var slc []interface{} // we expect expressions to be a slice. 
			slc = append(slc, v)
			tmp["expression"] = slc
			updtMap[k] = tmp
		}
		
	}
	return updtMap
}
// transforms extracted template information in []updt structure to create updateMap map[string]interface{} that the rest of the configuration flow is expecting. 
func parseUpdates(updts []updt) map[string]interface{} {
	var outMap map[string]interface{} = make(map[string]interface{})
	for _, updt := range updts {
		//first create top level structure of updtMap, which is a map of asset keys with []interface{} values.  
		if _, ok := outMap[updt.assetType]; !ok {
			var tmp []interface{}
			outMap[updt.assetType] = tmp
		}
		//varmap structure is usually map[string]map[string]interface{} like "variable_name": {"expression": exp} 
		//except for if variable_name == "id", in which case it is simply "id": "name"
		for _, id := range updt.ids {
			astMap := make(map[string]interface{}) //first level of varmap. 
			astMap["id"] = id
			for variable,value := range updt.vars {
				var tmp []interface{}
				varMap := make(map[string]interface{}) //second level of varmap
				valKind := reflect.ValueOf(value).Kind()
				if valKind == reflect.Slice {
					varMap["expression"] = value //store in expression as the expression key dominates over the default key later when collapsing configs. 
				} else if valKind == reflect.Map {
					tmp := parseUpdateMap(value.(map[string]interface{}))
					varMap = deepCopyMap(tmp)
					//astMap[variable] = varMap //adding expression entry to configMap happens in parseUpdateMap. 
				} else {
					tmp = append(tmp, value) //expect expressions to be a slice
					varMap["expression"] = tmp
				}
				astMap[variable] = varMap
				cleanVar := cleanString(variable)
				//need to create a corresponding "expression" field in the configMap if it doesn't already exist
				//so config combiner can find it and replace like with like. 
				if _,ok := configMap[updt.assetType].([]interface{}); !ok {
					log.Fatalln("Could not find update asset type [", updt.assetType, "] in configMap. Check spelling of top level asset type in twins_*_updt.json files. Exiting")
				}
				for _, cfgAst := range configMap[updt.assetType].([]interface{}) { // cfgAst is the asset from confgiMap with the same id as the current updateMap entry
					if cfgAst.(map[string]interface{})["id"].(string) == id {
						// the for loop below implements the following in a case-insensitive (albeit slower) way: if cfgVar, ok := cfgAst.(map[string]interface{})[variable]; ok {}
						found := false
						var cfgVarKey string
						for k,_ := range cfgAst.(map[string]interface{}) {
							ky := cleanString(k)
							if ky == cleanVar {
								found = true
								cfgVarKey = k
								break
							}
						}
						if found {
							cfgVar := cfgAst.(map[string]interface{})[cfgVarKey]
							if _, found := varMap["expression"]; !found {
								for key, _ := range varMap{
									// need to do a case-insensitive look up here as well. This implements : if expanded, success := cfgVar.(map[string]interface{})[key]; success{}
									//if expanded, success := cfgVar.(map[string]interface{})[key]; success{
									cleanKey := cleanString(key)
									var expanded interface{}
									success := false
									for k,v := range cfgVar.(map[string]interface{}){
										cleanCfgVarK := cleanString(k)
										if cleanKey == cleanCfgVarK {
											success = true
											expanded = v
										}
									}
									if success {
										expanded.(map[string]interface{})["expression"]=[]interface{}{nil}
										break
									}
								}
							} else {
								cfgVar.(map[string]interface{})["expression"] = []interface{}{nil} // We only care that the expression key exists here, and don't care about its value since it will be overwritten. 
							}
						} else {
							log.Println("[parseUpdate] Unable to create expression field for configuration variable", cfgVarKey, "for asset", id)
						}
					}
				}
			}
			outMap[updt.assetType] = append(outMap[updt.assetType].([]interface{}), astMap)
		}
	}
	return outMap
}
func extractName(idTmpl string, token string) (string, string, bool) {
	//return strings.Cut(idTmpl, token) //This will work in go 1.18.
	var substrings []string
	substrings = strings.Split(idTmpl,token)
	if len(substrings) != 2 {
		log.Println("[extractName] could not parse templated id", idTmpl, "using token", token, ". Either not found or found multiple times.")
		return idTmpl, "", false
	}
	return substrings[0], substrings[1], true
}
// creates a slice of names with incremental integers. 
func extractSequence(from interface{}, to interface{}, idTmpl string, token string) ([]string, bool) {
	var ok bool
	var ids []string
	var before string
	var after string
	if from,ok = from.(float64); !ok {
		log.Println("[extractSequence] unexpected value for 'from' field. Could not cast to integer. Value received: ", from)
		return []string{""}, false
	}
	fromInt := int(from.(float64))
	if to,ok = to.(float64); !ok {
		log.Println("[extractSequence] unexpected value for 'to' field. Could not cast to integer. Value received: ", to)
		return []string{""}, false
	}
	toInt := int(to.(float64))
	if before, after, ok = extractName(idTmpl, token); !ok {
		return []string{""}, false
	}
	for i:= fromInt; i <= toInt; i++ {
		str := strconv.Itoa(i) //Itoa converts an integer value to a string
		ids = append(ids, strings.Join([]string{before,str,after},""))
	}
	return ids, true
}
// Parses the list of names passed in list returning a slice of concatenated names
func extractList(list interface{}, idTmpl string, token string) ([]string, bool) {
	var ids []string
	//var idTmpl string
	var before string
	var after string
	var ok bool
	listKind := reflect.ValueOf(list).Kind()
	if listKind != reflect.Slice {
		log.Println("[extractList] Expected slice type for list, got instead", listKind)
		return ids, false
	}
	if before, after, ok = extractName(idTmpl, token); !ok {
		return ids, false
	}
	for _,name := range list.([]interface{}) {
		nameKind := reflect.ValueOf(name).Kind()
		if nameKind != reflect.String {
			log.Println("[extractList] expected list items to be strings. List item:", name, "is detected as type:", nameKind)
			return ids, false
		}
		ids = append(ids, strings.Join([]string{before,name.(string),after}, ""))
	}
	return ids, true
}
// parses template expansion information in tmplMap and returns expanded variable IDs slice. 
func createIDS(tmplMap map[string]interface{}) ([]string, bool) {
	var token string
	var vars interface{}
	var reps interface{}
	var ids []string
	var list interface{}
	var from interface{}
	var to interface{}
	var idTmpl interface{}
	var ok bool
	// first check if everything is ok and the datatype we expect. 
	if vars,ok = tmplMap["vars"]; !ok {
		return []string{""}, false
	}
	if reps,ok = tmplMap["reps"]; !ok {
		return []string{""}, false
	}
	varsKind := reflect.ValueOf(vars).Kind()
	repsKind := reflect.ValueOf(reps).Kind()
	if varsKind != reflect.Map {
		log.Println("[createIDS] expected vars object to be a map. Got instead:", varsKind)
		return ids, false
	}
	if repsKind != reflect.Map {
		log.Println("[createIDS] expected reps object to be a map. Got instead:", repsKind)
		return ids, false
	}
	varsMap := vars.(map[string]interface{})
	repsMap := reps.(map[string]interface{})
	if token,ok = repsMap["token"].(string); !ok {
		log.Println("[createIDS] unable to extract token from reps object.")
		return ids, ok
	}
	if idTmpl, ok = varsMap["id"]; !ok {
		log.Println("[createIDS] could not extract templated id from vars object.")
		return ids, false
	}
	idTmplKind := reflect.ValueOf(idTmpl).Kind() 
	if idTmplKind != reflect.String {
		log.Println("[createIDS] expected templated ID to be of type string. Got instead: ", idTmplKind, " value recieved: ", idTmpl)
		return ids, false
	}
	// detect which style of replacement has been selected. Either "list" or ("from" and "to") will exist. If other combinations are present it is an error. 
	if list,ok = repsMap["list"]; ok { 
		if _,ok = repsMap["from"]; ok {
			log.Println("[createIDS] both list-based and sequence-based rules definition found in reps object. Unable to arbitrate")
			return ids, false
		} else if _,ok = repsMap["to"]; ok {
			log.Println("[createIDS] both list-based and sequence-based rules definition found in reps object. Unable to arbitrate")
			return ids, false
		}
		ids, ok = extractList(list,idTmpl.(string),token)
	} else if from,ok = repsMap["from"]; ok {
		if _,ok = repsMap["list"]; ok {
			log.Println("[createIDS] both list-based and sequence-based rules definition found in reps object. Unable to arbitrate")
			return ids, false	
		}
		if to,ok = repsMap["to"]; !ok {
			log.Println("[createIDS] Incomplete sequence-based rules definition found. Unable to parse template.")
			return ids, false
		}
		ids, ok = extractSequence(from,to,idTmpl.(string),token)
	}
	return ids, ok
}
// expands the templated configs that are initially unmarshaled into updateMap. 
// the return of this function is then assigned to updateMap, overwriting the initial unmarshal. 
// these configurations are then combined with the expanded configs in configMap before expression parsing. 
// values for variables in the templated update map will be assigned to an "expression" key, which will allow later steps to 
// view it as an updated value and replace the default where necessary. Also adds an "expression" key to the corresponding variable in configMap
// if it doesn't already exist so that the combiner will work properly and can remain implementation agnostic.
func createUpdate() map[string]interface{} {
	tmpMap := make(map[string]interface{})
	updateMap = cleanConfig(updateMap)
	var updts []updt
	//printMap(updateMap)
	for k,v := range updateMap {
		asset := handleNames(cleanString(k))
		vKind := reflect.ValueOf(v).Kind()
		if vKind != reflect.Slice { // updateMap is allowed to edit updateRate etc, so skip these for now.
			tmpMap[k] = v //output map needs to include these values. 
			continue
		}
		for _, astI := range v.([]interface{}) { //astI is a template entry for a given asset. 
			astKind := reflect.ValueOf(astI).Kind()
			if astKind != reflect.Map {
				continue // TODO GB: logging? 
			}
			updt := new(updt)
			var vars interface{}
			var ok bool 
			if vars, ok = astI.(map[string]interface{})["vars"]; !ok {
				continue // TODO GB: logging
			}
			varsKind := reflect.ValueOf(vars).Kind()
			if varsKind != reflect.Map {
				log.Println("[createUpdte] Expected vars structure to be a map, got instead:", varsKind, "asset:", asset, "update template:", astI)
				continue
			}
			updt.assetType = asset
			if updt.ids, ok = createIDS(astI.(map[string]interface{})); !ok {
				log.Fatal("[createUpdate] Unable to create asset IDs. Currently unpacking asset: ", asset, " update template: ", astI, " Unrecoverable error, exiting.")
			}
			delete(vars.(map[string]interface{}), "id") //remove the id key from the vars map since we don't need it anymore and deleting it here means we don't have to have a bunch of special checks to avoid it. 
			updt.vars = vars.(map[string]interface{})
			updts = append(updts, *updt)
		}
	}
	outMap := parseUpdates(updts)
	for k,v := range tmpMap { //add back in the updateRate etc calibrations. 
		outMap[k] = v
	}
	return outMap
}
//Creates full default configuration map using the keys of the names map to identify asset type and the value to identify asset id. 
func makeConfig(names map[string][]string) (map[string]interface{}) {
	fullConfig := make(map[string]interface{})
	//first pull out 'global' calibrations like tick rate, publish rate, etc. 
	for key,value := range configMap {
		if reflect.ValueOf(value).Kind() != reflect.Slice {
			fullConfig[key] = value
		}
	}
	for k,v := range names { //k: asset type, v: slice of asset names
		if _, ok := configMap[k]; !ok{
			log.Print("Could not find asset type,", k, "in default configurations. Moving to next asset type.")
			continue
		}
		var slc []interface{}
		for i := range v {
			// need to make a deep copy of the entry in configMap. Otherwise if we just shallow copy
			// we'll end up with all of the assets pointing to the same entry which is not what we want
			ast := make(map[string]interface{})
			ast = deepCopyMap(configMap[k].(map[string]interface{}))
			ast["id"] = v[i]
			slc = append(slc, ast)
		}
		fullConfig[k] = slc
	}
	return fullConfig
}

func handleNames(inName string) string {
	var outName string
	switch inName { //handle "special" versions of asset tree naming
	case "generator", "generators", "gen":
		outName = "gens"
	case "feeder", "feeders", "feed":
		outName = "feeds"
	case "photovoltaic":
		outName = "pv"
	case "storage":
		outName = "ess"
	case "transformer", "trans", "xfmr":
		outName = "transformers"
	case "grid":
		outName = "grids"
	case "load":
		outName = "loads"
	default:
		outName = inName
	}
	return outName
}

//create simple map of asset types containing slices of asset names
func parseTree(tree *treeCfgNode, names map[string][]string) (map[string][]string) {
	astType := strings.ToLower(tree.Asset_type)
	astType = handleNames(astType)
	tree.Asset_type = astType
	//If asset type listed in tree does not match defaults, skip to the next asset in the tree.
	if _, ok := configMap[astType]; !ok {
		log.Println("Could not find asset type", tree.Asset_type, "from asset tree id", tree.ID, "in default configs. Moving to next asset")
	} else if tree.ID == ""{
		log.Println("Invalid tree id: [", tree.ID, "]. Asset type: [", tree.Asset_type, "]. Moving to next asset")
	//if this is the first time we've encountered this asset type, make a new entry in names
	} else if _, ok := names[astType]; !ok {
		var slice []string
		names[astType] = slice
		names[astType] = append(names[astType], tree.ID)
	//otherwise append to the existing
	} else {
		names[astType] = append(names[astType], tree.ID)
	}
	//recurse through tree structure
	for i:= range tree.Children{
	 	names = parseTree(&tree.Children[i], names)
	}
	//Delete unused asset types from configMap. This saves some processing later and also cleans up the final expanded config for viewing later. 
	for k,v := range configMap {
		if _,ok := names[cleanString(k)]; !ok {
			switch v.(type) {
			case []interface{}:
				delete(configMap, k)
			}
		}
	}
	return  names
}

//Removes all note fields from configMap before expansion. 
func cleanConfig(cfg map[string]interface{}) (map[string]interface{}) {
	for k,v := range cfg {
		if strings.HasPrefix(strings.ToLower(k),"note") || strings.HasPrefix(k,"_") {
			delete(cfg, k)
		}
		//Handle maps nested in maps and slices
		if reflect.ValueOf(v).Kind() == reflect.Map {
			cfg[k] = cleanConfig(v.(map[string]interface{}))
		} else if reflect.ValueOf(v).Kind() == reflect.Slice {
			slc := v.([]interface{})
			for i := range slc {
				if reflect.ValueOf(slc[i]).Kind()== reflect.Map{
					cfg[k].([]interface{})[i] = cleanConfig(slc[i].(map[string]interface{}))
				}
			}
		}
	}
	return cfg
}
// createConfig uses the tree structure to expand default configurations into a full 'twins.json'-like configuration containing variables and expressions
func createConfig(tree *treeCfgNode) (map[string]interface{}) {
	configMap = cleanConfig(configMap)
	names := make(map[string][]string) // key: asset_type, value: slice of asset names
	names= parseTree(tree, names) 
	for k,v := range configMap { //remove unused assets from configMap. This prevents issues with marshalling the configMap into the cfg struct later. Secondarily provides a bit of a performance improvement
		if _,ok := v.(map[string]interface{}); !ok {
			continue
		}
		if _,ok := names[k]; !ok {
			delete(configMap, k)
		}
	}
	expandedMap := makeConfig(names)
	return expandedMap
}
// readConfig() looks for args to the program, follows the first arg
// as a path, and looks for twins.json in it
// if no arguments passed, look for configurations in dbi
func readConfig(config *cfg) {
	// Looking for twins.json
	//var cpath string
	var configjson []byte
	var err error
	log.Println("Attempting to configure from local file...")
	if cpath == "" { //use path from command line flag if specified, otherwise look for it in second argument (legacy load path for backwards compatibility)
		if len(os.Args) < 2 {
			log.Print("Config path argument not found and dbi load failed. Trying current working directory")
			cpath = "twins.json"
			//log.Print("Attempting to load config from dbi")
	
		} else {
			info, err := os.Stat(os.Args[1])
			if os.IsNotExist(err) {
				log.Fatal("TWINS configuration file not found at: ", os.Args[1])
			} else if info.IsDir() {
				cpath = path.Join(os.Args[1], "twins.json")
			} else {
				cpath = os.Args[1]
			}
		}
	}
	configjson, err = ioutil.ReadFile(cpath)
	if err != nil {
		log.Fatalf("Couldn't read the file %s: %s", cpath, err)
	}
	err = json.Unmarshal(configjson, config)
	if err != nil {
		log.Fatal("Failed to Unmarshal config file")
	}
}

func createPwrTree() {
	_, ok := treeMap["root"]
	if !ok {
		log.Fatal("Expecting twins tree to start with object named \"root\". Object not found, exiting")
	}
	bytes, err := json.Marshal(treeMap["root"])
	if err != nil {
		log.Print("Error reading configuration tree. Not able to be marshaled")
		log.Fatal("Received configurations: ", treeMap)
	}
	json.Unmarshal(bytes, &pwrTree)
	//fmt.Println(pwrTree)
}
//Creates the config struct from combined configurations by first marshalling the data in configMap into a json blob then unmarshalling into the config struct
func createStruct() {
	bytes, err := json.Marshal(configMap)
	if err != nil {
		log.Print("Error reading configurations after combination. Not able to be marshaled")
		if verbose { 
			printMap(configMap) 
		}
		log.Fatal()
	}
	json.Unmarshal(bytes, &config)
}
// receiveConfig() is a special case to handle configs being set over FIMS from dbi
// The body of the FIMS message shall contain a config json blob, so save it to unmarshal later
// similar to how loading a config from file location worked previously. 
func receiveConfig(msg fims.FimsMsg) bool {
	if msg.Method == "pub" || msg.Method == "del" || msg.Method == "post" {
		return false
	}
	ok := false
	if msg.Uri == "/twins/config_dflt"{
		configMap, ok= msg.Body.(map[string]interface{})
		if !ok{
			log.Print("Could not read default config from dbi.")
			return false
		}
		configMap = cleanConfig(configMap)
		if len(configMap) == 0 {
			log.Print("Could not read default config from dbi.")
			return false
		}
	} else if msg.Uri =="/twins/config_update"{
		if _, ok := msg.Body.(map[string]interface{}); !ok {
			log.Print("Could not read update config from dbi. Continuing with default asset configuraitons.")
			return false
		}
		out := cleanConfig(msg.Body.(map[string]interface{}))
		if len(out) == 0{
			log.Print("Could not read update config from dbi. Continuing with default asset configuraitons.")
			return false
		}
		for k,v := range out{ //append new values to updateMap for each twins_*_updt.json 
			updateMap[k]=v
		}
	} else if msg.Uri == "/twins/config_tree" {
		treeMap, ok = msg.Body.(map[string]interface{})
		if !ok {
			log.Print("Could not read tree structure from dbi.")
			return false
		}
		treeMap = cleanConfig(treeMap)
		if len(treeMap) == 0 {
			log.Print("Could not read tree structure from dbi.")
			return false
		}
	} else if msg.Uri == "/twins/configdocs" {
		if _,ok := msg.Body.([]interface{}); !ok {
			log.Printf("Unable to read document names from dbi.")
			return false
		}
		for _,doc := range msg.Body.([]interface{}) {
			if _,ok := doc.(string); !ok {
				log.Printf("Unexpected data type return for document name from dbi.")
				return false
			}
			configDocs[doc.(string)] = false
		}
	} else {
		log.Println("Unexpected URI for configurations:", msg.Uri)
		return false
	}
	return true
}
//replaceSlices() attempts to replace the JSON array contained in cfgS with the JSON array contained in updtS which at this point are represented as []interface{} types
//First checks types and dimension of slice members, then replaces values if matching
//Calls replaceMaps() if encountering a map as a slice member. 
// If dimension of updtS is different than the dimension of cfgS, then the entierety of updtS will be placed in cfgS and a log entry will generate
func replaceSlices(cfgS []interface{}, updtS []interface{}) ([]interface{}) {
	if len(updtS) != len(cfgS) {
		log.Print("[replaceSlices] Update array is different diemnsion than default. Overwriting entirety of default with update array")
		log.Print("[replaceSlices] default array:", cfgS)
		log.Print("[replaceSlices] update array:", updtS)
		cfgS = updtS
		return cfgS
	}
	for i, s := range updtS {
		uKind := reflect.ValueOf(s).Kind()
		cKind := reflect.ValueOf(cfgS[i]).Kind()
		if uKind != cKind && (cfgS[i] != nil) { //if cfgS is nil, allow to be replaced. This indicates a calibration that has been updated by the user.
			//TODO GB: Should this instead give up for the entire slice?
			log.Print("[replaceSlices] slice members found to be different data types. Aborting update for this item")
			log.Print("[replaceSlices] default slice member:", cfgS[i])
			log.Print("[replaceSlices] update slice member:", updtS[i])
			continue
		}
		//If both update and default contain maps, unpack them recursively. Likewise for a multidimensional array. Doing it this way instead of simply replacing at this level 
		//allows for us to confim that slice members are alike at deeper levels to find potential misconfigurations.
		if cKind == reflect.Map {
			cfgS[i] = replaceMaps(cfgS[i].(map[string]interface{}), updtS[i].(map[string]interface{}))
		} else if cKind == reflect.Slice {
			cfgS[i] = replaceSlices(cfgS[i].([]interface{}), updtS[i].([]interface{}))
		} else {
			cfgS[i] = s
		}
	}
	return cfgS
}

//replaceMaps looks for keys existing in both uptM and cfgM. If key is found and values type and dimension match
//replace the corresponding value from updtM to cfgM
func replaceMaps(cfgM map[string]interface{}, updtM map[string]interface{}) (interface{}){
	for k, v := range updtM {
		key := cleanString(k)
		if key == "id" {
			continue
		}
		//if key from updtM does not exist in cfgM, skip to next key.
		//cannot simply do if _,ok := cfgM[k]; !ok {} as we want to be case insensitive here. 
		found := false
		var cfgKey string
		for ky, _ := range cfgM {
			tmp := cleanString(ky)
			if tmp == key {
				found = true
				cfgKey = ky
				break
			}
		}
		if !found {
			log.Println("[replaceMaps] Unable to find entry '", k, "' in configuration map. Please check spelling and format for templated configs.")
			continue
		}
		cVal := reflect.ValueOf(cfgM[cfgKey])
		val := reflect.ValueOf(updtM[k])
		valKind := val.Kind()
		cValKind := cVal.Kind()
		if valKind != cValKind {
			log.Println("[replaceMaps] Data types of map values between default and update configurations do not match. Aborting for this item")
			log.Println("[replaceMaps] Key: ", k, "Default value: ", cfgM[cfgKey], "Update value: ", updtM[k])
			continue
		}
		//ctrlword*cfg, statuscfg, and sbmu are handled here as special cases. This could be made to be implementation agnostic by making the added expression field
		//in configMap from parseUpdates() more robust to match the deeper structure, but that is more work than it's worth at the moment and is prone to bugs
		//also do the direct replacement here as the configuration updater otherwise expects to only replace like with like. Doing this keeps all of the special-caseness 
		//here in one spot.
		ctrlwrdfound, _ := regexp.Match("ctrlword[0-9]{1,}cfg", []byte(key)) 
		if ctrlwrdfound || key == "statuscfg" || key == "sbmu" {
			cfgM[cfgKey].(map[string]interface{})["default"] = v.(map[string]interface{})["expression"]
			delete(cfgM[cfgKey].(map[string]interface{}), "expression")
			delete(v.(map[string]interface{}), "expression")
		}
		//There can be nested maps or slices in asset configurations, so call replaceMaps() or replaceSlices() recursively in this case
		if valKind == reflect.Map{
			cfgM[cfgKey] = replaceMaps(cfgM[cfgKey].(map[string]interface{}), updtM[k].(map[string]interface{}))
		} else if valKind == reflect.Slice {
			cfgM[cfgKey] = replaceSlices(cfgM[cfgKey].([]interface{}), updtM[k].([]interface{}))
		} else {
			cfgM[cfgKey] = v
		}
	}
return cfgM
}
//combineConfigSlices() takes slices of asset configurations found by combineConfigs(), iterates through the update
//slice to pull out individual asset ids, and then attempts to find the corresponding asset id in the config slice
//then update values for matching keys. 
func combineConfigSlices(cfgSlc []interface{}, updtSlc []interface{}) (interface{}) {
updtKind := reflect.ValueOf(updtSlc).Kind()
cfgKind := reflect.ValueOf(cfgSlc).Kind()
//this should never pass, but it's a good idea to double check just in case to avoid a panic.
if updtKind != reflect.Slice || cfgKind != reflect.Slice {
	log.Println("How did you get here..?", updtKind, cfgKind)
	return cfgSlc
	}
for _, updtV := range updtSlc {
	if reflect.ValueOf(updtV).Kind() != reflect.Map{
		fmt.Println("This should be a map but it's not:", updtV)
		continue
	}
	//get asset name from update configs
	updtID, ok := updtV.(map[string]interface{})["id"]
	if !ok{
		continue
	}
	var found bool = false
	for i, cfgV := range cfgSlc {
		if reflect.ValueOf(cfgV).Kind() != reflect.Map{
			fmt.Println("This should be a map but it's not:", cfgV)
			continue
		}
		cfgID, ok := cfgV.(map[string]interface{})["id"]
		if !ok {
			continue
		}
		//If IDs match, then replace other key/value pairs from updtV to cfgV
		if updtID == cfgID {
			found = true
			log.Println("[combineConfigSlices] Updating values for asset: ", updtID)
			cfgSlc[i] = replaceMaps(cfgV.(map[string]interface{}), updtV.(map[string]interface{}))
		}
	}
	if !found {
		log.Println("[combineConfigSlices] Could not find asset named", updtID, "in asset tree. Asset IDs resulting from template expansion must match names in asset tree exactly.")
	}
}
return cfgSlc
}
//Combines overwrites configurations in cfgMap with values found in updtMap if keys, dimensions, and datatypes match. 
//This function and related functions make heavy use of type assertion, https://www.geeksforgeeks.org/type-assertions-in-golang/
//as well as reflection https://pkg.go.dev/reflect 
//In general, the underlying type of an interface{} is determined by reflect.ValueOf(interface_name).Kind() or by type checking
//and then the type is asserted by interface_name.(Type) to access the underlying value. Often this Type is a map[string]interface{}.
//Reflection is used to check Type first to avoid the type assertion panicking. The majority of reflect calls here are for that purpose.
//Passing reflect.Value types is avoided as these make debugging harder and the performance hit of multiple reflect.ValueOf calls is not an issue here 
func combineConfigs(cfgMap map[string]interface{}, updtMap map[string]interface{}) (map[string]interface{}){
	//First iterate through top level of config data.
	//Top level will either be a single setting (e.g. tick rate) or a key to a map[string]interface{} that usually contains arrays of assets as values
	//The assets in those arrays may have maps at indetermenant depth. TODO GB: put constraints on this nesting? or allow it and handle it in later functions?
	//map keys at top level will be an asset name with []interface{} value, each entry of which corresponds
	//to an individual named asset of that type. 
	//updtMap = cleanConfig(updtMap) //removes notes and BSON "_id"-like objects. Now this is done as soon as configuraitons are received as part of document checking. 
	for k, v := range updtMap {
		cfg, ok := cfgMap[k]
		if !ok {
			log.Println("[combineConfigs] could not find update key [", k, "] in default configurations. Moving to next key")
			continue
		}
		val := reflect.ValueOf(v)
		cVal := reflect.ValueOf(cfg)
		if val.Kind() != cVal.Kind() {
			log.Println("[combineConfigs] Encountered different types for default and update. Default: ", cVal.Type(), "Update: ", val.Type())
			log.Println("[combineConfigs] Update value: ", v)
			log.Println("[combineConfigs] aborting item, moving to next entry")
			continue 
		}
		//At this top level the map should contain a slice of maps of assets. combineConfigSlices unpacks these and combines their configurations one by one
		if val.Kind() == reflect.Slice {
			cfgMap[k] = combineConfigSlices(cfg.([]interface{}), v.([]interface{}))
		//This shouldn't happen in the current state, but in the future we might have another map layer on top for versioning so unpack that and protect for it. 
		} else if val.Kind() == reflect.Map {
			for _, s := range v.(map[string]interface{}) {
				if reflect.ValueOf(s).Kind() == reflect.Map {
					cfg, ok := cfgMap[k]
					if !ok || reflect.ValueOf(cfg).Kind() != reflect.Map {
						continue
					}
					cfgMap[k] = combineConfigs(cfg.(map[string]interface{}), s.(map[string]interface{}))
				}
			}
		} else {
			log.Println("[combineConfigs] Updating", k, "from value", cfgMap[k], "to value", v)
			cfgMap[k] = v //This is for updateRate, etc. that live at the top level in twins.json
		}
	}
	return cfgMap
}

func buildState(config *cfg, fimsMap map[string]interface{}) (treeNode, map[string]asset, error) {
	// Get your list of assets from the config file
	// Each component (Grids, Feeds, etc.) implements the asset interface by
	// implementing the function signatures defined in assets.go
	// In this way, while the components are different struct types,
	// they can be stored in the same data structure as each other as `asset`s
	//fmt.Println(config)
	aList := []asset{}
	for i := range config.Grids {
		// get pointers with & so we're not trying to copy all the assets
		a := &config.Grids[i]
		aList = append(aList, a)
	}
	for i := range config.Feeds {
		a := &config.Feeds[i]
		aList = append(aList, a)
	}
	for i := range config.Ess {
		a := &config.Ess[i]
		aList = append(aList, a)
	}
	for i := range config.Gen {
		a := &config.Gen[i]
		aList = append(aList, a)
	}
	for i := range config.Loads {
		a := &config.Loads[i]
		aList = append(aList, a)
	}
	for i := range config.PV {
		a := &config.PV[i]
		aList = append(aList, a)
	}
	for i := range config.Solar {
		a := &config.Solar[i]
		aList = append(aList, a)
	}
	for i := range config.Transformers {
		a := &config.Transformers[i]
		aList = append(aList, a)
	}
	for i := range config.PCS {
		a := &config.PCS[i]
		aList = append(aList, a)
	}
	for i := range config.DCDC {
		a := &config.DCDC[i]
		aList = append(aList, a)
	}
	for i := range config.BMS {
		a := &config.BMS[i]
		aList = append(aList, a)
	}
	for i := range config.DCBUS {
		a := &config.DCBUS[i]
		aList = append(aList, a)
	}
	// reform the asset list into an asset map, keyed by the asset ID
	// then initialize each asset
	assets := make(map[string]asset)
	for _, a := range aList {
        assets[a.GetID()] = a
	    a.Init()
	}
	// assetsMap is populated by looking through assets.json for "twins" directives
	// assetsMap[asset ID][asset struct field name] = {list of assets.json register ids that field should publish/receive as}
	assetsMap := make(map[string]map[string][]string)
	// TODO: commented out for testing, needs to be generic for other FlexGen products
	// buildAssetsMaps(assetsMap)

	// Build the FIMS publish map. Also serves to allow sets and gets
	for i := range aList {
		// fimsMapEntry represents key/value pairs in fimsMap
		fimsMapEntry := make(map[string]interface{})
		// The asset type is an interface, so variables of type asset contain
		// two pieces of information, the type of the underlying data (e.g. `ess` or `pv`),
		// and a pointer to the variable itself
		// reflect.ValueOf() gets the pointer to the variable, .Elem() gets the variable itself
		thisAsset := reflect.ValueOf(aList[i]).Elem()
		ids := []string{aList[i].GetID()}
		// while assets have IDs, they can also have aliases, so we don't forget to account for those
		// the ... operator takes elements of a slice and feeds them as individual arguments to the function
		ids = append(ids, aList[i].GetAliases()...)
	FieldLoop: // special syntax to allow a `continue` from a lower level loop
		for j := 0; j < thisAsset.NumField(); j++ {
			// thisAsset.Field(j) gets us the field from the structure
			thisField := thisAsset.Field(j) 
			// The .Type() method gets us metainformation about datatype, in this case we're interested in the names of the fields of the asset structure
			// The type is a structure, so .Field works on it to get a field from the structure
			fieldName := strings.ToLower(thisAsset.Type().Field(j).Name)
			// A Field is a structure with information about a structure's fields. Converting it to an interface enables us to get at the underlying value and type information
			// Before converting it, however, we must check to be sure that it can be converted to an interface to prevent a panic.
			if thisField.CanInterface() {
				fieldInter := thisField.Interface()
				fieldKind := reflect.ValueOf(fieldInter).Kind()
				var fieldAddresses []interface{}
				var fieldStrings []string
				omit := []string{"ID", "Aliases", "StatusCfg", "CtrlWord1Cfg", "CtrlWord2Cfg", "CtrlWord3Cfg", "CtrlWord4Cfg", "Dactive", "Dreactive"}
				for _, o := range omit {
					if fieldName == o {
						// if this field is omitted, we want to get to the next field, not just look at the next omitted name
						continue FieldLoop
					}
				}
				if fieldKind == reflect.Struct {
					var lowerKeys []string
					var lowerAddresses []interface{}
					lowerKeys, lowerAddresses = getFimsMapStruct(thisField)
					for k := 0; k < len(lowerKeys); k++ {
						expandedKey := fieldName + "_" + lowerKeys[k]
						fieldStrings = append(fieldStrings, expandedKey)
						fieldAddresses = append(fieldAddresses, lowerAddresses[k])
					} 
				} else if fieldKind == reflect.Slice {
					var lowerKeys []string
					var lowerAddresses []interface{}
					lowerKeys, lowerAddresses = getFimsMapSlice(thisField)
					for k := 0; k < len(lowerKeys); k++ {
						expandedKey := fieldName + "_" + lowerKeys[k]
						fieldStrings = append(fieldStrings, expandedKey)
						fieldAddresses = append(fieldAddresses, lowerAddresses[k])
					} 
				} else {
					// .CanInterface() protects from panicking on unexported struct fields, which
					// are those that are lower case, and shouldn't be published or need to
					// be received from FIMS.
					// .Addr() gets a pointer to that struct field so updates to the struct data are always found with fimsMap
					// .Interface() wraps that pointer as interface{} so encoding/json can deal with the datatype (float64, bool, etc.) later
					if thisAsset.Addr().CanInterface() {
						fieldStrings = append(fieldStrings, fieldName)
						fieldAddresses = append(fieldAddresses, thisField.Addr().Interface())
					}
				}
				for k:= 0; k < len(fieldStrings); k++ {
					found := false
					// check if there is a twins_id from assets.json associated with this field for this asset or its aliases
					for _, id := range ids {
						// convert struct variables to strings for publishing
						if regIDs, ok := assetsMap[id][fieldStrings[k]]; ok {
							for _, regID := range regIDs {
								fimsMapEntry[regID] = fieldAddresses[k]
							}
							found = true
						}
					}
					if !found {
						fimsMapEntry[fieldStrings[k]] = fieldAddresses[k]
					}
				}
			} else {
				// If the field is not exported, and thus cannot be converted to an interface, skip it
				continue FieldLoop
			}
		}
		for _, id := range ids {
			// this would result in something like fimsMap["ess_1"]["active_power"] = interface{float64(87.4)}
			fimsMap[id] = fimsMapEntry
		}
	}
	// Build the solver tree from the config file, associate them with the assets
	treeRoot := treeNode{assets[config.Root.ID], []treeNode{}}
	treeRoot.children = buildChildren(treeRoot, assets, config.Root.Children)
	log.Printf("Configured %d assets\n", len(assets))
	return treeRoot, assets, nil
}

// buildChildren() recursively finds and builds the asset tree data structure by looking up
// each asset by ID from the config file "root" object
func buildChildren(parent treeNode, assets map[string]asset, sib []treeCfgNode) []treeNode {
	var siblings []treeNode
	var ok bool
	if len(sib) > 0 {
		for _, s := range sib {
			var node treeNode
			node.asset, ok = assets[s.ID]
			if ok {
				node.children = buildChildren(node, assets, s.Children)
				siblings = append(siblings, node)
			} else {
				log.Fatal("Didn't find the id in asset map ", s.ID)
			}
		}
	}
	return siblings
}

// instance represents the data structure of each "instance" in `assets.json`
type instance struct {
	ID         string
	Components []struct {
		// these `json:...` struct tags tell the json decoder to map "component_id" to ID,
		// where by default it looks for the same name as the field, except lowercase ("id")
		ID        string `json:"component_id"`
		Variables map[string]struct {
			RegID   string `json:"register_id"`
			TwinsID string `json:"twins_id"`
		}
		Controls map[string]struct {
			RegID   string `json:"register_id"`
			TwinsID string `json:"twins_id"`
		} `json:"ui_controls"` // the json struct tag also works for Controls -> ui_controls
	}
}

// unpackInstance() goes through the json structure in assets.json, looking through
// each component (corresponding to a Twins asset ID) and variable (corresponding
// to Twins asset struct fields)
func unpackInstance(inst instance, assetsMap map[string]map[string][]string) {
	for _, c := range inst.Components {
		for _, v := range c.Variables {
			if v.RegID != "" && v.TwinsID != "" { // We've found an entry in assets.json with a twins_id
				if _, ok := assetsMap[c.ID]; !ok { // check if entry is present in map
					assetsMap[c.ID] = make(map[string][]string)
				}
				assetsMap[c.ID][v.TwinsID] = append(assetsMap[c.ID][v.TwinsID], v.RegID)
			}
		}
		for _, v := range c.Controls {
			if v.RegID != "" && v.TwinsID != "" { // We've found an entry in assets.json with a twins_id
				if _, ok := assetsMap[c.ID]; !ok { // check if entry is present in map
					assetsMap[c.ID] = make(map[string][]string)
				}
				assetsMap[c.ID][v.TwinsID] = append(assetsMap[c.ID][v.TwinsID], v.RegID)
			}
		}
	}
}

// buildAssetsMaps() loads assets.json to look for "twins_id" fields. It does this
// for each set of ESS, Feeders, Generators and Solar. This tells Twins to publish
// and receive FIMS messages for that asset using the names that assets.json expects
// keying for assetsMap is
// assetsMap[asset ID][asset struct field name] = {list of assets.json register ids that field should publish/receive as}
func buildAssetsMaps(assetsMap map[string]map[string][]string) {
	// The anonymous structs in `decoded` parse out the JSON automatically,
	// since the structure and fields of `assets.json` are known in advance we
	// can avoid type switches on nested map[string]interface{}, and let
	// encoding/json do the lifting
	var decoded struct {
		Assets struct {
			ESS struct {
				Instances []instance `json:"asset_instances"`
			}
			Feeders struct {
				Instances []instance `json:"asset_instances"`
			}
			Generators struct {
				Instances []instance `json:"asset_instances"`
			}
			Solar struct {
				Instances []instance `json:"asset_instances"`
			}
		}
	}
	// Find the assets file. 
	// If no configuration or asset file is provided, try using assets.json in the current folder.
	// If no asset file is provided, but a configuration path is provided, try using assets.json in the same path
	// If an asset path is provided, use assets.json at that path. If a file is provided, use that file.
	var cpath string
	if len(os.Args) < 2 {
		log.Print("Config path argument not found. Usage 'twins /path/to/config'. Trying current working directory")
		cpath = "assets.json"
	} else if len(os.Args) < 3 {
		info, err := os.Stat(os.Args[1]) 
		if os.IsNotExist(err) {
			log.Fatal("TWINS asset file not found at: ", os.Args[1])
		} else if info.IsDir() {
			cpath = path.Join(os.Args[1], "assets.json")
		} else {
			if strings.Contains(os.Args[1], "/") {
				slist := strings.Split(os.Args[1], "/")
				slist[len(slist) - 1] = "assets.json"
				cpath = strings.Join(slist, "/")
			} else {
				cpath = "assets.json"
			}
			cpath = "assets.json"
		}
	} else {
		info, err := os.Stat(os.Args[2])
		if os.IsNotExist(err) {
			log.Fatal("TWINS asset file not found at: ", os.Args[2])
		} else if info.IsDir() {
			cpath = path.Join(os.Args[2], "assets.json")
		} else {
			cpath = os.Args[2]
		}
	}

	configjson, err := ioutil.ReadFile(cpath)
	if err != nil {
		log.Fatalf("Couldn't read the file %s: %s", cpath, err)
	}
	err = json.Unmarshal(configjson, &decoded)
	if err != nil {
		log.Fatal("Failed to Unmarshal assets config file")
	}
	for _, e := range decoded.Assets.ESS.Instances {
		unpackInstance(e, assetsMap)
	}
	for _, e := range decoded.Assets.Feeders.Instances {
		unpackInstance(e, assetsMap)
	}
	for _, e := range decoded.Assets.Generators.Instances {
		unpackInstance(e, assetsMap)
	}
	for _, e := range decoded.Assets.Solar.Instances {
		unpackInstance(e, assetsMap)
	}
}

// getFimsMapStruct() finds and builds a FIMS address string for a struct field in an asset.
// This function recursively steps through structfields to build a FIMS address string 
// and point it to the fields inside the structs. 
// inputStruct is assumed to be of reflect.Value data type, which can be gotten by calling 
// the .Field() method on another reflect.Value data type that represents a struct
func getFimsMapStruct(inputStruct reflect.Value) ([]string, []interface{}) {
	var fieldAddresses []interface{}
	var structStrings []string
	for i := 0; i < inputStruct.NumField(); i++ {
		thisField := inputStruct.Field(i) // thisField represents each field in the structure
		fieldName := strings.ToLower(inputStruct.Type().Field(i).Name)
		if thisField.CanInterface() {
			fieldInter := thisField.Interface()
			fieldKind := reflect.ValueOf(fieldInter).Kind()
			var lowerKeys []string
			var lowerAddresses []interface{}
			if fieldKind == reflect.Struct {
				lowerKeys, lowerAddresses = getFimsMapStruct(thisField)
				for j := 0; j < len(lowerKeys); j++ {
					expandedKey := fieldName + "_" + lowerKeys[j]
					structStrings = append(structStrings, expandedKey)
					fieldAddresses = append(fieldAddresses, lowerAddresses[j])
				} 
			} else if fieldKind == reflect.Slice {
				lowerKeys, lowerAddresses = getFimsMapSlice(thisField)
				for j := 0; j < len(lowerKeys); j++ {
					expandedKey := fieldName + "_" + lowerKeys[j]
					structStrings = append(structStrings, expandedKey)
					fieldAddresses = append(fieldAddresses, lowerAddresses[j])
				} 
			} else {
				expandedKey := fieldName
				if thisField.Addr().CanInterface() {
					structStrings = append(structStrings, expandedKey)
					fieldAddresses = append(fieldAddresses, thisField.Addr().Interface())
				}
			}
		}
	}
	return structStrings, fieldAddresses
}

// getFimsMapSlice() finds and builds a FIMS address string for a slice field in an asset.
// This function recursively steps through slice indices to build a FIMS address string 
// and point it to the ineex inside the slice. 
// inputSLice is assumed to be of reflect.Value data type, which can be gotten by calling 
// the .Field() method on a struct or using reflect.ValueOf on a slice
func getFimsMapSlice(inputSlice reflect.Value) ([]string, []interface{}) {
	var indexAddresses []interface{}
	var sliceStrings []string
	for i := 0; i < inputSlice.Len(); i++ {
		thisIndex := inputSlice.Index(i) // thisIndex represents each index in the slice
		interIndex := thisIndex.Interface()
		indexName := strconv.Itoa(i)
		indexKind := reflect.ValueOf(interIndex).Kind()
		var lowerKeys []string
		var lowerAddresses []interface{}
		if indexKind == reflect.Struct {
			lowerKeys, lowerAddresses = getFimsMapStruct(thisIndex)
			for j := 0; j < len(lowerKeys); j++ {
				expandedKey := indexName + "_" + lowerKeys[j]
				sliceStrings = append(sliceStrings, expandedKey)
				indexAddresses = append(indexAddresses, lowerAddresses[j])
			} 
		} else if indexKind == reflect.Slice {
			lowerKeys, lowerAddresses = getFimsMapSlice(thisIndex)
			for j := 0; j < len(lowerKeys); j++ {
				expandedKey := indexName + "_" + lowerKeys[j]
				sliceStrings = append(sliceStrings, expandedKey)
				indexAddresses = append(indexAddresses, lowerAddresses[j])
			} 
		} else {
			expandedKey := indexName
			if thisIndex.Addr().CanInterface() {
				sliceStrings = append(sliceStrings, expandedKey)
				indexAddresses = append(indexAddresses, thisIndex.Addr().Interface())
			}
		}
	}
	return sliceStrings, indexAddresses
}
