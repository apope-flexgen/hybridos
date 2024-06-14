package main

import (
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/pkg/profile"

	flexgen "github.com/flexgen-power/go_flexgen"
	"github.com/flexgen-power/go_flexgen/buildinfo"
	"github.com/flexgen-power/go_flexgen/cfgFetch"
	log "github.com/flexgen-power/go_flexgen/logger"
)

type Config struct {
	// Map of client IDs to client objects. Each client is a folder where files to be sent will appear.
	Clients map[string]ClientConfig
	// Map of server IDs to server objects. Each server is a destination to which files will be sent.
	Servers map[string]ServerConfig
	// Directory in which cloud_sync's database of file-send statuses will be stored.
	DbDir string `json:"db_directory"`
	// After this number of failed file sends, cloud_sync will pause and check all SSH connections.
	RetryLimit int `json:"retry_limit"`
	// If cloud_sync has to pause and check SSH connections (see RetryLimit), it will sleep for this many seconds.
	SleepLimitSeconds int `json:"sleep_limit_seconds"`
	// Size to allocate for the clear queue and retry queue channels.
	BufSz int `json:"buffer_size"`
}

var config Config

func main() {
	// source of the configuration data. by default, get from DBI. can also be passed path to .json file
	var configSrc string
	flag.StringVar(&configSrc, "c", "dbi", "specify the config file for the process")
	flag.StringVar(&configSrc, "config", "dbi", "specify the config file for the process")
	// optional profiling. note: only works if flags are in front of config file
	var prof string
	flag.StringVar(&prof, "p", "", "enables profiling and specifies type")
	// optional path to .json file that contains log settings to use instead of the default log settings
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	// parse all of the above flags
	flag.Parse()

	// launch profiler if command-line argument given. see Dave Cheney YouTube talk on profiling for more context on how to use
	if prof == "cpu" {
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	}

	err := log.InitConfig("cloud_sync").Init("cloud_sync")
	if err != nil {
		fmt.Printf("Error initializing logger for cloud_sync: %s.\n", err.Error())
		os.Exit(-1)
	}

	err = configureFlexService()
	if err != nil {
		log.Fatalf("Error configuring flexService: %v.", err)
	}

	err = getConfiguration(configSrc)
	if err != nil {
		log.Fatalf("Configuration error: %v.", err)
	}

	// verify at least one client and at least one server was included in configuration
	if len(config.Servers) < 1 {
		log.Fatalf("Error validating config: no servers found in config.")
	}
	if len(config.Clients) < 1 {
		log.Fatalf("Error validating config: no clients found in config.")
	}

	// configure each server and add it to the server map
	for name, serverConfig := range config.Servers {
		cleanedName := strings.ReplaceAll(strings.ToLower(name), " ", "_")
		newServer, err := createServer(cleanedName, serverConfig)
		if err != nil {
			log.Fatalf("Error creating server with name %s: %v.", cleanedName, err)
		}
		servers[newServer.name] = newServer
		log.Infof("Added server %s to server map.", newServer.name)
	}

	// process each client in configuration
	for name, clientConfig := range config.Clients {
		// construct a client based on configuration data and add it to the map.
		// includes loading initial files from source directory into the send queue
		cleanedName := strings.ReplaceAll(strings.ToLower(name), " ", "_")
		newClient, err := createClient(cleanedName, clientConfig)
		if err != nil {
			log.Fatalf("Error creating client with name %s: %v.", cleanedName, err)
		}
		clients[newClient.name] = newClient
		log.Infof("Added client %s to client map.", newClient.name)

		// launch goroutine to add already-existing files from client's error directory to the retry queues of the
		// servers to which the files are recorded as having been failed to be sent
		err = newClient.sortFailures()
		if err != nil {
			log.Fatalf("Error sorting failures for client %s: %v.", newClient.name, err)
		}

		// monitor the client's source directory and add new files to the send queue
		go newClient.monitor()
		log.Infof("Started: monitor (%s)", newClient.name)

		if newClient.config.Tar.Mode == "tar" {
			go newClient.tar()
			log.Infof("Started: tar     (%s)", newClient.name)
		} else if newClient.config.Tar.Mode == "untar" {
			go newClient.untar()
			log.Infof("Started: untar     (%s)", newClient.name)
		}

		// send files from the send queue to each server for which the client is configured
		go newClient.send()
		log.Infof("Started: send    (%s)", newClient.name)

		// as files finish being sent and are put on the clear queue by the send thread, take them off the clear queue and delete them
		go newClient.clear()
		log.Infof("Started: clear   (%s)", newClient.name)

		for _, serv := range newClient.connectedServers {
			// setup connection for each server the client is configured to send to
			err = serv.setupConnection(newClient)
			if err != nil {
				log.Fatalf("Failed setting up connection from %s to %s: %v", newClient.name, serv.name, err)
			}
			// launch one retry thread for each server for which the client is configured.
			// attempts to send files that have previously failed
			go newClient.retry(serv)
			log.Infof("Started: retry   (%s-%s)", newClient.name, serv.name)
		}
	}

	// Start actual transfer work along each client-server connection.
	// The send and retry routines perform transfers by sending transfer requests to the associated transfer stage.
	for _, cl := range clients {
		for _, serv := range cl.connectedServers {
			err = serv.runTransfer(cl)
			if err != nil {
				log.Fatalf("Failed to start transfer stage from %s to %s: %v", cl.name, serv.name, err)
			}
			log.Infof("Started: transfer   (%s-%s)", cl.name, serv.name)
		}
	}

	// CloudSync should run indefinitely, so select {} is used to block program termination indefinitely
	log.Infof("All threads started. cloud_sync now running indefinitely.")
	select {}
}

