const char* std_v_pcs_manager_s = R"JSON(
{
    "/config/pcs": {
        "AlarmDestination"       : "/assets/pcs/summary:alarms",
        "FaultDestination"       : "/assets/pcs/summary:faults",
        "NoFaultMsg"             : "Normal",
        "NoAlarmMsg"             : "Normal",
        "enable"                 : false,
        "RatedActivePower"       : 0,
        "RatedReactivePower"     : 0,
        "RatedApparentPower"     : 0,
        "RatedPowerFactor"       : 0,

        "MaxActivePowerSetpoint" : 0,
        "MaxChargePower"         : 0,
        "MaxDischargePower"      : 0,

        "RampRateMargin"         : 5,
        "PowerMargin"            : 5,
        "OffGridFrequencyMargin" : 0.1,
        "OffGridVoltageMargin"   : 5
    },

    "/assets/pcs/summary": {
        "alarms": {"name": "Alarms", "value": 0, "options":[],"enabled":true},
        "faults": {"name": "Faults", "value": 0, "options":[],"enabled":true},
        "maint_mode": {
            "enable": "/config/pcs:enable",
            "name": "Maintenance Mode",
            "value": false,
            "enabled": true,
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "start": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/pcs:Start@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "standby": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/pcs:Standby@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "stop": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:Stop@triggerCmd"  , "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:ActivePowerSetpoint"   , "outValue": 0    },
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:ReactivePowerSetpoint" , "outValue": 0    }
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "grid_follow_pq": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/pcs:GridFollowPQ@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "grid_form_vf": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/pcs:GridFormVF@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "grid_form_vsg": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/pcs:GridFormVSG@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "active_power_setpoint": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"ifChanged": false, "uri": "/controls/pcs:ActivePowerSetpoint"}]
                }]
            },
            "options": []
        },
        "reactive_power_setpoint": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"ifChanged": false, "uri": "/controls/pcs:ReactivePowerSetpoint"}]
                }]
            },
            "options": []
        },
        "active_power_ramp_rate": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerRampRate"},
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerRampRate@triggerCmd", "outValue": true}
                    ]
                }]
            },
            "options": []
        },
        "reactive_power_ramp_rate": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerRampRate"},
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerRampRate@triggerCmd", "outValue": true}
                    ]
                }]
            },
            "options": []
        },
        "off_grid_frequency_setpoint": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "enabled": true,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridFrequencySetpoint"},
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridFrequencySetpoint@triggerCmd", "outValue": true}
                    ]
                }]
            },
            "options": []
        },
        "off_grid_voltage_setpoint": {
            "enable": "/config/pcs:enable",
            "value": 0,
            "enabled": true,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridVoltageSetpoint"},
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridVoltageSetpoint@triggerCmd", "outValue": true}
                    ]
                }]
            },
            "options": []
        },
        "clear_faults": {
            "enable": "/config/pcs:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [
                        {"func": "LogInfo"}
                    ],
                    "remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:ClearFaults@triggerCmd", "outValue": true}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        }
    },

    "/vlinks/pcs": {
        "ui_comms_ok_status"      : { "value": "/assets/pcs/summary:comms_ok_status"      , "vlink": "/status/pcs:CommsOKStatus"      },
        "ui_system_state_status"  : { "value": "/assets/pcs/summary:system_state_status"  , "vlink": "/status/pcs:SystemStateStatus"  },
        "ui_grid_mode_status"     : { "value": "/assets/pcs/summary:grid_mode_status"     , "vlink": "/status/pcs:GridModeStatus"     },
        "ui_l1_l2_voltage"        : { "value": "/assets/pcs/summary:l1_l2_voltage"        , "vlink": "/status/pcs:L1L2Voltage"        },
        "ui_l2_l3_voltage"        : { "value": "/assets/pcs/summary:l2_l3_voltage"        , "vlink": "/status/pcs:L2L3Voltage"        },
        "ui_l3_l1_voltage"        : { "value": "/assets/pcs/summary:l3_l1_voltage"        , "vlink": "/status/pcs:L3L1Voltage"        },
        "ui_l1_current"           : { "value": "/assets/pcs/summary:l1_current"           , "vlink": "/status/pcs:L1Current"          },
        "ui_l2_current"           : { "value": "/assets/pcs/summary:l2_current"           , "vlink": "/status/pcs:L2Current"          },
        "ui_l3_current"           : { "value": "/assets/pcs/summary:l3_current"           , "vlink": "/status/pcs:L3Current"          },
        "ui_power_factor"         : { "value": "/assets/pcs/summary:power_factor"         , "vlink": "/status/pcs:PowerFactor"        },
        "ui_frequency"            : { "value": "/assets/pcs/summary:frequency"            , "vlink": "/status/pcs:Frequency"          },
        "ui_active_power"         : { "value": "/assets/pcs/summary:active_power"         , "vlink": "/status/pcs:ActivePower"        },
        "ui_reactive_power"       : { "value": "/assets/pcs/summary:reactive_power"       , "vlink": "/status/pcs:ReactivePower"      },
        "ui_apparent_power"       : { "value": "/assets/pcs/summary:apparent_power"       , "vlink": "/status/pcs:ApparentPower"      },
        "ui_pcs_dc_voltage"       : { "value": "/assets/pcs/summary:pcs_dc_voltage"       , "vlink": "/status/pcs:PCSDCVoltage"       },
        "ui_pcs_dc_current"       : { "value": "/assets/pcs/summary:pcs_dc_current"       , "vlink": "/status/pcs:PCSDCCurrent"       },
        "ui_pcs_dc_power"         : { "value": "/assets/pcs/summary:pcs_dc_power"         , "vlink": "/status/pcs:PCSDCPower"         },
        "ui_max_igbt_temperature" : { "value": "/assets/pcs/summary:max_igbt_temperature" , "vlink": "/status/pcs:MaxIGBTTemperature" },
        "ui_modules_online"       : { "value": "/assets/pcs/summary:modules_online"       , "vlink": "/status/pcs:ModulesOnline"      },
        "ui_modules_available"    : { "value": "/assets/pcs/summary:modules_available"    , "vlink": "/status/pcs:ModulesAvailable"   },
        "ui_maint_mode"           : { "value": "/assets/pcs/summary:maint_mode"           , "vlink": "/status/pcs:MaintMode"          },

        "ui_active_power_setpoint_feedback"             : { "value": "/assets/pcs/summary:active_power_setpoint_feedback"             , "vlink": "/status/pcs:ActivePowerSetpointFeedback"           },
        "ui_reactive_power_setpoint_feedback"           : { "value": "/assets/pcs/summary:reactive_power_setpoint_feedback"           , "vlink": "/status/pcs:ReactivePowerSetpointFeedback"         },
        "ui_active_power_ramp_rate_setpoint_feedback"   : { "value": "/assets/pcs/summary:active_power_ramp_rate_setpoint_feedback"   , "vlink": "/status/pcs:ActivePowerRampRateSetpointFeedback"   },
        "ui_reactive_power_ramp_rate_setpoint_feedback" : { "value": "/assets/pcs/summary:reactive_power_ramp_rate_setpoint_feedback" , "vlink": "/status/pcs:ReactivePowerRampRateSetpointFeedback" },
        "ui_off_grid_frequency_setpoint_feedback"       : { "value": "/assets/pcs/summary:off_grid_frequency_setpoint_feedback"       , "vlink": "/status/pcs:OffGridFrequencySetpointFeedback"      },
        "ui_off_grid_voltage_setpoint_feedback"         : { "value": "/assets/pcs/summary:off_grid_voltage_setpoint_feedback"         , "vlink": "/status/pcs:OffGridVoltageSetpointFeedback"        },

        "ui_rated_active_power"   : { "value": "/assets/pcs/summary:rated_active_power"   , "vlink": "/limits/pcs:RatedActivePower"   },
        "ui_rated_reactive_power" : { "value": "/assets/pcs/summary:rated_reactive_power" , "vlink": "/limits/pcs:RatedReactivePower" },
        "ui_rated_apparent_power" : { "value": "/assets/pcs/summary:rated_apparent_power" , "vlink": "/limits/pcs:RatedApparentPower" },
        "ui_rated_power_factor"   : { "value": "/assets/pcs/summary:rated_power_factor"   , "vlink": "/limits/pcs:RatedPowerFactor"   },

        "ui_max_active_power_setpoint" : { "value": "/assets/pcs/summary:max_active_power_setpoint" , "vlink": "/config/pcs:MaxActivePowerSetpoint" },
        "ui_max_charge_power"          : { "value": "/assets/pcs/summary:max_charge_power"          , "vlink": "/config/pcs:MaxChargePower"         },
        "ui_max_discharge_power"       : { "value": "/assets/pcs/summary:max_discharge_power"       , "vlink": "/config/pcs:MaxDischargePower"      }
    },

    "/system/commands": {
        "runPCSOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/pcs:pubAssets_pcs",     "aname":"pcs", "value":0, "every":1},
                {"uri":"/sched/pcs:runMonitor_pcs",    "aname":"pcs", "value":0, "every":0.1}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/pcs": {
        "pubAssets_pcs": {
            "value": 0,
            "enable": "/config/pcs:enable",
            "mode": "ui",
            "table": "/assets/pcs/summary",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_pcs": {
            "value":0,
            "enable": "/config/pcs:enable",
            "monitor":"wake_monitor",
            "aname":"pcs",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/schedule/wake_monitor/pcs":{
        "/status/pcs:HeartbeatRead"   : {"func": "CheckMonitorVar"},

        "/status/pcs:ChildFaulted" : {"func": "CalculateVar"},
        "/status/pcs:ChildAlarming" : {"func": "CalculateVar"},
        "/status/pcs:IsFaulted"   : {"func": "CalculateVar"},
        "/status/pcs:IsAlarming"  : {"func": "CalculateVar"}
    },

    "/limits/pcs": {
        "RatedActivePower"   : 0,
        "RatedReactivePower" : 0,
        "RatedApparentPower" : 0
    },

    "/status/pcs": {
        "MaintMode"                             : false ,
        "CommsOKStatus"                         : "N/A" ,
        "SystemStateStatus"                     : "N/A" ,
        "GridModeStatus"                        : "N/A" ,
        "L1L2Voltage"                           : -1.0  ,
        "L2L3Voltage"                           : -1.0  ,
        "L3L1Voltage"                           : -1.0  ,
        "L1Current"                             : -1.0  ,
        "L2Current"                             : -1.0  ,
        "L3Current"                             : -1.0  ,
        "PowerFactor"                           : -1.0  ,
        "Frequency"                             : -1.0  ,
        "ActivePower"                           : -1.0  ,
        "ReactivePower"                         : -1.0  ,
        "ApparentPower"                         : -1.0  ,
        "PCSDCVoltage"                          : -1.0  ,
        "PCSDCCurrent"                          : -1.0  ,
        "PCSDCPower"                            : -1.0  ,
        "MaxIGBTTemperature"                    : -1.0  ,
        "ModulesOnline"                         : -1    ,
        "ModulesAvailable"                      : -1    ,
        "ActivePowerSetpointFeedback"           : -1.0  ,
        "ReactivePowerSetpointFeedback"         : -1.0  ,
        "ActivePowerRampRateSetpointFeedback"   : -1.0  ,
        "ReactivePowerRampRateSetpointFeedback" : -1.0  ,
        "OffGridFrequencySetpointFeedback"      : -1.0  ,
        "OffGridVoltageSetpointFeedback"        : -1.0  ,
        "ChildFaulted" : { "value": -1, "numVars": 1, "variable1": "pcs:IsFaulted" , "operation": "or" },
        "ChildAlarming": { "value": -1, "numVars": 1, "variable1": "pcs:IsAlarming", "operation": "or" },
        "IsFaulted": {
            "value": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/pcs:ChildFaulted",
            "variable2": "/assets/pcs/summary:faults",
            "expression": "{1} or {2}"
        },
        "IsAlarming": {
            "value": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/pcs:ChildAlarming",
            "variable2": "/assets/pcs/summary:alarms",
            "expression": "{1} or {2}"
        },
        "HeartbeatRead": {
            "value": 0,
            "EnableStateCheck": true,
            "EnableCommsCheck": true,
            "AlarmTimeout": 10,
            "FaultTimeout": 15,
            "RecoverTimeout": 0.1
        },
        "CommsOK": {
            "value": false,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": true , "ifChanged": false, "uri": "/status/pcs:CommsOKStatus", "outValue": "Online" },
                        {"inValue": false, "ifChanged": false, "uri": "/status/pcs:CommsOKStatus", "outValue": "Offline"},

                        {"inValue": true , "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[2]", "outValue": false},
                        {"inValue": false, "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[2]", "outValue": true }
                    ]
                }]
            }
        },
        "SystemState": {
            "value": -1,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/pcs:SystemStateStatus", "outValue": "Stop"    },
                        {"inValue": 2, "uri":"/status/pcs:SystemStateStatus", "outValue": "Standby"},
                        {"inValue": 3, "uri":"/status/pcs:SystemStateStatus", "outValue": "Run"},
                        {"inValue": 4, "uri":"/status/pcs:SystemStateStatus", "outValue": "Fault"  }
                    ]
                }]
            }
        },
        "GridMode": {
            "value": -1,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/pcs:GridModeStatus", "outValue": "FollowPQ"},
                        {"inValue": 2, "uri":"/status/pcs:GridModeStatus", "outValue": "FormVF"  },
                        {"inValue": 3, "uri":"/status/pcs:GridModeStatus", "outValue": "FormVSG" }
                    ]
                }]
            }
        }
    }
}
)JSON";
