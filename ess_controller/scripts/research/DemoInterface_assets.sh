#!/bin/sh
# demo of a controller interface

PCS_ID=pcs_1



STUFF='"setamap":true,"pname":"ess","amname":"pcs","default":0,'


# first setup  the base component 
# this defines the signals we need for a PCS interface  our equivalent of pcs.h
# each item will define an amap name and a default location for that value 

# fims_send -m set -r /$$ -u /ess/interface/pcs '{
#     "pcs_interface":{ 
#         "value": 0, 
#         "new_options" :[
#             {"CurrentStatus":              "/status/pcs:CurrentStatus"         },
#             {"ActivePower":                "/status/pcs:ActivePower"         },
#             {"ReactivePower":              "/status/pcs:ReactivePower"       },
#             {"GridVoltage1":              "/status/pcs:GridVoltage1"        },
#             {"GridVoltage2":              "/status/pcs:GridVoltage2"        },
#             {"GridVoltage3":              "/status/pcs:GridVoltage3"        },
#             {"GridCurrent1":              "/status/pcs:GridCurrent1"        },
#             {"GridCurrent2":              "/status/pcs:GridCurrent2"        },
#             {"GridCurrent3":              "/status/pcs:GridCurrent3"        }, 
#             {"GridFrequency":              "/status/pcs:GridFrequency"       },
#             {"DCPower":                    "/status/pcs:DCPower"             }, 
#             {"DCVoltage":                  "/status/pcs:DCVoltage"           },  
#             {"DCCurrent":                  "/status/pcs:DCCurrent"            }, 
#             {"PowerFactor":                "/status/pcs:PowerFactor"          }, 
#             {"DailyCharging":              "/status/pcs:DailyChargeEnergy"    },
#             {"DailyDischarging":           "/status/pcs:DailyDischargeEnergy" },
#             {"TotalCharging":              "/status/pcs:TotalChargeEnergy"    },
#             {"TotalDischarging":           "/status/pcs:TotalDischargeEnergy" },
#             {"LeakageCurrent":             "/status/pcs:LeakageCurrent"       },
#             {"AmbientTemp":                "/status/pcs:AmbientTemp"          },
#             {"ModuleTemp1":               "/status/pcs:ModuleTemp1"          }, 
#             {"ModuleTemp2":               "/status/pcs:ModuleTemp2"          }, 
#             {"ModuleTemp3":               "/status/pcs:ModuleTemp3"          }, 
#             {"HeartbeatRead":              "/status/pcs:PCSHeartbeat"            }, 

#             {"MaxChargeCurrent":          "/config/pcs:MaxChargeCurrent"         },
#             {"MaxDischargeCurrent":       "/config/pcs:MaxDischargeCurrent"      },
#             {"MaxChargePower":            "/config/pcs:MaxChargePower"         },
#             {"MaxDischargePower":         "/config/pcs:MaxDischargePower"      },
#             {"AvailChargePower":            "/config/pcs:AvailChargePower"         },
#             {"AvailDischargePower":         "/config/pcs:AvailDischargePower"      },

#             {"MaxTemperature":            "/config/pcs:MaxTemperature"         },
#             {"MinTemperature":            "/config/pcs:MinTemperature"         },

#             {"StartStop":                   "/control/pcs:StartStop"            },
#             {"ActivePowerSetpoint":         "/control/pcs:ActivePowerSetpoint"   },
#             {"ReactivePowerSetpoint":       "/control/pcs:ReactivePowerSetpoint" },
#             {"OffGridVoltageSetpoint":      "/control/pcs:VoltageSetpoint"        },
#             {"OffGridFrequencySetpoint":    "/control/pcs:FrequencySetpoint"      }
#          ]
#     }
# }'

