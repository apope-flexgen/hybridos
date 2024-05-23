package main

import (
	"fmt"
	"path"
	"strings"

	"github.com/flexgen-power/hybridos/ftd/pkg/ftd"
)

func generateESSControllerFTDConfig(urisToFormats map[string]string) (cfg ftd.Config, report []string) {
	report = []string{}

	// setup configurators that we might use
	configurators := map[string]*uriConfigurator{
		"site": {
			required:                true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_ess",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_summary": {
			required: true,
			warm:     true,
			fields:   getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms",
				Measurement:   "assets_bms_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_instances": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms",
				Measurement:   "assets_bms_instances",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms",
				Measurement:   "assets_bms_racks",
				Group:         "assets_bms_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_1_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_1",
				Measurement:   "assets_bms_1_racks",
				Group:         "assets_bms_1_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_1_summary": {
			warm:   true,
			fields: getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_1",
				Measurement:   "assets_bms_1_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_2_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_2",
				Measurement:   "assets_bms_2_racks",
				Group:         "assets_bms_2_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_2_summary": {
			warm:   true,
			fields: getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_2",
				Measurement:   "assets_bms_2_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_3_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_3",
				Measurement:   "assets_bms_3_racks",
				Group:         "assets_bms_3_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_3_summary": {
			warm:   true,
			fields: getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_3",
				Measurement:   "assets_bms_3_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_4_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_4",
				Measurement:   "assets_bms_4_racks",
				Group:         "assets_bms_4_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_4_summary": {
			warm:   true,
			fields: getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_4",
				Measurement:   "assets_bms_4_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_5_racks": {
			warm:   true,
			fields: getECAssetsBMSRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_5",
				Measurement:   "assets_bms_5_racks",
				Group:         "assets_bms_5_racks_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_5_summary": {
			warm:   true,
			fields: getECAssetsBMSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_5",
				Measurement:   "assets_bms_5_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_pcs_summary": {
			required: true,
			warm:     true,
			fields:   getECAssetsPCSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/pcs",
				Measurement:   "assets_pcs_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_pcs_instances": {
			required: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/pcs",
				Measurement:   "assets_pcs_instances",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_racks": {
			required: true,
			warm:     true,
			fields:   getECComponentsRacksFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "ess_bms_racks",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_bms_info": {
			warm:   true,
			fields: getECComponentsBMSInfoFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components_bms_info",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_bms_controls": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components_bms_controls",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_pcs_info": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components_pcs_info",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_pcs_controls": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components_pcs_controls",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components": {
			warm:   true,
			fields: getECComponentsFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "ess_info",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms",
				Measurement:   "assets_bms_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_1_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_1",
				Measurement:   "assets_bms_1_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_2_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_2",
				Measurement:   "assets_bms_2_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_3_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_3",
				Measurement:   "assets_bms_3_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_4_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_4",
				Measurement:   "assets_bms_4_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_bms_5_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/bms_5",
				Measurement:   "assets_bms_5_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_pcs_sets": {
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/pcs",
				Measurement:   "assets_pcs_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
	}
	addOverwatchConfigurators(&configurators)
	addEventsConfigurator(&configurators)

	// iterate through uris and add them to corresponding configurators if necessary
	for uri, format := range urisToFormats {
		baseUri := path.Dir(uri)
		source := path.Base(uri)
		switch baseUri {
		case "/site":
			configurators["site"].activated = true
		case "/assets/bms":
			if source == "summary" {
				configurators["assets_bms_summary"].addSourceAndActivate(source, format)
			} else if strings.Contains(source, "rack") || strings.Contains(source, "sbmu") {
				configurators["assets_bms_racks"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_instances"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_sets"].activated = true
		case "/assets/bms_1":
			if source == "summary" {
				configurators["assets_bms_1_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_1_racks"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_1_sets"].activated = true
		case "/assets/bms_2":
			if source == "summary" {
				configurators["assets_bms_2_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_2_racks"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_2_sets"].activated = true
		case "/assets/bms_3":
			if source == "summary" {
				configurators["assets_bms_3_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_3_racks"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_3_sets"].activated = true
		case "/assets/bms_4":
			if source == "summary" {
				configurators["assets_bms_4_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_4_racks"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_4_sets"].activated = true
		case "/assets/bms_5":
			if source == "summary" {
				configurators["assets_bms_5_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_bms_5_racks"].addSourceAndActivate(source, format)
			}
			configurators["assets_bms_5_sets"].activated = true
		case "/assets/pcs":
			if source == "summary" {
				configurators["assets_pcs_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_pcs_instances"].addSourceAndActivate(source, format)
			}
			configurators["assets_pcs_sets"].activated = true
		case "/components":
			if strings.Contains(source, "rack") || strings.Contains(source, "sbmu") {
				// this check happens first so we catch sources like bms_1_rack_1_info
				configurators["components_racks"].addSourceAndActivate(source, format)
			} else if strings.HasPrefix(source, "bms_") && strings.HasSuffix(source, "_info") {
				configurators["components_bms_info"].addSourceAndActivate(source, format)
			} else if strings.HasPrefix(source, "bms_") && (strings.HasSuffix(source, "_controls") || strings.HasSuffix(source, "_control")) {
				configurators["components_bms_controls"].addSourceAndActivate(source, format)
			} else if strings.HasPrefix(source, "pcs_") && strings.HasSuffix(source, "_info") {
				configurators["components_pcs_info"].addSourceAndActivate(source, format)
			} else if strings.HasPrefix(source, "pcs_") && (strings.HasSuffix(source, "_controls") || strings.HasSuffix(source, "_control")) {
				configurators["components_pcs_controls"].addSourceAndActivate(source, format)
			} else {
				configurators["components"].addSourceAndActivate(source, format)
			}
		case "/systemstats":
			// nothing to do since overwatch pub capture will always be added to the configuration
		case "/events":
			// skip configuring events pub uris
		default:
			report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
		}
	}

	cfg, configuratorReport := generateConfigFromConfigurators(configurators)
	report = append(report, configuratorReport...)
	return cfg, report
}

// Constant list of fields added to each assets bms racks uri configurator
func getECAssetsBMSRacksFields() []string {
	return []string{
		// listed in datapoints list under table asset_bms_racks/assets_bms_#_racks
		"system_current",
		"dc_current",
		"system_voltage",
		"dc_voltage",
		"system_power",
		"dc_power",
		"soc",
		"soh",
		"min_cell_voltage",
		"max_cell_voltage",
		"avg_cell_voltage",
		"cell_temp_delta",
		"cell_voltage_delta",
		"max_cell_temp",
		"min_cell_temp",
		"avg_cell_temp",
		"system_state",
		"status",
		"dc_contactor_status",
		"dc_closed_status",
		"discharge_power_limit",
		"charge_power_limit",
		"source",
		// listed in datapoints list under table assets-bms-rack-##
		"rack_soc",
		"rack_soh",
		"rack_current",
		"rack_voltage",
		"rack_power",
	}
}

// Constant list of fields added to each assets bms summary uri configurator
func getECAssetsBMSSummaryFields() []string {
	return []string{
		// listed in datapoints list under table assets_bms_#_summary
		"dischargeable_energy",
		"chargeable_energy",
		// listed in datapoints list under table assets_bms_summary
		"soc",
		"max_cell_voltage_in_the_system",
		"max_cell_temp",
		"max_cell_voltage",
		"min_cell_temp",
		"min_cell_voltage",
	}
}

// Constant list of fields added to components bms info uri configurator
func getECComponentsBMSInfoFields() []string {
	return []string{
		// listed in datapoints list under table components_bms_info
		"max_cell_cell_rack_loc",
		"max_cell_temp_loc",
		"max_cell_temp_module_loc",
		"max_volt_cell_loc",
		"max_volt_cell_module_loc",
		"max_volt_cell_rack_loc",
		"min_cell_temp_loc",
		"min_cell_temp_module_loc",
		"min_cell_temp_rack_loc",
		"min_volt_cell_loc",
		"min_volt_cell_module_loc",
		"min_volt_cell_rack_loc",
		"batt_cabinett_ac_relay_#",
		"hvac_1_compressor_status",
		"hvac_10_compressor_status",
		"hvac_11_compressor_status",
		"hvac_2_compressor_status",
		"hvac_3_compressor_status",
		"hvac_4_compressor_status",
		"hvac_5_compressor_status",
		"hvac_6_compressor_status",
		"hvac_7_compressor_status",
		"hvac_8_compressor_status",
		"hvac_9_compressor_status",
		"hvac_1_condenser_temp",
		"hvac_10_condenser_temp",
		"hvac_11_condenser_temp",
		"hvac_2_condenser_temp",
		"hvac_3_condenser_temp",
		"hvac_4_condenser_temp",
		"hvac_5_condenser_temp",
		"hvac_6_condenser_temp",
		"hvac_7_condenser_temp",
		"hvac_8_condenser_temp",
		"hvac_9_condenser_temp",
		"hvac_1_unit_running_status",
		"hvac_10_unit_running_status",
		"hvac_11_unit_running_status",
		"hvac_2_unit_running_status",
		"hvac_3_unit_running_status",
		"hvac_4_unit_running_status",
		"hvac_5_unit_running_status",
		"hvac_6_unit_running_status",
		"hvac_7_unit_running_status",
		"hvac_8_unit_running_status",
		"hvac_9_unit_running_status",
		"hvac_1_inside_return_temp",
		"hvac_10_inside_return_temp",
		"hvac_11_inside_return_temp",
		"hvac_2_inside_return_temp",
		"hvac_3_inside_return_temp",
		"hvac_4_inside_return_temp",
		"hvac_5_inside_return_temp",
		"hvac_6_inside_return_temp",
		"hvac_7_inside_return_temp",
		"hvac_8_inside_return_temp",
		"hvac_9_inside_return_temp",
		"hvac_1_inside_return_humidity",
		"hvac_10_inside_return_humidity",
		"hvac_11_inside_return_humidity",
		"hvac_2_inside_return_humidity",
		"hvac_3_inside_return_humidity",
		"hvac_4_inside_return_humidity",
		"hvac_5_inside_return_humidity",
		"hvac_6_inside_return_humidity",
		"hvac_7_inside_return_humidity",
		"hvac_8_inside_return_humidity",
		"hvac_9_inside_return_humidity",
		"hvac_1_external_fan_status",
		"hvac_10_external_fan_status",
		"hvac_11_external_fan_status",
		"hvac_2_external_fan_status",
		"hvac_3_external_fan_status",
		"hvac_4_external_fan_status",
		"hvac_5_external_fan_status",
		"hvac_6_external_fan_status",
		"hvac_7_external_fan_status",
		"hvac_8_external_fan_status",
		"hvac_9_external_fan_status",
		"hvac_1_internal_fan_status",
		"hvac_10_internal_fan_status",
		"hvac_11_internal_fan_status",
		"hvac_2_internal_fan_status",
		"hvac_3_internal_fan_status",
		"hvac_4_internal_fan_status",
		"hvac_5_internal_fan_status",
		"hvac_6_internal_fan_status",
		"hvac_7_internal_fan_status",
		"hvac_8_internal_fan_status",
		"hvac_9_internal_fan_status",
		"hvac_1_heat_exchanger_coil_temp",
		"hvac_10_heat_exchanger_coil_temp",
		"hvac_11_heat_exchanger_coil_temp",
		"hvac_2_heat_exchanger_coil_temp",
		"hvac_3_heat_exchanger_coil_temp",
		"hvac_4_heat_exchanger_coil_temp",
		"hvac_5_heat_exchanger_coil_temp",
		"hvac_6_heat_exchanger_coil_temp",
		"hvac_7_heat_exchanger_coil_temp",
		"hvac_8_heat_exchanger_coil_temp",
		"hvac_9_heat_exchanger_coil_temp",
		"soc",
	}
}

// Constant list of fields added to each components racks uri configurator
func getECComponentsRacksFields() []string {
	return []string{
		// listed in datapoints list under table ess_bms_racks
		"max_single_cell_temperature_position",
		"min_single_cell_temperature_position",
		"max_single_cell_voltage_position",
		"min_single_cell_voltage_position",
		"max_cell_temp_num",
		"max_cell_temp_slave_num",
		"max_cell_voltage_num",
		"max_cell_voltage_slave_num",
		"min_cell_slave_num",
		"min_cell_temp_num",
		"min_cell_voltage_num",
		"min_cell_voltage_slave_num",
		"air_con_heating_band",
		"air_con_heating_stop_point",
		"air_con_high_humidity_point",
		"air_con_high_temp_point",
		"air_con_indoor_temp",
		"air_con_internal_fan_stop_point",
		"air_con_low_temp_point",
		"air_con_refrigeration_band",
		"air_con_refrigeration_stop_point",
		"air_con_running_status",
		// listed in datapoints list under table components_bms_racks
		"max_cell_voltage",
		"min_cell_voltage",
		"max_cell_temp",
		"min_cell_temp",
		"soc",
	}
}

// Constant list of fields added to each generic components uri configurator
func getECComponentsFields() []string {
	return []string{
		// listed in datapoints list under table ess_ess_info
		"system_min_cell_temperature",
		"system_min_temp_cell_id",
		"system_min_temp_cell's_rack_id",
		"system_min_temp_cell's_slave_id",
		"system_min_vol_cell_id",
		"system_min_vol_cell's_rack_id",
		"system_min_vol_cell's_slave_id",
		"system_max_cell_temperature",
		"system_max_temp_cell_id",
		"system_max_temp_cell's_rack_id",
		"system_max_temp_cell's_slave_id",
		"system_max_vol_cell_id",
		"system_max_vol_cell's_rack_id",
		"system_max_vol_cell's_slave_id",
		"hvac_compressor_state",
		"hvac_condensation_temp",
		"hvac_heat_coil_temp",
		"hvac_exterior_fan_state",
		"hvac_interior_fan_state",
		"hvac_interior_temp",
		"hvac_cooling_setpoint_upper_limit",
		"hvac_heating_setpoint_lower_limit",
		"hvac_heat_temp_setting",
		"hvac_temp_setting",
		"hvac_humidity",
		"hvac_gen_sys_state",
		"min_cell_voltage_in_the_system",
		"max_cell_voltage_in_the_system",
		"system_soc",
		"watt_demand_3_phase_received",
		"watt_demand_3_phase_delivered",
		"dischargeable_energy",
		"dischargeable_power",
		"chargeable_energy",
		"chargeable_power",
	}
}

// Constant list of fields added to each assets pcs summary uri configurator
func getECAssetsPCSSummaryFields() []string {
	return []string{
		// listed in datapoints list under table assets_pcs_summary
		"active_power",
	}
}
