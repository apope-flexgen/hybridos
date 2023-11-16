/**
 * Central Operating Process Supervisor (COPS)
 *
 * Created January 2021
 *
 * COPS is a process supervisor that checks on other processes
 * using a heartbeat to make sure they are still running. If a
 * process becomes hung or fails, COPS will take action. Failure
 * actions include sending an alarm notification to the events
 * module, printing a warning to the terminal, and sending a
 * kill command to the system process so that system services
 * can hopefully restart the process.
 *
 * COPS also records statistics about the health of the processes
 * that it monitors. COPS will publish these statistics periodically
 * over FIMS on the URI /cops/processStats. They can also be
 * accessed with a FIMS GET request.
 *
 * Running List of All Threads Running in COPS
 * - fimsListen / processFIMS
 * - manageHeartbeats
 * - patrolProcesses
 * - statisticsReport (long, perhaps opportunity for optimization)
 * - connectOverTCP / processC2C
 *
 */

package main

import (
	"fims"
	"flag"
	"fmt"
	"os"
	"reflect"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
)

// Global variables
var processName = "cops"                        // Retrieve configuration from DBI using this process name
var processJurisdiction map[string]*processInfo // Map of processes that will be checked to see if they are up and running
var f fims.Fims                                 // FIMS connection
var beginningTime time.Time                     // Timestamp of when COPS started running
var updatedProcesses []*processInfo
var heartRate, patrolRate, briefingRate, statsUpdateTicker *time.Ticker // Tickers for main loop

func main() {
	// Parse command line arguments.
	cfgSource := parseFlags()

	// Initialize logger configuration.
	err := log.InitConfig("cops").Init("cops")
	if err != nil {
		fmt.Printf("COPS log initialization failed with error: %v\n", err)
		os.Exit(-1)
	}

	// Configure a new FIMS connection.
	f, err = fims.Configure("COPS", "/cops")
	if err != nil {
		log.Fatalf("Error configuring FIMS: %v.", err)
	}

	// Start a FIMS channel that will receive FIMS requests.
	fimsReceive := make(chan fims.FimsMsg)
	go f.ReceiveChannel(fimsReceive)

	// Read in configuration file from source.
	if err := parse(cfgSource); err != nil {
		log.Fatalf("Parsing new config: %v.", err)
	}

	getDBIUpdateModeStatus()

	// Operating loop
	for {
		select {
		case msg := <-fimsReceive:
			processFIMS(msg)
		case <-heartRate.C:
			manageHeartbeats()
		case <-patrolRate.C:
			if err := patrolProcesses(); err != nil {
				log.Errorf("Patrolling processes: %v.", err)
			}
		case <-briefingRate.C:
			go publishBriefing()
		case <-statsUpdateTicker.C:
			go updateResourceUsageData()
		}
	}
}

// Parse command line flags. Defaults to "dbi" as the configuration source if no flag provided.
func parseFlags() (cfgSource string) {
	// cops config file
	cfgUsage := "Give a path to the config file or 'dbi' if config is stored in the database"
	flag.StringVar(&cfgSource, "c", "dbi", cfgUsage)
	flag.StringVar(&cfgSource, "config", "dbi", cfgUsage)
	// logger config file
	flag.StringVar(&log.ConfigFile, "logCfg", "", "If included default values will be overturned. Use this in tandem with the config file to print a specific severity/print to screen.")
	flag.Parse()
	return cfgSource
}

// Returns if an interface is a map with string keys and interface{} elements
func isMapStringInterface(m interface{}) bool {
	var i interface{}
	return reflect.TypeOf(m) == reflect.MapOf(reflect.TypeOf("string"), reflect.TypeOf(&i).Elem())
}

// Parse a string value from a map
func parseStringFromMap(key string, body interface{}) (receivedString string, errorMsg string) {
	extractedVal, errMsg := extractMapStringInterfaceValue(body, key, reflect.TypeOf(receivedString))
	if errMsg != "" {
		return "", errMsg
	}
	return extractedVal.String(), ""
}

// Extracts a specified field from a map[string]interface{}, checks that it is of the expected type, and returns it as a Value
func extractMapStringInterfaceValue(i interface{}, fieldName string, fieldType reflect.Type) (v reflect.Value, errMsg string) {
	if !isMapStringInterface(i) {
		errMsg = "interface{} is not a map[string]interface{}"
		return
	}
	field, ok := i.(map[string]interface{})[fieldName]
	if !ok {
		errMsg = fmt.Sprintf("%v field not found in map[string]interface{}", fieldName)
		return
	}
	rawVal := reflect.ValueOf(field)
	if !rawVal.Type().ConvertibleTo(fieldType) {
		errMsg = fmt.Sprintf("%v field in map[string]interface{} is not of proper type", fieldName)
		return
	}
	return rawVal.Convert(fieldType), ""
}

// Put "defer timeTaken(time.Now(), "funcName")" at the beginning of a long function to measure execution time
//lint:ignore U1000 ignore "unused function" warning as this function is kept to be used by coders during development
func timeTaken(t time.Time, name string) {
	elapsed := time.Since(t)
	log.Infof("TIME: %s took %s\n", name, elapsed)
}

// Helper function to convert a 64 bit uint to a Unix timestamp.
// Unix timestamps are represented in seconds since the Unix epoch -
// dividing the 64 bit integer by 10^6 provides seconds with a remainder of nanoseconds.
func convertMicrosecondsToUnixTime(timeUint64 uint64) time.Time {
	return time.Unix(int64(timeUint64)/1e6, int64(timeUint64)%1e6)
}

// Determine time elapsed from a provided unix Timestamp
// and format it as: #w#d#h#m#s.
func formatUnixDuration(unixTimestamp time.Time) string {
	duration := time.Since(unixTimestamp)
	durationInSeconds := duration.Truncate(time.Second)
	durationString := fmt.Sprintf("%s", durationInSeconds.String())
	return durationString
}
