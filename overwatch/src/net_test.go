package main

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func makeNetCollector(dmActive bool) NetCollector {
	defaultDataMan := DataManager{
		Active:   dmActive,
		Interval: 5,
	}

	nett := NetCollector{
		DataMan:    defaultDataMan,
		Fims:       true,
		Interfaces: true,
		IPs:        map[string]string{},
		Stats:      []string{},
	}

	return nett
}

func TestNetInit(t *testing.T) {
	assert := assert.New(t)

	//dataman inactive
	nett := makeNetCollector(false)
	err := nett.init()
	assert.ErrorContains(err, "net is inactive")

	//no status to track
	nett.DataMan.Active = true
	nett.Fims = false
	nett.Interfaces = false
	err = nett.init()
	assert.ErrorContains(err, "net is set to active but provides no stats to track")

	//could not read /sys/class/net dir
	dataDir = "../unit_test_files/net_testcases/testcase_0"
	nett.Fims = true
	nett.Interfaces = true
	err = nett.init()
	assert.ErrorContains(err, "could not read /sys/class/net:")

	//valid case
	dataDir = "../unit_test_files/net_testcases/port/testcase_1"
	nett.Fims = true
	nett.Interfaces = true
	err = nett.init()
	assert.Nil(err)

	assert.ElementsMatch([]string{"eno", "eth0", "eth1"}, nett.interfaces)
	assert.NotNil(nett.buffer.Bytes())
}

func TestGetSockInfo(t *testing.T) {
	assert := assert.New(t)

	//valid case
	nett := makeNetCollector(true)
	dataDir = "../unit_test_files/net_testcases/sock/testcase_1"
	nett.Stats = []string{"wmem_max", "rmem_max"}
	data := nett.getSockInfo()
	expected := map[string]interface{}{
		"fims_connections": 3,
		"fims_status":      1,
		"wmem_max":         212992,
		"rmem_max":         212992,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

	//cannot not read /proc/net/unix and stats
	dataDir = "../unit_test_files/net_testcases/sock/testcase_0"
	data = nett.getSockInfo()
	assert.Equal(0, len(data))

	//cannot not read /proc/net/unix but has stats
	dataDir = "../unit_test_files/net_testcases/sock/testcase_2"
	data = nett.getSockInfo()
	expected = map[string]interface{}{
		"wmem_max": 212992,
		"rmem_max": 212992,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

	//problem parsing uint
	dataDir = "../unit_test_files/net_testcases/sock/testcase_3"
	data = nett.getSockInfo()
	expected = map[string]interface{}{
		"fims_connections": 3,
		"fims_status":      1,
		"wmem_max":         212992,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

}

func TestGetPortInfo(t *testing.T) {
	assert := assert.New(t)

	nett := makeNetCollector(true)
	//valid case
	dataDir = "../unit_test_files/net_testcases/port/testcase_1"
	err := nett.init()
	assert.Nil(err)
	data := nett.getPortInfo()
	expected := map[string]interface{}{
		"eno_status":  -1,
		"eth0_status": 1,
		"eth1_status": 1,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

	//missing one operstate
	dataDir = "../unit_test_files/net_testcases/port/testcase_2"
	assert.Nil(err)
	data = nett.getPortInfo()
	expected = map[string]interface{}{
		"eth0_status": 1,
		"eth1_status": 1,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

	//switch case default condition, status not up or down
	dataDir = "../unit_test_files/net_testcases/port/testcase_3"
	assert.Nil(err)
	data = nett.getPortInfo()
	expected = map[string]interface{}{
		"eno_status":  404,
		"eth0_status": 1,
		"eth1_status": 1,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

}
