package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"syscall"

	"flag"

	"github.com/flexgen-power/ftd/pkg/ftd"
	flexgen "github.com/flexgen-power/go_flexgen"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	"github.com/flexgen-power/go_flexgen/cfgFetch"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

var pipeline ftd.Pipeline

func main() {
	err := configure()
	if err != nil {
		log.Fatalf("Error configuring FTD: %s.", err.Error())
	}

	// create a context to pass to the pipeline so if one stage fails, all are shutdown gracefully;
	// also handle a potential SIGTERM sent by systemd to ensure we gracefully terminate when
	// commanded to stop or restart (https://www.freedesktop.org/software/systemd/man/systemd.kill.html)
	mainContext, shutdown := signal.NotifyContext(context.Background(), syscall.SIGTERM)
	defer shutdown()
	group, groupContext := errgroup.WithContext(mainContext)

	err = pipeline.Run(group, groupContext)
	if err != nil {
		log.Fatalf("Pipeline closed with error: %v", err)
	} else {
		log.Fatalf("Pipeline closed gracefully.")
	}
}

// Perform all configuration for FTD.
func configure() error {
	cfgSource := parseFlags()
	// init logger config
	err := log.InitConfig("ftd").Init("ftd")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}
	err = configureFlexService()
	if err != nil {
		return fmt.Errorf("failed to configure FlexService: %w", err)
	}

	// Configure based on provided sources
	log.Infof("Config source specified: %s", cfgSource)
	err = retrieveAndReadConfiguration(cfgSource)
	if err != nil {
		return fmt.Errorf("failed to retrieve or read configuration: %w", err)
	}

	// check if output directory exist. If not try creating one
	if !flexgen.IsFileExist(ftd.GlobalConfig.ArchivePath) {
		log.Infof("%s doesnt exist. Creating directory", ftd.GlobalConfig.ArchivePath)
		err := os.MkdirAll(ftd.GlobalConfig.ArchivePath, 0755)
		if err != nil {
			return fmt.Errorf("failed to make directory for output archives: %w", err)
		}
	}
	return nil
}

// Parses command-line flags such as module configuration source and logger configuration source.
func parseFlags() (cfgSource string) {
	flag.StringVar(&cfgSource, "c", "config.json", "specify the config file for the process")
	flag.StringVar(&cfgSource, "config", "config.json", "specify the config file for the process")
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	flag.Parse()
	return cfgSource
}

// Starts the UNIX socket server to accept and respond to flexCtl API requests.
// flexCtl API endpoints are defined within this function as well.
func configureFlexService() error {
	serv := flexgen.GetService()
	curProcName := filepath.Base(os.Args[0])
	err := serv.Init(true, curProcName)
	if err != nil {
		return fmt.Errorf("failed to initialize FlexService server: %w", err)
	}
	serv.SetVersion(fmt.Sprintf("ftd %s %s %s", build.BuildVersion.Tag, build.BuildVersion.Build, build.BuildVersion.Commit)) //set version
	log.Infof("Unix Server ready")

	//register some of debugging APIs
	err = serv.RegisterApi(flexgen.ApiCommand{
		ApiName: "show-uris",
		ApiDesc: "displays all URIs received from fims server",
		ApiCallback: flexgen.Callback(func(args []interface{}) (string, error) {
			var retVal string = ""
			for uriStr, obj := range pipeline.Collator.FimsMsgs {
				responseLine := ""
				responseLine += "-\t" + uriStr
				if obj.Config == nil {
					retVal += responseLine + "\n"
					continue
				}
				if obj.Config.Group != "" {
					retVal += responseLine + "\t" + obj.Config.Group + "\n"
				} else {
					retVal += responseLine + "\n"
				}
			}
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-uris API: %w", err)
	}

	err = serv.RegisterApi(flexgen.ApiCommand{
		ApiName: "show-state",
		ApiDesc: "displays the state of controller",
		ApiCallback: flexgen.Callback(func(args []interface{}) (string, error) {
			var my_state = "Secondary"
			if ftd.ControllerStateIsPrimary() {
				my_state = "Primary"
			}
			retVal := fmt.Sprintf("FTD running as %s instance\n", my_state)
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-state API: %w", err)
	}
	return nil
}

// Retrieves and reads in FTD configuration data.
func retrieveAndReadConfiguration(cfgSource string) error {
	// retrieve configuration from dbi or from file
	configBody, err := cfgFetch.Retrieve("ftd", cfgSource)
	if err != nil {
		return fmt.Errorf("failed to retrieve configuration data: %w", err)
	}

	// translate configuration data to internal data structures
	err = ftd.HandleConfiguration(configBody)
	if err != nil {
		return fmt.Errorf("failed to parse configuration data: %w", err)
	}
	return nil
}
