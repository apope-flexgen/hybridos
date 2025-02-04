const char* std_ess_controller_manager_s = R"JSON(
{
    "/config/ess": {
        "FaultDestination": "/assets/ess/summary:faults",
        "AlarmDestination": "/assets/ess/summary:alarms",
        "EventSourceFormat": {"value": "assetName"},
        "LogDir": "/var/log/flexgen/ess_controller",
        "logging_enabled": true,
        "NoAlarmMsg": "Normal",
        "NoFaultMsg": "Normal",
        "site_control_enable": false,
        "enable": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [{"uri": "/config/ess:site_control_enable"}]
                }]
            }
        }
        
    },

    "/assets/ess/summary": {
        "alarms": {"name": "Alarms", "value": 0, "options":[],"enabled":true},
        "faults": {"name": "Faults", "value": 0, "options":[],"enabled":true},
        "site_controller_com_status" : "N/A",
        "site_controller_heartbeat"  : -1   ,
        "site_controller_lockout"    : false,
        "max_apparent_power"         : -1.0 ,
        "max_charge_power"           : -1.0 ,
        "max_discharge_power"        : -1.0 ,
        "max_reactive_power"         : -1.0 ,
        "build"                      : "N/A",
        "time"                       : "N/A",
        "cpu_temp"                   : -1.0 ,
        "curr_real_mem"              : -1.0 ,
        "peak_real_mem"              : -1.0 ,
        "site_control_enable"        : false,
        "fire_fault"                 : false,
        "door_alarm"                 : false,
        "e_stop_fault"               : false,
        "clear_faults": {
            "enable": "/config/ess:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/ess:ClearFaults", "outValue": 1}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        }
    },

    "/system/commands": {
        "runESSOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                { "aname": "ess", "every": 1  , "uri": "/sched/ess:pubSiteStatus"  , "value": 1 },
                { "aname": "ess", "every": 1  , "uri": "/sched/ess:pubSiteAlarms"  , "value": 1 },
                { "aname": "ess", "every": 1  , "uri": "/sched/ess:pubSiteFaults"  , "value": 1 },
                { "aname": "ess", "every": 1  , "uri": "/sched/ess:pubAssets_ess"  , "value": 1 },
                { "aname": "ess", "every": 0.1, "uri": "/sched/ess:runMonitor_ess" , "value": 1 },
                { "aname": "ess", "every": 0.1, "uri": "/sched/ess:pubESS"         , "value": 1 },
                { "aname": "ess", "every": 0.1, "uri": "/sched/ess:every100ms_ess" , "value": 1 },
                { "aname": "ess", "every": 1  , "uri": "/sched/ess:every1000ms_ess", "value": 1 }
            ],
            "actions": {
                "onSet": [{"func": [{"func": "SchedItemOpts"}]}]
            }
        }
    },

    "/sched/ess": {
        "pubSiteStatus": {
            "value": 0,
            "enable": "/config/ess:enable",
            "mode": "naked",
            "table": "/status/site",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "pubSiteAlarms": {
            "value": 0,
            "enable": "/config/ess:enable",
            "mode": "naked",
            "table": "/alarms/site",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "pubSiteFaults": {
            "value": 0,
            "enable": "/config/ess:enable",
            "mode": "naked",
            "table": "/faults/site",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "pubAssets_ess": {
            "value": 0,
            "note": "Publish /assets/ess/summary to display UI data every second",
            "enable": "/config/ess:enable",
            "mode": "ui",
            "table": "/assets/ess/summary",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_ess": {
            "value":1,
            "note": "Run list of functions defined in /schedule/wake_monitor/ess every 100ms",
            "enable": "/config/ess:enable",
            "monitor":"wake_monitor",
            "aname":"ess",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        },
        "every100ms_ess":{
            "value":1,
            "note": "Run power command handler and update system time every 100ms",
            "enable": "/config/ess:enable",
            "actions":{
                "onSet":[{
                    "func":[
                        {"func":"HandlePowerCmd"},
                        {"func":"UpdateSysTime"}
                    ]
                }]
            }
        },
        "every1000ms_ess":{
            "value":1,
            "note": "Check CPU statistics every second",
            "enable": "/config/ess:enable",
            "actions":{
                "onSet":[{"func":[{"func":"Every1000mS"}]}]
            }
        }
    },

    "/vlinks/ess": {
        "ui_site_controller_heartbeat"  : { "value": "/assets/ess/summary:site_controller_heartbeat"  , "vlink": "/controls/ess:SiteControllerHeartbeat" },
        "ui_site_controller_com_status" : { "value": "/assets/ess/summary:site_controller_com_status" , "vlink": "/status/ess:CommsOKStatus"             },
        "ui_site_controller_lockout"    : { "value": "/assets/ess/summary:site_controller_lockout"    , "vlink": "/status/ess:SiteControllerLockout"     },
        "ui_max_apparent_power"         : { "value": "/assets/ess/summary:max_apparent_power"         , "vlink": "/limits/ess:MaxApparentPower"          },
        "ui_max_charge_power"           : { "value": "/assets/ess/summary:max_charge_power"           , "vlink": "/limits/ess:MaxChargePower"            },
        "ui_max_discharge_power"        : { "value": "/assets/ess/summary:max_discharge_power"        , "vlink": "/limits/ess:MaxDischargePower"         },
        "ui_max_reactive_power"         : { "value": "/assets/ess/summary:max_reactive_power"         , "vlink": "/limits/ess:MaxReactivePower"          },
        "ui_build"                      : { "value": "/assets/ess/summary:build"                      , "vlink": "/status/ess:build"                     },
        "ui_time"                       : { "value": "/assets/ess/summary:time"                       , "vlink": "/status/ess:timeString"                },
        "ui_cpu_temp"                   : { "value": "/assets/ess/summary:cpu_temp"                   , "vlink": "/status/ess:system_temp"               },
        "ui_curr_real_mem"              : { "value": "/assets/ess/summary:curr_real_mem"              , "vlink": "/status/ess:currRealMem"               },
        "ui_peak_real_mem"              : { "value": "/assets/ess/summary:peak_real_mem"              , "vlink": "/status/ess:peakRealMem"               },
        "ui_fire_fault"                 : { "value": "/assets/ess/summary:fire_fault"                 , "vlink": "/status/ess:FireFault"                 },
        "ui_door_alarm"                 : { "value": "/assets/ess/summary:door_alarm"                 , "vlink": "/status/ess:DoorAlarm"                 },
        "ui_e_stop_fault"               : { "value": "/assets/ess/summary:e_stop_fault"               , "vlink": "/status/ess:EStopFault"                }
    },

    "/schedule/wake_monitor/ess":{
        "/status/ess:IsFaulted"             : {"func": "CalculateVar" },
        "/status/ess:IsAlarming"            : {"func": "CalculateVar" },
        "/status/ess:SiteControllerLockout" : {"func": "CalculateVar" }
    },

    "/status/ess": {
        "CommsOKStatus" : "N/A",
        "timeString"    : "N/A",
        "system_temp"   : -1.0 ,
        "currRealMem"   : -1.0 ,
        "peakRealMem"   : -1.0 ,
        "FireFault"     : false,
        "DoorAlarm"     : false,
        "EStopFault"    : false,
        "CommsOK": {
            "value": false,
            "actions": {
                "onSet":[{
                    "remap":[
                        { "inValue": true , "ifChanged": false, "uri": "/status/ess:CommsOKStatus", "outValue": "Online" },
                        { "inValue": false, "ifChanged": false, "uri": "/status/ess:CommsOKStatus", "outValue": "Offline"},

                        {"inValue": true , "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[0]", "outValue": false},
                        {"inValue": false, "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[0]", "outValue": true }
                    ]
                }]
            }
        },
        "IsFaulted": {
            "value": false,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/pcs:IsFaulted",
            "variable2": "/status/bms:IsFaulted",
            "variable3": "/assets/ess/summary:faults",
            "expression": "{1} or {2} or {3}"
        },
        "IsAlarming": {
            "value": false,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/pcs:IsAlarming",
            "variable2": "/status/bms:IsAlarming",
            "variable3": "/assets/ess/summary:alarms",
            "expression": "{1} or {2} or {3}"
        },
        "SiteControllerLockout": {
            "value": false,
            "enable": "/config/ess:enable",
            "numVars": 1,
            "variable1": "ess:MaintMode",
            "operation": "or",
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"inValue": true, "outValue": false, "uri": "/config/ess:site_control_enable"},
                        {"inValue": false, "outValue": true, "uri": "/config/ess:site_control_enable"}
                    ]
                }]
            }
        }
    }
}
)JSON";
