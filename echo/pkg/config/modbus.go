package config

import (
	"encoding/json"
	"fmt"
	"log"
	"net/url"
	"strings"

	"github.com/buger/jsonparser"
)

// Modbus struct for what is needed
type ModClient struct {
	Config        map[string]interface{} // Stores our file data, jsonparser parses this
	Server        map[string]interface{} // Store our output server.json data for a modbus server
	Erm           map[string]interface{} // echo register map (registers that need to be echo'd)
	Eid           map[string]interface{} // stores the generated echo.json
	Handler       ArrHandler
	Uris          []string
	Offsethistory map[int64]map[string][]float64 // stores a map of component : register type : offsets
}

// Perform modbus client file validation here
func (mc *ModClient) Validate() error {
	if mc == nil {
		return fmt.Errorf("modclient is nil")
	}

	if mc.Config == nil {
		return fmt.Errorf("modclient config is nil")
	}

	return nil
}

// Parse the input config file into modbus json format
func (c *Client) getModConfig(filename string) error {
	mc := ModClient{}
	var err error

	// Get Cfg File data
	configBytes, err := GetFileBytes(filename)
	if err != nil {
		log.Fatalf("error retrieving client config data: %v", err)
	}
	err = json.Unmarshal(configBytes, &mc.Config)
	if err != nil {
		log.Fatalf("error unmarshalling client config data: %v", err)
	}

	// Validate our data
	if err := mc.Validate(); err != nil {
		log.Fatalf("error validating modbus client data: %v", err)
	}
	// Set all the Uris that are in the client file
	if component_list, ok := mc.Config["components"]; ok {
		for _, comp := range component_list.([]interface{}) {
			// Generate newuri and add it
			if component_map, ok := comp.(map[string]interface{}); ok {
				if component_id, ok := component_map["id"]; ok {
					if component_id_string, ok := component_id.(string); ok {
						newUri := fmt.Sprintf("/components/" + component_id_string)
						if !findString(newUri, mc.Uris) {
							mc.Uris = append(mc.Uris, newUri)
						}
					}
				}
			}
		}
	} else {
		log.Fatalf("error retrieving 'components' field from client config")
	}

	// Set config to main client
	c.cfgType = &mc
	return nil
}

// Function that makes the layout of echo.json
func (mc *ModClient) GenerateEchoStruct() (Layout, error) {
	var layout Layout
	var err error
	// Generate the echo.json basically
	if layout.Outputs, err = GenerateModbusOutputs(mc.Uris, mc.Erm, mc.Config); err != nil {
		log.Fatalf("error generating outputs: %v", err)
	}

	return layout, nil
}

// Function that sets up the "system" object in our modbus server file
func (mc *ModClient) SystemInfoCreation(ipaddress string, is_gcom_modbus bool) error {
	var err error
	var fileInfo map[string]interface{} = make(map[string]interface{})
	var systemInfo map[string]interface{} = make(map[string]interface{})
	var connMap map[string]interface{} = make(map[string]interface{})
	mc.Server = make(map[string]interface{})

	connBytes, err := GetObjectBytes(mc.Config, "connection") //Grab the connection info from client
	if err != nil {
		log.Fatalf("error parsing config: %v", err)
	}

	err = json.Unmarshal(connBytes, &connMap)
	if err != nil {
		log.Fatalf("error parsing config: %v", err)
	}

	// Set our system info values
	if connMap["id"] != nil {
		systemInfo["id"] = strings.Replace(connMap["id"].(string), " ", "_", -1)
	} else if connMap["id"] == nil {
		systemInfo["id"] = strings.Replace(connMap["name"].(string), " ", "_", -1)
	}
	if connMap["name"] != nil {
		systemInfo["name"] = strings.Replace(connMap["name"].(string), " ", "_", -1)
	} else if connMap["name"] == nil {
		systemInfo["name"] = strings.Replace(connMap["id"].(string), " ", "_", -1)
	}
	if connMap["device protocol"] != nil {
		systemInfo["protocol"] = strings.Replace(connMap["device protocol"].(string), " ", "_", -1)
	}
	if connMap["device protocol version"] != nil {
		systemInfo["version"] = strings.Replace(connMap["device protocol version"].(string), " ", "_", -1)
	}
	if ipaddress != "0.0.0.0" {
		systemInfo["ip_address"] = ipaddress
	} else {
		systemInfo["ip_address"] = ipaddress
	}

	if port, ok := connMap["port"]; ok {
		systemInfo["port"] = port.(float64)
	}
	if connMap["device_id"] != nil {
		systemInfo["device_id"] = connMap["device_id"]
	} else {
		if (!is_gcom_modbus) {
			systemInfo["device_id"] = float64(1)
		}
	}
	mc.Server["fileInfo"] = fileInfo
	mc.Server["system"] = systemInfo
	// fmt.Println(systemInfo)
	return nil
}

