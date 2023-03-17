/* 2021 FlexGen Internal Remapping System

* To build:
	$ go build
* To run:
	$ ./cmd -config=../configs/CFGFILE -mode=MODE
	-mode can be either [modbus] or [dnp3]
	* add a [-h] to the execution to provide a help message on flags

*/
package main

import "github.com/flexgen-power/echo/pkg/logger"

func main() {

	// Handle inital flag inputs
	run := cmd.ParseFlags()

	// Handle execution for runtime or strictly file generation
	if run {
		logger.Log.Info().Msg("Executing runtime execution ...")
		cmd.Execute()
	} else {
		logger.Log.Info().Msg("Generating files ...")

		// Generate a server.json, echo.sh, and a echo.json provided the flag [-echo] is supplied
		if err := cmd.GenerateFiles(); err != nil {
			logger.Log.Fatal().Err(err).Msgf("Error generating files")
		}
	}

}
