package main

import (
	"encoding/json"
	"io/ioutil"
	"log"
	"os"
	"path"
	"reflect"
	"strings"
	"strconv"
)

// treeCfgNode is a struct to unmarshal the power system
// tree in twins.json into
type treeCfgNode struct {
	ID       string
	Children []treeCfgNode
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
	Gens           []gen
	Solar          []pv
	Loads          []load
	Transformers   []xfmr
	PCS            []pcs
	BMS            []bms
	Root           treeCfgNode
}

// readConfig() looks for args to the program, follows the first arg
// as a path, and looks for twins.json in it
func readConfig(config *cfg) {
	// Looking for twins.json
	var cpath string
	if len(os.Args) < 2 {
		log.Print("Config path argument not found. Usage 'twins /path/to/config'. Trying current working directory")
		cpath = "twins.json"
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
	
	configjson, err := ioutil.ReadFile(cpath)
	if err != nil {
		log.Fatalf("Couldn't read the file %s: %s", cpath, err)
	}
	err = json.Unmarshal(configjson, config)
	if err != nil {
		log.Fatal("Failed to Unmarshal config file")
	}
}

func buildState(config *cfg, fimsMap map[string]interface{}) (treeNode, map[string]asset, error) {
	// Get your list of assets from the config file
	// Each component (Grids, Feeds, etc.) implements the asset interface by
	// implementing the function signatures defined in assets.go
	// In this way, while the components are different struct types,
	// they can be stored in the same data structure as each other as `asset`s
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
	for i := range config.Gens {
		a := &config.Gens[i]
		aList = append(aList, a)
	}
	for i := range config.Loads {
		a := &config.Loads[i]
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
	for i := range config.BMS {
		a := &config.BMS[i]
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
