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
	"github.com/flexgen-power/go_flexgen/cfgfetch"
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
var configSource string
var defaultConfigFilePath = "/usr/local/etc/config/go_metrics"

func main() {
	// Handle inital flag inputs for cli commands
	flag.StringVar(&mode, "mode", "", "set mode: [modbus | dnp3]")
	flag.StringVar(&scfg, "echo", "", "client config file path for server echo generation only")
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
	flag.StringVar(&configSource, "c", "", "source of configuration. Use \"dbi\" to read from DBI or provide a filepath to read from file\nUsage: go_metrics -c [config source]")
	flag.StringVar(&configSource, "config", "", "source of configuration. Use \"dbi\" to read from DBI or provide a filepath to read from file\nUsage: go_metrics -c [config source]")
	flag.Parse()

	metrics.MdoFile = mdo
	metrics.ConfigErrorsFile = configErrors
	metrics.ProcessName = processName
	if len(processName) == 0 {
		processName = "go_metrics"
	}
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

	var new_metrics_info metrics.NewMetricsInfo
	new_metrics_info, err = getAndParseConfig()
	if err != nil {
		log.Fatalf("Configuration error: %v.", err)
	}

	// set up all of the publish tickers
	new_metrics_info = metrics.GetPubTickers(new_metrics_info)

	// extract parent uris (so we know what to subscribe to)
	// and also make an easy way to "fetch" data into our input Unions
	metrics.GetSubscribeUris(new_metrics_info)

	// Get any additional configuration documents that may exist
	go metrics.GetAdditionalConfigurations()

	var wg sync.WaitGroup
	wg.Add(1)
	metrics.StartEvalsAndPubs(new_metrics_info, &wg)
	wg.Wait()
}

// Check if a config file path is valid. Will try various forms of the path to rule out other possibilities that might be available
func isValidConfigFilePath(configFilePath string) bool {
	// check for file extension. If not there, try a couple of things before giving up
	if !strings.Contains(configFilePath, ".json") {
		configPathAndFile = configFilePath + "metrics.json"
	} else {
		configPathAndFile = configFilePath
	}
	if _, err := os.Stat(configPathAndFile); err != nil {
		configPathAndFile = configFilePath + "_metrics.json"
		if _, err = os.Stat(configPathAndFile); err != nil {
			return false
		}
	}
	return true
}

// Find the file path to use for configuration if none has been provided
// Used as legacy behavior for when no configuration source has been provided by -c
// or if a configuration destination needs to be chosen for the templated file generated by --template_output
func findConfigFilePath() (configFilePath string, err error) {
	// Legacy behavior if the config file path is not provided
	// resolve the command line args into absolute filepath
	for _, flag := range flag.Args() {
		configFilePath, err = filepath.Abs(flag)
		break
	}
	if len(flag.Args()) == 0 || err != nil {
		_, err = os.Stat("../configs/metrics.json")
		if err == nil {
			configFilePath, err = filepath.Abs("metrics.json")
		}
	}

	if err != nil || !isValidConfigFilePath(configFilePath) {
		// If none of the options above worked, try the default path
		if !isValidConfigFilePath(defaultConfigFilePath) {
			return "", fmt.Errorf("please supply a source of configuration. Usage: go_metrics -c [config source (dbi or filepath)]")
		} else {
			// The default path passed validation
			return defaultConfigFilePath, nil
		}
	}
	// The command line or relative paths passed validation
	return configFilePath, nil
}

// Retrieve and parse configuration based on the source provided
// If no source is provided, try a number of alternatives before return an error to be reported to the user
func getAndParseConfig() (metrics.NewMetricsInfo, error) {
	var err error
	if configSource == "" {
		configSource, err = findConfigFilePath()
		if err != nil {
			return metrics.NewMetricsInfo{}, err
		}
	}

	// Retrieve the configuration from the given source
	cfgMap, err := cfgfetch.Retrieve(processName, configSource)
	if err != nil {
		return metrics.NewMetricsInfo{}, fmt.Errorf("error retrieving configuration from %s: %w", configSource, err)
	}
	// The retrieved value is required to be a map when received from fims
	cfgBytes, err := json.Marshal(cfgMap)
	if err != nil {
		return metrics.NewMetricsInfo{}, fmt.Errorf("error parsing configuration: %w", err)
	}

	metrics.ConfigSource = configSource
	new_metrics_info, _, _ := metrics.UnmarshalConfig(cfgBytes, "")

	if templateOutput {
		// Setup the destination of the generated template config
		configFilePath := configSource
		if strings.EqualFold(configSource, "dbi") {
			// We want to explicitly store a file even when dbi is being used
			configFilePath, err = findConfigFilePath()
			if err != nil {
				return metrics.NewMetricsInfo{}, fmt.Errorf("error finding template config destination: %v", err)
			}
		}
		outputFile, _ := json.MarshalIndent(metrics.MetricsConfig, "", "    ")

		if len(templateConfig) > 0 {
			fd, _ := os.OpenFile(templateConfig, os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			defer fd.Close()
			fd.Write(outputFile)
		} else {
			fd, _ := os.OpenFile(metrics.GetParentUri(configFilePath)+"/templated_final_config.json", os.O_RDWR|os.O_CREATE|os.O_TRUNC, 0644)
			defer fd.Close()
			fd.Write(outputFile)
		}
	}
	return new_metrics_info, nil
}
