package main

import (
	"context"
	"flag"
	"fmt"
	"os"
	"path/filepath"

	buffman "github.com/flexgen-power/go_flexgen/buffman"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	"github.com/flexgen-power/go_flexgen/fileops"
	"github.com/flexgen-power/go_flexgen/flexservice"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

// The pipeline stages used to process data: archives -> monitor -> validator -> writers -> db_clients
var (
	// Monitors input path for archives to process
	monitor *archiveMonitor

	// Carries the filepaths of archives noticed by the monitor to be decoded by the validator
	archiveQueue buffman.BufferManager

	// Decodes and validates archive data
	validator *archiveValidator

	// Writes decoded data to InfluxDB
	writerToInflux *influxWriter
	// Writes decoded data to MongoDB
	writerToMongo *mongoWriter

	// The destination DBs we want to write to
	destinations = []string{"influx", "mongo"}
)

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

	// Start thread for monitor
	log.MsgDebug("Starting monitor")
	monitor = newArchiveMonitor(config.InputPath)
	err = monitor.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start monitor stage: %v", err)
	}

	// Open buffer manager for validator work
	archiveQueue = buffman.New(buffman.Stack, config.NumValidateWorkers, monitor.out)

	// Start thread for validator
	log.MsgDebug("Starting validator")
	validator = newArchiveValidator(archiveQueue.Out(), destinations)
	err = validator.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start validator stage: %v", err)
	}

	// Start thread for writers
	log.MsgDebug("Starting writers")
	log.MsgDebug("Starting writer to influx")
	writerToInflux = newInfluxWriter(validator.outs["influx"])
	err = writerToInflux.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start writer to %s stage: %v", "influx", err)
	}
	writerToMongo = newMongoWriter(validator.outs["mongo"])
	err = writerToMongo.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start writer to %s stage: %v", "mongo", err)
	}

	// Open up flex service now that pipeline has been initialized
	err = initFlexService()
	if err != nil {
		log.Fatalf("Error initializing FlexService: %v.", err)
	}

	// Block until a fatal error
	log.Infof("All child routines started. dts main routine now running indefinitely.")
	err = group.Wait()
	if err != nil {
		log.Fatalf("Pipeline encountered a fatal error: %v", err)
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
	err = parseConfig(cfgSource)
	if err != nil {
		log.Fatalf("Failed to configure: %v", err)
	}

	fmt.Printf("Configured with %+v\n", config)
}

// Ensure that all directories necessary for the program to run exist
func initDirectories() error {
	var err error
	err = fileops.EnsureDirectoryExists(config.InputPath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure input path exists: %w", err)
	}
	err = fileops.EnsureDirectoryExists(config.FailedValidatePath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure validate error path exists: %w", err)
	}
	err = fileops.EnsureDirectoryExists(config.FailedWritePath, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to ensure write error path exists: %w", err)
	}
	// Forward path may be empty
	if len(config.FwdPath) > 0 {
		err = fileops.EnsureDirectoryExists(config.FwdPath, os.ModePerm)
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
			retVal += fmt.Sprintf("%s\n%#v\n", "---Influx writer---", writerToInflux)
			retVal += fmt.Sprintf("%s\n%#v\n", "---Mongo writer---", writerToMongo)
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
			retVal += fmt.Sprintf("# of files that failed validation - %d\n", validator.failCt)
			// Influx writer
			retVal += fmt.Sprintf("stats for db instance -  %s\n", "influx")
			retVal += fmt.Sprintf("\t db type is %s deployed at %s\n", "influx", writerToInflux.dbUrl)
			retVal += fmt.Sprintf("\t wrote %d files \n", writerToInflux.writeCnt)
			retVal += fmt.Sprintf("\t failed to write %d files \n", writerToInflux.failCnt)
			// Mongo writer
			retVal += fmt.Sprintf("stats for db instance -  %s\n", "mongo")
			retVal += fmt.Sprintf("\t db type is %s deployed at %s\n", "mongo", writerToMongo.dbUrl)
			retVal += fmt.Sprintf("\t wrote %d files \n", writerToMongo.writeCnt)
			retVal += fmt.Sprintf("\t failed to write %d files \n", writerToMongo.failCnt)
			return retVal, nil
		}),
	})
	if err != nil {
		return fmt.Errorf("failed to register show-status API endpoint: %w", err)
	}
	return nil
}
