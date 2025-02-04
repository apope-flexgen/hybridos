const char* std_v_bms_manager_controls_s = R"JSON(
{
    "/status/pcs": {
        "SystemStateStatus": "N/A"
    },

    "/schedule/wake_monitor/bms":{
        "/status/bms:OpenContactorsEnable"  : {"func": "CalculateVar"},
        "/status/bms:CloseContactorsEnable" : {"func": "CalculateVar"},
        "/status/bms:ClearFaultsEnable"     : {"func": "CalculateVar"},
        "/status/bms:MaintMode"             : {"func": "CalculateVar"}
    },

    "/controls/bms": {
        "OpenContactors": {
            "value": false,
            "enable": "/config/bms:enable",
            "targav": true,
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}, {"func": "LogInfo"}]}]
            }
        },
        "CloseContactors": {
            "value": false,
            "enable": "/config/bms:enable",
            "targav": true,
            "actions": {
                "onSet":[{"func":[{"func":"SchedItemOpts"}, {"func": "LogInfo"}]}]
            }
        },
        "ClearFaults": {
            "value": false,
            "enable": "/config/bms:enable",
            "targav": true,
            "actions": {
                "onSet":[
                    {"func":[{"func":"SchedItemOpts"}, {"func": "LogInfo"}]},
                    {"remap":[
                        {"inValue": true, "ifChanged": false, "uri": "/faults/bms:clear_faults"  , "outValue": "Clear" },
                        {"inValue": true, "ifChanged": false, "uri": "/alarms/bms:clear_alarms"  , "outValue": "Clear" },
                        {"inValue": true, "ifChanged": false, "uri": "/status/bms:FaultShutdown" , "outValue": false   }
                    ]}
                ]
            }
        }
    },

    "/status/bms": {
        "MaintMode": {
            "value": false,
            "numVars": 1,
            "variable1": "bms:MaintMode",
            "operation": "or"
        },
        "OpenContactorsEnable"  : {
            "value": 0,
            "numVars": 1,
            "variable1": "bms:OpenContactorsEnable",
            "operation": "or",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/summary:open_contactors@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/bms/summary:open_contactors@enabled", "outValue":true}
                    ]
                }]
            }
        },
        "CloseContactorsEnable" : {
            "value": 0,
            "numVars": 1,
            "variable1": "bms:CloseContactorsEnable",
            "operation": "or",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/summary:close_contactors@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/bms/summary:close_contactors@enabled", "outValue": true}
                    ]
                }]
            }
        },
        "ClearFaultsEnable": {
            "value": 0,
            "numVars": 2,
            "variable1": "/status/bms:IsFaulted",
            "variable2": "/status/bms:IsAlarming",
            "useExpr": true,
            "expression": "{1} or {2}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/summary:clear_faults@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/bms/summary:clear_faults@enabled", "outValue": true}
                    ]
                }]
            }
        }
    }
}
)JSON";
