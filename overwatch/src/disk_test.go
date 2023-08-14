package main

import (
	"bufio"
	"bytes"
	"os/exec"
	"strconv"
	"strings"
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
)

func makeDiskCollector(dmActive bool) DiskCollector {
	defaultDataMan := DataManager{
		Active:   dmActive,
		Interval: 5,
	}

	disk := DiskCollector{
		DataMan:        defaultDataMan,
		Mounts:         true,
		Dirs:           []string{},
		FileCountLimit: 50000,
		MBSizeLimit:    5000,
	}

	return disk
}
func TestDiskInit(t *testing.T) {
	assert := assert.New(t)

	// disk is inactive
	dataDir = "../unit_test_files/disk_testcases/testcase_1"
	disk := makeDiskCollector(false)
	err := disk.init()
	assert.ErrorContains(err, "disk is inactive")

	//disk is set to active but provides no stats to track
	disk.DataMan.Active = true
	disk.Mounts = false
	err = disk.init()
	assert.ErrorContains(err, "disk is set to active but provides no stats to track")

	//cannot open /proc/diskstats
	disk.Mounts = true
	dataDir = "../unit_test_files/disk_testcases/testcase_0"
	err = disk.init()
	assert.ErrorContains(err, "could not read /proc/diskstats:")

	//cannot open /proc/mounts
	disk.Mounts = true
	dataDir = "../unit_test_files/disk_testcases/testcase_3"
	err = disk.init()
	assert.ErrorContains(err, "could not read /proc/mounts:")

	//cannot open /proc/partitions
	disk.Mounts = true
	dataDir = "../unit_test_files/disk_testcases/testcase_4"
	err = disk.init()
	assert.ErrorContains(err, "could not read /proc/partitions:")

	//valid case with no disk.Dirs data
	disk.Mounts = true
	dataDir = "../unit_test_files/disk_testcases/testcase_1"
	err = disk.init()
	assert.Nil(err, "Disk Init should success'")

	expected_mounts := map[string]string{
		"sda1": "/",
		"sda2": "/",
		"sda3": "/",
		"sda4": "",
		"sda5": "/home",
	}
	assert.Equal(len(expected_mounts), len(disk.mounts), "disk.mounts != expected mounts")
	for k, v := range disk.mounts {
		assert.Equalf(expected_mounts[k], v, "Difference of key %v", k)
	}

	expected_sizes := map[string]uint64{
		"sda1": 153600 / 1024,
		"sda2": 256000 / 1024,
		"sda3": 31457280 / 1024,
		"sda4": 4064256 / 1024,
		"sda5": 47952896 / 1024,
	}
	assert.Equal(len(expected_sizes), len(disk.sizes), "disk.sizes != expected size")
	for k, v := range disk.sizes {
		assert.Equalf(expected_sizes[k], v, "Difference of key %v", k)
	}
	assert.Equal(0, len(disk.dirDisabled), "disk.dirDisabled")
	assert.Equal(0, len(disk.dirLast), "disk.dirLast")
	assert.True(disk.firstExec, "disk.firstExec")

	// has customized disk.Dirs
	disk.Dirs = []string{"./", "../unit_test_files", "../unit_test_files/disk_testcases"}
	err = disk.init()
	assert.Nil(err)
	assert.Equal(len(disk.Dirs), len(disk.dirDisabled), "disk.dirDisabled")
	assert.Equal(len(disk.Dirs), len(disk.dirLast), "disk.dirLast")
	for _, k := range disk.Dirs {
		assert.False(disk.dirDisabled[k])
		assert.NotEmpty(disk.dirLast[k])
	}

	disk.Mounts = true
	dataDir = "../unit_test_files/disk_testcases/testcase_6"
	err = disk.init()
	assert.Error(err)

	expected_mounts = map[string]string{
		"sda1": "/",
		"sda2": "/",
		"sda3": "/",
		"sda4": "",
		"sda5": "/home",
	}
	assert.Equal(len(expected_mounts), len(disk.mounts), "disk.mounts != expected mounts")
	for k, v := range disk.mounts {
		assert.Equalf(expected_mounts[k], v, "Difference of key %v", k)
	}

	expected_sizes = map[string]uint64{
		"sda1": 153600 / 1024,
		"sda2": 256000 / 1024,
	}
	assert.Equal(len(expected_sizes), len(disk.sizes), "disk.sizes != expected size")
	for k, v := range disk.sizes {
		assert.Equalf(expected_sizes[k], v, "Difference of key %v", k)
	}
}

// helper function: execute df -m to get current mount info
func genExpectedMntUtil(name string, mnt string, data *map[string]interface{}) {
	var availMB, pctfull uint64
	out, err := exec.Command("df", "-m", mnt).Output()
	if err != nil {
		return
	}
	s := bufio.NewScanner(bytes.NewReader(out))
	for s.Scan() {
		fields := strings.Fields(s.Text())
		if fields[5] == mnt {
			//Available
			availMB, err = strconv.ParseUint(fields[3], 0, 64)
			if err != nil {
				return
			}
			(*data)[name+"_"+safeParseDir(mnt)+"_availMB"] = (int)(availMB)

			//usage%
			pctfull, err = strconv.ParseUint(strings.TrimSuffix(fields[4], "%"), 0, 64)
			if err != nil {
				return
			}
			(*data)[name+"_"+safeParseDir(mnt)+"_pctfull"] = (int)(pctfull)
		}
	}
}