// This function iterates through modbus client data and alters/stores it back to modbus struct
func (mc *ModClient) CreateServerFile(is_gcom_modbus bool) ([]byte, error) {
	var ServerRegInfo = make(map[string]interface{})
	var AllServerRegGroups = make([]map[string]interface{}, 0)
	var regType string
	var uri string
	var offbyone bool
	var component_id_string string
	device_id := int64(1)
	system_level_device_id := false
	if !is_gcom_modbus {
		if mc.Server["system"] != nil {
			if _, ok := mc.Server["system"].(map[string]interface{}); ok {
				if mc.Server["system"].(map[string]interface{})["device_id"] != nil {
					device_id_float := mc.Server["system"].(map[string]interface{})["device_id"].(float64)
					device_id = int64(device_id_float)
					system_level_device_id = true
				}
			}
		}
	}
	var echoMap url.Values = url.Values{}

	// Define some register level actions to perform
	regfunc := func(m map[string]interface{}, current_device_id int64, handler ArrHandler) error {

		// If m Map refers to our register type array
		if m["map"] != nil {
			// Get bytes of map for given reg type
			b, err1 := json.Marshal(m["map"])
			if err1 != nil {
				return fmt.Errorf("> csf-regfunc() > err no map bytes. m: %v", m)
			}

			// Get regType
			if register_group_type, ok := m["type"]; ok {
				regType = register_group_type.(string)
				regType = strings.ToLower(regType)
				regType = strings.Replace(regType, " ", "_", -1)
			}
			mc.Offsethistory[current_device_id][regType] = make([]float64, 0)

			// Recursively iterate down into our maps for given register type
			jsonparser.ArrayEach(b, handler)

			// Else we have selected a specific register
		} else if m["offset"] != nil {
			if !findInt(m["offset"].(float64), mc.Offsethistory[current_device_id][regType]) {
				mc.Offsethistory[current_device_id][regType] = append(mc.Offsethistory[current_device_id][regType], m["offset"].(float64))
				if offbyone {
					m["offset"] = m["offset"].(float64) - 1
				}
				if m["echo_id"] == nil && m["id"] != nil {
					echoMap.Add(uri, m["id"].(string)) //Used to make a list of all the registers that dont have an echo_id
				}
				// Set our component uri
				m["uri"] = uri
				delete(m, "echo_id")
				// Append result
				if ServerRegInfo[regType] == nil {
					ServerRegInfo[regType] = make([]map[string]interface{}, 0)
				}
				ServerRegInfo[regType] = append(ServerRegInfo[regType].([]map[string]interface{}), m)
			} else {
				log.Fatalf("There is a duplicate offset, please fix component id: %s, register_id: %s, register offset: %f\n", uri, m["id"], m["offset"])
			}
		}
		return nil
	}

	// Define component action function
	compfunc := func(m map[string]interface{}, handler ArrHandler) error {

		// Create our array of unique component ID's, compUri is used down the line in func handlers
		if m["id"] != nil {
			var ok bool
			if component_id_string, ok = m["id"].(string); ok {
				uri = "/components/" + component_id_string
				if mc.Offsethistory == nil {
					mc.Offsethistory = make(map[int64]map[string][]float64)
				}
				if m["device_id"] != nil {
					var device_id_float float64
					device_id_float, ok = m["device_id"].(float64)
					device_id = int64(device_id_float)
					if system_level_device_id && !is_gcom_modbus {
						mc.Server["system"].(map[string]interface{})["device_id"] = device_id
					}
				} 
				if !ok {
					device_id = int64(1)
				}
				if mc.Offsethistory[device_id] == nil {
					mc.Offsethistory[device_id] = make(map[string][]float64)
				}

				if len(ServerRegInfo) > 0 && is_gcom_modbus{
					added := false
					for i, register_group := range AllServerRegGroups {
						if temp_device_id, ok := register_group["device_id"]; ok {
							device_id_int := temp_device_id.(int64)
							if device_id_int == device_id {
								AllServerRegGroups[i] = ServerRegInfo
								added = true
							}
						}
					}
					if !added {
						AllServerRegGroups = append(AllServerRegGroups, ServerRegInfo)
					}
				}

				set_reg_group := false
				for _, register_group := range AllServerRegGroups {
					if temp_device_id, ok := register_group["device_id"]; ok {
						device_id_int := temp_device_id.(int64)
						if device_id_int == device_id {
							ServerRegInfo = register_group
							set_reg_group = true
						}
					}
				}
				if !set_reg_group && is_gcom_modbus {
					ServerRegInfo = make(map[string]interface{})
					ServerRegInfo["device_id"] = device_id
				}
				
			}
		
			if register_groups, ok := m["registers"]; ok {
				for _, reg := range register_groups.([]interface{}) {
					// Perform actions
					reg_map, ok := reg.(map[string]interface{})
					if ok {
						if is_gcom_modbus && reg_map["device_id"] != nil {
							device_id = reg_map["device_id"].(int64)
						}
						if err := regfunc(reg_map, device_id, mc.Handler); err != nil {
							log.Fatalf(" > Iterator() >> error unmarshaling register: %v", err)
						}
					}
				}
			} else {
				log.Fatalf("Could not find registers mapping in client config")
			}
		}
		return nil
	}

	mc.Handler = func(data []byte, dataType jsonparser.ValueType, offset int, err error) {
		var m map[string]interface{}

		// Unmarshal our json data
		if err := json.Unmarshal(data, &m); err != nil {
			log.Fatalf(" > Iterator() >> error unmarshaling register: %v", err)
		}

		if m == nil {
			log.Fatalf(" > Iterator() >> Map is nil %v", m)
		}

		// Perform actions
		if err := regfunc(m, device_id, mc.Handler); err != nil {
			log.Fatalf(" > Iterator() >> error unmarshaling register: %v", err)
		}
	}

	// Create our component iterator
	for _, comp := range mc.Config["components"].([]interface{}) {
		// Perform actions
		if comp.(map[string]interface{})["off_by_one"] == nil || !comp.(map[string]interface{})["off_by_one"].(bool) {
			offbyone = false
		} else if comp.(map[string]interface{})["off_by_one"].(bool) {
			offbyone = true
		}
		if err := compfunc(comp.(map[string]interface{}), mc.Handler); err != nil {
			log.Fatalf(" > Iterator() >> error unmarshaling register: %v", err)
		}
	}
	// Set our registers back to the server
	if is_gcom_modbus {
		if len(ServerRegInfo) > 0{
			added := false
			for i, register_group := range AllServerRegGroups {
				if temp_device_id, ok := register_group["device_id"]; ok {
					device_id_int := temp_device_id.(int64)
					if device_id_int == device_id {
						AllServerRegGroups[i] = ServerRegInfo
						added = true
					}
				}
			}
			if !added {
				AllServerRegGroups = append(AllServerRegGroups, ServerRegInfo)
			}
		}
		mc.Server["registers"] = AllServerRegGroups
	} else {
		mc.Server["registers"] = ServerRegInfo
	}

	// Order our server file by specific order
	output, err := MarshalOrderJSON(mc.Server, []string{"fileInfo", "system", "registers"})
	if err != nil {
		return nil, fmt.Errorf("error ordering our server file: %v", err)
	}

	// Save our echomap - transform into a map[string]interface{}
	m := make(map[string]interface{})

	for k, v := range echoMap {
		n := make(map[string]interface{})
		for _, val := range v {
			n[val] = 0
		}
		m[k] = n
	}
	mc.Erm = m //assign map to struct variable

	return output, nil
}