fims_send -m set  -u /ess/cfg/cfile/ess/pcs_assets '{
    "pname":"ess",
    "amname":"pcs",
    "/assets/pcs/pcs_1": {
        "maint_active_power_setpoint": 0,
        "maint_reactive_power_setpoint": 0,
        "maint_voltage_setpoint": 0,
        "maint_voltage_slewrate": 0,
        "clear_faults":"Clear", 
        "maint_mode": {
            "name": "Maintenance Mode",
            "value": false,
            "enabled": true,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"uri":	"/assets/pcs/pcs_1:clear_faults@enabled"},
                        {"uri":	"/assets/pcs/pcs_1:maint_active_power_setpoint@enabled"},
                        {"uri":	"/assets/pcs/pcs_1:maint_reactive_power_setpoint@enabled"},
                        {"uri":	"/assets/pcs/pcs_1:maint_voltage_setpoint@enabled"},
                        {"uri":	"/assets/pcs/pcs_1:maint_voltage_slewrate@enabled"}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        },

        "start": {
            "name": "Start",
            "note": "UI control variable for running the PCS. If the PCS is in grid-forming (off-grid) mode, send a voltage/frequency command to the PCS before we send a start command",
            "value": false,
            "enabled": false,
            "actions": {
                "onSet": [{
                    "func": [
                        {"ifChanged": false, "func": "LogInfo", "amap": "ess"}
                    ],
                    "remap": [
                        {"enable":"/config/pcs:BlackStart", "inValue": true, "ifChanged": false, "uri": "/controls/pcs:CheckIfOffGrid@triggerCmd", "outValue": true},
                        {"enable":"/config/pcs:BlackStart","inValue": true, "ifChanged": false, "uri": "/controls/pcs:CheckIfOffGrid",            "outValue": 0,   "note": "Check if the PCS is in off-grid mode. If so, send 480 V / 60 Hz commands to the PCS before starting up the PCS"},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:StartCmd@triggerCmd",       "outValue": true},
                        {"inValue": true, "ifChanged": false, "uri": "/controls/pcs:StartCmd",                  "outValue": 207, "note": "Send start command to PCS"}
                    ]
                }]
            },
            "options": [
                {"name": "No", "return_value": false},
                {"name": "Yes","return_value": true}
            ]
        }
    }
}'
 
echo  did we load it
fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq

fims_send -m get  -r /$$ -u /ess/naked/assets/pcs/pcs_1 | jq

echo  look at setpoint 
fims_send -m get  -r /$$ -u /ess/full/assets/pcs/pcs_1/maint_active_power_setpoint | jq

echo  now put it in maint mode 
fims_send -m set  -r /$$ -u /ess/naked/assets/pcs/pcs_1/maint_mode  true | jq

echo look again
fims_send -m get  -r /$$ -u /ess/full/assets/pcs/pcs_1/maint_active_power_setpoint | jq

###
echo Question ... how do we enable the "start button" ....
fims_send -m get  -r /$$ -u /ess/full/assets/pcs/pcs_1/start  | jq | grep -i "start\|enable"


echo hint we need to monitor the status 

fims_send -m set  -u /ess/cfg/cfile/ess/pcs_status '{
"pname":"ess",
"amname":"pcs",

"/status/pcs":{   
    "SystemState":"Unknown",

    "StartEnable": {
            "value": 0,
            "note": "Enable the start PCS UI control variable if the BMS DC contactors are closed, the PCS is in maintenance mode, and the PCS is either off or in standby",
            "useExpr": true,
            "numVars": 3,
            "variable1": "/status/bms:DCClosed",
            "variable2": "/assets/pcs/pcs_1:maint_mode",
            "variable3": "/status/pcs:SystemState",
            "expression": "{1} and {2} and ({3} == Off or {3} == Ready)",
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/pcs/pcs_1:start@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/pcs/pcs_1:start@enabled", "outValue":true}
                    ]
                }]
            }
        }
    }
}'

fims_send -m get  -r /$$ -u /ess/full/status/pcs  | jq

echo we already have  /assets/pcs/pcs_1:maint_mode as  true
echo we need /status/bms:DCClosed to be true
echo we need /status/pcs:SystemState to be Off or Ready

echo we also need something to check StartEnable every so often.

echo check Demointerface_monitor

# "/schedule/wake_monitor/pcs":{
#         "/status/pcs:PCSHeartbeat":              { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},
#         "/status/pcs:ActivePower":               { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},
#         "/status/pcs:DCVoltage":                 { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CheckMonitorVar"},

#         "/status/pcs:ActivePowerCmd_adjusted":   { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:ReactivePowerCmd_adjusted": { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:ApparentPower":             { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:MaxPCSApparentPower":       { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:DischargeCalibrationCalc":  { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:ChargeCalibrationCalc":     { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},

