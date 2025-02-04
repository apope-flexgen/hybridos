const char* std_bms_manager_s = R"JSON(
{
    "/config/##BMS_ID##": {
        "AlarmDestination": "/assets/bms/##BMS_ID##:alarms",
        "FaultDestination": "/assets/bms/##BMS_ID##:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal",
        "enable": false
    },

    "/assets/bms/##BMS_ID##": {
        "alarms": {"name": "Alarms", "value": 0, "options":[],"enabled":true},
        "faults": {"name": "Faults", "value": 0, "options":[],"enabled":true},
        "maint_mode": {
            "enable": "/config/bms:enable",
            "name": "Maintenance Mode",
            "value": false,
            "enabled": true,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "close_contactors": {
            "enable": "/config/bms:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/##BMS_ID##:CloseContactors@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "open_contactors": {
            "enable": "/config/bms:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/##BMS_ID##:OpenContactors@triggerCmd", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        }
    },

    "/vlinks/##BMS_ID##": {
        "ui_comms_ok"                                 : { "value": "/assets/bms/##BMS_ID##:comms_ok_status"                          , "vlink": "/status/##BMS_ID##:CommsOKStatus"                       },
        "ui_dc_closed_status"                         : { "value": "/assets/bms/##BMS_ID##:dc_closed_status"                         , "vlink": "/status/##BMS_ID##:DCClosedStatus"                      },
        "ui_system_soc"                               : { "value": "/assets/bms/##BMS_ID##:soc"                                      , "vlink": "/status/##BMS_ID##:SOC"                                 },
        "ui_system_soh"                               : { "value": "/assets/bms/##BMS_ID##:soh"                                      , "vlink": "/status/##BMS_ID##:SOH"                                 },
        "ui_chargeable_energy"                        : { "value": "/assets/bms/##BMS_ID##:chargeable_energy"                        , "vlink": "/status/##BMS_ID##:ChargeableEnergy"                    },
        "ui_dischargeable_energy"                     : { "value": "/assets/bms/##BMS_ID##:dischargeable_energy"                     , "vlink": "/status/##BMS_ID##:DischargeableEnergy"                 },
        "ui_system_current"                           : { "value": "/assets/bms/##BMS_ID##:current"                                  , "vlink": "/status/##BMS_ID##:DCCurrent"                           },
        "ui_system_voltage"                           : { "value": "/assets/bms/##BMS_ID##:voltage"                                  , "vlink": "/status/##BMS_ID##:DCVoltage"                           },
        "ui_system_power"                             : { "value": "/assets/bms/##BMS_ID##:power"                                    , "vlink": "/status/##BMS_ID##:DCPower"                             },
        "ui_max_cell_temp"                            : { "value": "/assets/bms/##BMS_ID##:max_cell_temp"                            , "vlink": "/status/##BMS_ID##:MaxCellTemp"                         },
        "ui_avg_cell_temp"                            : { "value": "/assets/bms/##BMS_ID##:avg_cell_temp"                            , "vlink": "/status/##BMS_ID##:AvgCellTemp"                         },
        "ui_min_cell_temp"                            : { "value": "/assets/bms/##BMS_ID##:min_cell_temp"                            , "vlink": "/status/##BMS_ID##:MinCellTemp"                         },
        "ui_cell_temp_delta"                          : { "value": "/assets/bms/##BMS_ID##:cell_temp_delta"                          , "vlink": "/status/##BMS_ID##:CellTempDelta"                       },
        "ui_max_cell_voltage"                         : { "value": "/assets/bms/##BMS_ID##:max_cell_voltage"                         , "vlink": "/status/##BMS_ID##:MaxCellVoltage"                      },
        "ui_avg_cell_voltage"                         : { "value": "/assets/bms/##BMS_ID##:avg_cell_voltage"                         , "vlink": "/status/##BMS_ID##:AvgCellVoltage"                      },
        "ui_min_cell_voltage"                         : { "value": "/assets/bms/##BMS_ID##:min_cell_voltage"                         , "vlink": "/status/##BMS_ID##:MinCellVoltage"                      },
        "ui_cell_voltage_delta"                       : { "value": "/assets/bms/##BMS_ID##:cell_voltage_delta"                       , "vlink": "/status/##BMS_ID##:CellVoltageDelta"                    },
        "ui_racks_in_service"                         : { "value": "/assets/bms/##BMS_ID##:racks_in_service"                         , "vlink": "/status/##BMS_ID##:RacksInService"                      },
        "ui_racks_total"                              : { "value": "/assets/bms/##BMS_ID##:racks_total"                              , "vlink": "/status/##BMS_ID##:RacksTotal"                          },
        "ui_bms_dc_contactor_control_feedback"        : { "value": "/assets/bms/##BMS_ID##:bms_dc_contactor_control_feedback"        , "vlink": "/status/##BMS_ID##:BMSDCContactorControlFeedback"       },
        "ui_bms_dc_contactor_control_feedback_status" : { "value": "/assets/bms/##BMS_ID##:bms_dc_contactor_control_feedback_status" , "vlink": "/status/##BMS_ID##:BMSDCContactorControlFeedbackStatus" },
        "ui_maint_mode"                               : { "value": "/assets/bms/##BMS_ID##:maint_mode"                               , "vlink": "/status/##BMS_ID##:MaintMode"                           },

        "ui_charge_power_limit"      : { "value": "/assets/bms/##BMS_ID##:charge_power_limit"    , "vlink": "/limits/##BMS_ID##:ChargePowerLimit"    },
        "ui_discharge_power_limit"   : { "value": "/assets/bms/##BMS_ID##:discharge_power_limit" , "vlink": "/limits/##BMS_ID##:DischargePowerLimit" },
        "limits_chargeable_power"    : { "value": "/status/##BMS_ID##:ChargePowerLimit"          , "vlink": "/limits/##BMS_ID##:ChargePowerLimit"    },
        "limits_dischargeable_power" : { "value": "/status/##BMS_ID##:DischargePowerLimit"       , "vlink": "/limits/##BMS_ID##:DischargePowerLimit" }
    },

    "/system/commands": {
        "runBMSOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/##BMS_ID##:pubAssets_##BMS_ID##" , "aname":"##BMS_ID##", "value":0, "every":1  },
                {"uri":"/sched/##BMS_ID##:runMonitor_##BMS_ID##", "aname":"##BMS_ID##", "value":0, "every":0.1}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/##BMS_ID##": {
        "pubAssets_##BMS_ID##": {
            "value": 1,
            "enable": "/config/bms:enable",
            "mode": "ui",
            "table": "/assets/bms/##BMS_ID##",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_##BMS_ID##": {
            "value":1,
            "enable": "/config/bms:enable",
            "monitor":"wake_monitor",
            "aname":"##BMS_ID##",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/schedule/wake_monitor/##BMS_ID##":{
        "/status/##BMS_ID##:HeartbeatRead": {"func": "CheckMonitorVar"},

        "/status/##BMS_ID##:CurrentCheckDone" : {"func": "CalculateVar"},
        "/status/##BMS_ID##:CellTempDelta"    : {"func": "CalculateVar"},
        "/status/##BMS_ID##:CellVoltageDelta" : {"func": "CalculateVar"},
        "/status/##BMS_ID##:IsFaulted"        : {"func": "CalculateVar"},
        "/status/##BMS_ID##:IsAlarming"       : {"func": "CalculateVar"}
    },
    
    "/limits/##BMS_ID##": {
        "ChargePowerLimit"     : 0,
        "DischargePowerLimit"  : 0
    },

    "/status/##BMS_ID##": {
        "CommsOKStatus"                       : "N/A",
        "DCClosedStatus"                      : "N/A",
        "BMSDCContactorControlFeedbackStatus" : "N/A",
        "MaintMode"                     : false,
        "SOC"                           : -1.0 ,
        "SOH"                           : -1.0 ,
        "ChargeableEnergy"              : -1.0 ,
        "DischargeableEnergy"           : -1.0 ,
        "DCCurrent"                     : -1.0 ,
        "DCVoltage"                     : -1.0 ,
        "DCPower"                       : -1.0 ,
        "MaxCellTemp"                   : -1.0 ,
        "AvgCellTemp"                   : -1.0 ,
        "MinCellTemp"                   : -1.0 ,
        "MaxCellVoltage"                : -1.0 ,
        "AvgCellVoltage"                : -1.0 ,
        "MinCellVoltage"                : -1.0 ,
        "RacksInService"                : -1   ,
        "RacksTotal"                    : -1   ,
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
                        { "inValue": true , "ifChanged": false, "uri": "/status/##BMS_ID##:CommsOKStatus", "outValue": "Online" },
                        { "inValue": false, "ifChanged": false, "uri": "/status/##BMS_ID##:CommsOKStatus", "outValue": "Offline"}
                    ]
                }]
            }
        },
        "DCClosed": {
            "value": -1,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/##BMS_ID##:DCClosedStatus", "outValue": "Opened"},
                        {"inValue": 2, "uri":"/status/##BMS_ID##:DCClosedStatus", "outValue": "Closed"}
                    ]
                }]
            }
        },
        "BMSDCContactorControlFeedback": {
            "value": -1,
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/##BMS_ID##:BMSDCContactorControlFeedbackStatus", "outValue": "Open" },
                        {"inValue": 2, "uri":"/status/##BMS_ID##:BMSDCContactorControlFeedbackStatus", "outValue": "Close"}
                    ]
                }]
            }
        },
        "FaultShutdown": {
            "value": false,
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": true, "ifChanged": false, "uri":"/status/bms:FaultShutdown", "outValue": true}
                    ]
                }]
            }
        },
        "CurrentCheckDone": {
            "value": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##:DCCurrent",
            "variable2": "/config/##BMS_ID##:CurrentCheckThreshold",
            "expression": " {2} >= {1} and {1} >= (-1 * {2})"
        },
        "CellTempDelta": {
            "value": -1,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##:MinCellTemp",
            "variable2": "/status/##BMS_ID##:MaxCellTemp",
            "expression": "{2} - {1}"
        },
        "CellVoltageDelta": {
            "value": -1,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##:MinCellVoltage",
            "variable2": "/status/##BMS_ID##:MaxCellVoltage",
            "expression": "{2} - {1}"
        },
        "IsFaulted": {
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/assets/bms/##BMS_ID##:faults",
            "expression": "{1} == 1"
        },
        "IsAlarming": {
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/assets/bms/##BMS_ID##:alarms",
            "expression": "{1} == 1"
        }
    }
}
)JSON";
