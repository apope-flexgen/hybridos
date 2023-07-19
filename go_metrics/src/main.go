package main

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"sync"

	config "github.com/flexgen-power/echo/pkg/config"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	log "github.com/flexgen-power/go_flexgen/logger"
	metrics "github.com/flexgen-power/go_metrics/pkg/go_metrics"

	//build "go_flexgen/buildinfo"

	"github.com/pkg/profile"
)

// fims globals
var f fims.Fims
var fimsMap chan fims.FimsMsgRaw
var msg fims.FimsMsgRaw

// global globals
var mode string
var scfg string
var outputpath string
var ipaddress string
var prof string
var configErrors string
var mdo string
var configPathAndFile string
var processName string
var templateOutput bool
var templateConfig string

func main() {
	// Handle inital flag inputs for cli commands
	flag.StringVar(&mode, "mode", "", "set mode: [modbus | dnp3]")
	flag.StringVar(&scfg, "c", "", "client config file path for server echo generation only")
	flag.StringVar(&outputpath, "output", "", "this is the file path where the server and echo file will be going")
	flag.StringVar(&ipaddress, "ip", "0.0.0.0", "address used in the server files fo r client server connection")
	flag.StringVar(&prof, "prof", "", "profile [cpu | mem]")
	flag.StringVar(&configErrors, "e", "", "check the config file for errors and output any errors to the specified file")
	flag.StringVar(&configErrors, "errors", "", "check the config file for errors and output any errors to the specified file")
	flag.StringVar(&mdo, "mdo", "", "output the state of the Metrics Data Object")
	flag.StringVar(&processName, "name", "", "the process name to send as and subscribe to")
	flag.StringVar(&processName, "n", "", "the process name to send as and subscribe to")
	flag.BoolVar(&templateOutput, "template_output", false, "save the templated config file to path/to/config/templated_final_config.json")
	flag.StringVar(&log.ConfigFile, "logCfg", "", "location to save the log file; if included default values will be overturned")
	flag.StringVar(&templateConfig, "template_output_file", "", "save the templated config file to the specified location")
	flag.Parse()

	metrics.MdoFile = mdo
	metrics.ConfigErrorsFile = configErrors
	metrics.ProcessName = processName
	if len(templateConfig) > 0 {
		templateOutput = true
	}

	err := log.InitConfig("go_metrics").Init("go_metrics")
	if err != nil {
		fmt.Printf("Error initializing logger for go_metrics: %s\n", err.Error())
		os.Exit(-1)
	}
	log.Infof("Starting go_metrics")
	defer log.Infof("Exiting go_metrics")

	if prof == "cpu" { //profiling argument, only works if flags are in front of config file
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	}

	log.Infof("go_metrics \n\tTag:%s \n\tVersion:%s\n\tCommit: %s\n", build.BuildVersion.Tag, build.BuildVersion.Build, build.BuildVersion.Commit)

	// Handle execution for runtime or strictly file generation
	if scfg != "" {

		// Generate a server.json, and a echo.json
		// Generate our files, retrieve our client data
		c, err := config.GenerateClientAndServerFile(mode, scfg, outputpath, ipaddress)
		if err != nil {
			log.Fatalf("error generating client server file: %v", err)
		}

		// Generate an echo json file for the given client
		if err := c.GenerateEchoJSONFile(scfg, outputpath); err != nil {
			log.Fatalf("error generating echo json file: %v", err)
		}
	}

	getAndParseFile()

	// set up all of the publish tickers
	metrics.GetPubTickers()

	// extract parent uris (so we know what to subscribe to)
	// and also make an easy way to "fetch" data into our input Unions
	metrics.GetSubscribeUris()

	var wg sync.WaitGroup
	wg.Add(1)
	metrics.StartEvalsAndPubs(&wg)
	wg.Wait()
}

func getAndParseFile() {
	var configPath string
	var err error

	// resolve the command line args into absolute filepath
	for _, flag := range flag.Args() {
		configPath, err = filepath.Abs(flag)
		break
	}
	if len(flag.Args()) == 0 || err != nil {
		if _, err = os.Stat("../configs/metrics.json"); err != nil {
			log.Fatalf("Please supply a path to .json configuration file. Usage: go_metrics [path/to/config/file]")
		} else {
			configPathAndFile, err = filepath.Abs("metrics.json")
			if err != nil {
				log.Fatalf("Could not resolve filepath")
			}
		}
	}

	// check for file extension. If not there, try a couple of things before giving up
	if !strings.Contains(configPath, ".json") {
		configPathAndFile = configPath + "metrics.json"
	} else {
		configPathAndFile = configPath
	}
	if _, err := os.Stat(configPathAndFile); err != nil {
		configPathAndFile = configPath + "_metrics.json"
		if _, err = os.Stat(configPathAndFile); err != nil {
			log.Fatalf(".json configuration not found in %s. Please supply a path to .json configuration file. Usage: go_metrics [path/to/config/file]", configPath)
		}
	}

	// read in the file and unmarshal it into a MetricsFile struct
	data, err := os.ReadFile(configPathAndFile)
	if err != nil {
		log.Fatalf("Error reading json file: %s", err)
	}
	metrics.ConfigFileLoc = configPathAndFile
	metrics.UnmarshalConfig(data)

	if templateOutput {
		outputFile, _ := json.MarshalIndent(metrics.MetricsConfig, "", "    ")

		if len(templateConfig) > 0 {
			fd, _ := os.OpenFile(templateConfig, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			defer fd.Close()
			fd.Write(outputFile)
		} else {
			fd, _ := os.OpenFile(metrics.GetParentUri(configPath)+"/templated_final_config.json", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			defer fd.Close()
			fd.Write(outputFile)
		}
	}
}
