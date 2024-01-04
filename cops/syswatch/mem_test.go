package syswatch

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func MakeMemCollector(dmActive bool, fields ...string) MemCollector {
	defaultDataMan := DataManager{
		Active:   dmActive,
		Interval: 5,
	}

	mem := MemCollector{
		DataMan: defaultDataMan,
		Stats:   fields,
	}

	return mem
}

func TestMemInit(t *testing.T) {
	//mem is inactive
	assert := assert.New(t)
	mem := MakeMemCollector(false)
	err := mem.init()
	assert.Equal("mem is inactive", err.Error())

	//mem is set to active but provides no stats to track
	mem.DataMan.Active = true
	err = mem.init()
	assert.ErrorContains(err, "mem is set to active but provides no stats to track")

	//normal case, test if lines are correct
	mem1 := MakeMemCollector(true, "MemAvailable", "MemTotal", "MemFree", "Active", "Dirty")
	dataDir = "../unit_test_files/mem_testcases/testcase_1"
	err = mem1.init()
	expected := []int{2, 0, 1, 6, 16}
	assert.Nil(err)
	assert.ElementsMatch(expected, mem1.lines, "Two elements do not match")

	//cannot read /proc/meminfo
	dataDir = "../unit_test_files/mem_testcases/testcase_0"
	err = mem1.init()
	assert.ErrorContains(err, "could not read /proc/meminfo")

}

func TestGetMemInfo(t *testing.T) {
	assert := assert.New(t)

	//could not read /proc/meminfo
	mem := MakeMemCollector(true, "MemAvailable", "MemTotal", "MemFree", "Active", "Dirty")
	dataDir = "../unit_test_files/mem_testcases/testcase_0"
	data := mem.getMemInfo()
	assert.Empty(data)

	//default fields
	testcase := "../unit_test_files/mem_testcases/testcase_1"
	expected := map[string]interface{}{
		"totalKB":     3879624,
		"freeKB":      307552,
		"availableKB": 2632236,
		"activeKB":    1642368,
		"dirtyKB":     180,
	}
	compareDataOfGetMem(t, expected, testcase, "MemAvailable", "MemTotal", "MemFree", "Active", "Dirty")

	//customized fields
	expected = map[string]interface{}{
		"unevictableKB":  0,
		"directmap2mKB":  1992704,
		"bounceKB":       0,
		"writebacktmpKB": 0,
		"slabKB":         260240,
	}
	compareDataOfGetMem(t, expected, testcase, "Unevictable", "DirectMap2M", "Bounce", "WritebackTmp", "Slab")

	//error handling: Hugepagesize cannot be parse into uint
	expected = map[string]interface{}{
		"unevictableKB":  0,
		"directmap2mKB":  1992704,
		"bounceKB":       0,
		"writebacktmpKB": 0,
		"slabKB":         260240,
	}
	compareDataOfGetMem(t, expected, testcase, "Unevictable", "DirectMap2M", "Bounce", "WritebackTmp", "Slab", "Hugepagesize")

	// error handling: only log error, will cause program failed later call the second field
	expected = map[string]interface{}{
		"unevictableKB": 0,
		"directmap2mKB": 1992704,
		"bounceKB":      0,
	}
	compareDataOfGetMem(t, expected, testcase, "Unevictable", "DirectMap2M", "Bounce", "DirectMap1G")

}

// helper function to run testcase and cross validate the result
func compareDataOfGetMem(t *testing.T, expected map[string]interface{}, tcFile string, fields ...string) {
	assert := assert.New(t)
	mem := MakeMemCollector(true, fields...)
	dataDir = tcFile

	err := mem.init()
	assert.Nil(err)

	data := mem.getMemInfo()
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}
}

func TestMemScrape(t *testing.T) {
	assert := assert.New(t)
	mem := MakeMemCollector(true, "MemAvailable", "MemTotal", "MemFree", "Active", "Dirty")
	dataDir = "../unit_test_files/mem_testcases/testcase_1"
	err := mem.init()
	assert.Nil(err)
	data := mem.scrape()
	expected := map[string]interface{}{
		"collector":   "mem",
		"totalKB":     3879624,
		"freeKB":      307552,
		"availableKB": 2632236,
		"activeKB":    1642368,
		"dirtyKB":     180,
	}
	assert.Equal(len(data), len(expected))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

	// no status to track
	mem = MakeMemCollector(true)
	dataDir = "../unit_test_files/mem_testcases/testcase_1"
	err = mem.init()
	assert.Error(err)
	data = mem.scrape()
	expected = map[string]interface{}{
		"collector": "mem",
	}
	assert.Equal(len(data), len(expected))
	for k, v := range data {
		assert.Equal(expected[k], v)
	}

}
