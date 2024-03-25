package ftd

import (
	"encoding/json"
	"reflect"
	"testing"
)

// Test extracting config from a json
func TestExtractRootConfiguration(t *testing.T) {
	configStr := `{
		"1": {
			"db_name": "site_controller_01_cold",
			"period": 900,
			"num_archive_workers": 1,
			"archive": "/home/hybridos/powercloud/outbox/cold",
			"uris": [
				{
					"uri": "/site",
					"destination": "influx",
					"measurement": "site"
				},
				{
					"uri": "/features",
					"destination": "influx",
					"measurement": "features"
				},
				{
					"uri": "/assets/ess",
					"sources":
					[
						"ess_01", "ess_02", "ess_03", "ess_04", "ess_05", "ess_06",
						"ess_07", "ess_08", "ess_09", "ess_10", "ess_11", "ess_12",
						"ess_13", "ess_14", "ess_15", "ess_16", "ess_17", "ess_18"
					],
					"group": "ess_group",
					"message_methods": 
					[
						"pub", "post", "set"
					],
					"destination": "influx",
					"measurement": "ess"
				},
				{
					"uri": "/assets/ess",
					"sources":
					[
						"summary"
					],
					"destination": "influx",
					"measurement": "ess"
				},
				{
					"uri": "/assets/feeders",
					"sources":
					[
						"feed_1", "feed_2", "summary"
					],
					"destination": "influx",
					"measurement": "feeders"
				},
				{
					"uri": "/components",
					"sources":
					[
						"ess_01", "ess_02", "ess_03", "ess_04", "ess_05", "ess_06",
						"ess_07", "ess_08", "ess_09", "ess_10", "ess_11", "ess_12",
						"ess_13", "ess_14", "ess_15", "ess_16", "ess_17", "ess_18"
					],
					"destination": "influx",
					"measurement": "ess_modbus"
				},
				{
					"uri": "/components",
					"sources":
					[
						"sel_3350_rtac"
					],
					"destination": "influx",
					"measurement": "rtac"
				},
				{
					"uri": "/components",
					"sources":
					[
						"apc_ups"
					],
					"destination": "influx",
					"measurement": "components"
				},
				{
					"uri": "/metrics",
					"destination": "influx",
					"measurement": "metrics"
				},
				{
					"uri": "/events",
					"destination": "mongo",
					"measurement": "events"
				},
				{
					"uri": "/systemstats/cpu",
					"destination": "influx",
					"measurement": "cpustats"
				},
				{
					"uri": "/systemstats/mem",
					"destination": "influx",
					"measurement": "memstats"
				},
				{
					"uri": "/systemstats/net",
					"destination": "influx",
					"measurement": "netstats"
				},
				{
					"uri": "/systemstats/disk",
					"destination": "influx",
					"measurement": "diskstats"
				},
				{
					"uri": "/systemstats/process",
					"destination": "influx",
					"measurement": "processstats"
				}
			]
		},
		"2": {
			"db_name": "site_controller_01_warm",
			"period": 60,
			"num_archive_workers": 1,
			"archive": "/home/hybridos/powercloud/outbox/warm",
			"uris": [
				{
					"uri": "/site",
					"sources": [
						"operation"
					],
					"fields": [
						"site_status"
					],
					"destination": "influx",
					"measurement": "site_operations"
				},
				{
					"uri": "/site",
					"sources": [
						"input_sources"
					],
					"fields": [
						"ui"
					],
					"destination": "influx",
					"measurement": "site_input_sources"
				},
				{
					"uri": "/features",
					"sources": [
						"/active_power"
					],
					"fields": [
						"runmode1_kW_mode_status",
						"feature_kW_demand",
						"site_kW_demand",
						"ess_actual_kW"
					],
					"destination": "influx",
					"measurement": "features_active_power"
				},
				{
					"uri": "/features",
					"sources": [
						"/reactive_power"
					],
					"fields": [
						"runmode1_kVAR_mode_status"
					],
					"destination": "influx",
					"measurement": "features_reactive_power"
				},
				{
					"uri": "/assets/ess",
					"sources":
					[
						"ess_01", "ess_02", "ess_03", "ess_04", "ess_05", "ess_06",
						"ess_07", "ess_08", "ess_09", "ess_10", "ess_11", "ess_12",
						"ess_13", "ess_14", "ess_15", "ess_16", "ess_17", "ess_18"
					],
					"fields": [
						"active_power",
						"active_power_setpoint",
						"active_power_setpoint",
						"apprent_power",
						"avg_cell_temp",
						"chargeable_energy",
						"current_l1",
						"current_l2",
						"current_l3",
						"dc_contactors_closed",
						"dc_voltage",
						"dischargeable_energy",
						"frequency",
						"max_cell_temp",
						"max_cell_voltage",
						"min_cell_temp",
						"min_cell_voltage",
						"modbus_heartbeat",
						"pcs_dc_current",
						"power_factor",
						"racks_in_service",
						"reactive_power",
						"reactive_power_setpoint",
						"soc",
						"soh",
						"status",
						"voltage_l1_l2",
						"voltage_l2_l3",
						"voltage_l3_l1"
					],
					"group": "ess_group",
					"destination": "influx",
					"measurement": "ess"
				},
				{
					"uri": "/assets/ess",
					"sources":
					[
						"summary"
					],
					"fields": [
						"ess_average_soc",
						"num_ess_available",
						"num_ess_running"
					],
					"destination": "influx",
					"measurement": "ess_summary"
				},
				{
					"uri": "/components",
					"sources":
					[
						"sel_3350_rtac"
					],
					"fields": [
						"bess_breaker_status",
						"site_current_a",
						"site_current_b",
						"site_current_c",
						"site_frequency",
						"site_kw",
						"site_kwh_in",
						"site_kwh_out",
						"site_voltage"
					],
					"destination": "influx",
					"measurement": "rtac"
				},
				{
					"uri": "/events",
					"destination": "mongo",
					"measurement": "events"
				},
				{
					"destination": "influx",
					"measurement": "ess_set",
					"message_methods": ["set"],
					"uri": "/assets/ess"
				}
			]
		}
	}`

	configBody := map[string]interface{}{}
	json.Unmarshal([]byte(configStr), &configBody)

	cfg, err := ExtractRootConfiguration(configBody)
	if err != nil {
		t.Fatalf("Failed to extract configuration with err: %v", err)
	}

	//run test with flag -v to see output of config ingestion (defaults to only show lane 1)
	//only works when there is no error in config
	t.Logf("Here is the config: %v\n", cfg.Lane1)

	//checks uri /assets/ess in lane 1 for the configured message_methods
	var edited_config UriConfig
	done := false
	edit_target := []string{"pub", "post", "set"}
	for _, entry := range cfg.Lane1.Uris {
		if done {
			break
		}
		if entry.BaseUri == "/assets/ess" {
			edited_config = entry
			config_message_method_match := reflect.DeepEqual(edited_config.Method, edit_target)
			if !config_message_method_match {
				t.Error("Edited message_method for /assets/ess does not match expected value")
			}
			done = true
		}
	}

	//checks the /site uri in the config for default message_methods
	var default_config UriConfig
	done = false
	default_target := []string{"pub", "post"}
	for _, def := range cfg.Lane1.Uris {
		if done {
			break
		}
		if def.BaseUri == "/site" {
			default_config = def
			default_message_method_match := reflect.DeepEqual(default_config.Method, default_target)
			if !default_message_method_match {
				t.Error("Default message_method for /site does not match [\"pub\", \"post\"]")
			}
		}
	}

	if cfg.Lane3 != nil {
		t.Error("Config lane 3 is not nil")
	}
	if cfg.Lane2.Uris[0].Fields[0] != "site_status" {
		t.Error("Config lane 2 uris[0] fields[0] is not site_status")
	}
}