//Used to generate the entire echo.json
func GenerateModbusOutputs(opuris []string, emap map[string]interface{}, cfgs map[string]interface{}) ([]*Output, error) {
	var regmaps []interface{}

	// Initialize all necessary outputs
	outputs := make([]*Output, len(opuris))

	// Initialize our outputs
	for i := range outputs {
		outputs[i] = new(Output)
	}

	// Populate outputs with output uris
	for i, op := range outputs {
		if cfgs["components"].([]interface{})[i].(map[string]interface{})["frequency"] != nil {
			op.PublishRate = cfgs["components"].([]interface{})[i].(map[string]interface{})["frequency"].(float64) // setting pubrate to file frequency
		}
		op.Echo = make(map[string]interface{})
		ip := make([]Input, 0)
		op.Inputs = ip
		if cfgs["components"].([]interface{})[i].(map[string]interface{})["heartbeat_enabled"] != nil && cfgs["components"].([]interface{})[i].(map[string]interface{})["component_heartbeat_read_uri"] != nil {
			if cfgs["components"].([]interface{})[i].(map[string]interface{})["heartbeat_enabled"].(bool) {
				op.Heartbeat = cfgs["components"].([]interface{})[i].(map[string]interface{})["component_heartbeat_read_uri"].(string)
			}
		}
	}

	outstruct := outputs

	// Populate our list of uris
	if len(outstruct) != len(opuris) {
		return nil, fmt.Errorf("error mismatch count of outputs and uris")
	}

	for i, op := range outstruct {
		op.Uri = opuris[i]
	}

	// Populate all our outputs with their corresponding echo maps
	for _, op := range outstruct {
		if emap[op.Uri] != nil {
			op.Echo = emap[op.Uri].(map[string]interface{})
		}
	}

	// Generate our input array for a given output
	for _, op := range outstruct {

		// Retrieve array of registers for given output (modbus specific)
		strList := strings.Split(op.Uri, "/")
		id := strList[len(strList)-1]
		for _, comp := range cfgs["components"].([]interface{}) {
			if comp.(map[string]interface{})["id"] == id {
				regmaps = comp.(map[string]interface{})["registers"].([]interface{})
			}
		}
		if regmaps == nil {
			return nil, fmt.Errorf("error getting register map list")
		}

		// Retrieve register map that ties to the specific op.Uri
		ipm, err := generateInputMap(regmaps)
		if err != nil {
			return nil, fmt.Errorf("error generating input mapping for %s: %v", op.Uri, err)
		}
		for key, v := range ipm {

			//	logger.Log.Debug().Msgf("t is %v", t)
			ip := Input{}
			ip.Registers = make(map[string]string)
			ip.Registers = v.(map[string]string)
			ip.Uri = key
			op.Inputs = append(op.Inputs, ip)
		}
	}
	return outputs, nil
}

