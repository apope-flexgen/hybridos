package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
)

type MemCollector struct {
	// configurable options
	DataMan DataManager `json:"collection"`
	Stats   []string

	// internal vars
	lines []int
}

// === Collector funcs ===

func (mem *MemCollector) init() error {
	if !mem.DataMan.Active {
		return fmt.Errorf("mem is inactive")
	}
	if len(mem.Stats) == 0 {
		return fmt.Errorf("mem is set to active but provides no stats to track")
	}

	f, err := os.Open("/proc/meminfo")
	if err != nil {
		return fmt.Errorf("could not read /proc/meminfo: %v", err)
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	line := 0

	for s.Scan() { // scan each line for tracked stats
		for _, str := range mem.Stats {
			if strings.Contains(s.Text(), str+":") {
				mem.lines = append(mem.lines, line)
				break
			}
		}
		line++
	}

	go mem.DataMan.start(mem)
	return nil
}

func (mem *MemCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "mem",
	}

	if len(mem.Stats) > 0 {
		data = mergeMaps(data, mem.getMemInfo())
	}
	return data
}

// === MemCollector funcs ===
// these will all be run as goroutines within the Collector.scrape() func

func (mem *MemCollector) getMemInfo() map[string]interface{} {
	data := make(map[string]interface{})

	// read in the meminfo stats kept by the OS
	f, err := os.Open("/proc/meminfo")
	if err != nil {
		log.Errorf("could not read /proc/meminfo: %v", err)
	}
	defer f.Close()

	s := bufio.NewScanner(f)
	line, i := 0, 0
	for s.Scan() { // scan each line for tracked stats
		if i >= len(mem.lines) {
			break
		}
		if line != mem.lines[i] {
			line++
			continue
		}
		i++
		line++

		// each line has a name and value - ignore units
		fields := strings.Fields(s.Text())
		if len(fields) < 2 {
			log.Errorf("/proc/meminfo line error: %q", s.Text())
		}

		name := fields[0][:len(fields[0])-1]
		v, err := strconv.ParseUint(fields[1], 0, 32)
		if err != nil {
			log.Errorf("could not parse uint: %v", err)
		}

		switch name {
		case "MemAvailable":
			data["availableKB"] = (int)(v)
		case "MemFree":
			data["freeKB"] = (int)(v)
		case "MemTotal":
			data["totalKB"] = (int)(v)
		default:
			data[strings.ToLower(name)+"KB"] = (int)(v)
		}
	}

	return data
}
