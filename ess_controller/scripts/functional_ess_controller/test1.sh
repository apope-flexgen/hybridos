# echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'


read -n1 -p "Press any key to continue - 1";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/bms  '
{
    "DCClosed": {
        "value": false
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
        "enabled": false,
        "ui_type": "control",
        "type": "enum_button",
        "actions":	{
            "onSet":	[{
                "remap":	[
                    {"inValue":true, "uri":	"/config/bms:close_contactors_enable", "outValue": true},
                    {"inValue":false, "uri":	"/config/bms:close_contactors_enable", "outValue": true},
                    {"inValue":true, "uri":	"/controls/bms:CloseContactors@triggerCmd", "outValue": true},
                    {"inValue":false, "uri":	"/controls/bms:CloseContactors@triggerCmd", "outValue": true}
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
        "enabled": false,
        "ui_type": "control",
        "type": "enum_button",
        "actions":	{
            "onSet":	[{
                "remap":	[
                    {"inValue":true, "uri":	"/config/bms:open_contactors_enable", "outValue": true},
                    {"inValue":false, "uri":	"/config/bms:open_contactors_enable", "outValue": true},
                    {"inValue":true, "uri":	"/controls/bms:OpenContactors@triggerCmd", "outValue": true},
                    {"inValue":false, "uri":	"/controls/bms:OpenContactors@triggerCmd", "outValue": true}
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


read -n1 -p "Press any key to continue - 4";echo

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


read -n1 -p "Press any key to continue - 5";echo

# Component variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/bms  '
{
    "status": {
        "actions": {
            "onSet": [
                {
                    "remap": [
                        {
                            "inValue": 0,
                            "outValue": "DC Contactors Open",
                            "uri": "/status/bms:Status"
                        },
                        {
                            "inValue": 1,
                            "outValue": "DC Contactors Closed",
                            "uri": "/status/bms:Status"
                        },
                        {
                            "inValue": 0,
                            "outValue": false,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "inValue": 1,
                            "outValue": true,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "enable": "/config/bms:enum_opt",
                            "outValue": true,
                            "uri": "/components/bms:status@ifChanged",
                            "useAv": true
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "ifChanged": false,
        "value": 0
    },
    "close_contactors": {
        "actions": {
            "onSet": [
                {
                    "remap": [
                        {
                            "inValue": true, 
                            "uri": "/sched/bms:CloseContactors@endTime", 
                            "outValue": 0
                        },
                        {
                            "inValue": true, 
                            "uri": "/sched/bms:CloseContactors", 
                            "outValue": "CloseContactors"
                        },
                        {
                            "inValue": 1,
                            "outValue": "Start",
                            "uri": "/status/bms:Status"
                        },
                        {
                            "inValue": 0,
                            "outValue": false,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "inValue": 1,
                            "outValue": true,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "enable": "/config/bms:enum_opt",
                            "outValue": true,
                            "uri": "/components/bms:status@ifChanged",
                            "useAv": true
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "ifChanged": false,
        "value": 0
    },
    "open_contactors": {
        "actions": {
            "onSet": [
                {
                    "remap": [
                        {
                            "inValue": true, 
                            "uri": "/sched/bms:schedOpenContactors@endTime", 
                            "outValue": 0
                        },
                        {
                            "inValue": true, 
                            "uri": "/sched/bms:schedOpenContactors", 
                            "outValue": "schedOpenContactors"
                        },
                        {
                            "inValue": "TODO: add input value to compare against value field or remove remap entry",
                            "outValue": "TODO: add output value to set to uri",
                            "uri": "/status/bms:Status"
                        },
                        {
                            "inValue": "TODO: add input value to compare against value field or remove remap entry",
                            "outValue": false,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "inValue": "TODO: add input value to compare against value field or remove remap entry",
                            "outValue": true,
                            "uri": "/status/bms:DCClosed"
                        },
                        {
                            "enable": "/config/bms:enum_opt",
                            "outValue": true,
                            "uri": "/components/bms:status@ifChanged",
                            "useAv": true
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "ifChanged": false,
        "value": 0
    },
    "dc_contactors_closed": {
        "value": false
    }
} '

read -n1 -p "Press any key to continue - 6";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/pcs  '
{
    "SystemState": {
        "value": "Stop"
    },
    "DCClosed": {
        "value": false
    }
} '


read -n1 -p "Press any key to continue - 7";echo

# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms  '
{
    "runMonitor_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "RunMonitor"
                        }
                    ]
                }
            ]
        },
        "aname": "bms",
        "enable": "/config/bms:enable",
        "monitor": "wake_monitor",
        "note": "Periodically runs functions for variables defined in /schedule/wake_monitor/bms",
        "value": 1
    },
    "CloseContactors_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "CloseContactors"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:close_contactors_enable",
        "value": 1
    },
    "OpenContactors_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "OpenContactors"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:open_contactors_enable",
        "value": 1
    }
} '


read -n1 -p "Press any key to continue - 8";echo


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
                "uri":"/sched/bms:CloseContactors_bms",
                "every":0.1,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:OpenContactors_bms",
                "every":0.1,
                "offset":0,
                "debug":0
            }
        ],
        "targav": "/system/commands:run",
        "value": true
    }
}
'

read -n1 -p "Press any key to continue - 9";echo


/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms/CloseContactorsEnable true

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors@enabled true

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true
