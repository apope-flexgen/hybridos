#pragma once

const char* std_ess_controller_controls_s = R"JSON(
{
    "/schedule/wake_monitor/ess":{
        "/controls/ess:SiteControllerHeartbeat": {"func": "CheckMonitorVar" },
        
        "/status/ess:ClearFaultsEnable":         {"func": "CalculateVar" }
    },

    "/controls/ess": {
        "SiteControllerHeartbeat": {
            "value": 0,
            "enable": "/config/ess:enable",
            "EnableStateCheck": true,
            "EnableCommsCheck": true,
            "AlarmTimeout": 10,
            "FaultTimeout": 15,
            "RecoverTimeout": 0.1
        },
        "StartStop": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/pcs:Stop@triggerCmd"       , "outValue": true },
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/pcs:ActivePowerSetpoint"   , "outValue": 0    },
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/pcs:ReactivePowerSetpoint" , "outValue": 0    },
                        {"inValue": 2 , "ifChanged": false, "uri": "/controls/pcs:Standby@triggerCmd"    , "outValue": true },
                        {"inValue": 3 , "ifChanged": false, "uri": "/controls/pcs:Start@triggerCmd"      , "outValue": true }
                    ]
                }]
            }
        },
        "GridMode": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/pcs:GridFollowPQ@triggerCmd" , "outValue": true },
                        {"inValue": 2 , "ifChanged": false, "uri": "/controls/pcs:GridFormVF@triggerCmd"   , "outValue": true },
                        {"inValue": 3 , "ifChanged": false, "uri": "/controls/pcs:GridFormVSG@triggerCmd"  , "outValue": true }
                    ]
                }]
            }
        },
        "ActivePowerCmd": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerSetpoint"}
                    ]
                }]
            }
        },
        "ReactivePowerCmd": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerSetpoint"}
                    ]
                }]
            }
        },
        "ActivePowerRampRate": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerRampRate"                             },
                        {"ifChanged": false, "uri": "/controls/pcs:ActivePowerRampRate@triggerCmd", "outValue": true}
                    ]
                }]
            }
        },
        "ReactivePowerRampRate": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerRampRate"                             },
                        {"ifChanged": false, "uri": "/controls/pcs:ReactivePowerRampRate@triggerCmd", "outValue": true}
                    ]
                }]
            }
        },
        "ClearFaults": {
            "value": 0,
            "enable": "/config/ess:enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": 1 , "ifChanged": false, "uri": "/status/ess:FaultShutdown"            , "outValue": false   },
                        {"inValue": 1 , "ifChanged": false, "uri": "/faults/ess:clear_faults"             , "outValue": "Clear" },
                        {"inValue": 1 , "ifChanged": false, "uri": "/alarms/ess:clear_alarms"             , "outValue": "Clear" },
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/bms:ClearFaults"            , "outValue": true    },
                        {"inValue": 1 , "ifChanged": false, "uri": "/controls/pcs:ClearFaults@triggerCmd" , "outValue": true    }
                    ]
                }]
            }
        },
        "BMSDCContactors": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": 1, "ifChanged": false, "uri":"/controls/bms:OpenContactors" , "outValue": true},
                        {"inValue": 2, "ifChanged": false, "uri":"/controls/bms:CloseContactors", "outValue": true}
                    ]
                }]
            }
        },
        "OffGridFrequencySetpoint": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridFrequencySetpoint"                             },
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridFrequencySetpoint@triggerCmd", "outValue": true}
                    ]
                }]
            }
        },
        "OffGridVoltageSetpoint": {
            "value": 0,
            "enable": "/config/ess:site_control_enable",
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridVoltageSetpoint"                             },
                        {"ifChanged": false, "uri": "/controls/pcs:OffGridVoltageSetpoint@triggerCmd", "outValue": true}
                    ]
                }]
            }
        }
    },

    "/status/ess": {
        "ClearFaultsEnable": {
            "value": 0,
            "numVars": 2,
            "variable1": "/status/ess:IsAlarming",
            "variable2": "/status/ess:IsFaulted",
            "useExpr": true,
            "expression": "{1} or {2}",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/ess/summary:clear_faults@enabled", "outValue": false},
                        {"inValue":1, "uri":"/assets/ess/summary:clear_faults@enabled", "outValue": true}
                    ]
                }]
            }
        }
    }
}
)JSON";