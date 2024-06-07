package dts

import (
	"encoding/json"
	"os"
	"testing"
)

// Test that the config used by default in containers is valid
func TestDefaultContainerConfig(t *testing.T) {
	defaultCfgFileBytes, err := os.ReadFile("../../config/dts.json")
	if err != nil {
		t.Errorf("Failed to open file: %v", err)
	}

	err = json.Unmarshal(defaultCfgFileBytes, &GlobalConfig)
	if err != nil {
		t.Errorf("Failed to unmarshal config: %v", err)
	}
	t.Logf("%+v", GlobalConfig)

	err = validateConfig()
	if err != nil {
		t.Errorf("Invalid config: %v", err)
	}
}

// Test unmarshalling and validation
func TestUnmarshalAndValidateConfig(t *testing.T) {
	testCfgString :=
		`{
			"note": "This is a note which should not affect anything.",
			"input_path": "/home/hybridos/historian/inbox",
			"failed_validate_path": "/home/hybridos/historian/validate_error",
			"failed_write_path": "/home/hybridos/historian/write_error",
			"ext": [".tar.gz", ".batchpqtgz"],
			"num_validate_workers": 3,
			"num_influx_prepare_batches_workers": 3,
			"num_influx_send_batches_workers": 3,
			"db_health_check_delay_seconds": 0.05
		}`

	// test that default values are assigned properly.
	err := json.Unmarshal([]byte(testCfgString), &GlobalConfig)
	if err != nil {
		t.Errorf("Failed to unmarshal config: %v", err)
	}
	t.Logf("%+v", GlobalConfig)

	if GlobalConfig.InfluxAddr != "localhost:8086" {
		t.Errorf("Influx address was unexpected value %v", GlobalConfig.InfluxAddr)
	}
	if GlobalConfig.MongoAddr != "localhost:27017" {
		t.Errorf("Mongo address was unexpected value %v", GlobalConfig.MongoAddr)
	}
	if GlobalConfig.RetryConnectPeriodSeconds != 1 {
		t.Errorf("Retry connect period seconds was unexpected value %v", GlobalConfig.RetryConnectPeriodSeconds)
	}

	err = validateConfig()
	if err != nil {
		t.Errorf("Invalid config: %v", err)
	}

	// test a config that has retention policies and continuous queries

	testCfgString =
		`{
			"note": "This is a note which should not affect anything.",
			"input_path": "/home/hybridos/historian/inbox",
			"failed_validate_path": "/home/hybridos/historian/validate_error",
			"failed_write_path": "/home/hybridos/historian/write_error",
			"ext": [".tar.gz"],
			"num_validate_workers": 3,
			"num_influx_prepare_batches_workers": 3,
			"num_influx_send_batches_workers": 3,
			"db_health_check_delay_seconds": 0.05,
			"retention_policies": [
				{
					"duration": "INF",
					"name": "infinite_rp"
				},
				{
					"duration": "30d",
					"default": true,
					"name": "30_day_rp"
				},
				{
					"duration": "120d",
					"name": "120_day_rp"
				}
			],
			"continuous_queries": [
				{
					"group_by": "time(10m), *",
					"resample": "EVERY 10m FOR 20m",
					"name": "10min_cq",
					"rp": "infinite_rp",
					"select": "MIN(*), MAX(*), MEAN(*), LAST(*)"
				},
				{
					"group_by": "time(1m), *",
					"resample": "EVERY 5m FOR 10m",
					"name": "1min_cq",
					"rp": "120_day_rp",
					"select": "MIN(*), MAX(*), MEAN(*), LAST(*)"
				}
			]
		}`

	// test that default values are assigned properly.
	err = json.Unmarshal([]byte(testCfgString), &GlobalConfig)
	if err != nil {
		t.Errorf("Failed to unmarshal config: %v", err)
	}
	t.Logf("%+v", GlobalConfig)

	if GlobalConfig.InfluxAddr != "localhost:8086" {
		t.Errorf("Influx address was unexpected value %v", GlobalConfig.InfluxAddr)
	}
	if GlobalConfig.MongoAddr != "localhost:27017" {
		t.Errorf("Mongo address was unexpected value %v", GlobalConfig.MongoAddr)
	}
	if GlobalConfig.RetryConnectPeriodSeconds != 1 {
		t.Errorf("Retry connect period seconds was unexpected value %v", GlobalConfig.RetryConnectPeriodSeconds)
	}

	if len(GlobalConfig.ContinuousQueries) == 0 {
		t.Error("Continuous queries were unexpectedly nonexistant")
	} else {
		for i, cq := range GlobalConfig.ContinuousQueries {
			if cq.Measurement != ":MEASUREMENT" {
				t.Errorf("CQ index %d measurement was unexpected value %v", i, cq.Measurement)
			}
			if cq.From != "/.*/" {
				t.Errorf("CQ index %d from was unexpected value %v", i, cq.From)
			}
			if cq.Into != "" {
				t.Errorf("CQ index %d measurement was unexpected value %v", i, cq.Into)
			}
		}
	}

	err = validateConfig()
	if err != nil {
		t.Errorf("Invalid config: %v", err)
	}
}
