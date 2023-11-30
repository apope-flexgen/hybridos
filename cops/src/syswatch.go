// Implement syswatch package to collect hardware level statistics.
package main

import (
	"os"
	"os/signal"

	sys "github.com/flexgen-power/hybridos/cops/syswatch"
)

// Start data collection on hardware statistics.
// Run function as it's own dedicated goroutine.
func runCollectors(prof string) {

	// Setup profiling: -prof=["cpu", "mem", "trace"]
	sys.Setup(prof)

	// Start collection of system metrics.
	sys.StartCollectors()

	// TODO: handle pub stats in later ticket
	// Begin periodic pubs to FIMS containing system data.
	//go sys.PublishSystemStats(f)

	if prof == "trace" {
		signalChan := make(chan os.Signal, 1)
		signal.Notify(signalChan, os.Interrupt)
		<-signalChan
	} else {
		select {}
	}
}
