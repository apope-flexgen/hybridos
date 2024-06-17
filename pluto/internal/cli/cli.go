package cli

import (
	"flag"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// List of command-line arguments.
type Config struct {
	Hello  bool // Pass "-hello" at startup to invoke a hello test log.
	NoFims bool // Pass "-nofims" at startup to disable FIMS.
	// ...
	// Continue list of new command line arguments here if applicable.
}

// Parse arguments from the command line.
func Parse(args []string) (*Config, error) {
	// Create a new flag set.
	fs := flag.NewFlagSet("Pluto", flag.ExitOnError)

	// Instantiate our list of commands.
	hello := fs.Bool("hello", false, "says hello.")
	nofims := fs.Bool("nofims", false, "disables FIMS.")
	// Continue more definitions here ...

	// Parse the command line arguments.
	if err := fs.Parse(args); err != nil {
		return nil, fmt.Errorf("parsing flagset: %v", err)
	}

	// Initialize our cli configuration.
	cfg := &Config{
		Hello:  *hello,
		NoFims: *nofims,
	}

	return cfg, nil
}

// Testing function. Says hello.
func Hello() {
	log.Infof("Hello.")
}