#         "/status/pcs:StartEnable":               { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:ShutdownEnable":            { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:StandbyEnable":             { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:GridFormEnable":            { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:GridFollowEnable":          { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"},
#         "/status/pcs:PCSHeartbeatCheckEnable":   { "enable": true, "rate": 0.1, "amap": "pcs", "func":"CalculateVar"}
#     },

# fims_send -m set  -u /ess/cfg/cfile/ess/pcs_controls '{
# "pname":"ess",
# "amname":"pcs",

#   "/controls/pcs":{   
#         "enable":false,

#         "StartCmd": {
#             "value": 207,
#             "debug":true,
#             "enable":"/controls/pcs:enable",
#             "note00": "Start command to send to PCS if the PCS is either off and ",
#             "note01":" the BMS DC contactors are closed or the PCS is in standby. Check the PCS grid mode as well",
#             "cmdVar": "/components/pcs_parameter_setting:start_stop",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 5,
#             "variable1": "/status/pcs:SystemState",
#             "variable2": "/status/bms:DCClosed",
#             "variable3": "/status/pcs:WorkMode",
#             "variable4": "/status/pcs:OffGridVoltageSetpoint",
#             "variable5": "/status/pcs:OffGridFrequencySetpoint",
#             "useExpr": true,
#             "expression": "(({1} == Off and {2}) or {1} == Ready) and ({3} == On-grid or ({3} == Off-grid and {4} != -480 and {5} == 60))",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "StartCmdSuccess": {
#             "value": false,
#             "note": "If the start command has been successfully sent to the PCS, verify that the PCS is now running. Otherwise, send an alarm",
#             "actions": {
#                 "onSet": [{
#                     "remap": [
#                         {"inValue": true,  "ifChanged": false, "uri": "/controls/pcs:VerifyStartup@triggerCmd", "outValue": true},
#                         {"inValue": true,  "ifChanged": false, "uri": "/controls/pcs:VerifyStartup",            "outValue": 0},

#                         {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:start_cmd",                  "outValue": "Failed to send/verify start command (207) to PCS"}
#                     ]
#                 }]
#             }
#         },
#         "ShutdownCmd": {
#             "value": 206,
#             "enable":"/controls/pcs:enable",
#             "note00": "Shutdown command to send to PCS if 0 kW and 0 kVAr are set to the PCS",
#             "note01": "No longer waiting for DC current to decrease, and the PCS is not off or faulted",
#             "cmdVar": "/components/pcs_parameter_setting:start_stop",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 3,
#             "variable1": "/status/bms:BMSCurrentCheckStop@seenMaxAlarm",
#             "variable2": "/status/bms:BMSCurrentCheckStop@seenMinAlarm",
#             "variable3": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "not {1} and not {2} and ({3} == Ready or {3} == Running or {3} == Starting)",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "StandbyCmd": {
#             "value": 205,
#             "enable":"/controls/pcs:enable",
#             "note": "Standby command to send to PCS if the PCS is either off and the BMS DC contactors are closed or the PCS is running",
#             "cmdVar": "/components/pcs_parameter_setting:start_stop",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 2,
#             "variable1": "/status/pcs:SystemState",
#             "variable2": "/status/bms:DCClosed",
#             "useExpr": true,
#             "expression": "({1} == Off and {2}) or {1} == Running",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },

#         "VerifyStartup": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Verify that the PCS system state is now running when the start command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "{1} == Running",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VerifyShutdown": {
#             "enable":"/controls/pcs:enable",
#             "value": 0,
#             "note": "Verify that the PCS system state is now off when the shutdown command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "{1} == Off or {1} == Fault",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VerifyStandby": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Verify that the PCS system state is now in standby (ready) when the standby command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "{1} == Ready",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },

