// Implement runtime logic pertaining to overwatch
package syswatch

import (
	"bufio"
	"fims"
	"fmt"
	"os"
	"runtime/trace"
	"strconv"
	"strings"
	"time"

	log "github.com/flexgen-power/go_flexgen/logger"

	"github.com/pkg/profile"
)

var (
	// global vars
	config  *Collectors
	writeCh = make(chan map[string]interface{})
	dataDir string
)

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
}

// Start all collection on all system level metrics.
func StartCollectors() {
	collectors := map[string]Collector{
		"mem":     &config.Mem,
		"cpu":     &config.CPU,
		"disk":    &config.Disk,
		"net":     &config.Net,
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
			log.Errorf("unknown collection source for data: %v\nmoving on...", data)
			continue
		}
		collector := fmt.Sprintf("%v", val)
		delete(data, "collector")

		// Don't publish process stats.
		if collector == "process" {
			continue
		}

		// Publish data
		msg := fims.FimsMsg{
			Method:  "pub",
			Uri:     "/cops/stats/system/" + collector,
			Replyto: "",
			Body:    data,
		}
		_, err := f.Send(msg)
		if err != nil {
			log.Errorf(err.Error())
			continue
		} else {
			log.Tracef("Successfully sent!")
		}
	}
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
