const char* std_v_bms_manager_s = R"JSON(
{
    "/config/bms": {
        "AlarmDestination": "/assets/bms/summary:alarms",
        "FaultDestination": "/assets/bms/summary:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal",
        "enable": false
    },

    "/assets/bms/summary": {
        "alarms": {"name": "Alarms", "value": 0, "options":[],"enabled":true},
        "faults": {"name": "Faults", "value": 0, "options":[],"enabled":true},
        "close_contactors": {
            "enable": "/config/bms:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/bms:CloseContactors", "outValue": true}]
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
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/bms:OpenContactors", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },
        "clear_faults": {
            "enable": "/config/bms:enable",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [{"func": "LogInfo"}],
                    "remap": [{"inValue": true, "ifChanged": false, "uri": "/controls/bms:ClearFaults", "outValue": true}]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        }
    },

    "/system/commands": {
        "runBMSOpts": {
            "value": false,
            "enabled": false,
            "targav": "/system/commands:run",
            "options": [
                {"uri":"/sched/bms:pubAssets_bms" , "aname":"bms", "value":0, "every":1  },
                {"uri":"/sched/bms:runMonitor_bms", "aname":"bms", "value":0, "every":0.1}
            ],
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}]}]
            }
        }
    },

    "/sched/bms": {
        "pubAssets_bms": {
            "value": 1,
            "enable": "/config/bms:enable",
            "mode": "ui",
            "table": "/assets/bms/summary",
            "actions": {
                "onSet":[{"func":[{"func":"RunPub"}]}]
            }
        },
        "runMonitor_bms": {
            "value":1,
            "enable": "/config/bms:enable",
            "monitor":"wake_monitor",
            "aname":"bms",
            "actions":{
                "onSet":[{"func":[{"func":"RunMonitor"}]}]
            }
        }
    },

    "/vlinks/bms": {
        "ui_comms_ok"                                 : { "value": "/assets/bms/summary:comms_ok_status"                          , "vlink": "/status/bms:CommsOKStatus"                       },
        "ui_dc_closed_status"                         : { "value": "/assets/bms/summary:dc_closed_status"                         , "vlink": "/status/bms:DCClosedStatus"                      },
        "ui_system_soc"                               : { "value": "/assets/bms/summary:soc"                                      , "vlink": "/status/bms:SOC"                                 },
        "ui_system_soh"                               : { "value": "/assets/bms/summary:soh"                                      , "vlink": "/status/bms:SOH"                                 },
        "ui_chargeable_energy"                        : { "value": "/assets/bms/summary:chargeable_energy"                        , "vlink": "/status/bms:ChargeableEnergy"                    },
        "ui_dischargeable_energy"                     : { "value": "/assets/bms/summary:dischargeable_energy"                     , "vlink": "/status/bms:DischargeableEnergy"                 },
        "ui_system_current"                           : { "value": "/assets/bms/summary:current"                                  , "vlink": "/status/bms:DCCurrent"                           },
        "ui_system_voltage"                           : { "value": "/assets/bms/summary:voltage"                                  , "vlink": "/status/bms:DCVoltage"                           },
        "ui_system_power"                             : { "value": "/assets/bms/summary:power"                                    , "vlink": "/status/bms:DCPower"                             },
        "ui_max_cell_temp"                            : { "value": "/assets/bms/summary:max_cell_temp"                            , "vlink": "/status/bms:MaxCellTemp"                         },
        "ui_avg_cell_temp"                            : { "value": "/assets/bms/summary:avg_cell_temp"                            , "vlink": "/status/bms:AvgCellTemp"                         },
        "ui_min_cell_temp"                            : { "value": "/assets/bms/summary:min_cell_temp"                            , "vlink": "/status/bms:MinCellTemp"                         },
        "ui_cell_temp_delta"                          : { "value": "/assets/bms/summary:cell_temp_delta"                          , "vlink": "/status/bms:CellTempDelta"                       },
        "ui_max_cell_voltage"                         : { "value": "/assets/bms/summary:max_cell_voltage"                         , "vlink": "/status/bms:MaxCellVoltage"                      },
        "ui_avg_cell_voltage"                         : { "value": "/assets/bms/summary:avg_cell_voltage"                         , "vlink": "/status/bms:AvgCellVoltage"                      },
        "ui_min_cell_voltage"                         : { "value": "/assets/bms/summary:min_cell_voltage"                         , "vlink": "/status/bms:MinCellVoltage"                      },
        "ui_cell_voltage_delta"                       : { "value": "/assets/bms/summary:cell_voltage_delta"                       , "vlink": "/status/bms:CellVoltageDelta"                    },
        "ui_racks_in_service"                         : { "value": "/assets/bms/summary:racks_in_service"                         , "vlink": "/status/bms:RacksInService"                      },
        "ui_racks_total"                              : { "value": "/assets/bms/summary:racks_total"                              , "vlink": "/status/bms:RacksTotal"                          },
        "ui_bms_dc_contactor_control_feedback"        : { "value": "/assets/bms/summary:bms_dc_contactor_control_feedback"        , "vlink": "/status/bms:BMSDCContactorControlFeedback"       },
        "ui_bms_dc_contactor_control_feedback_status" : { "value": "/assets/bms/summary:bms_dc_contactor_control_feedback_status" , "vlink": "/status/bms:BMSDCContactorControlFeedbackStatus" },

        "ui_chargeable_power"    : { "value": "/assets/bms/summary:charge_power_limit"    , "vlink": "/limits/bms:ChargePowerLimit"    },
        "ui_dischargeable_power" : { "value": "/assets/bms/summary:discharge_power_limit" , "vlink": "/limits/bms:DischargePowerLimit" }
    },

    "/limits/bms": {
        "ChargePowerLimit"    : { "value": 0, "numVars": 1, "variable1": "bms:ChargePowerLimit"    , "operation": "+" },
        "DischargePowerLimit" : { "value": 0, "numVars": 1, "variable1": "bms:DischargePowerLimit" , "operation": "+" }
    },

    "/schedule/wake_monitor/bms":{
        "/limits/bms:ChargePowerLimit"    : {"func": "CalculateVar"},
        "/limits/bms:DischargePowerLimit" : {"func": "CalculateVar"},

        "/status/bms:CommsOK"                       : {"func": "CalculateVar"},
        "/status/bms:SOC"                           : {"func": "CalculateVar"},
        "/status/bms:SOH"                           : {"func": "CalculateVar"},
        "/status/bms:ChargeableEnergy"              : {"func": "CalculateVar"},
        "/status/bms:DischargeableEnergy"           : {"func": "CalculateVar"},
        "/status/bms:DCCurrent"                     : {"func": "CalculateVar"},
        "/status/bms:DCVoltage"                     : {"func": "CalculateVar"},
        "/status/bms:DCPower"                       : {"func": "CalculateVar"},
        "/status/bms:MaxCellTemp"                   : {"func": "CalculateVar"},
        "/status/bms:AvgCellTemp"                   : {"func": "CalculateVar"},
        "/status/bms:MinCellTemp"                   : {"func": "CalculateVar"},
        "/status/bms:CellTempDelta"                 : {"func": "CalculateVar"},
        "/status/bms:MaxCellVoltage"                : {"func": "CalculateVar"},
        "/status/bms:AvgCellVoltage"                : {"func": "CalculateVar"},
        "/status/bms:MinCellVoltage"                : {"func": "CalculateVar"},
        "/status/bms:CellVoltageDelta"              : {"func": "CalculateVar"},
        "/status/bms:RacksInService"                : {"func": "CalculateVar"},
        "/status/bms:RacksTotal"                    : {"func": "CalculateVar"},
        "/status/bms:BMSDCContactorControlFeedback" : {"func": "CalculateVar"},
        "/status/bms:DCClosed"                      : {"func": "CalculateVar"},
        "/status/bms:CurrentCheckDone"              : {"func": "CalculateVar"},
        "/status/bms:ChildFaulted"                  : {"func": "CalculateVar"},
        "/status/bms:ChildAlarming"                 : {"func": "CalculateVar"},
        "/status/bms:IsFaulted"                     : {"func": "CalculateVar"},
        "/status/bms:IsAlarming"                    : {"func": "CalculateVar"}
    },

    "/status/bms": {
        "DCClosedStatus"                      : "N/A",
        "BMSDCContactorControlFeedbackStatus" : "N/A",
        "CommsOKStatus"                       : "N/A",
        "SOC"                           : { "value": -1.0 , "numVars": 1, "variable1": "bms:SOC"                           , "operation": "avg" },
        "SOH"                           : { "value": -1.0 , "numVars": 1, "variable1": "bms:SOH"                           , "operation": "min" },
        "ChargeableEnergy"              : { "value": -1.0 , "numVars": 1, "variable1": "bms:ChargeableEnergy"              , "operation": "+"   },
        "DischargeableEnergy"           : { "value": -1.0 , "numVars": 1, "variable1": "bms:DischargeableEnergy"           , "operation": "+"   },
        "DCCurrent"                     : { "value": -1.0 , "numVars": 1, "variable1": "bms:DCCurrent"                     , "operation": "+"   },
        "DCVoltage"                     : { "value": -1.0 , "numVars": 1, "variable1": "bms:DCVoltage"                     , "operation": "avg" },
        "DCPower"                       : { "value": -1.0 , "numVars": 1, "variable1": "bms:DCPower"                       , "operation": "+"   },
        "MaxCellTemp"                   : { "value": -1.0 , "numVars": 1, "variable1": "bms:MaxCellTemp"                   , "operation": "max" },
        "AvgCellTemp"                   : { "value": -1.0 , "numVars": 1, "variable1": "bms:AvgCellTemp"                   , "operation": "avg" },
        "MinCellTemp"                   : { "value": -1.0 , "numVars": 1, "variable1": "bms:MinCellTemp"                   , "operation": "min" },
        "CellTempDelta"                 : { "value": -1.0 , "numVars": 1, "variable1": "bms:CellTempDelta"                 , "operation": "max" },
        "MaxCellVoltage"                : { "value": -1.0 , "numVars": 1, "variable1": "bms:MaxCellVoltage"                , "operation": "max" },
        "AvgCellVoltage"                : { "value": -1.0 , "numVars": 1, "variable1": "bms:AvgCellVoltage"                , "operation": "avg" },
        "MinCellVoltage"                : { "value": -1.0 , "numVars": 1, "variable1": "bms:MinCellVoltage"                , "operation": "min" },
        "CellVoltageDelta"              : { "value": -1.0 , "numVars": 1, "variable1": "bms:CellVoltageDelta"              , "operation": "max" },
        "RacksInService"                : { "value": -1.0 , "numVars": 1, "variable1": "bms:RacksInService"                , "operation": "+"   },
        "RacksTotal"                    : { "value": -1.0 , "numVars": 1, "variable1": "bms:RacksTotal"                    , "operation": "+"   },
        "ChildFaulted"                  : { "value": -1   , "numVars": 1, "variable1": "bms:IsFaulted"                     , "operation": "or"  },
        "ChildAlarming"                 : { "value": -1   , "numVars": 1, "variable1": "bms:IsAlarming"                    , "operation": "or"  },
        "IsFaulted": {
            "value": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/bms:ChildFaulted",
            "variable2": "/assets/bms/summary:faults",
            "expression": "{1} or {2}"
        },
        "IsAlarming": {
            "value": false,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/bms:ChildAlarming",
            "variable2": "/assets/bms/summary:alarms",
            "expression": "{1} or {2}"
        },
        "CommsOK": {
            "value": false,
            "numVars": 1,
            "variable1": "bms:CommsOK",
            "operation": "and",
            "actions": {
                "onSet":[{
                    "remap":[
                        { "inValue": true , "ifChanged": false, "uri": "/status/bms:CommsOKStatus", "outValue": "Online" },
                        { "inValue": false, "ifChanged": false, "uri": "/status/bms:CommsOKStatus", "outValue": "Offline"},

                        {"inValue": true , "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[1]", "outValue": false},
                        {"inValue": false, "ifChanged": false, "uri": "/faults/site:fg_bess_comms_faults[1]", "outValue": true }
                    ]
                }]
            }
        },
        "DCClosed": {
            "value": -1,
            "numVars": 1,
            "variable1": "bms:DCClosed",
            "operation": "max",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/bms:DCClosedStatus", "outValue": "Opened"},
                        {"inValue": 2, "uri":"/status/bms:DCClosedStatus", "outValue": "Closed"},

                        {"inValue": 1, "uri":"/status/site:dc_contactors_closed_bool", "outValue": 0},
                        {"inValue": 2, "uri":"/status/site:dc_contactors_closed_bool", "outValue": 1}
                    ]
                }]
            }
        },
        "BMSDCContactorControlFeedback": {
            "value": -1,
            "numVars": 1,
            "variable1": "bms:BMSDCContactorControlFeedback",
            "operation": "max",
            "actions": {
                "onSet":[{
                    "remap":[
                        {"inValue": 1, "uri":"/status/bms:BMSDCContactorControlFeedbackStatus", "outValue": "Open" },
                        {"inValue": 2, "uri":"/status/bms:BMSDCContactorControlFeedbackStatus", "outValue": "Close"}
                    ]
                }]
            }
        },
        "CurrentCheckDone": {
            "value": false,
            "numVars": 1,
            "variable1": "bms:CurrentCheckDone",
            "operation": "and"
        }
    }
}
)JSON";
