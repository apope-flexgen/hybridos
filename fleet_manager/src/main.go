/*
 * main.go
 *
 * package: fleet_manager
 *
 * This package contains the primary features of the Fleet Manager product. It interfaces with other modules
 * such as Scheduler and DNP3 to facilitate the operational control of all sites in a single fleet.
 *
 */

package main

import (
	"flag"
	"fmt"
	"os"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

func main() {
	// configuration & initialization
	cfgSource := parseFlags()

	err := log.InitConfig("fleet_man").Init("fleet_man")
	if err != nil {
		fmt.Fprintf(os.Stderr, "Logger failed to initialize: %v", err)
		os.Exit(-1)
	}

	log.Infof("fleet_manager starting.")
	defer log.Infof("fleet_manager stopping.")

	// must be called before configureFims to avoid overlapping FIMS connections
	masterCfg, err = retrieveConfig(cfgSource)
	if err != nil {
		log.Fatalf("Error retrieving configuration: %v.", err)
	}
	log.Infof("fleet_manager configured with: %+v.", masterCfg)

	fimsReceive, err := configureFims()
	if err != nil {
		log.Fatalf("Error configuring FIMS: %v.", err)
	}

	getDbiData()

	processDataTicker := time.NewTicker(time.Duration(masterCfg.DataProcessPeriodMs) * time.Millisecond)
	publishDataTicker := time.NewTicker(time.Duration(masterCfg.DataPublishPeriodMs) * time.Millisecond)
	unverifiedMessageTicker := time.NewTicker(time.Duration(1) * time.Second)

	// main operating loop
	for {
		if !f.Connected() {
			log.Fatalf("Lost connection to FIMS.")
		}

		select {
		case msg := <-fimsReceive:
			err := handleFimsMsg(msg)
			if err != nil {
				log.Errorf("Error handling FIMS msg with method %s and target URI %s: %s", msg.Method, msg.Uri, err.Error())
			}
		case <-processDataTicker.C:
			processData()
		case <-publishDataTicker.C:
			publishData()
		case <-unverifiedMessageTicker.C:
			err := f.ResendUnverifiedMessages()
			if err != nil {
				log.FullError("Failed to reissue message: ", err)
			}
		}
	}
}

// Parses the command-line flags. Loads the logger configuration file path into the log package,
// and returns the fleet_manager configuration source ('dbi' or a file path).
func parseFlags() (cfgSource string) {
	cfgUsage := "Give a path to the config file or 'dbi' if config is stored in the database"
	flag.StringVar(&cfgSource, "c", "dbi", cfgUsage)
	flag.StringVar(&cfgSource, "config", "dbi", cfgUsage)

	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")

	flag.Parse()
	return cfgSource
}

// processData is the central thread for Fleet Manager features and any other data processing.
func processData() {
	fleet.update()
	features.update()
	fleet.sendControlValues()
}

// publishData sends a FIMS PUBlish with Fleet Manager's data to update other modules such as the UI.
func publishData() {
	publishSites()
	publishFeatures()
}