// Initializes the flexService server and registers custom APIs to it.
func configureFlexService() error {
	flexService := flexgen.GetService()
	err := flexService.Init(true, filepath.Base(os.Args[0]))
	if err != nil {
		return fmt.Errorf("failed to initialize flexService: %w", err)
	}
	flexService.SetVersion(fmt.Sprintf("cloud_sync %s %s %s", buildinfo.BuildVersion.Tag, buildinfo.BuildVersion.Build, buildinfo.BuildVersion.Commit))

	err = flexService.RegisterApi(flexgen.ApiCommand{
		ApiName: "print-stats",
		ApiDesc: "displays current working state of cloud_sync",
		ApiCallback: flexgen.Callback(func(args []interface{}) (string, error) {
			var retVal string = ""
			retVal += fmt.Sprintf("# servers configured - %d\n", len(config.Servers))
			retVal += fmt.Sprintf("# clients configured - %d\n", len(config.Clients))
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register print-stats API: %w", err)
	}

	err = flexService.RegisterApi(flexgen.ApiCommand{
		ApiName: "show-servers",
		ApiDesc: "shows server statuses",
		ApiCallback: flexgen.Callback(func(args []interface{}) (string, error) {
			var retVal string = ""
			for name, server := range servers {
				retVal += fmt.Sprintf("%s:\n", name)
				retVal += fmt.Sprintf("transferring to %s at location %s\n", server.config.IP, server.config.Dir)
			}
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-servers API: %w", err)
	}
	return nil
}

// Retrieves configuration data and unmarshalls it into the config struct.
// If given source is "dbi", retrieves config data from DBI via FIMS.
// Otherwise, treats source as config file path and reads from the file.
func getConfiguration(configSrc string) error {
	var configBytes []byte
	if strings.EqualFold(configSrc, "dbi") {
		configMap, err := cfgFetch.Retrieve("cloud_sync", "dbi")
		if err != nil {
			return fmt.Errorf("failed to retrieve configuration data from DBI: %w", err)
		}

		configBytes, err = json.Marshal(configMap)
		if err != nil {
			return fmt.Errorf("failed to convert DBI's returned configMap to json bytes: %w", err)
		}
	} else {
		buf, err := os.ReadFile(configSrc)
		if err != nil {
			return fmt.Errorf("failed to read config file: %w", err)
		}

		configBytes = buf
	}

	err := json.Unmarshal(configBytes, &config)
	if err != nil {
		return fmt.Errorf("failed to unmarshall configBytes: %w", err)
	}
	log.Infof("Configuration: %+v", config)
	return nil
}

// Checks if the given directory exists, and creates it if it does not exist.
func ensureDirectoryExists(directoryPath string) error {
	_, err := os.Stat(directoryPath)
	if err != nil {
		log.Errorf("failed to ensure directory exists: %v", err)
	} else {
		return nil
	}
	if !errors.Is(err, fs.ErrNotExist) {
		return fmt.Errorf("failed to get stat: %w", err)
	}

	err = os.MkdirAll(directoryPath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to make directory: %w", err)
	}
	return nil
}
