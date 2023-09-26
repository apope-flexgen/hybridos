#pragma once

const char* std_ess_controller_faults_s = R"JSON(
{
    "/faults/ess": {
        "clear_faults": {
            "value": "N/A",
            "type": "fault",
            "numVars": 1,
            "variable1": "SiteControllerHeartbeat",
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "ess"}]},
                    {"remap":[
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_bess_comms_faults" , "outValue": 0},
                        {"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_bess_faults"       , "outValue": 0}
                    ]}
                ]
            }
        }
    },
    "/alarms/ess": {
        "clear_alarms": {
            "value": "N/A",
            "type": "alarms",
            "numVars": 1,
            "variable1": "SiteControllerHeartbeat",
            "actions": {
                "onSet": [
                    {"func": [{"func": "process_sys_alarm", "amap": "ess"}]},
                    {"remap":[{"inValue": "Clear", "ifChanged": false, "uri":"/faults/site:fg_bess_alarms" , "outValue": 0}]}
                ]
            }
        }
    },
    "/status/ess": {
        "FaultShutdown": {
            "value": false,
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[{"ifChanged": false, "uri": "/status/pcs:FaultShutdown"}]
                }]
            }
        }
    },
    "/status/bms": {
        "FaultShutdown": {
            "value": false,
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": true, "ifChanged": false, "uri": "/controls/bms:OpenContactors" , "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/status/ess:FaultShutdown"    , "outValue": true}
                    ]
                }]
            }
        }
    },
    "/status/pcs": {
        "FaultShutdown": {
            "value": false,
            "actions": {
                "onSet":[{
                    "func": [{"func": "LogInfo"}],
                    "remap":[
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:Stop@triggerCmd", "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:ActivePowerSetpoint"   , "outValue": 0    },
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:ReactivePowerSetpoint" , "outValue": 0    }
                    ]
                }]
            }
        }
    }
}
)JSON";