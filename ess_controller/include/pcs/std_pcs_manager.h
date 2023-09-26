#pragma once

const char* std_pcs_manager_s = R"JSON(
{
    "/config/##PCS_ID##": {
        "AlarmDestination": "/assets/pcs/##PCS_ID##:alarms",
        "FaultDestination": "/assets/pcs/##PCS_ID##:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal",
        "enable": false
    },

    "/assets/pcs/##PCS_ID##": {
        "alarms": {"name": "Alarms", "value": 0, "options":[],"enabled":true},
        "faults": {"name": "Faults", "value": 0, "options":[],"enabled":true}
    },

    "/vlinks/##PCS_ID##": {
        "ui_##PCS_ID##_l1_l2_voltage"        : { "value": "/assets/pcs/##PCS_ID##:l1_l2_voltage"        , "vlink": "/status/##PCS_ID##:L1L2Voltage"        },
        "ui_##PCS_ID##_l2_l3_voltage"        : { "value": "/assets/pcs/##PCS_ID##:l2_l3_voltage"        , "vlink": "/status/##PCS_ID##:L2L3Voltage"        },
        "ui_##PCS_ID##_l3_l1_voltage"        : { "value": "/assets/pcs/##PCS_ID##:l3_l1_voltage"        , "vlink": "/status/##PCS_ID##:L3L1Voltage"        },
        "ui_##PCS_ID##_l1_current"           : { "value": "/assets/pcs/##PCS_ID##:l1_current"           , "vlink": "/status/##PCS_ID##:L1Current"          },
        "ui_##PCS_ID##_l2_current"           : { "value": "/assets/pcs/##PCS_ID##:l2_current"           , "vlink": "/status/##PCS_ID##:L2Current"          },
        "ui_##PCS_ID##_l3_current"           : { "value": "/assets/pcs/##PCS_ID##:l3_current"           , "vlink": "/status/##PCS_ID##:L3Current"          },
        "ui_##PCS_ID##_power_factor"         : { "value": "/assets/pcs/##PCS_ID##:power_factor"         , "vlink": "/status/##PCS_ID##:PowerFactor"        },
        "ui_##PCS_ID##_frequency"            : { "value": "/assets/pcs/##PCS_ID##:frequency"            , "vlink": "/status/##PCS_ID##:Frequency"          },
        "ui_##PCS_ID##_active_power"         : { "value": "/assets/pcs/##PCS_ID##:active_power"         , "vlink": "/status/##PCS_ID##:ActivePower"        },
        "ui_##PCS_ID##_reactive_power"       : { "value": "/assets/pcs/##PCS_ID##:reactive_power"       , "vlink": "/status/##PCS_ID##:ReactivePower"      },
        "ui_##PCS_ID##_apparent_power"       : { "value": "/assets/pcs/##PCS_ID##:apparent_power"       , "vlink": "/status/##PCS_ID##:ApparentPower"      },
        "ui_##PCS_ID##_pcs_dc_voltage"       : { "value": "/assets/pcs/##PCS_ID##:pcs_dc_voltage"       , "vlink": "/status/##PCS_ID##:PCSDCVoltage"       },
        "ui_##PCS_ID##_pcs_dc_current"       : { "value": "/assets/pcs/##PCS_ID##:pcs_dc_current"       , "vlink": "/status/##PCS_ID##:PCSDCCurrent"       },
        "ui_##PCS_ID##_pcs_dc_power"         : { "value": "/assets/pcs/##PCS_ID##:pcs_dc_power"         , "vlink": "/status/##PCS_ID##:PCSDCPower"         },
        "ui_##PCS_ID##_max_igbt_temperature" : { "value": "/assets/pcs/##PCS_ID##:max_igbt_temperature" , "vlink": "/status/##PCS_ID##:MaxIGBTTemperature" },
        "ui_##PCS_ID##_modules_online"       : { "value": "/assets/pcs/##PCS_ID##:modules_online"       , "vlink": "/status/##PCS_ID##:ModulesOnline"      },
        "ui_##PCS_ID##_modules_available"    : { "value": "/assets/pcs/##PCS_ID##:modules_available"    , "vlink": "/status/##PCS_ID##:ModulesAvailable"   }
    },

    "/system/commands": {
        "runPCSOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/##PCS_ID##:pubAssets_##PCS_ID##" , "aname":"##PCS_ID##", "value":0, "every":1  },
                {"uri":"/sched/##PCS_ID##:runMonitor_##PCS_ID##", "aname":"##PCS_ID##", "value":0, "every":0.1}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/##PCS_ID##": {
        "pubAssets_##PCS_ID##": {
            "value": 1,
            "enable": "/config/pcs:enable",
            "mode": "ui",
            "table": "/assets/pcs/##PCS_ID##",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_##PCS_ID##": {
            "value":1,
            "enable": "/config/pcs:enable",
            "monitor":"wake_monitor",
            "aname":"##PCS_ID##",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/schedule/wake_monitor/##PCS_ID##":{
        "/status/##PCS_ID##:IsFaulted"  : {"func": "CalculateVar"},
        "/status/##PCS_ID##:IsAlarming" : {"func": "CalculateVar"}
    },

    "/status/##PCS_ID##": {
        "L1L2Voltage"        : -1.0 ,
        "L2L3Voltage"        : -1.0 ,
        "L3L1Voltage"        : -1.0 ,
        "L1Current"          : -1.0 ,
        "L2Current"          : -1.0 ,
        "L3Current"          : -1.0 ,
        "PowerFactor"        : -1.0 ,
        "Frequency"          : -1.0 ,
        "ActivePower"        : -1.0 ,
        "ReactivePower"      : -1.0 ,
        "ApparentPower"      : -1.0 ,
        "PCSDCVoltage"       : -1.0 ,
        "PCSDCCurrent"       : -1.0 ,
        "PCSDCPower"         : -1.0 ,
        "MaxIGBTTemperature" : -1.0 ,
        "ModulesOnline"      : -1   ,
        "ModulesAvailable"   : -1   ,
        "FaultShutdown": {
            "value": false,
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": true, "ifChanged": false, "uri":"/status/pcs:FaultShutdown", "outValue": true}
                    ]
                }]
            }
        },
        "IsFaulted": {
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/assets/pcs/##PCS_ID##:faults",
            "expression": "{1} == 1"
        },
        "IsAlarming": {
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/assets/pcs/##PCS_ID##:alarms",
            "expression": "{1} == 1"
        }
    }
}
)JSON";