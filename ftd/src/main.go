package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"sync/atomic"

	"flag"

	flexgen "github.com/flexgen-power/go_flexgen"
	build "github.com/flexgen-power/go_flexgen/buildinfo"
	log "github.com/flexgen-power/go_flexgen/logger"
	"golang.org/x/sync/errgroup"
)

var (
	// listens for messages to come through its FIMS connection and adds
	// them to the message queue, filtering out messages that are to be
	// handled uniquely (i.e. COPS messages)
	listener *fimsListener

	// message queue handles storing messages that need to be processed
	// while collator is busy so that listener does not get blocked
	msgQueue fimsMsgBufferManager

	// takes messages from the message queue and loads them into their
	// respective encoder, periodically passing encoders to the archiver
	collator *msgCollator

	// takes encoders from the collator and archives the encoded data
	// into a .tar.gz file
	archiver *msgArchiver
)

var controllerState uint32 // A value of 0 indicates primary, anything else is secondary, reads and writes to this variable must be atomic

func main() {
	err := configure()
	if err != nil {
		log.Fatalf("Error configuring FTD: %s.", err.Error())
	}

	// create a context to pass to all stages so if one fails, all are shutdown gracefully
	mainContext, cancelAll := context.WithCancel(context.Background())
	defer cancelAll()
	group, groupContext := errgroup.WithContext(mainContext)

	// fims messages first arrive on the fims channel in the listener, which adds them to the msgQueue to
	// await processing. filters out messages that must be processed separately/uniquely (i.e. COPS messages)
	log.MsgDebug("Starting listener")
	listener = newFimsListener()
	err = listener.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start listener stage: %v", err)
	}

	// messages are stored in the msgQueue until the collator is free to process them
	msgQueue = newFimsMsgBufferManager(1, listener.out)

	// collator encodes each message into an encoder that manages the message's URI. after a given
	// "archive period", collator passes a batch of URI encoders to the archiver
	log.MsgDebug("Starting collator")
	collator = newCollator(config.ArchivePeriod, msgQueue.out)
	err = collator.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start encoder stage: %v", err)
	}

	// archiver receives batches of encoders and archives each one into a .tar.gz file
	log.MsgDebug("Starting archiver")
	archiver = newMsgArchiver(collator.out)
	err = archiver.run(group, groupContext)
	if err != nil {
		log.Fatalf("Failed to start archiver stage: %v", err)
	}

	// block until a fatal error
	err = group.Wait()
	if err != nil {
		log.Fatalf("Pipeline encountered a fatal error: %v", err)
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
	if !flexgen.IsFileExist(config.ArchivePath) {
		log.Infof("%s doesnt exist. Creating directory", config.ArchivePath)
		err := os.MkdirAll(config.ArchivePath, 0755)
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
			for uriStr, obj := range collator.fimsMsgs {
				responseLine := ""
				responseLine += "-\t" + uriStr
				if obj.config == nil {
					retVal += responseLine + "\n"
					continue
				}
				if obj.config.Group != "" {
					retVal += responseLine + "\t" + obj.config.Group + "\n"
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
			if atomic.LoadUint32(&controllerState) == 0 {
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
