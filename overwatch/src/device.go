package main

import (
	"fmt"
	"os/exec"
	"strings"

	log "github.com/flexgen-power/go_flexgen/logger"
)

type DeviceCollector struct {
	// configurable
	DataMan DataManager `json:"collection"`

	// non-configurable
	prevRPM string
	prevVer string
}

// === Collector funcs ===

func (device *DeviceCollector) init() error {
	if !device.DataMan.Active {
		return fmt.Errorf("device is inactive")
	}
	// initialize current rpm and version
	rpmInfo := device.getMetaRPM()
	device.prevRPM = fmt.Sprint(rpmInfo["meta_rpm"])
	device.prevVer = fmt.Sprint(rpmInfo["meta_ver"])

	go device.DataMan.start(device)
	return nil
}

func (device *DeviceCollector) scrape() map[string]interface{} {
	data := map[string]interface{}{
		"collector": "device",
	}

	data = mergeMaps(data, device.getMetaRPM())
	data = mergeMaps(data, device.getFIMSVer())

	return data
}

// === DeviceCollector funcs ===
// these will all be run within the Collector.scrape() func

func (device *DeviceCollector) getMetaRPM() map[string]interface{} {
	data := make(map[string]interface{})

	out, err := exec.Command("rpm", "-qa", "*_meta-*").Output() // match installed meta rpm string
	if err != nil {
		log.Errorf("could not get meta rpm: %v", err)
	}

	if len(out) == 0 {
		data["meta_rpm"] = "NONE"
		data["meta_ver"] = "NONE"
	} else {
		info := strings.Split(string(out), "-")
		data["meta_rpm"] = strings.ReplaceAll(info[0], "_meta", "")
		data["meta_ver"] = info[1]
	}

	data["meta_changed"] = 0
	if data["meta_rpm"] != device.prevRPM || data["meta_ver"] != device.prevVer {
		data["meta_changed"] = 1
	}

	device.prevRPM = fmt.Sprint(data["meta_rpm"])
	device.prevVer = fmt.Sprint(data["meta_ver"])

	return data
}

func (device *DeviceCollector) getFIMSVer() map[string]interface{} {
	data := make(map[string]interface{})

	out, err := exec.Command("rpm", "-qa", "fims").Output() // match installed meta rpm string
	if err != nil {
		log.Errorf("could not get fims rpm: %v", err)
	}

	if len(out) == 0 {
		data["fims_ver"] = "NONE"
	} else {
		info := strings.Split(string(out), "-")
		data["fims_ver"] = info[1]
	}

	return data
}
