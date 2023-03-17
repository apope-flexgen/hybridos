// Client package implements generic client data retrieval
package client

// import (
// 	"fmt"

// 	"github.com/flexgen-power/echo/pkg/logger"
// )

// // Func getConfig retrieves configuration client data and stores it to respective struct types
// func (c *Client) getConfig(filename string) error {

// 	// Check mode
// 	switch c.mode {
// 	case "dnp3":
// 		logger.Log.Debug().Msg("Handling mode DNP3 ... ")
// 		if err := c.getDNPConfig(filename); err != nil {
// 			return fmt.Errorf("error retrieving modbus config file: %v", err)
// 		}
// 	case "modbus":
// 		logger.Log.Debug().Msg("Handling mode Modbus ... ")
// 		if err := c.getModConfig(filename); err != nil {
// 			return fmt.Errorf("error retrieving modbus config file: %v", err)
// 		}
// 	default:
// 		return fmt.Errorf("error determining mode. Pick [modbus|dnp3]")
// 	}

// 	return nil
// }
