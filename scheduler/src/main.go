/**
 * Scheduler
 * main.go
 *
 * Created May 2021
 *
 * Scheduler is a back-end module that allows HybridOS users to schedule, update, or
 * cancel FIMS message events. Example use case: schedule FIMS message to have ESSs
 * charge for an hour in the afternoon then discharge for an hour in the evening.
 *
 */

package main

import (
	"encoding/json"
	"fims"
	"flag"
	"fmt"
	"io/ioutil"
	"os"
	"os/signal"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	"github.com/flexgen-power/scheduler/internal/docfetch"
	"github.com/flexgen-power/scheduler/internal/flextime"
	"github.com/flexgen-power/scheduler/internal/websockets"
	"golang.org/x/sys/unix"
)

// isPrimaryScheduler indicates if the box Scheduler is running on is operating in primary mode.
// At startup, Scheduler assumes secondary mode until it is told otherwise by COPS.
var isPrimaryScheduler bool = false

func main() {
	// scheduler configuration
	var cfgSource string
	cfgUsage := "Give a path to the config file or 'dbi' if config is stored in the database"
	flag.StringVar(&cfgSource, "c", "dbi", cfgUsage)
	flag.StringVar(&cfgSource, "config", "dbi", cfgUsage)
	// logger configuration
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	flag.Parse()

	// initialize logger
	err := log.InitConfig("scheduler").Init("scheduler")
	if err != nil {
		fmt.Printf("Error initializing logger for Scheduler: %s\n", err.Error())
		os.Exit(-1)
	}

	log.Infof("Starting Scheduler")
	defer log.Infof("Exiting Scheduler")

	// setup os signal handling to catch SIGTERM, SIGINT
	// (Note: SIGKILL and SIGSTOP cannot be handled. Neither can synchronous signals like segfaults from within the program)
	processSignalChannel := make(chan os.Signal, 1) // channel buffer required since os/signal package does not block on sending
	signal.Notify(processSignalChannel, unix.SIGTERM, unix.SIGINT)

	// configure time zone variable
	localTimezone, err = flextime.GetLocalTimezone()
	if err != nil {
		log.Errorf("Error getting local time zone from OS: %v.", err)
		return
	}

	// get initial data from config source
	var initializationDocs map[string]json.RawMessage
	if strings.EqualFold(cfgSource, "dbi") {
		initializationDocs, err = docfetch.FetchDocumentsFromDbi("scheduler")
		if err != nil {
			log.Errorf("Error fetching documents from DBI: %v.", err)
			return
		}
	} else {
		cfgBytes, err := ioutil.ReadFile(cfgSource)
		if err != nil {
			log.Errorf("Error reading configuration from file %s: %v.", cfgSource, err)
			return
		}
		initializationDocs = map[string]json.RawMessage{
			"configuration": cfgBytes,
		}
	}

	// Configure the FIMS connection to be used for the rest of runtime.
	// Must do this after using docfetch since it establishes its own FIMS
	// connection; each process can only have one FIMS connection at a time.
	fimsConn, err := fims.Configure("scheduler", "/scheduler")
	if err != nil {
		log.Fatalf("Error configuring FIMS: %v.", err)
	}
	// Assign our connection to the global pointer
	f = &fimsConn

	// Start a FIMS channel that will recieve FIMS requests.
	fimsReceive := make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)

	if err = initializeScheduler(initializationDocs, fimsReceive); err != nil {
		log.Errorf("Error initializing Scheduler: %v.", err)
		return
	}

	eventCheckTicker := time.NewTicker(time.Duration(time.Second))

	for {
		if !f.Connected() {
			log.MsgFatal("Lost connection to FIMS server. Exiting the program now.")
		}
		select {
		case msg := <-fimsReceive:
			processFims(msg)
		case <-eventCheckTicker.C:
			// check all schedules every second
			checkMasterSchedule()
		case <-setpointEnforcementTicker.C:
			// check status values of active event and send SETs if deviation from control values.
			// does not execute if Fleet Manager, there is no active event, or this feature is turned off
			if schedCfg.LocalSchedule != nil && schedCfg.LocalSchedule.SetpointEnforcement.Enabled {
				err := verifyActiveEventStatus()
				if err != nil {
					log.Errorf("Error verifying active event status: %v.", err)
				}
			}
		case msg := <-websockets.MsgReceive:
			log.Infof("Received WebSocket %s message to %s with body %v.", msg.Method, msg.Uri, msg.Body)
			processWebSocketMsg(msg)
		case connUpdate := <-websockets.ClientConnUpdates:
			processSiteConnectionUpdate(connUpdate)
		case <-websockets.SocketServerConnUpdates:
			f.SendPub("/scheduler/connected", buildConnectionMap())
		case sig := <-processSignalChannel:
			log.Fatalf("Terminating after receiving handled process signal: %d (%s)", sig, sig.String())
		}
	}
}
