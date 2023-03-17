// File interceptor.go implements generation of our interceptor
package interceptor

import (
	"encoding/json"
	"fmt"
	"strings"

	"github.com/flexgen-power/echo/pkg/cfg"
)

// Stores the top level echo json setup
type Interceptor struct {
	Outputs *[]*Output `json:"outputs"`
}

type Output struct {
	Uri         string    `json:"uri"`
	PublishRate float64   `json:"publishRate"`
	Heartbeat   string    `json:"heartbeat,omitempty"`
	Inputs      *[]*Input `json:"inputs"`
	Echo        cfg.Map   `json:"echo,omitempty"`
}
type Input struct {
	Uri       string            `json:"uri"`
	Registers map[string]string `json:"registers"` // map key is the input (twins for example) app's reg id's
}

// Function provides an Interceptor filled with amount of needed outputs with uris
func (cept *Interceptor) GenerateModbusOutputs(opuris []string, emap cfg.Map, cfgs cfg.Map) error {
	var regmaps []interface{}
	// Initialize all necessary outputs
	// cept.allocateOutputs(len(opuris))
	outputs := make([]*Output, len(opuris))

	// Initialize our outputs
	for i := range outputs {
		outputs[i] = new(Output)
	}

	// Populate outputs with output uris
	for i, op := range outputs {
		op.PublishRate = cfgs["components"].([]interface{})[i].(map[string]interface{})["frequency"].(float64) // setting pubrate to file frequency
		op.Echo = make(cfg.Map)
		ip := make([]*Input, 0)
		op.Inputs = &ip
		if cfgs["components"].([]interface{})[i].(map[string]interface{})["heartbeat_enabled"] != nil {
			if cfgs["components"].([]interface{})[i].(map[string]interface{})["heartbeat_enabled"].(bool) {
				op.Heartbeat = cfgs["components"].([]interface{})[i].(map[string]interface{})["component_heartbeat_read_uri"].(string)
			}
		}
	}

	cept.Outputs = &outputs

	// Populate our list of uris
	if len(*cept.Outputs) != len(opuris) {
		return fmt.Errorf("error mismatch count of outputs and uris")
	}

	for i, op := range *cept.Outputs {
		op.Uri = opuris[i]
	}

	// Populate all our outputs with their corresponding echo maps
	for _, op := range *cept.Outputs {
		if emap[op.Uri] != nil {
			op.Echo = emap[op.Uri].(map[string]interface{})
		}
	}

	// Generate our input array for a given output
	for _, op := range *cept.Outputs {

		// Retrieve array of registers for given output (modbus specific)
		strList := strings.Split(op.Uri, "/")
		id := strList[len(strList)-1]
		for _, comp := range cfgs["components"].([]interface{}) {
			if comp.(map[string]interface{})["id"] == id {
				regmaps = comp.(map[string]interface{})["registers"].([]interface{})
			}
		}
		if regmaps == nil {
			return fmt.Errorf("error getting register map list")
		}

		// Retrieve register map that ties to the specific op.Uri
		ipm, err := generateInputMap(regmaps)
		if err != nil {
			return fmt.Errorf("error generating input mapping for %s: %v", op.Uri, err)
		}
		for key, v := range ipm {

			//	logger.Log.Debug().Msgf("t is %v", t)
			ip := &Input{}
			ip.Registers = make(map[string]string)
			ip.Registers = v.(map[string]string)
			ip.Uri = key
			*op.Inputs = append(*op.Inputs, ip)
		}
	}
	return nil
}

// Function creates a map[input_uri]map[cid]eid for a given output uri
func generateInputMap(regmaps []interface{}) (cfg.Map, error) {
	ipm := make(cfg.Map)
	var registers cfg.Maps
	// Iterate through the reg types to build our echoID map
	for _, regmap := range regmaps {
		registers = make(cfg.Maps, 1)
		rBytes, err := cfg.GetObjectBytes(regmap, "map")
		if err != nil {
			return nil, fmt.Errorf("error getting components list: %v", err)
		}
		if err := json.Unmarshal(rBytes, &registers); err != nil {
			return nil, fmt.Errorf("error unmarshaling registers into a []map: %v", err)
		}

		// Loop through the registers under a given regmap
		for _, reg := range registers {
			if reg["echo_id"] != nil {

				// Retrieve our euri and eid
				// euri, eid := cfg.ParseFrags(reg["echo_id"].(string)).SplitFrags()
				strList := strings.Split(reg["echo_id"].(string), "/")
				euri := "/" + strList[1] + "/" + strList[2]
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
func (cept *Interceptor) GenerateDNPOutputs(opuris []string, emap cfg.Map, cfgs cfg.Map) error {
	var regmaps []interface{}
	var err error

	outputs := make([]*Output, len(opuris))

	// Initialize our outputs
	for i := range outputs {
		outputs[i] = new(Output)
	}

	// Populate outputs with output uris
	for _, op := range outputs {
		op.PublishRate = cfgs["system"].(map[string]interface{})["frequency"].(float64) // setting pubrate to file frequency
		op.Echo = make(cfg.Map)
		ip := make([]*Input, 0)
		op.Inputs = &ip
	}

	cept.Outputs = &outputs

	// Populate our list of uris
	if len(*cept.Outputs) != len(opuris) {
		return fmt.Errorf("error mismatch count of outputs and uris")
	}

	for i, op := range *cept.Outputs {
		op.Uri = opuris[i]
	}

	// Populate all our outputs with their corresponding echo maps
	for _, op := range *cept.Outputs {
		if emap[op.Uri] != nil {
			op.Echo = emap[op.Uri].(map[string]interface{})
		}
	}

	// Generate our input array for a given output
	for _, op := range *cept.Outputs {
		var ipm cfg.Map = make(cfg.Map)
		for _, typename := range cfgs["registers"].([]interface{}) {
			regmaps = typename.(map[string]interface{})["map"].([]interface{})

			// Retrieve register map that ties to the specific op.Uri
			ipm, err = generateDNPInputMap(regmaps, ipm, op.Uri)
			if err != nil {
				return fmt.Errorf("error generating input mapping for %s: %v", op.Uri, err)
			}
		}
		for key, v := range ipm {
			ip := &Input{}
			ip.Registers = make(map[string]string)
			ip.Registers = v.(map[string]string)
			ip.Uri = key
			*op.Inputs = append(*op.Inputs, ip)
		}
	}
	return nil
} // Function creates a map[input_uri]map[cid]eid for a given output uri
func generateDNPInputMap(regmaps []interface{}, ipm map[string]interface{}, uri string) (cfg.Map, error) {

	// Loop through the registers under a given regmap
	for _, reg := range regmaps {
		if reg.(map[string]interface{})["echo_id"] != nil && reg.(map[string]interface{})["uri"] == uri {

			// Retrieve our euri and eid
			strList := strings.Split(reg.(map[string]interface{})["echo_id"].(string), "/")
			euri := "/" + strList[1] + "/" + strList[2]
			echo_id := strList[len(strList)-1]

			// Check if we have a mapping for the euri yet
			if ipm[euri] == nil {
				ipm[euri] = make(map[string]string)
			}

			// Add a new entry for a given reg id = echo id
			ipm[euri].(map[string]string)[reg.(map[string]interface{})["id"].(string)] = echo_id
		}
	}
	return ipm, nil
}
