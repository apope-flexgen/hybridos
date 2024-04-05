// Implement runtime logic pertaining to overwatch
package syswatch

import (
	"bufio"
	"fims"
	"fmt"
	"os"
	"runtime"
	"runtime/trace"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"

	"github.com/pkg/profile"
)

var (
	// global vars
	config    *Collectors
	summary   = &summaryStats{}
	writeCh   = make(chan map[string]interface{})
	dataDir   string
	coreCount int
)

// Store a few selected summarized hardware statistics to be reported on FIMS only
type summaryStats struct {
	timeOfLastSystemRestart string
	uptime                  string
	cpuUsage                float32
	memUsage                float32
}

// Return an interface to publish on FIMS of the summarized hardware statistics.
func GetHardwareSummary() map[string]interface{} {
	body := make(map[string]interface{})
	body["pct_cpu_usage"] = summary.cpuUsage
	body["pct_mem_usage"] = summary.memUsage
	body["uptime"] = summary.uptime
	body["timestamp_of_last_restart"] = summary.timeOfLastSystemRestart

	return body
}

// Initialize and run system hardware data collection.
// Profiling flags can be one of the following in the array:
// -prof=["cpu", "mem", "trace"]
// Include a processList to specify list of processes to report on.
func Setup(prof string, processList []string) {

	// Profiling argument, only works if flags are in front of config file.
	if prof == "cpu" {
		defer profile.Start(profile.CPUProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "mem" {
		defer profile.Start(profile.MemProfile, profile.ProfilePath(".")).Stop()
	} else if prof == "trace" {
		f, err := os.Create("trace.out")
		if err != nil {
			panic(err)
		}
		defer f.Close()

		err = trace.Start(f)
		if err != nil {
			panic(err)
		}
		defer trace.Stop()
	}

	// Setup default configuration for collectors.
	err := configureDefaults(processList)
	if err != nil {
		log.Fatalf("Config error: %v", err)
	}

	// Retrieve the core count of the system
	coreCount = runtime.NumCPU()
}

// Start all collection on all system level metrics.
func StartCollectors() {
	collectors := map[string]Collector{
		"mem":     &config.Mem,
		"cpu":     &config.CPU,
		"process": &config.Process,
	}

	for name, collector := range collectors {
		err := collector.init()
		if err != nil {
			log.Errorf("%s did not initialize: %v", name, err)
			continue
		}
		log.Infof("started %s", name)
		time.Sleep(time.Second)
	}

	log.Infof("initial collector startup complete.")
}

// Publish system data to FIMS. Configure function be called based upon interval
// in main operational loop for COPS.
func PublishSystemStats(f fims.Fims) {

	for data := range writeCh {
		val, exists := data["collector"]
		if !exists {
			log.Debugf("unknown collection source for data: %v\nmoving on...", data)
			continue
		}
		collector := fmt.Sprintf("%v", val)
		delete(data, "collector")

		// Update the summarized info with a given collector.
		switch collector {
		case "cpu":
			summary.updateCPU(data)
		case "mem":
			summary.updateMem(data)
		case "process":
			// Internally update global process data from the write channel.
			procInfos.update(data)
		default:
			// Ignore the other collector types for now.
		}

		// Publish only summarized data. Don't publish other collectors.
		msg := fims.FimsMsg{
			Method:  "pub",
			Uri:     "/cops/stats/system/summary",
			Replyto: "",
			Body:    GetHardwareSummary(),
		}
		_, err := f.Send(msg)
		if err != nil {
			log.Errorf("Error publishing hardware statistics: %v", err)
			continue
		} else {
			log.Tracef("Successfully sent!")
		}
	}
}

// Take cpu collector data and update hardware summary statistics.
func (s *summaryStats) updateCPU(data map[string]interface{}) {
	var cpuload float32
	var uptime float32
	var ok bool

	// Retrieve and set the uptime in seconds.
	if val, found := data["uptimesec"]; found {
		if _, ok = val.(float32); ok {
			uptime = data["uptimesec"].(float32)

			// Update timestamp since last system restart.
			systemStart := time.Now().Add(-time.Duration(uptime) * time.Second)
			s.timeOfLastSystemRestart = systemStart.Format("01-02-2006 15:04:05")
			s.uptime = FormatUnixDuration(systemStart)
		}
	} else {
		// Don't set nil values to summary. Just provide a warn statement.
		log.Warnf("uptimesec not found in cpu collection.")
	}

	// Retrieve cpu average load and calculate % in use.
	key := fmt.Sprintf("loadavg_%vm", cpuLoadAvg)
	if val, found := data[key]; found {
		if _, ok = val.(float32); ok {
			cpuload = data[key].(float32)

			// Guard against invalid core count.
			if coreCount > 0 {
				s.cpuUsage = -1
				s.cpuUsage = cpuload / float32(coreCount) * 100
			} else {
				s.cpuUsage = -1
			}
		}
	} else {
		// Return without setting nil values.
		log.Warnf("cpu load average not found in cpu collection.")
	}
}

// Take mem collector data and update hardware summary statistics.
func (s *summaryStats) updateMem(data map[string]interface{}) {
	var activeKB float32
	var totalKB float32
	var ok bool

	// Retrieve available mem statistic.
	if val, found := data["activeKB"]; found {
		if _, ok = val.(int); ok {
			activeKB = float32(data["activeKB"].(int))
		}
	} else {
		// Don't calculate mem % with nil data, return instead.
		log.Warnf("activeKB not found in memory collection.")
		return
	}

	// Retrieve total mem statistic.
	if val, found := data["totalKB"]; found {
		if _, ok = val.(int); ok {
			totalKB = float32(data["totalKB"].(int))
		}
	} else {
		// Don't calculate mem % with nil data, return instead
		log.Warnf("totalKB not found in memory collection.")
		return
	}

	// Guard in the event total system memory returned 0.
	if totalKB == 0 {
		log.Errorf("Total system memory returned 0. Unable to calculate memory usage.")
		return
	}

	// Calculate total % memory usage.
	s.memUsage = activeKB / totalKB * 100
}

// === HELPER FUNCS ===

func parseSoloUIntFile(filename string) (uint64, error) {
	f, err := os.Open(filename)
	if err != nil {
		return 0, err
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	if s.Scan() {
		data, err := strconv.ParseUint(s.Text(), 0, 32)
		if err != nil {
			return 0, err
		} else {
			return data, nil
		}
	}
	return 0, fmt.Errorf("no data in file: " + filename)
}

func safeParseDir(dir string) string {
	switch dir {
	case "/":
		return "root"
	default:
		return strings.ReplaceAll(dir[1:], "/", "_")
	}
}

func mergeMaps(maps ...map[string]interface{}) map[string]interface{} {
	dest := maps[0]
	maps = maps[1:]
	for _, m := range maps {
		for key, val := range m {
			dest[key] = val
		}
	}

	return dest
}

func deepCopyMap(original map[string]interface{}) map[string]interface{} {
	deepCopy := make(map[string]interface{}, len(original))

	for val, key := range original {
		deepCopy[val] = key
	}

	return deepCopy
}

// Determine time elapsed from a provided unix Timestamp
// and format it as: #w#d#h#m#s.
func FormatUnixDuration(unixTimestamp time.Time) string {
	duration := time.Since(unixTimestamp).Truncate(time.Second)
	return duration.String()
}
