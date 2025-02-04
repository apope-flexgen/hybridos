/**
 * COPS Health Statistics
 *
 * Created February 2021
 *
 * This file contains various functions for allowing COPS
 * to record and report various statistics about the
 * health of the processes it is monitoring.
 *
 */

package main

import (
	"fmt"
	"os"
	"os/exec"
	"regexp"
	"strconv"
	"strings"
	"sync"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"
	sys "github.com/flexgen-power/hybridos/cops/syswatch"
)

type processHealthStats struct {
	lastMemUsagePercent       float64
	maxMemUsagePercent        float64
	avgMemUsagePercent        float64
	sumRecentMemUsagePercents float64
	recentMemUsagePercents    []float64
	avgCPUUsagePercent        float64
	totalRestarts             int
	copsRestarts              int
	timestampOfLastRestart    string
	elapsedTimeSinceRestart   string
	lastConfirmedAlive        time.Time
	lastResponseTime          time.Duration
	maxResponseTime           time.Duration
	avgResponseTimeMS         float32
	sumRecentResponseTimesMS  int
	recentResponseTimes       []time.Duration
	recentRestarts            []time.Time
}

var tempSource string // Source of controller temperature reading

// Builds a map of all the processes and their health statistics
func buildHealthStatsMap() (statsMap map[string]interface{}) {
	statsMap = make(map[string]interface{})
	statsMap["COPSTimeOfStart"] = beginningTime
	statsMap["procStats"] = buildListOfProcsWithHealthStats()
	return
}

// Create list of processes' health statistics
func buildListOfProcsWithHealthStats() (listOfProcsWithHealthStats []map[string]interface{}) {
	for _, process := range processJurisdiction {
		procStats := make(map[string]interface{})
		procStats[process.name] = process.buildStatsReport()
		listOfProcsWithHealthStats = append(listOfProcsWithHealthStats, procStats)
	}
	return
}

// Create object with a process's health statistics info
func (process processInfo) buildStatsReport() map[string]interface{} {
	healthParams := make(map[string]interface{})
	var mu sync.Mutex // make this an atomic operation so it does not overlap with updating the stats themselves
	mu.Lock()
	defer mu.Unlock()
	healthParams["alive"] = process.alive
	healthParams["service_status"] = process.serviceStatus
	healthParams["unit_file_state"] = process.unitFileState
	// the time format should be consistent with modbus_client (strftime "%m-%d-%Y %T.", i.e. 01-25-2023 11:16:12.396265)
	healthParams["last_restart"] = process.healthStats.timestampOfLastRestart
	healthParams["elapsed_time"] = process.healthStats.elapsedTimeSinceRestart
	healthParams["total_restarts"] = process.healthStats.totalRestarts
	healthParams["cops_restarts"] = process.healthStats.copsRestarts
	healthParams["last_response_time_ms"] = process.healthStats.lastResponseTime.Milliseconds()
	healthParams["max_response_time_ms"] = process.healthStats.maxResponseTime.Milliseconds()
	healthParams["avg_response_time_ms"] = process.healthStats.avgResponseTimeMS
	healthParams["avg_cpu_usage_pct"] = process.healthStats.avgCPUUsagePercent
	healthParams["last_mem_usage_pct"] = process.healthStats.lastMemUsagePercent
	healthParams["max_mem_usage_pct"] = process.healthStats.maxMemUsagePercent
	healthParams["avg_mem_usage_pct"] = process.healthStats.avgMemUsagePercent
	healthParams["controls"] = process.generateControlsMap()
	healthParams["dependencies"] = process.dependencies
	// Only report connection status for enabled processes
	if process.enableConnectionStatus {
		healthParams["is_connected"] = process.connected
	}
	return healthParams
}

// Generates a controls map that reports enabled/disabled statuses for each process.
func (process *processInfo) generateControlsMap() map[string]interface{} {

	// Build controls map.
	controls := map[string]interface{}{
		"start": map[string]interface{}{
			"value":   false,
			"enabled": process.enableControls.startEnabled,
		},
		"stop": map[string]interface{}{
			"value":   false,
			"enabled": process.enableControls.stopEnabled,
		},
		"restart": map[string]interface{}{
			"value":   false,
			"enabled": process.enableControls.restartEnabled,
		},
	}

	return controls
}