#         "HeartbeatTimeoutSetting": {
#             "value": 60,
#             "enable":"/controls/pcs:enable",
#             "note": "Initialize heartbeat timeout interval value on system startup",
#             "cmdVar": "/components/pcs_parameter_setting:hearbeat_timeout_interval",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "triggerCmd": true,
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "OffGridMode": {
#             "value": 85,
#             "enable":"/controls/pcs:enable",
#             "note": "Set PCS to grid-forming (off-grid) mode",
#             "cmdVar": "/components/pcs_parameter_setting:grid_mode_setting",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == On-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "OnGridMode": {
#             "value": 170,
#             "enable":"/controls/pcs:enable",
#             "note": "Set PCS to grid-following (on-grid) mode",
#             "cmdVar": "/components/pcs_parameter_setting:grid_mode_setting",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == Off-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "OnGridConstACPower": {
#             "value": 4,
#             "enable":"/controls/pcs:enable",
#             "note": "Set the PCS to on-grid const AC power when the PCS is in grid-following (on-grid) mode",
#             "cmdVar": "/components/pcs_parameter_setting:on_grid_mode",
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == On-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },

#         "VerifyOffGridMode": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Verify that the PCS is now in grid-forming (off-grid) mode when the grid-forming command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == Off-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VerifyOnGridMode": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Verify that the PCS is now in grid-following (on-grid) mode when the grid-following command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == On-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VerifyOnGridConstACPower": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Verify that the PCS is now set to grid-following const AC power when the on-grid const AC power command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:OnGridChgModeAbbrev",
#             "useExpr": true,
#             "expression": "{1} == AC-Power",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },

#         "CheckIfOnGrid": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Check if the PCS is currently in grid-following (on-grid) mode",
#             "enableAlert": false,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == On-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "CheckIfOffGrid": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Check if the PCS is currently in grid-forming (off-grid) mode",
#             "enableAlert": false,
#             "numVars": 1,
#             "variable1": "/status/pcs:WorkMode",
#             "useExpr": true,
#             "expression": "{1} == Off-grid",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "CheckIfStopped": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Check if the PCS is in a fault state",
#             "enableAlert": false,
#             "numVars": 1,
#             "sendCmdHoldTimeout": 1,
#             "variable1": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "{1} == Fault or {1} == Off",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },

