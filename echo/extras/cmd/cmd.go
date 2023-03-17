// Package implements main.go level flag handling and execution types
package cmd

import (
	"flag"
	"fmt"

	"github.com/flexgen-power/echo/pkg/client"
	"github.com/flexgen-power/echo/pkg/logger"
	"github.com/rs/zerolog"
)

// Some globals to store our flags to be used throughout
var mode string
var cfgstr string
var echostr string
var echo *bool
var run *bool
var debug *bool

// Generate and output an echo.json and a server file
func GenerateFiles() error {

	// Generate our files, retrieve our client data
	logger.Log.Info().Msg("Generating client server file ... ")
	c, err := client.GenerateClientAndServerFile(mode, cfgstr)
	if err != nil {
		return fmt.Errorf("error generating client server file: %v", err)
	}

	// Generate an echo json file for the given client
	if *echo {
		logger.Log.Info().Msg("Generating echo.json file ... ")
		if err := c.GenerateEchoJSONFile(); err != nil {
			return fmt.Errorf("error generating echo json file: %v", err)
		}
	}

	return nil
}

// Execute will handle the runtime looping and tickers
// func Execute() {

// 	// Check to see if user provided client file to create an echo.json
// 	interceptor, err := itc.NewIntereptor(cfgstr)
// 	if err != nil {
// 		logger.Log.Fatal().Err(err).Msg("Error creating new Interceptor")
// 	}

// 	// temporary use for intercepotr so we dont get an error
// 	if interceptor == nil {
// 		logger.Log.Fatal().Msg("Nil interceptor ... ")

// 	}

// 	// Populate our gmap with the interceptor data

// }

// ParseFlags handles the cli input flags and places them into global vars
func ParseFlags() bool {

	// Define some user level flags
	debug = flag.Bool("debug", false, "sets log level to debug")
	run = flag.Bool("run", false, "executes our runtime program (not yet supported)")
	echo = flag.Bool("echo", false, "generates an echo.json file with provided client config file")
	flag.StringVar(&mode, "mode", "", "set mode: [modbus | dnp3]")
	flag.StringVar(&cfgstr, "config", "", "client config file path")
	// flag.StringVar(&echostr, "ecfg", "", "The json used to start echo")
	flag.Parse()

	// Verify the flags recieved are in tact to continue program execution
	verifyFlags()

	return *run //, echostr
}

// Verify the provided flags are valid against each other
func verifyFlags() {

	// Handle debug flag
	if *debug {
		logger.Log.Info().Msg("Set log level to debug")
		zerolog.SetGlobalLevel(zerolog.DebugLevel)
	}

	// Check for client config file and mode if we are not executing runtime remapping
	if !*run {
		if cfgstr == "" { // Check for config file path
			logger.Log.Fatal().Msg("did not specify a client file. use [-h] for more info")
		}
		if mode == "" { // Check for provided mode
			logger.Log.Fatal().Msg("did not specify a mode. use [-h] for more info")
		}
	}

	// if run and echo
	if *run && !*echo {
		logger.Log.Info().Msg("Starting runtime execution. Expecting config = echo.json file")
	}

	// handle case for generating echo file and then immediately starting execution

	// if run , then expects path to echo.json
}