func TestGetMountInfo(t *testing.T) {
	assert := assert.New(t)

	disk := makeDiskCollector(true)

	//cannot open /proc/diskstats
	dataDir = "../unit_test_files/disk_testcases/testcase_0"
	err := disk.init()
	assert.ErrorContains(err, "could not read /proc/diskstats:")
	data := disk.getMountInfo()
	assert.Equal(0, len(data))

	//valid testcase
	dataDir = "../unit_test_files/disk_testcases/testcase_1"
	err = disk.init()
	assert.Nil(err)
	data = disk.getMountInfo()
	expected := map[string]interface{}{
		"sda1_root_rtimesec": 0,
		"sda1_root_wtimesec": 0,
		"sda2_root_rtimesec": 0,
		"sda2_root_wtimesec": 0,
		"sda3_root_rtimesec": 59,
		"sda3_root_wtimesec": 117150,
		"sda5_home_rtimesec": 10,
		"sda5_home_wtimesec": 759700,
	}
	for k, v := range disk.mounts {
		genExpectedMntUtil(k, v, &expected)
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.InDeltaf(expected[k], v, 1.0, "Difference of key %v", k)
	}

	//can not parse wtimesec/rtimesec into uint
	dataDir = "../unit_test_files/disk_testcases/testcase_2"
	data = disk.getMountInfo()
	expected = map[string]interface{}{
		"sda1_root_wtimesec": 0,
		"sda2_root_rtimesec": 0,
		"sda3_root_rtimesec": 59,
		"sda3_root_wtimesec": 117150,
		"sda5_home_rtimesec": 10,
		"sda5_home_wtimesec": 759700,
	}
	for k, v := range disk.mounts {
		genExpectedMntUtil(k, v, &expected)
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.InDeltaf(expected[k], v, 1.0, "Difference of key %v", k)
	}

	//empty mounts
	dataDir = "../unit_test_files/disk_testcases/testcase_3"
	err = disk.init()
	assert.ErrorContains(err, "could not read /proc/mounts:")
	data = disk.getMountInfo()
	assert.Equal(0, len(data))

	//one line in /proc/diskstats has more than 14 fields
	dataDir = "../unit_test_files/disk_testcases/testcase_5"
	err = disk.init()
	assert.Nil(err)
	data = disk.getMountInfo()
	expected = map[string]interface{}{
		"sda1_root_rtimesec": 0,
		"sda1_root_wtimesec": 0,
		"sda3_root_rtimesec": 59,
		"sda3_root_wtimesec": 117150,
		"sda5_home_rtimesec": 10,
		"sda5_home_wtimesec": 759700,
	}
	for k, v := range disk.mounts {
		genExpectedMntUtil(k, v, &expected)
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.InDeltaf(expected[k], v, 1.0, "Difference of key %v", k)
	}

	//Statfs returned error
	dataDir = "../unit_test_files/disk_testcases/testcase_7"
	err = disk.init()
	assert.Nil(err)
	data = disk.getMountInfo()
	expected = map[string]interface{}{
		"sda1_root_rtimesec":     0,
		"sda1_root_wtimesec":     0,
		"sda2_root_rtimesec":     0,
		"sda2_root_wtimesec":     0,
		"sda3_root_rtimesec":     59,
		"sda3_root_wtimesec":     117150,
		"sda5_home2131_rtimesec": 10,
		"sda5_home2131_wtimesec": 759700,
	}
	for k, v := range disk.mounts {
		genExpectedMntUtil(k, v, &expected)
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.InDeltaf(expected[k], v, 1.0, "Difference of key %v", k)
	}
}

func TestGetDirInfo(t *testing.T) {
	assert := assert.New(t)

	disk := makeDiskCollector(true)
	disk.Dirs = []string{"../unit_test_files/disk_testcases"}
	dataDir = "../unit_test_files/disk_testcases/testcase_1"
	err := disk.init()
	assert.Nil(err, "disk.init should success")

	data := disk.getDirInfo()
	expected := map[string]interface{}{
		"._unit_test_files_disk_testcases_changed": -1,
		"._unit_test_files_disk_testcases_files":   8,
		"._unit_test_files_disk_testcases_sizeMB":  1,
	}
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equalf(expected[k], v, "Difference of key %v", k)
	}
	assert.False(disk.firstExec)

	//exceed maximum filecount
	err = disk.init()
	assert.Nil(err, "disk.init should success")
	disk.FileCountLimit = 3
	data = disk.getDirInfo()
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equalf(expected[k], v, "Difference of key %v", k)
	}
	assert.False(disk.firstExec)
	assert.True(disk.dirDisabled["../unit_test_files/disk_testcases"])

	//exceed maximum detectable size
	err = disk.init()
	assert.Nil(err, "disk.init should success")
	disk.FileCountLimit = 100000
	disk.MBSizeLimit = 0
	data = disk.getDirInfo()
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equalf(expected[k], v, "Difference of key %v", k)
	}
	assert.False(disk.firstExec)
	assert.True(disk.dirDisabled["../unit_test_files/disk_testcases"])

	//file not changed, skip checking
	data = disk.getDirInfo()
	assert.Equal(1, len(data))
	assert.Equal(-1, data["._unit_test_files_disk_testcases_changed"])

	//exceed maximum detectable size but still in 30 mins, do not check for 30 mins
	disk.firstExec = true
	data = disk.getDirInfo()
	assert.Equal(1, len(data))
	assert.Equal(-1, data["._unit_test_files_disk_testcases_changed"])

	//exceed maximum detectable size but pass 30 mins, check again
	disk.MBSizeLimit = 10000
	disk.firstExec = true
	disk.dirLast["../unit_test_files/disk_testcases"] = time.Date(2000, time.April, 12, 10, 59, 10, 100, time.UTC)
	data = disk.getDirInfo()
	assert.Equal(len(expected), len(data))
	for k, v := range data {
		assert.Equalf(expected[k], v, "Difference of key %v", k)
	}
	assert.False(disk.dirDisabled["../unit_test_files/disk_testcases"])
}
