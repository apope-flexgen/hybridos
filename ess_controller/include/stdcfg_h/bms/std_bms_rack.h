const char* std_bms_rack_s = R"JSON(
{
    "/config/##BMS_ID##_##RACK_ID##": {
        "AlarmDestination": "/assets/##BMS_ID##_##RACK_ID##:alarms",
        "FaultDestination": "/assets/##BMS_ID##_##RACK_ID##:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal",
        "enable": false
    },

    "/vlinks/##BMS_ID##_##RACK_ID##": {
        "ui_##BMS_ID##_##RACK_ID##_dc_closed_status"     : { "value": "/assets/##BMS_ID##/##RACK_ID##:dc_closed_status"     , "vlink": "/status/##BMS_ID##_##RACK_ID##:DCClosedStatus"      },
        "ui_##BMS_ID##_##RACK_ID##_soc"                  : { "value": "/assets/##BMS_ID##/##RACK_ID##:soc"                  , "vlink": "/status/##BMS_ID##_##RACK_ID##:SOC"                 },
        "ui_##BMS_ID##_##RACK_ID##_soh"                  : { "value": "/assets/##BMS_ID##/##RACK_ID##:soh"                  , "vlink": "/status/##BMS_ID##_##RACK_ID##:SOH"                 },
        "ui_##BMS_ID##_##RACK_ID##_dc_current"           : { "value": "/assets/##BMS_ID##/##RACK_ID##:dc_current"           , "vlink": "/status/##BMS_ID##_##RACK_ID##:DCCurrent"           },
        "ui_##BMS_ID##_##RACK_ID##_dc_voltage"           : { "value": "/assets/##BMS_ID##/##RACK_ID##:dc_voltage"           , "vlink": "/status/##BMS_ID##_##RACK_ID##:DCVoltage"           },
        "ui_##BMS_ID##_##RACK_ID##_dc_power"             : { "value": "/assets/##BMS_ID##/##RACK_ID##:dc_power"             , "vlink": "/status/##BMS_ID##_##RACK_ID##:DCPower"             },
        "ui_##BMS_ID##_##RACK_ID##_max_cell_temp"        : { "value": "/assets/##BMS_ID##/##RACK_ID##:max_cell_temp"        , "vlink": "/status/##BMS_ID##_##RACK_ID##:MaxCellTemp"         },
        "ui_##BMS_ID##_##RACK_ID##_avg_cell_temp"        : { "value": "/assets/##BMS_ID##/##RACK_ID##:avg_cell_temp"        , "vlink": "/status/##BMS_ID##_##RACK_ID##:AvgCellTemp"         },
        "ui_##BMS_ID##_##RACK_ID##_min_cell_temp"        : { "value": "/assets/##BMS_ID##/##RACK_ID##:min_cell_temp"        , "vlink": "/status/##BMS_ID##_##RACK_ID##:MinCellTemp"         },
        "ui_##BMS_ID##_##RACK_ID##_cell_temp_delta"      : { "value": "/assets/##BMS_ID##/##RACK_ID##:cell_temp_delta"      , "vlink": "/status/##BMS_ID##_##RACK_ID##:CellTempDelta"       },
        "ui_##BMS_ID##_##RACK_ID##_max_cell_voltage"     : { "value": "/assets/##BMS_ID##/##RACK_ID##:max_cell_voltage"     , "vlink": "/status/##BMS_ID##_##RACK_ID##:MaxCellVoltage"      },
        "ui_##BMS_ID##_##RACK_ID##_avg_cell_voltage"     : { "value": "/assets/##BMS_ID##/##RACK_ID##:avg_cell_voltage"     , "vlink": "/status/##BMS_ID##_##RACK_ID##:AvgCellVoltage"      },
        "ui_##BMS_ID##_##RACK_ID##_min_cell_voltage"     : { "value": "/assets/##BMS_ID##/##RACK_ID##:min_cell_voltage"     , "vlink": "/status/##BMS_ID##_##RACK_ID##:MinCellVoltage"      },
        "ui_##BMS_ID##_##RACK_ID##_cell_voltage_delta"   : { "value": "/assets/##BMS_ID##/##RACK_ID##:cell_voltage_delta"   , "vlink": "/status/##BMS_ID##_##RACK_ID##:CellVoltageDelta"    },
        
        "ui_charge_power_limit"    : { "value": "/assets/##BMS_ID##/##RACK_ID##:charge_power_limit"    , "vlink": "/limits/##BMS_ID##_##RACK_ID##:ChargePowerLimit"    },
        "ui_discharge_power_limit" : { "value": "/assets/##BMS_ID##/##RACK_ID##:discharge_power_limit" , "vlink": "/limits/##BMS_ID##_##RACK_ID##:DischargePowerLimit" }
    },

    "/system/commands": {
        "runRackOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/##BMS_ID##_##RACK_ID##:pubAssets_##BMS_ID##_##RACK_ID##" , "aname":"##BMS_ID##_##RACK_ID##", "value":0, "every":1  },
                {"uri":"/sched/##BMS_ID##_##RACK_ID##:runMonitor_##BMS_ID##_##RACK_ID##", "aname":"##BMS_ID##_##RACK_ID##", "value":0, "every":0.1}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/##BMS_ID##_##RACK_ID##": {
        "pubAssets_##BMS_ID##_##RACK_ID##": {
            "value": 0,
            "enable": "/config/bms:enable",
            "mode": "ui",
            "table": "/assets/##BMS_ID##/##RACK_ID##",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_##BMS_ID##_##RACK_ID##": {
            "value":0,
            "enable": "/config/bms:enable",
            "monitor":"wake_monitor",
            "aname":"##BMS_ID##_##RACK_ID##",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/schedule/wake_monitor/##BMS_ID##_##RACK_ID##":{
        "/status/##BMS_ID##_##RACK_ID##:CellTempDelta"   : {"func": "CalculateVar"},
        "/status/##BMS_ID##_##RACK_ID##:CellVoltageDelta": {"func": "CalculateVar"}
    },

    "/limits/##BMS_ID##_##RACK_ID##": {
        "ChargePowerLimit"     : 0,
        "DischargePowerLimit"  : 0
    },
    
    "/status/##BMS_ID##_##RACK_ID##": {
        "DCClosedStatus"      : "N/A",
        "SOC"                 : -1.0 ,
        "SOH"                 : -1.0 ,
        "DCCurrent"           : -1.0 ,
        "DCVoltage"           : -1.0 ,
        "DCPower"             : -1.0 ,
        "MaxCellTemp"         : -1.0 ,
        "AvgCellTemp"         : -1.0 ,
        "MinCellTemp"         : -1.0 ,
        "MaxCellVoltage"      : -1.0 ,
        "AvgCellVoltage"      : -1.0 ,
        "MinCellVoltage"      : -1.0 ,
        "DCClosed": {
            "value": -1,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/##BMS_ID##_##RACK_ID##:DCClosedStatus", "outValue": "Opened"},
                        {"inValue": 2, "uri":"/status/##BMS_ID##_##RACK_ID##:DCClosedStatus", "outValue": "Closed"}
                    ]
                }]
            }
        },
        "CellTempDelta": {
            "value": -1,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##_##RACK_ID##:MinCellTemp",
            "variable2": "/status/##BMS_ID##_##RACK_ID##:MaxCellTemp",
            "expression": "{2} - {1}"
        },
        "CellVoltageDelta": {
            "value": -1,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##_##RACK_ID##:MinCellVoltage",
            "variable2": "/status/##BMS_ID##_##RACK_ID##:MaxCellVoltage",
            "expression": "{2} - {1}"
        }
    }
}
)JSON";
