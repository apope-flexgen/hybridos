package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"github.com/flexgen-power/dts/pkg/dts"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	"github.com/flexgen-power/go_flexgen/fileops"
	"github.com/flexgen-power/go_flexgen/flexservice"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

var pipeline dts.Pipeline

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

	err := log.InitConfig("dts").Init("dts")
	if err != nil {
		fmt.Println(err)
		os.Exit(-1)
	}

	fmt.Println("config source parsed is ", cfgSource)
	err = dts.ParseConfig(cfgSource)
	if err != nil {
		log.Fatalf("Failed to configure: %v", err)
	}

	fmt.Printf("Configured with %+v\n", dts.GlobalConfig)
}

// Ensure that all directories necessary for the program to run exist
func initDirectories() error {
	var err error
	err = fileops.EnsureDirectoryExists(dts.GlobalConfig.InputPath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure input path exists: %w", err)
	}
	err = fileops.EnsureDirectoryExists(dts.GlobalConfig.FailedValidatePath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure validate error path exists: %w", err)
	}
	err = fileops.EnsureDirectoryExists(dts.GlobalConfig.FailedWritePath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure write error path exists: %w", err)
	}
	// Forward path may be empty
	if len(dts.GlobalConfig.FwdPath) > 0 {
		err = fileops.EnsureDirectoryExists(dts.GlobalConfig.FwdPath, os.ModePerm)
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
	serv.SetVersion(fmt.Sprintf("dts %s %s %s", build.BuildVersion.Tag, build.BuildVersion.Build, build.BuildVersion.Commit)) //set version

	//API registrations for flexservice
	err = serv.RegisterApi(flexservice.ApiCommand{
		ApiName: "show-dbs",
		ApiDesc: "displays all active endpoints currently writing to",
		ApiCallback: flexservice.Callback(func(args []interface{}) (string, error) {
			var retVal string = ""
			retVal += fmt.Sprintf("%s\n%#v\n", "---Influx writer---", pipeline.WriterToInflux)
			retVal += fmt.Sprintf("%s\n%#v\n", "---Mongo writer---", pipeline.WriterToMongo)
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-dbs API endpoint: %w", err)
	}

	err = serv.RegisterApi(flexservice.ApiCommand{
		ApiName: "show-stats",
		ApiDesc: "Total number of files decoded",
		ApiCallback: flexservice.Callback(func(args []interface{}) (string, error) {
			var retVal string = ""
			retVal += fmt.Sprintf("# of files that failed validation - %d\n", pipeline.Validator.FailCt)
			// Influx writer
			retVal += fmt.Sprintf("stats for db instance -  %s\n", "influx")
			retVal += fmt.Sprintf("\t db type is %s deployed at %s\n", "influx", pipeline.WriterToInflux.DbUrl)
			retVal += fmt.Sprintf("\t wrote %d files \n", pipeline.WriterToInflux.WriteCnt)
			retVal += fmt.Sprintf("\t failed to write %d files \n", pipeline.WriterToInflux.FailCnt)
			// Mongo writer
			retVal += fmt.Sprintf("stats for db instance -  %s\n", "mongo")
			retVal += fmt.Sprintf("\t db type is %s deployed at %s\n", "mongo", pipeline.WriterToMongo.DbUrl)
			retVal += fmt.Sprintf("\t wrote %d files \n", pipeline.WriterToMongo.WriteCnt)
			retVal += fmt.Sprintf("\t failed to write %d files \n", pipeline.WriterToMongo.FailCnt)
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-status API endpoint: %w", err)
	}
	return nil
}
