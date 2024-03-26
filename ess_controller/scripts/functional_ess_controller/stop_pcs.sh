echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'


read -n1 -p "Press any key to continue - 1";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/bms  '
{
    "DCClosed": {
        "value": false
    },
    "CurrentBeforeStopIsOK": {
        "value": true
    },
    "FaultCnt": 0,
    "IsFaulted": {
        "enable": "/config/bms:enable",
        "expression": "{1} or {2} > 0",
        "numVars": 2,
        "useExpr": true,
        "value": false,
        "variable1": "/status/bms:SystemFault",
        "variable2": "/status/bms:TotalFaultCnt"
    },
    "SystemFault": false,
    "TotalFaultCnt": {
        "enable": "/config/bms:enable",
        "numVars": 2,
        "operation": "+",
        "value": 0,
        "variable1": "bms:TotalFaultCnt",
        "variable2": "/status/bms:FaultCnt"
    },
    "Status": "INIT"
} '

# // "enable": "/config/pcs:enable",

# // "variable1": "/status/bms:DCClosed",
# // "variable1": "/status/bms:CurrentBeforeStopIsOK",


# // "variable2": "/assets/pcs/summary:maint_mode",

# // "variable3": "/status/pcs:SystemState",
# // "variable4": "/status/pcs:IsFaulted"



read -n1 -p "Press any key to continue - 2";echo

# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary  '
{
    "clear_faults": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "inValue": true,
                            "outValue": true,
                            "uri": "/controls/bms:ClearFaults@triggerCmd"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/bms:ClearFaults"
                        }
                    ]
                }
            ]
        },
        "enabled": false,
        "ifChanged": false,
        "note": "UI control variable used to initiate clear faults command routine",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    },
    "maint_mode": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "uri": "/assets/bms/summary:clear_faults@enabled"
                        }
                    ]
                }
            ]
        },
        "enabled": true,
        "ifChanged": false,
        "note": "UI control variable used to set the current asset in maintenance mode",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": true
    },
    "close_contactors": {
        "name": "Close DC Contactor",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "control",
        "type": "enum_button",   
        "aname": "bms",
        "uri":"/sched/bms:LocalStartBMS_bms",
        "every":0.1,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        },
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ]
    },
    "open_contactors": {
        "name": "Open DC Contactor",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "control",
        "type": "enum_button",
        "aname": "bms",
        "uri":"/sched/bms:LocalStopBMS_bms",
        "every":0.1,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        },
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ]
    },
    "dc_contactors_closed": {
        "value": false
    }
} '

read -n1 -p "Press any key to continue - 3";echo

# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary  '
{
    "clear_faults": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "inValue": true,
                            "outValue": true,
                            "uri": "/controls/pcs:ClearFaults@triggerCmd"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:ClearFaults"
                        }
                    ]
                }
            ]
        },
        "enabled": false,
        "ifChanged": false,
        "note": "UI control variable used to initiate clear faults command routine",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    },
    "maint_mode": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "uri": "/assets/pcs/summary:clear_faults@enabled"
                        },
                        {
                            "uri": "/assets/pcs/summary:active_power_ramp_rate@enabled"
                        },
                        {
                            "uri": "/assets/pcs/summary:reactive_power_ramp_rate@enabled"
                        }
                    ]
                }
            ]
        },
        "enabled": true,
        "ifChanged": false,
        "note": "UI control variable used to set the current asset in maintenance mode",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    },
    "start": {
        "name": "Start PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "control",
        "type": "enum_button",   
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStartPCS",
        "every":0.1,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        },
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ]
    },
    "stop": {
        "name": "Stop PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "control",
        "type": "enum_button",
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStopPCS",
        "every":0.1,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        },
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ]
    },
    "standby": {
        "name": "Standby PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "control",
        "type": "enum_button",
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStandbyPCS",
        "every":0.1,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        },
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ]
    },

    "standbyx": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:ActivePowerSetpoint"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:ReactivePowerSetpoint"
                        },
                        {
                            "inValue": true,
                            "outValue": true,
                            "uri": "/controls/pcs:Standby@triggerCmd"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:Standby"
                        }
                    ]
                }
            ]
        },
        "enabled": false,
        "ifChanged": false,
        "note": "UI control variable used to initiate the standby command routine",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    },
    "startx": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "inValue": true,
                            "outValue": true,
                            "uri": "/controls/pcs:Start@triggerCmd"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:Start"
                        }
                    ]
                }
            ]
        },
        "enabled": false,
        "ifChanged": false,
        "note": "UI control variable used to initiate the start command routine",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    },
    "stopx": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LogInfo"
                        }
                    ]
                },
                {
                    "remap": [
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:ActivePowerSetpoint"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:ReactivePowerSetpoint"
                        },
                        {
                            "inValue": true,
                            "outValue": true,
                            "uri": "/controls/pcs:Stop@triggerCmd"
                        },
                        {
                            "inValue": true,
                            "outValue": 0,
                            "uri": "/controls/pcs:Stop"
                        }
                    ]
                }
            ]
        },
        "enabled": false,
        "ifChanged": false,
        "note": "UI control variable used to initiate the stop command routine",
        "options": [
            {
                "name": "No",
                "return_value": false
            },
            {
                "name": "Yes",
                "return_value": true
            }
        ],
        "value": false
    }
} '


