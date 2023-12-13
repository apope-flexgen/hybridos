#include <Reference_Configs.h>

/* C Standard Library Dependencies */
/* C++ Standard Library Dependencies */
/* External Dependencies */
/* System Internal Dependencies */
/* Local Internal Dependencies */
#include <Logger.h>

Reference_Configs::Reference_Configs() {
    variables_json_defaults = cJSON_Parse(variables_json_defaults_string.c_str());
    if (variables_json_defaults == NULL || cJSON_GetObjectItem(variables_json_defaults, "variables") == NULL) {
        FPS_ERROR_LOG("There is something wrong with this build. We failed to parse the static default variables.json.");
        exit(1);
    }
}

Reference_Configs::~Reference_Configs() {
    cJSON_Delete(variables_json_defaults);
}

const std::string Reference_Configs::variables_json_defaults_string = R"<raw-string>(
{
    "variables": {
        "internal": {
            "defaults": {
                "name": "Default Variable Types",
                "unit": "",
                "ui_type": "status",
                "type": "number",
                "var_type": "Float",
                "value": 0.0,
                "scaler": 1,
                "ui_enabled": true,
                "multiple_inputs": false,
                "options": []
            },
            "faults": {
                "name": "Site Manager Fault List",
                "ui_type": "fault",
                "type": "enum",
                "var_type": "Int",
                "value": 0,
                "options": [
                    {
                        "name": "Site Debug Fault Message",
                        "value": 0
                    },
                    {
                        "name": "Site Sequence Fault Detected",
                        "value": 1
                    },
                    {
                        "name": "Site Sequence Step Timeout",
                        "value": 2
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 3
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 4
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 5
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 6
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 7
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 8
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 9
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 10
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 11
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 12
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 13
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 14
                    },
                    {
                        "name": "Uninitialized Fault",
                        "value": 15
                    },
                    {
                        "name": "UPS Low State of Charge",
                        "value": 16
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 2",
                        "value": 17
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 3",
                        "value": 18
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 4",
                        "value": 19
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 5",
                        "value": 20
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 6",
                        "value": 21
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 7",
                        "value": 22
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 8",
                        "value": 23
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 9",
                        "value": 24
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 10",
                        "value": 25
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 11",
                        "value": 26
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 12",
                        "value": 27
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 13",
                        "value": 28
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 14",
                        "value": 29
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 15",
                        "value": 30
                    },
                    {
                        "name": "Uninitialized Reserved Bool Fault 16",
                        "value": 31
                    }
                ]
            },
            "alarms": {
                "name": "Site Manager Alarm List",
                "ui_type": "alarm",
                "type": "enum",
                "var_type": "Int",
                "value": 0,
                "options": [
                    {
                        "name": "Site Debug Alarm Message",
                        "value": 0
                    },
                    {
                        "name": "Asset Alarm Detected",
                        "value": 1
                    },
                    {
                        "name": "Asset Fault Detected",
                        "value": 2
                    },
                    {
                        "name": "Price Thresholds Invalid",
                        "value": 3
                    },
                    {
                        "name": "Watchdog Timeout",
                        "value": 4
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 5
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 6
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 7
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 8
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 9
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 10
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 11
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 12
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 13
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 14
                    },
                    {
                        "name": "Uninitialized Alarm",
                        "value": 15
                    },
                    {
                        "name": "UPS Low State of Charge",
                        "value": 16
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 2",
                        "value": 17
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 3",
                        "value": 18
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 4",
                        "value": 19
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 5",
                        "value": 20
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 6",
                        "value": 21
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 7",
                        "value": 22
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 8",
                        "value": 23
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 9",
                        "value": 24
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 10",
                        "value": 25
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 11",
                        "value": 26
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 12",
                        "value": 27
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 13",
                        "value": 28
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 14",
                        "value": 29
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 15",
                        "value": 30
                    },
                    {
                        "name": "Uninitialized Reserved Bool Alarm 16",
                        "value": 31
                    }
                ]
            },
            "exit_timer": {
                "name": "Sequence Failure Exit Timer",
                "unit": "ms",
                "ui_type": "none",
                "var_type": "Int",
                "value": 90000
            },
            "asset_priority_runmode2": {
                "name": "Asset Priority - Run Mode 2",
                "ui_type": "control",
                "var_type": "Int",
                "value": 0
            },
            "start_first_gen_kW": {
                "name": "First Generator Start Power Threshold",
                "ui_type": "none",
                "unit": "kW",
                "value": 1210000,
                "scaler": 1
            },
            "allow_gen_auto_restart": {
                "name": "Allow Generator Autostart",
                "var_type": "Bool",
                "value": true
            },
            "start_first_ess_kW": {
                "name": "First ESS Start Power Threshold",
                "ui_type": "none",
                "unit": "kW",
                "value": 0,
                "scaler": 1
            },
            "allow_ess_auto_restart": {
                "name": "Allow ESS Autostart",
                "var_type": "Bool",
                "value": true
            },
            "start_first_solar_kW": {
                "name": "First Solar Start Power Threshold",
                "ui_type": "none",
                "unit": "kW",
                "value": 0,
                "scaler": 1
            },
            "allow_solar_auto_restart": {
                "name": "Allow Solar Autostart",
                "var_type": "Bool",
                "value": true
            },
            "soc_min_all": {
                "name": "Minimum State of Charge for All ESS",
                "unit": "%"
            },
            "soc_max_all": {
                "name": "Maximum State of Charge for All ESS",
                "unit": "%"
            },
            "soc_avg_all": {
                "name": "Average State of Charge for All ESS",
                "unit": "%"
            },
            "soc_min_running": {
                "name": "Minimum State of Charge for Running ESS",
                "unit": "%"
            },
            "soc_max_running": {
                "name": "Maximum State of Charge for Running ESS",
                "unit": "%"
            },
            "soc_avg_running": {
                "name": "Average State of Charge for Running ESS",
                "unit": "%"
            },
            "available_features_runmode2_kW_mode": {
                "name": "Available Feature Modes - Run Mode 2 Active Power",
                "ui_type": "none",
                "type": "string",
                "var_type": "String",
                "value": "0x2",
                "options": [
                    {
                        "name": "Generator Charge",
                        "value": 0
                    },
                    {
                        "name": "Disabled",
                        "value": 1
                    }
                ]
            },
            "runmode2_kW_mode_cmd": {
                "name": "Mode Select - Run Mode 2 Active Power",
                "ui_type": "control",
                "type": "enum",
                "var_type": "Int",
                "value": 1
            },
            "runmode2_kW_mode_status": {
                "name": "Run Mode 2 Active Power Mode",
                "type": "string",
                "var_type": "String",
                "value": "Disabled"
            },
            "available_features_runmode2_kVAR_mode": {
                "name": "Available Feature Modes - Run Mode 2 Reactive Power",
                "ui_type": "none",
                "type": "string",
                "var_type": "String",
                "value": "0x1",
                "options": [
                    {
                        "name": "Disabled",
                        "value": 0
                    }
                ]
            },
            "runmode2_kVAR_mode_cmd": {
                "name": "Mode Select - Run Mode 2 Reactive Power",
                "ui_type": "control",
                "type": "enum",
                "var_type": "Int",
                "value": 0
            },
            "runmode2_kVAR_mode_status": {
                "name": "Run Mode 2 Reactive Power Mode",
                "type": "string",
                "ui_type": "none",
                "var_type": "String",
                "value": "Disabled"
            },
            "configured_primary": {
                "name": "Configured Primary Controller Status",
                "ui_type": "none",
                "type": "enum",
                "var_type": "Bool",
                "value": false
            }
        },
        "site": {
            "input_sources": [
                {
                    "name": "UI",
                    "uri_suffix": "ui",
                    "ui_type": "control",
                    "alt_ui_types": [],
                    "enabled": true
                }
            ],
            "configuration": {
                "input_source_status": {
                    "name": "Input Source Selection Status",
                    "ui_type": "none",
                    "type": "string",
                    "var_type": "String",
                    "value": "Unassigned"
                },
                "reserved_bool_1": {
                    "name": "Reserved Boolean 1",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_2": {
                    "name": "Reserved Boolean 2",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_3": {
                    "name": "Reserved Boolean 3",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_4": {
                    "name": "Reserved Boolean 4",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_5": {
                    "name": "Reserved Boolean 5",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_6": {
                    "name": "Reserved Boolean 6",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_7": {
                    "name": "Reserved Boolean 7",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_8": {
                    "name": "Reserved Boolean 8",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_9": {
                    "name": "Reserved Boolean 9",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_10": {
                    "name": "Reserved Boolean 10",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_11": {
                    "name": "Reserved Boolean 11",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_12": {
                    "name": "Reserved Boolean 12",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_13": {
                    "name": "Reserved Boolean 13",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_14": {
                    "name": "Reserved Boolean 14",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_15": {
                    "name": "Reserved Boolean 15",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_bool_16": {
                    "name": "Reserved Boolean 16",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "reserved_float_1": {
                    "name": "Reserved Float 1",
                    "ui_type": "none"
                },
                "reserved_float_2": {
                    "name": "Reserved Float 2",
                    "ui_type": "none"
                },
                "reserved_float_3": {
                    "name": "Reserved Float 3",
                    "ui_type": "none"
                },
                "reserved_float_4": {
                    "name": "Reserved Float 4",
                    "ui_type": "none"
                },
                "reserved_float_5": {
                    "name": "Reserved Float 5",
                    "ui_type": "none"
                },
                "reserved_float_6": {
                    "name": "Reserved Float 6",
                    "ui_type": "none"
                },
                "reserved_float_7": {
                    "name": "Reserved Float 7",
                    "ui_type": "none"
                },
                "reserved_float_8": {
                    "name": "Reserved Float 8",
                    "ui_type": "none"
                },
                "ess_kVAR_slew_rate": {
                    "name": "ESS kVAR Slew Rate",
                    "unit": "kVAR/s",
                    "ui_type": "none",
                    "var_type": "Int",
                    "value": 1210000
                },
                "gen_kVAR_slew_rate": {
                    "name": "Generator kVAR Slew Rate",
                    "unit": "kVAR/s",
                    "ui_type": "none",
                    "var_type": "Int",
                    "value": 1210000
                },
                "solar_kVAR_slew_rate": {
                    "name": "Solar kVAR Slew Rate",
                    "unit": "kVAR/s",
                    "ui_type": "none",
                    "var_type": "Int",
                    "value": 1210000
                },
                "power_priority_flag": {
                    "name": "Reactive Power Priority Flag",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "invert_poi_kW": {
                    "name": "POI Invert kW",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                }
            },
            "operation": {
                "enable_flag": {
                    "name": "Start Site",
                    "ui_type": "control",
                    "type": "enum_button",
                    "var_type": "Bool",
                    "value": false,
                    "options": [
                        {
                            "name": "Start Site",
                            "value": true
                        }
                    ]
                },
                "disable_flag": {
                    "name": "Disable Site",
                    "ui_type": "control",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false,
                    "options": [
                        {
                            "name": "False Case Not Used",
                            "value": false
                        },
                        {
                            "name": "Disable Site",
                            "value": true
                        }
                    ]
                },
                "standby_flag": {
                    "name": "Standby Site",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false,
                    "options": [
                        {
                            "name": "False Case Not Used",
                            "value": false
                        },
                        {
                            "name": "Standby Site",
                            "value": true
                        }
                    ]
                },
                "clear_faults_flag": {
                    "name": "Clear Faults",
                    "ui_type": "control",
                    "type": "enum_button",
                    "var_type": "Bool",
                    "value": false,
                    "options": [
                        {
                            "name": "Clear Faults",
                            "value": true
                        }
                    ]
                },
                "active_faults": {
                    "name": "Site Manager Active Faults",
                    "ui_type": "fault",
                    "type": "enum",
                    "var_type": "Int",
                    "value": 0
                },
                "active_alarms": {
                    "name": "Site Manager Active Alarm",
                    "ui_type": "alarm",
                    "type": "enum",
                    "var_type": "Int",
                    "value": 0
                },
                "site_state": {
                    "name": "Site Controller State",
                    "ui_type": "none",
                    "type": "string",
                    "var_type": "String",
                    "value": "Init"
                },
                "site_state_enum": {
                    "name": "Site Controller State Enum",
                    "ui_type": "none",
                    "type": "enum",
                    "var_type": "Int",
                    "value": 0
                },
                "site_status": {
                    "name": "Site Controller Status",
                    "type": "string",
                    "var_type": "String",
                    "value": "Default Init Path: Default Init Path"
                },
                "running_status_flag": {
                    "name": "Running Status",
                    "ui_type": "none",
                    "type": "enum",
                    "var_type": "Bool",
                    "value": false
                },
                "alarm_status_flag": {
                    "name": "Alarm Status",
                    "ui_type": "none",
                    "type": "enum",
                    "var_type": "Bool",
                    "value": false
                },
                "fault_status_flag": {
                    "name": "Fault Status",
                    "ui_type": "none",
                    "type": "enum",
                    "var_type": "Bool",
                    "value": false
                }
            },
            "summary": {
                "ess_instant_discharge": {
                    "name": "ESS Instant Discharge",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "ess_instant_charge_grid": {
                    "name": "ESS Instant Charge from Grid",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "ess_instant_charge_pv": {
                    "name": "ESS Instant Charge from Solar",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                }
            }
        },
        "features": {
            "active_power": {
                "ess_kW_cmd": {
                    "name": "ESS Active Power Setpoint",
                    "unit": "kW",
                    "scaler": 1
                },
                "gen_kW_cmd": {
                    "name": "Generator Active Power Setpoint",
                    "ui_type": "status",
                    "unit": "kW",
                    "scaler": 1
                },
                "solar_kW_cmd": {
                    "name": "Solar Active Power Setpoint",
                    "unit": "kW",
                    "scaler": 1
                },
                "feeder_kW_cmd": {
                    "name": "POI Active Power Setpoint",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "site_kW_demand": {
                    "name": "Site Demand Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "feature_kW_demand": {
                    "name": "Feature Demand Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "site_kW_charge_production": {
                    "name": "Charge Production Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "site_kW_discharge_production": {
                    "name": "Discharge Production Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "total_site_kW_rated_charge": {
                    "name": "Total Rated Charge Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "total_site_kW_rated_discharge": {
                    "name": "Total Rated Discharge Active Power",
                    "unit": "kW",
                    "ui_type": "none",
                    "scaler": 1
                },
                "total_site_kW_charge_limit": {
                    "name": "Minimum Total Site Charge",
                    "unit": "kW"
                },
                "total_site_kW_discharge_limit": {
                    "name": "Maximum Total Site Discharge",
                    "unit": "kW"
                },
                "site_kW_load": {
                    "name": "Site Load Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "site_kW_load_interval_ms": {
                    "name": "Duration of Site Load Rolling Average",
                    "ui_type": "none",
                    "var_type": "Int",
                    "unit": "ms",
                    "value": 5000,
                    "scaler": 1
                },
                "site_kW_load_inclusion": {
                    "name": "Site Load included in Feature",
                    "ui_type": "none",
                    "type": "enum_slider",
                    "var_type": "Bool",
                    "value": false
                },
                "ess_actual_kW": {
                    "name": "ESS Actual Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "gen_actual_kW": {
                    "name": "Generator Actual Active Power",
                    "ui_type": "status",
                    "unit": "kW",
                    "scaler": 1
                },
                "solar_actual_kW": {
                    "name": "Solar Actual Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "feeder_actual_kW": {
                    "name": "POI Actual Active Power",
                    "unit": "kW",
                    "scaler": 1
                },
                "max_potential_ess_kW": {
                    "name": "Max ESS Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "min_potential_ess_kW": {
                    "name": "Min ESS Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "rated_ess_kW": {
                    "name": "ESS Rated Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "max_potential_gen_kW": {
                    "name": "Max Generator Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "min_potential_gen_kW": {
                    "name": "Min Generator Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "rated_gen_kW": {
                    "name": "Gen Rated Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "max_potential_solar_kW": {
                    "name": "Max Solar Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "min_potential_solar_kW": {
                    "name": "Min Solar Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "rated_solar_kW": {
                    "name": "Solar Rated Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "max_potential_feeder_kW": {
                    "name": "Max POI Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "min_potential_feeder_kW": {
                    "name": "Min POI Potential Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "rated_feeder_kW": {
                    "name": "Feeder Rated Active Power",
                    "ui_type": "none",
                    "unit": "kW",
                    "scaler": 1
                },
                "runmode1_kW_mode_status": {
                    "name": "Run Mode 1 Active Power Mode",
                    "type": "string",
                    "var_type": "String",
                    "value": "Disabled"
                },
                "site_frequency": {
                    "name": "Site Frequency",
                    "unit": "Hz"
                }
            },
            "reactive_power": {
                "runmode1_kVAR_mode_status": {
                    "name": "Run Mode 1 Reactive Power Mode",
                    "type": "string",
                    "ui_type": "none",
                    "var_type": "String",
                    "value": "Disabled"
                },
                "ess_kVAR_cmd": {
                    "name": "ESS Reactive Power Setpoint",
                    "ui_type": "status",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "gen_kVAR_cmd": {
                    "name": "Generator Reactive Power Setpoint",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "solar_kVAR_cmd": {
                    "name": "Solar Reactive Power Setpoint",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "site_kVAR_demand": {
                    "name": "Site Demand Reactive Power",
                    "ui_type": "status",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "ess_actual_kVAR": {
                    "name": "ESS Actual Reactive Power",
                    "ui_type": "status",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "gen_actual_kVAR": {
                    "name": "Generator Actual Reactive Power",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "solar_actual_kVAR": {
                    "name": "Solar Actual Reactive Power",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "feeder_actual_kVAR": {
                    "name": "POI Actual Reactive Power",
                    "ui_type": "status",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "feeder_actual_pf": {
                    "name": "POI Actual Power Factor"
                },
                "potential_ess_kVAR": {
                    "name": "ESS Potential Reactive Power",
                    "ui_type": "status",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "potential_gen_kVAR": {
                    "name": "Generator Potential Reactive Power",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                },
                "potential_solar_kVAR": {
                    "name": "Solar Potential Reactive Power",
                    "ui_type": "none",
                    "unit": "VAR",
                    "scaler": 1000
                }
            },
            "standalone_power": {
                "start_first_gen_soc": {
                    "name": "First Generator Start Minimum SOC",
                    "ui_type": "control",
                    "unit": "%",
                    "value": 0
                }
            }
        }
    }
}
)<raw-string>";
