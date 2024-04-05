package syswatch

import (
	"bufio"
	"fmt"
	"io/ioutil"
	"os"
	"strconv"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
)

type CPUCollector struct {
	// configurable options
	DataMan DataManager `json:"collection"`
	LoadAvg int
	// TODO: Base COPS temperature gathering solely on this point.
	Temp   bool `json:"temperature"`
	Uptime bool

	// internal vars
	zones map[string]string
}

// Global value indicating the load average selected
var cpuLoadAvg int

// === Collector funcs ===

func (cpu *CPUCollector) init() error {
	if !cpu.DataMan.Active {
		return fmt.Errorf("cpu is inactive")
	}
	if cpu.LoadAvg == 0 && !cpu.Temp && !cpu.Uptime {
		return fmt.Errorf("cpu is set to active but provides no stats to track")
	}

	cpu.zones = make(map[string]string)
	cpuLoadAvg = cpu.LoadAvg

	// TODO: Handle contents to take temperatureSource from cops config file.
	// Only handle from cops config if it is provided in config file.
	contents, err := ioutil.ReadDir(dataDir + "/sys/class/thermal/")
	if err != nil {
		return fmt.Errorf("could not read /sys/class/thermal: %v", err)
	}

	for n, item := range contents {
		// TODO: handle when temperature is located elsewhere.
		// e.g. /sys/class/thermal/cooling_device<0-4>
		if strings.Contains(item.Name(), "thermal_zone") { // /sys/class/thermal/...thermal_zone/type
			var name string
			// read in type file
			f, err := os.Open(dataDir + "/sys/class/thermal/" + item.Name() + "/type")
			if err != nil {
				return fmt.Errorf("could not read %s: %v", "/sys/class/thermal/"+item.Name()+"/type", err)
			}
			defer f.Close()

			s := bufio.NewScanner(f)
			if s.Scan() {
				fields := strings.Fields(s.Text())
				if len(fields) != 1 {
					log.Errorf("unit #%v missing type data\n", n)
					continue
				}
				name = fmt.Sprintf(fields[0]+"%v", n)
			} else {
				log.Errorf("unit #%v missing type data\n", n)
				continue
			}

			cpu.zones[item.Name()] = name
		}
	}

	go cpu.DataMan.start(cpu)
	return nil
}

func (cpu *CPUCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "cpu",
	}

	// TODO: consider cpu load averages with 0 load average -
	// unlikely to round to zero.
	if cpu.LoadAvg != 0 {
		data = mergeMaps(data, cpu.getLoadInfo())
	}
	if cpu.Temp {
		data = mergeMaps(data, cpu.getTempInfo())
	}
	if cpu.Uptime {
		data = mergeMaps(data, cpu.getUptimeInfo())
	}

	return data
}

// === CPUCollector funcs ===
// these will all be run as goroutines within the Collector.scrape() func

func (cpu *CPUCollector) getUptimeInfo() map[string]interface{} {
	data := make(map[string]interface{})

	// read in the uptime stats kept by the OS
	f, err := os.Open(dataDir + "/proc/uptime")
	if err != nil {
		log.Errorf("could not read /proc/uptime: %v", err)
		return data
	}
	defer f.Close()

	// scan each line for tracked stats
	s := bufio.NewScanner(f)
	if s.Scan() {
		fields := strings.Fields(s.Text())

		// there should only be uptime and idle time fields
		if len(fields) != 2 {
			log.Errorf("/proc/uptime should have exactly 2 datapoints!")
			return data
		}

		v, err := strconv.ParseFloat(fields[0], 32)
		if err != nil {
			log.Errorf("could not parse float: %v", err)
		} else {
			data["uptimesec"] = (float32)(v)
		}

		v, err = strconv.ParseFloat(fields[1], 32)
		if err != nil {
			log.Errorf("could not parse float: %v", err)
		} else {
			data["idletimesec"] = (float32)(v)
		}
	} else {
		log.Errorf("could not scan /proc/uptime")
	}

	return data
}

func (cpu *CPUCollector) getLoadInfo() map[string]interface{} {
	data := make(map[string]interface{})

	// read in the loadavg stats kept by the OS
	f, err := os.Open(dataDir + "/proc/loadavg")
	if err != nil {
		log.Errorf("could not read /proc/loadavg: %v", err)
		return data
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	if s.Scan() { // scan each line for tracked stats
		fields := strings.Fields(s.Text())

		// there should be 5 fields
		if len(fields) != 5 {
			log.Errorf("/proc/loadavg should have exactly 5 datapoints!")
			return data
		}

		//1:0, 5: 1, 15: 2
		var ind int
		if cpu.LoadAvg >= 0 && cpu.LoadAvg < 5 {
			ind = 0
		} else if cpu.LoadAvg >= 5 && cpu.LoadAvg < 15 {
			ind = 1
		} else {
			ind = 2
		}

		v, err := strconv.ParseFloat(fields[ind], 32)
		if err != nil {
			log.Errorf("could not parse float: %v", err)
			return data
		}

		data[fmt.Sprintf("loadavg_%vm", cpu.LoadAvg)] = (float32)(v)
	} else {
		log.Errorf("could not scan /proc/loadavg")
	}

	return data
}

func (cpu *CPUCollector) getTempInfo() map[string]interface{} {
	data := make(map[string]interface{})

	for zone, name := range cpu.zones {
		// get the temperature of the zone
		temp, err := parseSoloUIntFile(dataDir + "/sys/class/thermal/" + zone + "/temp")
		if err != nil {
			data[fmt.Sprintf("%s_tempC", name)] = "NA"
			log.Errorf("could not get temp for zone %s", name)
			continue
		}
		data[fmt.Sprintf("%s_tempC", name)] = (int)(temp / 1000) // is stored as C*1000
	}

	return data
}