// Obtain process resource information based on process name.
// Returns average cpu and mem usage statistics utilizing overwatch business logic
func getProcessStats(processName string) (cpu, mem float64, err error) {
	// Get process stats for CPU and Mem % usage.
	data, err := sys.GetProcessCollectorData()
	if err != nil {
		return 0, 0, fmt.Errorf("get %s collector data: %v", processName, err)
	}

	// Extract cpu and memory from the process collection data.
	cpu, mem, err = sys.GetCPUandMemStats(data, processName)
	if err != nil {
		return 0, 0, fmt.Errorf("get %s cpu and mem stats: %v", processName, err)
	}

	// Validate bounds on average % usage statistics.
	if cpu < 0.0 {
		cpu = 0.0
	}
	if mem < 0.0 {
		mem = 0.0
	}
	return cpu, mem, nil
}

// Splits the output of the Linux command "ps" into separate lines and deletes duplicate whitespace between words
func cleanRawLinuxPSOutput(rawOutput []byte) (dataLines []string) {
	dataLines = strings.Split(strings.Trim(string(rawOutput), "\n"), "\n")[1:]
	whitespace := regexp.MustCompile(`\s+`)
	for i, line := range dataLines {
		dataLines[i] = strings.Trim(whitespace.ReplaceAllString(line, " "), " ")
	}
	return
}

// Creates a list of PIDs of the processes in COPS jurisdiction
func createListOfPIDs() (listOfPIDs []int) {
	for _, process := range processJurisdiction {
		if process.isStillAlive() {
			listOfPIDs = append(listOfPIDs, process.pid)
		}
	}
	return
}

// Runs the Linux command "ps" with correct options then cleans the output for further processing
func fetchSystemResourceData(processIDs []int) []string {
	rawOutput := runLinuxPS(processIDs)
	return cleanRawLinuxPSOutput(rawOutput)
}

// Uses OS-level commands to get resource usage data about each process in the input PID list
func getResourceUsage(processQueries []int) (dataMap map[int]interface{}) {
	dataMap = make(map[int]interface{})
	// if there are no alive processes to check, return empty map
	if len(processQueries) == 0 {
		return
	}
	systemResourceDataLines := fetchSystemResourceData(processQueries)
	for _, line := range systemResourceDataLines {
		dataEntry := make(map[string]float64)
		pid, cpuUsage, memUsage := parseProcessData(line)
		//TODO: deprecate old method of retrieving CPU
		dataEntry["CPU"] = cpuUsage
		dataEntry["MEM"] = memUsage
		dataMap[pid] = dataEntry
	}
	return
}

// Given a processed line of output data from the Linux "ps" command, parses out the PID, CPU Usage %, and Memory Usage %
func parseProcessData(line string) (pid int, cpuUsage float64, memUsage float64) {
	var err error
	splitData := strings.Split(line, " ")
	pid, err = strconv.Atoi(splitData[0])
	fatalErrorCheck(err, "Failed to parse PID")
	cpuUsage, err = strconv.ParseFloat(splitData[1], 64)
	fatalErrorCheck(err, "Failed to parse CPU Usage resource data")
	memUsage, err = strconv.ParseFloat(splitData[2], 64)
	fatalErrorCheck(err, "Failed to parse Mem Usage resource data")
	return
}

// When a process responds to COPS, update its related health statistics
func (process *processInfo) recordResponse() {
	var mu sync.Mutex
	mu.Lock()
	defer mu.Unlock()
	process.healthStats.lastResponseTime = time.Since(process.healthStats.lastConfirmedAlive)
	if process.healthStats.lastResponseTime > process.healthStats.maxResponseTime {
		process.healthStats.maxResponseTime = process.healthStats.lastResponseTime
	}
	process.healthStats.lastConfirmedAlive = time.Now()
	updateAvgResponseTime(process)
}

