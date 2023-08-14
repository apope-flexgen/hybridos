package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func makeDeviceCollector(dmActive bool) DeviceCollector {
	defaultDataMan := DataManager{
		Active:   dmActive,
		Interval: 1 * 60 * 60 * 24,
	}

	device := DeviceCollector{
		DataMan: defaultDataMan,
	}

	return device
}

func TestDeviceInit(t *testing.T) {
	assert := assert.New(t)

	//Device is inactive
	device := makeDeviceCollector(false)
	err := device.init()
	assert.ErrorContains(err, "device is inactive")

	//valid case
	device.DataMan.Active = true
	err = device.init()
	assert.Nil(err, "Device should init successfully")
	assert.NotEmptyf(device.prevRPM, "device.prevRPM should not be empty after init")
	assert.NotEmptyf(device.prevVer, "device.prevVer should not be empty after init")
}

func TestGetMetaRPM(t *testing.T) {
	assert := assert.New(t)

	device := makeDeviceCollector(true)
	data := device.getMetaRPM()
	assert.Equal(3, len(data))

	//meta_ver should have format like 1.1.*
	assert.Regexp(`^\d+\.\d+\.[\w|\.]+$|NONE$`, data["meta_ver"])
	assert.True(data["meta_changed"] == 1)
	assert.Equal(data["meta_rpm"], device.prevRPM)
	assert.Equal(data["meta_ver"], device.prevVer)
}

func TestGetFIMSVer(t *testing.T) {
	assert := assert.New(t)

	device := makeDeviceCollector(true)
	data := device.getFIMSVer()
	assert.Equal(1, len(data))
	assert.Regexp(`^\d+\.\d+\.[\w|\.]+$|NONE$`, data["fims_ver"]) //data["fims_ver"] should have format like 1.1.*
}

func TestDeviceScrape(t *testing.T) {
	assert := assert.New(t)
	device := makeDeviceCollector(true)
	data := device.scrape()

	assert.Equalf(5, len(data), "Scrape results should have 5 fields, but have %d.", len(data))
	assert.Equal("device", data["collector"])
}
