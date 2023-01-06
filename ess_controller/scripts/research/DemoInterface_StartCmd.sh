#!/bin/sh
# demo of a controller interface

PCS_ID=pcs_1


echo setup more details on start_cmd

fims_send -m set  -u /ess/cfg/cfile/ess/pcs_start_command '{
    "pname":"ess",
    "amname":"pcs",
    "/status/pcs": {
        "SystemState": "Off",
        "WorkMode":"Off-grid",
        "OffGridVoltageSetpoint":   440,
        "OffGridFrequencySetpoint":   60.1
    },

    "/status/bms": {
       "DCClosed": true
    },

    "/controls/pcs":{   
        "enable_start":false,

        "StartCmd": {
            "value": 207,
            "debug":true,
            "enable":"/controls/pcs:enable_start",
            "note00": "Start command to send to PCS if the PCS is either off and ",
            "note01":" the BMS DC contactors are closed or the PCS is in standby. Check the PCS grid mode as well",
            "cmdVar": "/components/pcs_1_parameter_setting:start_stop",
            "checkCmdHoldTimeout": 0.2,
            "checkCmdTimeout": 30,
            "sendCmdTimeout": 30,
            "numVars": 5,
            "variable1": "/status/pcs:SystemState",
            "variable2": "/status/bms:DCClosed",
            "variable3": "/status/pcs:WorkMode",
            "variable4": "/status/pcs:OffGridVoltageSetpoint",
            "variable5": "/status/pcs:OffGridFrequencySetpoint",
            "useExpr": true,
            "expression": "(({1} == Off and {2}) or {1} == Ready) and ({3} == On-grid or ({3} == Off-grid and {4} != -480 and {5} == 60))",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
            }
        },
        "StartCmdSuccess": {
            "value": false,
            "note": "If the start command has been successfully sent to the PCS, verify that the PCS is now running. Otherwise, send an alarm",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/pcs:VerifyStartup@triggerCmd", "outValue": true},
                        {"inValue": true,  "ifChanged": false, "uri": "/controls/pcs:VerifyStartup",            "outValue": 0},

                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:start_cmd",                  "outValue": "Failed to send/verify start command (207) to PCS"}
                    ]
                }]
            }
        },
        "VerifyStartup": {
            "value": 0,
            "note": "Verify that the PCS system state is now running when the start command is sent",
            "enableAlert": false,
            "sendCmdHoldTimeout": 3,
            "sendCmdTimeout": 30,
            "numVars": 1,
            "variable1": "/status/pcs:SystemState",
            "useExpr": true,
            "expression": "{1} == Running",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
            }
        },
        "VerifyStartupSuccess": {
            "value": false,
            "note": "If the PCS failed to start after an elasped time, send an alarm",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:start_failure", "outValue": "Failed to verify PCS is running"}
                    ]
                }]
            }
        }
   }
}'

echo now add StartCmd to the monitor and enable the start command actions 


fims_send -m set  -u /ess/cfg/cfile/ess/pcs_monitor_start '{
    "pname":"ess",
    "amname":"pcs",

    "/schedule/wake_monitor/pcs": {
        "/controls/pcs:StartCmd":                { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
        "/controls/pcs:VerifyStartup":           { "enable": false,  "amap": "pcs", "func":"HandleCmd"}
    }
    "/controls/pcs":{   
        "enable_start":true,
    }
}'



fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq

echo we have to enable a few things
echo ">>>>>>>" fims_send -m set -r/me -u /ess/full/controls/pcs/enable_start true 

echo ">>>>>>>" fims_send -m set -r/me -u /ess/assets/pcs/pcs_1/start true
echo for some reason we have to do this twice the first time.


echo note we got this in the log
echo '
          [292.773s  ] [warning ] [record         ] Cannot send command value [207] to [/components/pcs_parameter_setting:start_stop]. 
          Condition(s):
           (([/status/pcs:SystemState:Off] == Off and [/status/bms:DCClosed:true]) 
               or [/status/pcs:SystemState:Off] == Ready) and ([/status/pcs:WorkMode:Off-grid] == On-grid 
                   or ([/status/pcs:WorkMode:Off-grid] == Off-grid and [/status/pcs:OffGridVoltageSetpoint:440] != -480 
                         and [/status/pcs:OffGridFrequencySetpoint:60.1] == 60))
    '

echo  This is the problem
echo  '[/status/pcs:WorkMode:Off-grid] == On-grid'

echo so set /status/pcs:WorkMode  to On-grid and try again....

echo ">>>>>>" fims_send -m set -r/me -u /ess/full/status/pcs/WorkMode  \'\"On-grid\"\'

echo to make the system believe its running 
echo ">>>>>>" fims_send -m set -r/me -u /ess/full/status/pcs/SystemState  \'\"Running\"\'

echo 
echo ">>>>>>" fims_send -m set -r/me -u /ess/full/status/pcs/WorkMode  \'\"On-grid\"\'