// When a process restarts, update its related health statistics
func (process *processInfo) recordRestart() {
	var mu sync.Mutex
	mu.Lock() // make this an atomic operation so it does not overlap with building the stats report
	defer mu.Unlock()
	process.healthStats.recentRestarts = append(process.healthStats.recentRestarts, time.Now())
	process.healthStats.totalRestarts++
}

// TODO: Deprecate with DC-268 once merged
// Runs the Linux command "ps" with the correct options to get CPU Usage % and Memory Usage % for processes in input PID list
func runLinuxPS(processIDs []int) (rawOutput []byte) {
	pidString := stringifyPIDs(processIDs)
	ps := exec.Command("ps", "-o", "pid,%cpu,%mem", "p", pidString)
	rawOutput, err := ps.CombinedOutput()
	fatalErrorCheck(err, "ps command failed")
	return
}

// Converts a slice of int PIDs to a string of space separated PIDs for input to Linux command option
func stringifyPIDs(processIDs []int) string {
	return strings.Trim(fmt.Sprint(processIDs), "[]")
}

// Uses the last recorded response time to update the average response time statistic
func updateAvgResponseTime(process *processInfo) {
	if len(process.healthStats.recentResponseTimes) >= 10 {
		process.healthStats.sumRecentResponseTimesMS -= int(process.healthStats.recentResponseTimes[0].Milliseconds())
		process.healthStats.recentResponseTimes = process.healthStats.recentResponseTimes[1:]
	}
	process.healthStats.sumRecentResponseTimesMS += int(process.healthStats.lastResponseTime.Milliseconds())
	process.healthStats.recentResponseTimes = append(process.healthStats.recentResponseTimes, process.healthStats.lastResponseTime)
	process.healthStats.avgResponseTimeMS = float32(process.healthStats.sumRecentResponseTimesMS) / float32(len(process.healthStats.recentResponseTimes))
}

// Gets resource usage data from system and uses it to update relevant process health statistics
func updateResourceUsageData() {
	// Prevent a failure here from crashing all of COPS
	defer func() {
		if r := recover(); r != nil {
			log.Errorf("Error with health statistics update: %v", r)
		}
	}()

	// Obtain cpu and mem per process using overwatch's backend
	for _, process := range processJurisdiction {
		cpu, mem, err := getProcessStats(process.name)
		if err != nil {
			// Ignore errors if the stats are currently unavailable for a given poll. Maintain previous saved stats on a process
			//log.Errorf("getting %s cpu and mem stats: %v", process.name, err)
			continue
		}

		process.healthStats.avgCPUUsagePercent = cpu
		process.healthStats.avgMemUsagePercent = mem
	}

	// Execute update
	listofPIDs := createListOfPIDs()
	// if no processes are alive, do not attempt to get resource data with empty list of PIDs
	if len(listofPIDs) == 0 {
		return
	}
	resourceUsageData := getResourceUsage(listofPIDs)
	var mu sync.Mutex // make this an atomic operation so it does not overlap with building the stats report
	mu.Lock()
	defer mu.Unlock()
	for _, process := range processJurisdiction {
		// resource usage data object will not have data for dead processes, so do not try to access those map keys
		if _, ok := resourceUsageData[process.pid]; ok {
			process.healthStats.lastMemUsagePercent = resourceUsageData[process.pid].(map[string]float64)["MEM"]
			if process.healthStats.lastMemUsagePercent > process.healthStats.maxMemUsagePercent {
				process.healthStats.maxMemUsagePercent = process.healthStats.lastMemUsagePercent
			}
		}
	}
}

// Reads system temperature
func readSystemTemp() (float32, error) {
	tempByte, err := os.ReadFile(tempSource)
	if err != nil {
		return 0.0, fmt.Errorf("failed to read temperature from %s: %w", tempSource, err)
	}
	// Parse as float to ensure the correct numeric response
	tempVal, err := strconv.ParseFloat(strings.TrimSpace(string(tempByte)), 32)
	if err != nil {
		return 0.0, fmt.Errorf("failed to extract temperature measurement from %s: %w", tempSource, err)
	}
	// Temperature is given in millidegrees C
	return float32(tempVal) / 1000, nil
}