#         "FrequencySetpoint": {
#             "value": 60,
#             "enable":"/controls/pcs:enable",
#             "note": "Set frequency for the PCS as long as the frequency setpoint is between 55 Hz and 65 Hz",
#             "cmdVar": "/components/pcs_parameter_setting:off_grid_frequency_setpoint",
#             "enableAlert": false,
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "sendCmdTimeout": 30,
#             "includeCurrVal": true,
#             "useExpr": true,
#             "expression": "{1} >= 55 and {1} <= 65",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VoltageSlewRate": 100,
#         "VoltageSlew": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "enabled": true,
#             "RatedValue": 480,
#             "SlewRate": "/controls/pcs:VoltageSlewRate",
#             "SlewedVal": "/controls/pcs:VoltageSlewCmdTrigger"
#         },
#         "VoltageSlewCmdTrigger": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "actions": {
#                 "onSet": [{
#                     "remap": [
#                         {"ifChanged": true, "uri": "/controls/pcs:VoltageSlewCmd@triggerCmd", "outValue": true},
#                         {"ifChanged": true, "uri": "/controls/pcs:VoltageSlewCmd"}
#                     ]
#                 }]
#             }
#         },
#         "VoltageSlewCmd": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "cmdVar": "/components/pcs_parameter_setting:off_grid_voltage_setpoint",
#             "actions": {
#                 "onSet": [{
#                     "func": [
#                         {"func": "HandleCmd", "amap": "pcs"}
#                     ]
#                 }]
#             }
#         },
#         "ActivePowerSetpoint": {
#             "value": 0,
#             "enable":"/controls/pcs:enable",
#             "note": "Set active power",
#             "cmdVar": "/controls/ess:ActivePowerSetpoint",
#             "enableAlert": false,
#             "checkCmdTimeout": 30,
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "ReactivePowerAdjSwitch": {
#             "value": 162,
#             "enable":"/controls/pcs:enable",
#             "note": "For Sungrow PCS, set the reactive power adjustment switch to 162 (0xA2) to enable reactive power setpoint",
#             "cmdVar": "/components/pcs_parameter_setting:reactive_power_adj_switch",
#             "enableAlert": false,
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "ReactivePowerSetpoint": {
#             "value": 2200.0,
#             "enable":"/controls/pcs:enable",
#             "note": "Set reactive power only if the reactive power adjustment switch is set to 162 (0xA2)",
#             "cmdVar": "/controls/ess:ReactivePowerSetpoint",
#             "enableAlert": false,
#             "checkCmdHoldTimeout": 0.2,
#             "checkCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/components/pcs_parameter_setting:reactive_power_adj_switch",
#             "useExpr": true,
#             "expression": "{1} == 162",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         }
#     }
# }
# }'

# fims_send -m set  -u /ess/cfg/cfile/ess/pcs_monitor '{
#     "pname":"ess",
#     "amname":"pcs",
#     "/controls/pcs":{   
#         "enable_run":false
#     },

#     "/schedule/wake_monitor/pcs": {
#         "/controls/pcs:StartCmd":                { "enable": false,  "amap": "pcs", "func":"HandleCmd"}
#     },

#     "/schedule/wake_monitor2/pcs": {
#         "/controls/pcs:ShutdownCmd":             { "enable": true,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:StandbyCmd":              { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyStartup":           { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyShutdown":          { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyStandby":           { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:HeartbeatTimeoutSetting": { "enable": false,  "amap": "pcs", "func":"HandleCmd"},

#         "/controls/pcs:OffGridMode":             { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:OnGridMode":              { "enable": false,  "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:OnGridConstACPower":      { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyOffGridMode":       { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyOnGridMode":        { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VerifyOnGridConstACPower":{ "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:CheckIfOffGrid":          { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:FrequencySetpoint":       { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:ActivePowerSetpoint":     { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:ReactivePowerAdjSwitch":  { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:ReactivePowerSetpoint":   { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:CheckIfStopped":          { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:CheckIfOnGrid":           { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VoltageSlewCmd":          { "enable": false,   "amap": "pcs", "func":"HandleCmd"},
#         "/controls/pcs:VoltageSlew":             { "enable": false,   "amap": "pcs", "func": "SlewVal"}
#     },
#     "/system/commands": {
#         "runMon":{
#             "value":0,
#             "aname":"pcs",
#             "monitor":"wake_monitor",
#             "enable":"/controls/pcs:enable",
#             "help": "load the wake monitor setup system",
#             "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"ess"}]}]}
#         },
#         "run":{
#             "value":"test",
#             "uri":"/system/commands:runMon",
#             "every":1,
#             "enable":"/controls/pcs:enable_runxx",
#             "help": "run the runMon schedule var",
#             "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
#         }

#     },
#     "/status/pcs": {
#         "SystemState": "Off",
#         "WorkMode":"On-grid",
#         "OffGridVoltageSetpoint":   440,
#         "OffGridFrequencySetpoint":   60.1,
       
#         "VerifyStartup": {
#             "value": 0,
#             "note": "Verify that the PCS system state is now running when the start command is sent",
#             "enableAlert": false,
#             "sendCmdHoldTimeout": 3,
#             "sendCmdTimeout": 30,
#             "numVars": 1,
#             "variable1": "/status/pcs:SystemState",
#             "useExpr": true,
#             "expression": "{1} == Running",
#             "actions": {
#                 "onSet": [{"func": [{"func": "HandleCmd", "amap": "pcs"}]}]
#             }
#         },
#         "VerifyStartupSuccess": {
#             "value": false,
#             "note": "If the PCS failed to start after an elasped time, send an alarm",
#             "actions": {
#                 "onSet": [{
#                     "remap": [
#                         {"inValue": false, "ifChanged": false, "uri": "/alarms/pcs:start_failure", "outValue": "Failed to verify PCS is running"}
#                     ]
#                 }]
#             }
#         }
#     },
#     "/status/bms": {
#        "DCClosed": true
#     }
# }'



fims_send -m get  -r /$$ -u /ess/naked/config/cfile | jq




#
# then we have to instanciate a pcs as , say pcs_1
# this really means creating a copy of the pcs amap as an amap called pcs_1
# we can create the base components as well   /status/<amap_name>/... etc 

# control can use the aname and then the amap names.
# if status == stop and request == run then status = precharge
# if status == precharge and prevolts < volts  precharge += precharge step 
# if status == precharge and prevolts >= volts  status = ready 

# if status == running and CurrentSetpointRequest > 0 and capacity > currentSetpoint*tdelta then outputcurrrent = ramped request current    


# # now setup  the interface for a PCS unit to interface with real components
# fims_send-m set -r /$$ -u /ess/vlinks/pcs_1 '{
#     "active_power":            {  "vlink": "/status/pcs:ActivePower",           "value": "/components/${PCS_ID}_running_info:active_power"},
#     "reactive_power":          {  "vlink": "/status/pcs:ReactivePower",         "value": "/components/${PCS_ID}_running_info:reactive_power"},
#     "grid_voltage_1":          {  "vlink": "/status/pcs:GridVoltage1",          "value": "/components/${PCS_ID}_running_info:grid_voltage_vab"},
#     "grid_voltage_2":          {  "vlink": "/status/pcs:GridVoltage2",          "value": "/components/${PCS_ID}_running_info:grid_voltage_vbc"},
#     "grid_voltage_3":          {  "vlink": "/status/pcs:GridVoltage3",          "value": "/components/${PCS_ID}_running_info:grid_voltage_vca"},
#     "grid_current_1":          {  "vlink": "/status/pcs:GridCurrent1",          "value": "/components/${PCS_ID}_running_info:phase_a_current"},
#     "grid_current_2":          {  "vlink": "/status/pcs:GridCurrent2",          "value": "/components/${PCS_ID}_running_info:phase_b_current"},
#     "grid_current_3":          {  "vlink": "/status/pcs:GridCurrent3",          "value": "/components/${PCS_ID}_running_info:phase_c_current"},
#     "grid_frequency":          {  "vlink": "/status/pcs:GridFrequency",         "value": "/components/${PCS_ID}_running_info:grid_frequency"},
#     "dc_power":                {  "vlink": "/status/pcs:DCPower",               "value": "/components/${PCS_ID}_running_info:dc_power"},
#     "dc_voltage":              {  "vlink": "/status/pcs:DCVoltage",             "value": "/components/${PCS_ID}_running_info:dc_voltage"},
#     "dc_current":              {  "vlink": "/status/pcs:DCCurrent",             "value": "/components/${PCS_ID}_running_info:dc_current"},
#     "power_factor":            {  "vlink": "/status/pcs:PowerFactor",           "value": "/components/${PCS_ID}_running_info:power_factor"},
#     "daily_charging":          {  "vlink": "/status/pcs:DailyChargeEnergy",     "value": "/components/${PCS_ID}_running_info:daily_charging"},
#     "daily_discharging":       {  "vlink": "/status/pcs:DailyDischargeEnergy",  "value": "/components/${PCS_ID}_running_info:daily_discharging"},
#     "total_charging":          {  "vlink": "/status/pcs:TotalChargeEnergy",     "value": "/components/${PCS_ID}_running_info:total_charging"},
#     "total_discharging":       {  "vlink": "/status/pcs:TotalDischargeEnergy",  "value": "/components/${PCS_ID}_running_info:total_discharging"},
#     "leakage_current":         {  "vlink": "/status/pcs:LeakageCurrent",        "value": "/components/${PCS_ID}_running_info:leakage_current"},
#     "ambient_temp":            {  "vlink": "/status/pcs:AmbientTemp",           "value": "/components/${PCS_ID}_running_info:ambient_temp"},
#     "module_temp_1":           {  "vlink": "/status/pcs:ModuleTemp1",           "value": "/components/${PCS_ID}_running_info:module_temp_1"},
#     "module_temp_2":           {  "vlink": "/status/pcs:ModuleTemp2",           "value": "/components/${PCS_ID}_running_info:module_temp_2"},
#     "module_temp_3":           {  "vlink": "/status/pcs:ModuleTemp3",           "value": "/components/${PCS_ID}_running_info:module_temp_3"},

#     "heartbeat_read":          {  "vlink": "/status/pcs:PCSHeartbeat",          "value": "/components/pcs_0_running_info:HeartbeatCount"},

#     "off_grid_voltage_setpoint":   {  "vlink": "/assets/pcs/${PCS_ID}:voltage_command",      "value": "/components/${PCS_ID}_running_info:off_grid_voltage_setpoint"},
#     "off_grid_frequency_setpoint": {  "vlink": "/assets/pcs/${PCS_ID}:frequency_command",    "value": "/components/${PCS_ID}_running_info:off_grid_frequency_setpoint"}
# }'


