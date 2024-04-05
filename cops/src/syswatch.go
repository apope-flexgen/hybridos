// Implement syswatch package to collect hardware level statistics.
// Interface original overwatch codebase as a package to work with COPS.
package main

import (
	"os"
	"os/signal"

	sys "github.com/flexgen-power/hybridos/cops/syswatch"
)

// Start data collection on hardware statistics.
// Run function as it's own dedicated goroutine.
func runCollectors(prof string, syswatch bool) {

	// Setup profiling: -prof=["cpu", "mem", "trace"]
	sys.Setup(prof, generateProcessList(config.ProcessList))

	// Start collection of system metrics.
	sys.StartCollectors()

	// Begin periodic pubs to FIMS containing system data.
	if syswatch {
		go sys.PublishSystemStats(f)
	}

	if prof == "trace" {
		signalChan := make(chan os.Signal, 1)
		signal.Notify(signalChan, os.Interrupt)
		<-signalChan
	} else {
		select {}
	}
}

// Create []string type of process list from COPS configuration.
func generateProcessList(ps []Process) []string {
	var list []string

	// Concatenate list of process names.
	for _, p := range ps {
		list = append(list, p.Name)
	}

	return list
}
