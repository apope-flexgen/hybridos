// Inputs implments functionality for manipulating the input arrays
package interceptor

import (
	_ "github.com/flexgen-power/echo/pkg/logger"
)

// Missle defense system (MDS) input that reroutes incoming uri missiles from our interceptor
// to a containment zone (metrics), performs necessary alterations,
// and ships missile back out to the appropriate target
// type Input struct {
// 	Uri       string            `json:"uri"`
// 	Forward   bool              `json:"forward,omitempty"`
// 	Registers map[string]string `json:"registers"` // map key is the input (twins for example) app's reg id's
// }

// // Function creates a map[input_uri]map[cid]eid for a given output uri
// func generateInputMap(regmaps []cfg.Map) (cfg.Map, error) {
// 	ipm := make(cfg.Map)
// 	var registers cfg.Maps
// 	// Iterate through the reg types to build our echoID map
// 	for _, regmap := range regmaps {
// 		rBytes, err := cfg.GetObjectBytes(regmap, "map")
// 		if err != nil {
// 			return nil, fmt.Errorf("error getting components list: %v", err)
// 		}
// 		if err := json.Unmarshal(rBytes, &registers); err != nil {
// 			return nil, fmt.Errorf("error unmarshaling registers into a []map: %v", err)
// 		}

// 		// Loop through the registers under a given regmap
// 		for _, reg := range registers {
// 			if reg["echo_id"] != nil {

// 				// Retrieve our euri and eid
// 				euri, eid := cfg.ParseFrags(reg["echo_id"].(string)).SplitFrags()

// 				// Check if we have a mapping for the euri yet
// 				if ipm[euri] == nil {
// 					ipm[euri] = make(map[string]string)
// 				}

// 				// Add a new entry for a given reg id = echo id
// 				ipm[euri].(map[string]string)[eid] = reg["id"].(string)
// 			}
// 		}
// 	}

// 	return ipm, nil
// }

// Function takes in a register map and a uri and sets it to input struct
// func (ip *Input) setInput(uri string, regs map[string]string) {
// 	ip.Registers = make(map[string]string)
// 	ip.Registers = regs
// 	ip.Uri = uri
// }
