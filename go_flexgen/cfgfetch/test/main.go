// Calls cfgFetch retrieval with the given process name and config source.
// Usage: go run test/main.go <process name> <config source>
package main

import (
	"fmt"
	"os"

	cfgfetch "github.com/flexgen-power/hybridos/go_flexgen/cfgfetch"
	log "github.com/flexgen-power/hybridos/go_flexgen/logger"
)

func main() {
	// Setup logger
	err := (&log.Config{
		Moniker:           "cfgfetch test",
		ToConsole:         true,
		ToFile:            false,
		SeverityThreshold: log.TRACE,
	}).Init("test")
	if err != nil {
		fmt.Printf("Failed to start logger: %v\n", err)
		os.Exit(-1)
	}

	// Get arguments
	args := os.Args[1:]
	fmt.Printf("Arguments: %v\n", args)
	processName := args[0]
	configSource := args[1]

	// Call retrieval
	config, err := cfgfetch.Retrieve(processName, configSource)
	if err != nil {
		log.Fatalf("Failed to retrieve config: %v", err)
	}

	// Print config retrieved
	fmt.Printf("%v\n", config)
}
