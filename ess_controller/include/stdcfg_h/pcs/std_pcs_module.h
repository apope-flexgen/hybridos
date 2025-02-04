const char* std_pcs_module_s = R"JSON(
{
    "/config/##PCS_ID##_##MODULE_ID##": {
        "AlarmDestination": "/assets/##PCS_ID##_##MODULE_ID##:alarms",
        "FaultDestination": "/assets/##PCS_ID##_##MODULE_ID##:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal",
        "enable": false
    },

    "/vlinks/##PCS_ID##_##MODULE_ID##": {
        "ui_##PCS_ID##_##MODULE_ID##_l1_current"           : { "value": "/assets/##PCS_ID##/##MODULE_ID##:l1_current"           , "vlink": "/status/##PCS_ID##_##MODULE_ID##:L1Current"          },
        "ui_##PCS_ID##_##MODULE_ID##_l2_current"           : { "value": "/assets/##PCS_ID##/##MODULE_ID##:l2_current"           , "vlink": "/status/##PCS_ID##_##MODULE_ID##:L2Current"          },
        "ui_##PCS_ID##_##MODULE_ID##_l3_current"           : { "value": "/assets/##PCS_ID##/##MODULE_ID##:l3_current"           , "vlink": "/status/##PCS_ID##_##MODULE_ID##:L3Current"          },
        "ui_##PCS_ID##_##MODULE_ID##_active_power"         : { "value": "/assets/##PCS_ID##/##MODULE_ID##:active_power"         , "vlink": "/status/##PCS_ID##_##MODULE_ID##:ActivePower"        },
        "ui_##PCS_ID##_##MODULE_ID##_reactive_power"       : { "value": "/assets/##PCS_ID##/##MODULE_ID##:reactive_power"       , "vlink": "/status/##PCS_ID##_##MODULE_ID##:ReactivePower"      },
        "ui_##PCS_ID##_##MODULE_ID##_apparent_power"       : { "value": "/assets/##PCS_ID##/##MODULE_ID##:apparent_power"       , "vlink": "/status/##PCS_ID##_##MODULE_ID##:ApparentPower"      },
        "ui_##PCS_ID##_##MODULE_ID##_pcs_dc_voltage"       : { "value": "/assets/##PCS_ID##/##MODULE_ID##:pcs_dc_voltage"       , "vlink": "/status/##PCS_ID##_##MODULE_ID##:PCSDCVoltage"       },
        "ui_##PCS_ID##_##MODULE_ID##_pcs_dc_current"       : { "value": "/assets/##PCS_ID##/##MODULE_ID##:pcs_dc_current"       , "vlink": "/status/##PCS_ID##_##MODULE_ID##:PCSDCCurrent"       },
        "ui_##PCS_ID##_##MODULE_ID##_pcs_dc_power"         : { "value": "/assets/##PCS_ID##/##MODULE_ID##:pcs_dc_power"         , "vlink": "/status/##PCS_ID##_##MODULE_ID##:PCSDCPower"         },
        "ui_##PCS_ID##_##MODULE_ID##_max_igbt_temperature" : { "value": "/assets/##PCS_ID##/##MODULE_ID##:max_igbt_temperature" , "vlink": "/status/##PCS_ID##_##MODULE_ID##:MaxIGBTTemperature" }
    },

    "/system/commands": {
        "runModuleOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/##PCS_ID##_##MODULE_ID##:pubAssets_##PCS_ID##_##MODULE_ID##"  , "aname":"##PCS_ID##_##MODULE_ID##", "value":0, "every":1   },
                {"uri":"/sched/##PCS_ID##_##MODULE_ID##:runMonitor_##PCS_ID##_##MODULE_ID##" , "aname":"##PCS_ID##_##MODULE_ID##", "value":0, "every":0.1 }
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/##PCS_ID##_##MODULE_ID##": {
        "pubAssets_##PCS_ID##_##MODULE_ID##": {
            "value": 0,
            "enable": "/config/pcs:enable",
            "mode": "ui",
            "table": "/assets/##PCS_ID##/##MODULE_ID##",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_##PCS_ID##_##MODULE_ID##": {
            "value":0,
            "enable": "/config/pcs:enable",
            "monitor":"wake_monitor",
            "aname":"##PCS_ID##_##MODULE_ID##",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/schedule/wake_monitor/##PCS_ID##_##MODULE_ID##":{
        
    },

    "/status/##PCS_ID##_##MODULE_ID##": {
        "L1Current"          : -1.0 ,
        "L2Current"          : -1.0 ,
        "L3Current"          : -1.0 ,
        "ActivePower"        : -1.0 ,
        "ReactivePower"      : -1.0 ,
        "ApparentPower"      : -1.0 ,
        "PCSDCVoltage"       : -1.0 ,
        "PCSDCCurrent"       : -1.0 ,
        "PCSDCPower"         : -1.0 ,
        "MaxIGBTTemperature" : -1.0
    }
}
)JSON";
