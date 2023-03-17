// Outputs implments functionality for manipulating the output arrays
package interceptor

// Top level interceptor for an incoming uri missile
// type Output struct {
// 	Uri         string    `json:"uri"`
// 	PublishRate float64   `json:"publishRate"`
// 	Inputs      *[]*Input `json:"inputs"`
// 	Echo        cfg.Map   `json:"echo"`
// }

// Function takes in a map of input uris -> id's and allocates inputs for given ouput
// func (op *Output) allocateInputs(ipm cfg.Map) error {
// 	logger.Log.Debug().Msgf("Allocating inputs for %s", op.Uri)
// 	for key, v := range ipm {

// 		//	logger.Log.Debug().Msgf("t is %v", t)
// 		ip := &Input{}
// 		ip.Registers = make(map[string]string)
// 		ip.Registers = v.(map[string]string)
// 		ip.Uri = key
// 		*op.Inputs = append(*op.Inputs, ip)
// 	}

// 	return nil
// }

// Sets array of inputs for a given output struct, depends on op.Uri
// func (op *Output) generateInputArray(regmaps []cfg.Map) error {

// 	// Generate a map of inputs for a given output
// 	ipm, err := generateInputMap(regmaps)
// 	if err != nil {
// 		return fmt.Errorf("error generating input mapping for %s: %v", op.Uri, err)
// 	}

// 	logger.Log.Debug().Msgf("Generated input map for %s", op.Uri)

// 	// Allocate and populate the input array for a given output
// 	// if err := op.allocateInputs(ipm); err != nil {
// 	// 	return fmt.Errorf("error allocating inputs for %s: %v", op.Uri, err)
// 	// }
// 	logger.Log.Debug().Msgf("Allocating inputs for %s", op.Uri)
// 	for key, v := range ipm {

// 		//	logger.Log.Debug().Msgf("t is %v", t)
// 		ip := &Input{}
// 		ip.Registers = make(map[string]string)
// 		ip.Registers = v.(map[string]string)
// 		ip.Uri = key
// 		*op.Inputs = append(*op.Inputs, ip)
// 	}

// 	logger.Log.Debug().Msgf("Generated input array for %s", op.Uri)

// 	return nil
// }

// Function adds an echo mapping to a specific output component uri
// assumes emap has a key (comp id) to each register to be echoed under that key id
// func (op *Output) addEchoMap(emap cfg.Map) {
// 	for k, m := range emap {
// 		if op.Uri == k {
// 			logger.Log.Debug().Msgf("Adding %s echo map", op.Uri)
// 			for _, str := range m.([]string) {
// 				op.Echo[str] = 0
// 			}
// 		}
// 	}
// }
