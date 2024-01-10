


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
        "actions":	{
            "onSet":	[{
                "remap":	[
                    {"inValue":true, "uri":	"/sched/bms:LocalStart_bms@endTime", "outValue": 0},
                    {"inValue":true, "uri":	"/sched/bms:LocalStart_bms", "outValue": "LocalStart_bms"}
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
                    {"inValue":true, "uri":	"/sched/bms:LocalStop_bms@endTime", "outValue": 0},
                    {"inValue":true, "uri":	"/sched/bms:LocalStop_bms", "outValue": "LocalStop_bms"}
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
    }, 
    "ContactorStatusConfig": {
        "value": 0,
        "closed": 1,
        "open": 0
    }, 
    "ContactorCommandConfig": {
        "value": 0,
        "close": 2,
        "open": 3
    }
}'

# ESS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/ess  '
{
    "enable": false
} '

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

# # Set up on scheduler
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/ess  '
# {
#     "LocalStart_ess": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "amap": "ess",
#                             "func": "LocalStart"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/ess:enable",
#         "value": 1
#     },
#     "LocalStop_ess": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "amap": "ess",
#                             "func": "LocalStop"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/ess:enable",
#         "value": 1
#     }
# } '

# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms  '
{
    "LocalStart_bms":{
        "value":    "LocalStart_bms",
        "actions":{
        "onSet": [{ "func": [
                {"func": "HandleSchedLoad", "amap": "bms"}
            ]}]
        },
        "uri":      "/sched/bms:LocalStart_bms",
        "fcn":      "LocalStart",
        "id":       "LocalStart_bms",
        "amap":     "bms",
        "refTime":  0.268,
        "runAfter": 0.270,
        "repTime":  0.100,
        "endTime":  0.001
    },
    "LocalStop_bms":{
        "value":    "LocalStop_bms",
        "actions":{
        "onSet": [{ "func": [
                {"func": "HandleSchedLoad", "amap": "bms"}
            ]}]
        },
        "uri":      "/sched/bms:LocalStop_bms",
        "fcn":      "LocalStop",
        "id":       "LocalStop_bms",
        "amap":     "bms",
        "refTime":  0.267,
        "runAfter": 0.270,
        "repTime":  0.100,
        "endTime":  0.001
    }
} '



/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true


read -n1 -p "Press any key to send true to /assets/bms/summary/close_contactors";echo

# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true