// Function creates a map[input_uri]map[cid]eid for a given output uri
func generateInputMap(regmaps []interface{}) (map[string]interface{}, error) {
	ipm := make(map[string]interface{})
	var registers []map[string]interface{}
	// Iterate through the reg types to build our echoID map
	for _, regmap := range regmaps {
		registers = make([]map[string]interface{}, 1)
		rBytes, err := GetObjectBytes(regmap, "map")
		if err != nil {
			return nil, fmt.Errorf("error getting components list: %v", err)
		}
		if err := json.Unmarshal(rBytes, &registers); err != nil {
			return nil, fmt.Errorf("error unmarshaling registers into a []map: %v", err)
		}

		// Loop through the registers under a given regmap
		for _, reg := range registers {
			if reg["echo_id"] != nil {
				var euri string

				// Retrieve our euri and eid
				strList := strings.Split(reg["echo_id"].(string), "/")
				for i := 1; i < len(strList)-1; i++ {
					euri = euri + "/" + strList[i]
				}
				echo_id := strList[len(strList)-1]

				// Check if we have a mapping for the euri yet
				if ipm[euri] == nil {
					ipm[euri] = make(map[string]string)
				}

				// Add a new entry for a given reg id = echo id
				ipm[euri].(map[string]string)[reg["id"].(string)] = echo_id
			}
		}
	}

	return ipm, nil
}

//Substitute for contains function for integers
func findInt(target float64, list []float64) bool {
	for _, a := range list {
		if a == target {
			return true
		}
	}
	return false
}
