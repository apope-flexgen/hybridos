package main

import (
	"testing"
)

func MakeCpuCollector(dmActive bool, loadavg int, temp bool, uptime bool) CPUCollector {
	defaultDataMan := DataManager{
		Active:   dmActive,
		Interval: 5,
	}

	cpu := CPUCollector{
		DataMan: defaultDataMan,
		LoadAvg: loadavg,
		Temp:    temp,
		Uptime:  uptime,
	}

	return cpu
}

func TestCPUInit(t *testing.T) {
	//success case, check if the cpu.zone has expected data
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_1"
	cpu := MakeCpuCollector(true, 5, true, true)
	err := cpu.init()
	if err != nil {
		t.Errorf("Expected return nil, but got %v \n", err.Error())
	}
	expected_zone := map[string]string{
		"thermal_zone0": "a0",
		"thermal_zone1": "b1",
		"thermal_zone2": "c2",
		"thermal_zone3": "d3",
	}

	if len(cpu.zones) != len(expected_zone) {
		t.Errorf("Expected zone of size %d, but got %d \n", len(expected_zone), len(cpu.zones))
	}
	for zone, name := range cpu.zones {
		if name != expected_zone[zone] {
			t.Errorf("Expected zone name %v, but got %v \n", expected_zone[zone], name)
		}
	}

	//cpu is set to active but provides no stats to track
	cpu.LoadAvg = 0
	cpu.Temp = false
	cpu.Uptime = false
	err = cpu.init()
	expected := "cpu is set to active but provides no stats to track"
	if err.Error() != expected {
		t.Errorf("Expected error of %v, but got %v \n", expected, err.Error())
	}

	//cpu is inactive
	cpu.DataMan.Active = false
	err = cpu.init()
	expected = "cpu is inactive"
	if err.Error() != expected {
		t.Errorf("Expected error of %v, but got %v \n", expected, err.Error())
	}

	//cannot open folder thermal
	cpu = MakeCpuCollector(true, 5, true, true)
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_0"
	err = cpu.init()
	if err == nil {
		t.Errorf("Expected error of %v, but got nil \n", "could not read /sys/class/thermal")
	}

	//cannot open folder thermal/thermal_zoneX/type
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_2"
	err = cpu.init()
	if err == nil {
		t.Errorf("Expected error of %v, but got nil \n", "could not read /sys/class/thermal/thermal_zoneX/type")
	}

	// /sys/class/thermal/thermal_zone0/type is empty
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_3"
	err = cpu.init()
	if err != nil {
		t.Errorf("Expected error nil, but got %v \n", err.Error())
	}
	expected_zone = map[string]string{
		"thermal_zone1": "b1",
		"thermal_zone2": "c2",
		"thermal_zone3": "d3",
	}
	if len(cpu.zones) != len(expected_zone) {
		t.Errorf("Expected zone of size %d, but got %d \n", len(expected_zone), len(cpu.zones))
	}
	for zone, name := range cpu.zones {
		if name != expected_zone[zone] {
			t.Errorf("Expected zone name %v, but got %v \n", expected_zone[zone], name)
		}
	}

	// /sys/class/thermal/thermal_zone0/type has more than 1 fields
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_4"
	err = cpu.init()
	if err != nil {
		t.Errorf("Expected error nil, but got %v \n", err.Error())
	}
	expected_zone = map[string]string{
		"thermal_zone1": "b1",
		"thermal_zone2": "c2",
		"thermal_zone3": "d3",
	}
	if len(cpu.zones) != len(expected_zone) {
		t.Errorf("Expected zone of size %d, but got %d \n", len(expected_zone), len(cpu.zones))
	}
	for zone, name := range cpu.zones {
		if name != expected_zone[zone] {
			t.Errorf("Expected zone name %v, but got %v \n", expected_zone[zone], name)
		}
	}

}

func TestGetUpTimeInfo(t *testing.T) {
	cpu := MakeCpuCollector(true, 5, true, true)
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_1"
	data := cpu.getUptimeInfo()
	expected := map[string]float32{
		"uptimesec":   1400.42,
		"idletimesec": 3000.34,
	}
	if len(data) != len(expected) {
		t.Errorf("Expected data of size %d, but got %d \n", len(expected), len(data))
	}
	for k, v := range data {
		if v != expected[k] {
			t.Errorf("Expected data %f, but got %f \n", expected[k], v)
		}
	}

	// uptime has more than 2 fields
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_2"
	data = cpu.getUptimeInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

	// uptime first field cannot be parsed into a float
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_3"
	data = cpu.getUptimeInfo()
	if len(data) != 1 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}
	if data["idletimesec"] != expected["idletimesec"] {
		t.Errorf("Expected data %f, but got %f \n", expected["idletimesec"], data["idletimesec"])
	}

	// uptime second field cannot be parsed into a float
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_4"
	data = cpu.getUptimeInfo()
	if len(data) != 1 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}
	if data["uptimesec"] != expected["uptimesec"] {
		t.Errorf("Expected data %f, but got %f \n", expected["uptimesec"], data["uptimesec"])
	}

	//could not read /proc/uptime:
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_0"
	data = cpu.getUptimeInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

	//could not scan /proc/uptime:
	dataDir = "../unit_test_files/cpu_testcases/uptime/testcase_5"
	data = cpu.getUptimeInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

}

