package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"github.com/flexgen-power/archive_cocoon/pkg/archive_cocoon"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	"github.com/flexgen-power/go_flexgen/fileops"
	"github.com/flexgen-power/go_flexgen/flexservice"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

var pipeline archive_cocoon.Pipeline

func main() {
	initConfig()

	err := initDirectories()
	if err != nil {
		log.Fatalf("Failed to ensure all necessary directories exist: %v", err)
	}

	// Error group for all of our pipeline tasks
	mainContext, cancelAll := context.WithCancel(context.Background())
	defer cancelAll()
	group, groupContext := errgroup.WithContext(mainContext)

	// Open up flex service (note: this might technically cause a race condition due to variables exposed to flex service not being initialized yet)
	err = initFlexService()
	if err != nil {
		log.Fatalf("Error initializing FlexService: %v.", err)
	}

	// Run the main pipeline
	err = pipeline.Run(group, groupContext)
	if err != nil {
		log.Fatalf("Main pipeline stopped with error: %v", err)
	} else {
		log.Fatalf("Main pipeline stopped without an error")
	}
}

// Parse program execution flags
func parseFlags() (cfgSource string) {
	cfgUsage := "Give a path to the config file or 'dbi' if config is stored in the database"
	flag.StringVar(&cfgSource, "c", "dbi", cfgUsage)
	flag.StringVar(&cfgSource, "config", "dbi", cfgUsage)
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	flag.Parse()
	return cfgSource
}

// Initialize program from configuration determined from flags
func initConfig() {
	cfgSource := "" //config source to parse for options
	cfgSource = parseFlags()

	err := log.InitConfig("archive_cocoon").Init("archive_cocoon")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	fmt.Println("config source parsed is ", cfgSource)
	err = archive_cocoon.ParseConfig(cfgSource)
	if err != nil {
		log.Fatalf("Failed to configure: %v", err)
	}

	fmt.Printf("Configured with %+v\n", archive_cocoon.GlobalConfig)
}

// Ensure that all directories necessary for the program to run exist
func initDirectories() error {
	var err error
	err = fileops.EnsureDirectoryExists(archive_cocoon.GlobalConfig.InputPath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure input path exists: %w", err)
	}
	err = fileops.EnsureDirectoryExists(archive_cocoon.GlobalConfig.OutputPath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure output path exists: %w", err)
	}
	// failure path may be empty
	if len(archive_cocoon.GlobalConfig.FailedConvertPath) > 0 {
		err = fileops.EnsureDirectoryExists(archive_cocoon.GlobalConfig.FailedConvertPath, os.ModePerm)
		if err != nil {
			return fmt.Errorf("failed to ensure validate error path exists: %w", err)
		}
	}
	// Forward path may be empty
	if len(archive_cocoon.GlobalConfig.FwdPath) > 0 {
		err = fileops.EnsureDirectoryExists(archive_cocoon.GlobalConfig.FwdPath, os.ModePerm)
		if err != nil {
			return fmt.Errorf("failed to ensure forward path exists: %w", err)
		}
	}
	return nil
}

// Start the flexservice server
func initFlexService() error {
	serv := flexservice.GetService()
	curProcName := filepath.Base(os.Args[0])
	err := serv.Init(true, curProcName)
	if err != nil {
		return fmt.Errorf("failed to initialize FlexService server: %w", err)
	}
	serv.SetVersion(fmt.Sprintf("archive_cocoon %s %s %s", build.BuildVersion.Tag, build.BuildVersion.Build, build.BuildVersion.Commit)) //set version

	//API registrations for flexservice
	err = serv.RegisterApi(flexservice.ApiCommand{
		ApiName: "show-stats",
		ApiDesc: "Total number of files converter",
		ApiCallback: flexservice.Callback(func(args []interface{}) (string, error) {
			return fmt.Sprintf("# of files that failed conversion - %d\n", pipeline.Converter.FailCt), nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-status API endpoint: %w", err)
	}
	return nil
}
