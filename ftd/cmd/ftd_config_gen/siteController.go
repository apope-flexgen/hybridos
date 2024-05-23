package main

import (
	"fmt"
	"path"
	"strings"

	"github.com/flexgen-power/hybridos/ftd/pkg/ftd"
)

func generateSiteControllerFTDConfig(urisToFormats map[string]string) (cfg ftd.Config, report []string) {
	report = []string{}

	// setup configurators that we might use
	configurators := map[string]*uriConfigurator{
		"site_summary": {
			required: true,
			warm:     true,
			fields:   getSCSiteSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"site_operation": {
			required:        true,
			warm:            true,
			fields:          getSCSiteOperationFields(),
			bitStringFields: []string{"active_faults", "active_alarms"},
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_operation",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"site_input_sources": {
			required: true,
			warm:     true,
			fields:   getSCSiteInputSourcesFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_input_sources",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"site_configuration": {
			required: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_configuration",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"features_active_power": {
			required: true,
			warm:     true,
			fields:   getSCFeaturesActivePowerFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_active_power",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"features_reactive_power": {
			required: true,
			warm:     true,
			fields:   getSCFeaturesReactivePowerFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_reactive_power",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"features_standalone_power": {
			required: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_standalone_power",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"features_site_operation": {
			required: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_site_operation",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"features_summary": {
			required: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_ess_instances": {
			required:        true,
			warm:            true,
			fields:          getSCAssetsESSInstanceFields(),
			bitStringFields: []string{"faults", "alarms", "bms_fault", "pcs_fault"},
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/ess",
				Measurement:   "ess_instances",
				Group:         "assets_ess_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_ess_summary": {
			required: true,
			warm:     true,
			fields:   getSCAssetsESSSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/ess",
				Measurement:   "ess_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_feeders_instances": {
			required: true,
			warm:     true,
			fields:   getSCAssetsFeedersInstancesFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/feeders",
				Measurement:   "feeders_instances",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_feeders_summary": {
			required: true,
			warm:     true,
			fields:   getSCAssetsFeedersSummaryFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/feeders",
				Measurement:   "feeders_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_generators_instances": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/generators",
				Measurement:   "generators_instances",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_generators_summary": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/generators",
				Measurement:   "generators_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_solar_instances": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/solar",
				Measurement:   "solar_instances",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"assets_solar_summary": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/solar",
				Measurement:   "solar_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_flexgen_ess": {
			warm:   true,
			fields: getSCComponentsESSFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components_flexgen_ess",
				Group:         "components_flexgen_ess_group",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_flexgen_ess_hs": {
			warm:   true,
			fields: getSCComponentsESSFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "flexgen_ess_high_speed",
				Group:         "components_flexgen_ess_group_hs",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components_flexgen_ess_ls": {
			warm:   true,
			fields: getSCComponentsESSFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "flexgen_ess_low_speed",
				Group:         "components_flexgen_ess_group_ls",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"meters": {
			warm:                     true,
			measurementForEachSource: true,
			fields:                   getSCMetersFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "meters",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"rtac": {
			warm:   true,
			fields: getSCRTACFields(),
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "rtac",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"components": {
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/components",
				Measurement:   "components",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"metrics": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/metrics",
				Measurement:   "metrics",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"cops_stats": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/cops/stats",
				Measurement:   "cops_stats",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"cops_summary": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/cops/summary",
				Measurement:   "cops_summary",
				Method:        []string{"pub"},
				DestinationDb: "influx",
			},
		},
		"site_sets": {
			activated:               true,
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/site",
				Measurement:   "site_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"features_sets": {
			activated:               true,
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/features",
				Measurement:   "features_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_ess_sets": {
			activated:               true,
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/ess",
				Measurement:   "assets_ess_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_feeders_sets": {
			activated:               true,
			warm:                    true,
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/feeders",
				Measurement:   "assets_feeders_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_generators_sets": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/generators",
				Measurement:   "assets_generators_sets",
				Method:        []string{"set"},
				DestinationDb: "influx",
			},
		},
		"assets_solar_sets": {
			useRawUriConfigTemplate: true,
			uriConfigTemplate: ftd.UriConfig{
				BaseUri:       "/assets/solar",
				Measurement:   "assets_solar_sets",
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
			configurator, exists := configurators["site_"+source]
			if exists {
				configurator.addSourceAndActivate(source, format)
			} else {
				report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
			}
		case "/features":
			configurator, exists := configurators["features_"+source]
			if exists {
				configurator.addSourceAndActivate(source, format)
			} else {
				report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
			}
		case "/assets/ess":
			if source == "summary" {
				configurators["assets_ess_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_ess_instances"].addSourceAndActivate(source, format)
			}
		case "/assets/feeders":
			if source == "summary" {
				configurators["assets_feeders_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_feeders_instances"].addSourceAndActivate(source, format)
			}
		case "/assets/generators":
			if source == "summary" {
				configurators["assets_generators_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_generators_instances"].addSourceAndActivate(source, format)
			}
			configurators["assets_generators_sets"].activated = true
		case "/assets/solar":
			if source == "summary" {
				configurators["assets_solar_summary"].addSourceAndActivate(source, format)
			} else {
				configurators["assets_solar_instances"].addSourceAndActivate(source, format)
			}
			configurators["assets_solar_sets"].activated = true
		case "/components":
			if strings.Contains(source, "flexgen_ess") {
				if strings.HasSuffix(source, "_hs") {
					configurators["components_flexgen_ess_hs"].addSourceAndActivate(source, format)
				} else if strings.HasSuffix(source, "_ls") {
					configurators["components_flexgen_ess_ls"].addSourceAndActivate(source, format)
				} else {
					configurators["components_flexgen_ess"].addSourceAndActivate(source, format)
				}
			} else {
				switch source {
				case "sel_3530_rtac", "sel_3530_rtac_hs", "sel_3530_rtac_ls":
					configurators["rtac"].addSourceAndActivate(source, format)
				case "sel_735", "sel_751", "sel_351_1", "sel_351_2", "m_bess_aux_acuvim", "MTR1", "METB11", "METB21", "52f1", "52f2", "52f4":
					configurators["meters"].addSourceAndActivate(source, format)
				default:
					configurators["components"].addSourceAndActivate(source, format)
				}
			}
		case "/metrics":
			configurators["metrics"].activated = true
		case "/cops/stats":
			configurators["cops_stats"].activated = true
		case "/cops":
			if source == "summary" {
				configurators["cops_summary"].activated = true
			} else {
				report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
			}
		case "/systemstats":
			// nothing to do since overwatch pub capture will always be added to the configuration
		case "/events":
			// skip configuring events pub uris
		default:
			report = append(report, fmt.Sprintf("Could not determine how to configure uri %s.", uri))
		}
	}

	report = append(report, "Please ensure that we haven't accidentally configured any meters or rtac component uris into the generic components measuement. Each meter type should get its own measurement.")

	cfg, configuratorReport := generateConfigFromConfigurators(configurators)
	report = append(report, configuratorReport...)
	return cfg, report
}

// Constant list of fields added to each components ess uri configurator
func getSCComponentsESSFields() []string {
	return []string{
		// listed in datapoints list under table components_ess_#
		"active_power_setpoint",
		"pcs_active_power",
	}
}

// Constant list of fields added to assets ess instances uri configurator
func getSCAssetsESSInstanceFields() []string {
	return []string{
		// listed in datapoints list under table ess_ess_group
		"active_power_setpoint",
		"dc_contactors_closed",
		"racks_in_service",
		"system_chargeable_energy",
		"system_dischargeable_energy",
		"system_chargeable_power",
		"system_dischargeable_power",
		"modbus_heartbeat",
		"faults",
		"voltage_l1_l2",
		"voltage_l2_l3",
		"voltage_l3_l1",
		"current_l1",
		"current_l2",
		"current_l3",
		"current_dc",
		"pcs_dc_current",
		"voltage_dc",
		"frequency",
		"apparent_power",
		"pf",
		"power_factor",
		"reactive_power_setpoint",
		"reactive_power",
		"active_power",
		"bms_maximum_cell_temperature",
		"bms_maximum_cell_voltage",
		"bms_minimum_cell_temperature",
		"bms_minimum_cell_voltage",
		"bms_average_cell_temperature",
		"reactive_power_ramp_rate",
		"active_power_ramp_rate",
		"soc",
		"min_cell_temp",
		"max_cell_temp",
		"alarms",
		"min_cell_voltage",
		"max_cell_voltage",
		"soh",
		"status",
		"avg_cell_temp",
		"dc_voltage",
		"bms_fault",
		"pcs_fault",
		"maint_mode",
	}
}

// Constant list of fields added to assets ess summary uri configurator
func getSCAssetsESSSummaryFields() []string {
	return []string{
		// listed in datapoints list under table ess_summary
		"ess_chargeable_power",
		"ess_dischargeable_power",
		"ess_chargeable_energy",
		"ess_dischargeable_energy",
		"ess_total_reactive_power",
		"num_ess_available",
		"num_ess_running",
		"ess_average_soc",
	}
}

// Constant list of fields added to features active power uri configurator
func getSCFeaturesActivePowerFields() []string {
	return []string{
		// listed in datapoints list under table features_active_power
		"runmode1_kW_mode_status",
		"feature_kW_demand",
		"site_kW_demand",
		"ess_kW_cmd",
		"ess_actual_kW",
		"fr_baseload_cmd_kw_dnp3",
		"fr_baseload_cmd_kw",
	}
}

// Constant list of fields added to features reactive power uri configurator
func getSCFeaturesReactivePowerFields() []string {
	return []string{
		// listed in datapoints list under table features_reactive_power
		"ess_actual_kVAR",
		"ess_kVAR_cmd",
		"runmode1_kVAR_mode_status",
		"site_kVAR_demand",
	}
}

// Constant list of fields added to assets feeders instances uri configurator
func getSCAssetsFeedersInstancesFields() []string {
	return []string{
		// listed in datapoints list under table feeders_feed_#
		"breaker_status",
		"aux_active_energy",
		"kwh_received",
		"kwh_delivered",
		"active_power",
	}
}

// Constant list of fields added to assets feeders summary uri configurator
func getSCAssetsFeedersSummaryFields() []string {
	return []string{
		// listed in datapoints list under table feeders_summary
		"feed_1_active_power",
	}
}

// Constant list of fields added to meters uri configurator
func getSCMetersFields() []string {
	return []string{
		// listed in datapoints list under table meters_<meter_type>
		"hv_poi_energy_delivered",
		"hv_poi_energy_received",
		"kwh_delivered",
		"kwh_received",
		"apparent_power",
		"current_n",
		"kvarh_delivered",
		"kvarh_received",
		"voltage_l1",
		"voltage_l2",
		"voltage_l3",
		"active_power",
		"current_l1",
		"current_l2",
		"current_l3",
		"frequency",
		"power_factor",
		"site_energy_exported",
		"voltage_l1_l2",
		"voltage_l2_l3",
		"voltage_l3_l1",
		"mv_feeder_1_3_phase_mwhr_delivered",
		"mv_feeder_1_3_phase_mwhr_received",
		"mv_feeder_2_3_phase_mwhr_delivered",
		"mv_feeder_2_3_phase_mwhr_received",
		"mv_feeder_3_3_phase_mwhr_delivered",
		"mv_feeder_3_3_phase_mwhr_received",
		"mv_feeder_4_3_phase_mwhr_delivered",
		"mv_feeder_4_3_phase_mwhr_received",
	}
}

// Constant list of fields added to rtac uri configurator
func getSCRTACFields() []string {
	return []string{
		// listed in datapoints list under table rtac
		"frequency_3",
		"site_kw",
		"site_kwh_in",
		"site_kwh_out",
		"bess_breaker_status",
		"site_frequency",
		"site_voltage",
		"site_current_a",
		"site_current_b",
		"site_current_c",
		"site_2_aux_active_energy",
		"site_1_aux_active_energy",
		"kwh_delivered",
		"kwh_received",
		"active_power",
	}
}

// Constant list of fields added to site input sources uri configurator
func getSCSiteInputSourcesFields() []string {
	return []string{
		// listed in datapoints list under table site_input_sources
		"ui",
	}
}

// Constant list of fields added to site operation uri configurator
func getSCSiteOperationFields() []string {
	return []string{
		// listed in datapoints list under table site_operation
		"site_status",
		"time",
	}
}

// Constant list of fields added to site summary uri configurator
func getSCSiteSummaryFields() []string {
	return []string{
		// listed in datapoints list under table site_summary
		"active_faults",
		"active_alarms",
	}
}