func TestGetLoadInfo(t *testing.T) {
	//1 min
	cpu := MakeCpuCollector(true, 1, true, true)
	dataDir = "../unit_test_files/cpu_testcases/loadavg/testcase_1"
	data := cpu.getLoadInfo()
	var expected float32 = 0.61
	if data["loadavg_1m"] != expected {
		t.Errorf("Expected data %f, but got %f \n", expected, data["loadavg_5m"])
	}

	//5 min
	cpu.LoadAvg = 5
	data = cpu.getLoadInfo()
	expected = 0.64
	if data["loadavg_5m"] != expected {
		t.Errorf("Expected data %f, but got %f \n", expected, data["loadavg_10m"])
	}

	//15 min
	cpu.LoadAvg = 15
	data = cpu.getLoadInfo()
	expected = 0.55
	if data["loadavg_15m"] != expected {
		t.Errorf("Expected data %f, but got %f \n", expected, data["loadavg_15m"])
	}

	//20 min
	cpu.LoadAvg = 20
	data = cpu.getLoadInfo()
	expected = 0.55
	if data["loadavg_20m"] != expected {
		t.Errorf("Expected data %f, but got %f \n", expected, data["loadavg_15m"])
	}

	//could not read /proc/loadavg
	dataDir = "../unit_test_files/cpu_testcases/loadavg/testcase_0"
	data = cpu.getLoadInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

	// /proc/loadavg has more than 5 fields
	dataDir = "../unit_test_files/cpu_testcases/loadavg/testcase_2"
	data = cpu.getLoadInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

	//could not parse float:
	cpu.LoadAvg = 1
	dataDir = "../unit_test_files/cpu_testcases/loadavg/testcase_3"
	data = cpu.getLoadInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

	// could not scan /proc/loadavg
	dataDir = "../unit_test_files/cpu_testcases/loadavg/testcase_4"
	data = cpu.getLoadInfo()
	if len(data) != 0 {
		t.Errorf("Expected data of size %d, but got %d \n", 0, len(data))
	}

}

func TestGetTempInfo(t *testing.T) {
	cpu := MakeCpuCollector(true, 5, true, true)
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_1"
	cpu.init()
	data := cpu.getTempInfo()
	expected := map[string]interface{}{
		"a0_tempC": 40,
		"b1_tempC": 50,
		"c2_tempC": 60,
		"d3_tempC": 70,
	}

	if len(data) != len(expected) {
		t.Errorf("Expected data of size %d, but got %d \n", len(expected), len(data))
	}
	for k, v := range data {
		if v != expected[k] {
			t.Errorf("Expected data %d, but got %d \n", expected[k], v)
		}
	}

	// cannot parse temp as Uint
	dataDir = "../unit_test_files/cpu_testcases/temp/testcase_5"
	expected["a0_tempC"] = "NA"
	data = cpu.getTempInfo()
	if len(data) != len(expected) {
		t.Errorf("Expected data of size %d, but got %d \n", len(expected), len(data))
	}
	for k, v := range data {
		if v != expected[k] {
			t.Errorf("Expected data %d, but got %d \n", expected[k], v)
		}
	}

}

func TestCPUScrape(t *testing.T) {
	cpu := MakeCpuCollector(true, 5, true, true)
	dataDir = "../unit_test_files/cpu_testcases/scrape"
	cpu.init()
	data := cpu.scrape()
	expected := map[string]interface{}{
		"collector":   "cpu",
		"loadavg_5m":  (float32)(0.64),
		"uptimesec":   (float32)(1400.42),
		"idletimesec": (float32)(3000.34),
		"a0_tempC":    40,
		"b1_tempC":    50,
		"c2_tempC":    60,
		"d3_tempC":    70,
	}

	if len(data) != len(expected) {
		t.Errorf("Expected data of size %d, but got %d \n", len(expected), len(data))
	}
	for k, v := range data {
		if v != expected[k] {
			t.Errorf("Expected data %d, but got %d \n", expected[k], v)
		}
	}

}