read -n1 -p "Press any key to continue - 4";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms  '
{
    "CloseContactors": {
        "value": 0,
        "cmdVar": "/components/bms:close_contactors",
        "checkCmdTimeout": 3,
        "sendCmdTimeout": 3,
        "triggerCmd": false
    },
    "OpenContactors": {
        "value": 0,
        "cmdVar": "/components/bms:open_contactors",
        "checkCmdTimeout": 3,
        "sendCmdTimeout": 3,
        "triggerCmd": false
    }
}'

read -n1 -p "Press any key to continue - 5";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/pcs  '
{
    "Start": {
        "value": 1,
        "cmdVar": "/components/pcs:running_state",
        "checkCmdTimeout": 3,
        "sendCmdTimeout": 3,
        "triggerCmd": false
    },
    "Stop": {
        "value": 0,
        "cmdVar": "/components/pcs:running_state",
        "checkCmdTimeout": 3,
        "sendCmdTimeout": 3,
        "triggerCmd": false
    },
    "Standby": {
        "value": 2,
        "cmdVar": "/components/pcs:running_state",
        "checkCmdTimeout": 3,
        "sendCmdTimeout": 3,
        "triggerCmd": false
    }
}'


read -n1 -p "Press any key to continue - 6";echo

# BMS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms  '
{
    "AlarmDestination": "/assets/bms/summary:alarms",
    "CurrentBeforeStopThreshold": 35,
    "FaultDestination": "/assets/bms/summary:faults",
    "NoAlarmMsg": "Normal",
    "NoFaultMsg": "Normal",
    "enable": false,
    "enum_opt": false,
    "close_contactors_enable": false,
    "open_contactors_enable": false
} '

read -n1 -p "Press any key to continue - 7";echo

# BMS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/pcs  '
{
    "enable": false,
    "enum_opt": false
} '


read -n1 -p "Press any key to continue - 8";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/pcs  '
{
    "SystemState": {
        "value": "Stop"
    },
    "DCClosed": {
        "value": false
    },
    "IsFaulted": {
        "value": false
    }
} '

read -n1 -p "Press any key to continue - 9";echo


# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/pcs  '
{
    "runMonitor_pcs": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "RunMonitor"
                        }
                    ]
                }
            ]
        },
        "aname": "pcs",
        "enable": "/config/pcs:enable",
        "monitor": "wake_monitor",
        "note": "Periodically runs functions for variables defined in /schedule/wake_monitor/bms",
        "value": 1
    },
    "LocalStartPCS": {
        "value":    "LocalStartPCS",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LocalStartPCS"
                        }
                    ]
                }
            ]
        }
    },
    "LocalStopPCS": {
        "value":    "LocalStopPCS",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LocalStopPCS"
                        }
                    ]
                }
            ]
        }
    },
    "LocalStandbyPCS": {
        "value":    "LocalStandbyPCS",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "LocalStandbyPCS"
                        }
                    ]
                }
            ]
        }
    }
} '


read -n1 -p "Press any key to continue - 10";echo


fims_send -m set -r /$$ -u /ess/system/commands '
{
    "runOpts_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "SchedItemOpts"
                        }
                    ]
                }
            ]
        },
        "enabled": true,
        "note1": "Sets schedule variables (defined in /sched/bms) to /system/commands:run",
        "note2": "This allows the ESS Controller to kick off scheduled tasks based on the provided variables",
        "options": [
            {
                "aname": "bms",
                "every": 0.1,
                "uri": "/sched/bms:runMonitor_bms",
                "value": 1
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStartBMS_bms",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStopBMS_bms",
                "every":0,
                "offset":0,
                "debug":0
            }
        ],
        "targav": "/system/commands:run",
        "value": true
    },
    "runOpts_pcs": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "SchedItemOpts"
                        }
                    ]
                }
            ]
        },
        "enabled": true,
        "note1": "Sets schedule variables (defined in /sched/pcs) to /system/commands:run",
        "note2": "This allows the ESS Controller to kick off scheduled tasks based on the provided variables",
        "options": [
            {
                "aname": "bms",
                "every": 0.1,
                "uri": "/sched/bms:runMonitor_pcs",
                "value": 1
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStartPCS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStopPCS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStandbyPCS",
                "every":0,
                "offset":0,
                "debug":0
            }
        ],
        "targav": "/system/commands:run",
        "value": true
    }
}
'

read -n1 -p "Press any key to continue - 11";echo


/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/pcs/enable true

read -n1 -p "Press any key to continue - 12 - fims_send -m set -r /$$ -u /assets/pcs/summary/maint_mode true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/maint_mode true

read -n1 -p "Press any key to continue - 13 - fims_send -m set -r /$$ -u /ess/status/bms/DCClosed true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/bms/DCClosed true


read -n1 -p "Press any key to continue - 15 - fims_send -m set -u /ess/status/pcs/SystemState '{"value": "Running"}'";echo

/usr/local/bin/fims_send -m set -u /ess/status/pcs/SystemState '{"value": "Running"}'


read -n1 -p "Press any key to continue - 16 - fims_send -m set -r /$$ -u /assets/pcs/summary/stop true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/stop true

read -n1 -p "Press any key to continue - 17 - fims_send -m set -u /ess/status/pcs/SystemState '{"value": "Stop"}'";echo

/usr/local/bin/fims_send -m set -u /ess/status/pcs/SystemState '{"value": "Stop"}'

