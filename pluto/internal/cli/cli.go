package cli

import (
	"flag"
	"fmt"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// List of command-line arguments.
type Config struct {
	Hello bool // Pass "-hello" at startup to invoke a hello test log.
	// ...
	// Continue list of new command line arguments here if applicable.
}

// Parse arguments from the command line.
func Parse(args ...string) (*Config, error) {
	// Create a new flag set.
	fs := flag.NewFlagSet("Pluto", flag.ContinueOnError)

	// Instantiate our list of commands.
	hello := fs.Bool("hello", false, "says hello.")
	// Continue more definitions here ...

	// Parse the command line arguments.
	if err := fs.Parse(args); err != nil {
		return nil, fmt.Errorf("parsing flagset: %v", err)
	}

	// Initialize our cli configuration.
	cfg := &Config{
		Hello: *hello,
	}

	return cfg, nil
}

// Testing function. Says hello.
func Hello() {
	log.Infof("Hello.")
}
