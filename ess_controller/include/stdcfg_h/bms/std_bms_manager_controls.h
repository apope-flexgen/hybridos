const char* std_bms_manager_controls_s = R"JSON(
{
    "/status/pcs": {
        "SystemStateStatus": "N/A"
    },

    "/schedule/wake_monitor/##BMS_ID##":{
        "/controls/##BMS_ID##:OpenContactors"        : {"func": "HandleCmd", "amap": "bms"},
        "/controls/##BMS_ID##:VerifyOpenContactors"  : {"func": "HandleCmd", "amap": "bms"},
        "/controls/##BMS_ID##:CloseContactors"       : {"func": "HandleCmd", "amap": "bms"},
        "/controls/##BMS_ID##:VerifyCloseContactors" : {"func": "HandleCmd", "amap": "bms"},
        "/controls/##BMS_ID##:ClearFaults"           : {"func": "HandleCmd", "amap": "bms"},

        "/status/##BMS_ID##:OpenContactorsEnable"  : {"func": "CalculateVar"},
        "/status/##BMS_ID##:CloseContactorsEnable" : {"func": "CalculateVar"}
    },

    "/controls/bms": {
        "OpenContactors"  : {"options": [{"uri":"/controls/##BMS_ID##:OpenContactors@triggerCmd"  , "value":true}]},
        "CloseContactors" : {"options": [{"uri":"/controls/##BMS_ID##:CloseContactors@triggerCmd" , "value":true}]},
        "ClearFaults"     : {"options": [{"uri":"/controls/##BMS_ID##:ClearFaults@triggerCmd"     , "value":true}]}
    },

    "/controls/##BMS_ID##": {
        "OpenContactors": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 20,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "variable2": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Closed and ({2} == Fault or {2} == Stop)"
        },
        "VerifyOpenContactors": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "useExpr": true,
            "expression": "{1} == Opened"
        },
        "CloseContactors": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 10,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "variable2": "/status/bms:IsFaulted",
            "variable3": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Opened and not {2} and ({3} == Fault or {3} == Stop)"
        },
        "VerifyCloseContactors": {
            "value": 0,
            "enableAlert": true,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "useExpr": true,
            "expression": "{1} == Closed"
        },
        "ClearFaults": {
            "value": 0,
            "enableAlert": true
        }
    },
    "/status/##BMS_ID##": {
        "OpenContactorsEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "variable2": "/assets/bms/##BMS_ID##:maint_mode",
            "variable3": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Closed and {2} and ({3} == Fault or {3} == Stop)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/##BMS_ID##:open_contactors@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/bms/##BMS_ID##:open_contactors@enabled", "outValue":true}
                    ]
                }]
            }
        },
        "CloseContactorsEnable": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "variable1": "/status/##BMS_ID##:DCClosedStatus",
            "variable2": "/status/bms:IsFaulted",
            "variable3": "/assets/bms/##BMS_ID##:maint_mode",
            "variable4": "/status/pcs:SystemStateStatus",
            "expression": "{1} == Opened and not {2} and {3} and ({4} == Fault or {4} == Stop)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/##BMS_ID##:close_contactors@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/bms/##BMS_ID##:close_contactors@enabled", "outValue": true}
                    ]
                }]
            }
        },
        "OpenContactorsSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/##BMS_ID##:VerifyOpenContactors@triggerCmd", "outValue": true},
      
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_bms_control_alarms_1[3]" , "outValue": true                                           },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/##BMS_ID##:OpenContactorsCmd"    , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]
                }]
            }
        },
        "VerifyOpenContactorsSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false , "ifChanged": false, "uri": "/alarms/site:fg_bms_control_alarms_1[4]"    , "outValue": true                                      },
                        {"inValue": false , "ifChanged": false, "uri": "/alarms/##BMS_ID##:OpenContactorsCmdVerify" , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "CloseContactorsSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": true, "ifChanged": false, "uri": "/controls/##BMS_ID##:VerifyCloseContactors@triggerCmd", "outValue": true},
    
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_bms_control_alarms_1[1]" , "outValue": true                                            },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/##BMS_ID##:CloseContactorsCmd"   , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log."}
                    ]
                }]
            }
        },
        "VerifyCloseContactorsSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_bms_control_alarms_1[2]"     , "outValue": true                                       },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/##BMS_ID##:CloseContactorsCmdVerify" , "outValue": "Command was sent but the expected result was not observed before timeout."}
                    ]
                }]
            }
        },
        "ClearFaultsSuccess": {
            "value": false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/site:fg_bms_control_alarms_1[0]" , "outValue": true                                         },
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/##BMS_ID##:ClearFaultsCmd"       , "outValue": "Command preconditions were not met, the command was not sent. Details can be found in the ESS Events log." },
                        
                        {"inValue": true , "ifChanged": false, "uri": "/faults/##BMS_ID##:clear_faults"         , "outValue": "Clear"                                      },
                        {"inValue": true , "ifChanged": false, "uri": "/alarms/##BMS_ID##:clear_alarms"         , "outValue": "Clear"                                      }
                    ]
                }]
            }
        }
    }
}
)JSON";
