# echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'
# echo -e "\n\nsetup stop command."
# fims_send -m set -r /$$ -u /ess/system/commands '{"stop":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"StopSched"}]}]}}}'
# set up the pub functions for the assets scheduler | setupReference needs to be replaced with setupModelName for each new datamap function that we want to run
# fims_send -m set -r /$$ -u /ess/demo/code '
#                     { "dm_AV":{"value":1, "enabled":true, 
#                     "thread_name":"THREAD1", "datamap_name":"DATAMAP1",
#                     "actions":{"onSet":[{"func":[
#                         {"amap": "ess", "func": "setupReference"},
#                         {"amap": "ess", "func": "runThread"}]}]}}}'
# schedule the function at this uri 



# set contactors enable variables

read -n1 -p "Press any key to continue - Post [config] | Pre [status]";echo

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


read -n1 -p "Press any key to continue - Post [status] | Pre [assets]";echo

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


read -n1 -p "Press any key to continue - Post [assets] | Pre [controls]";echo

# # BMS controls variables
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms  '
# {
#     "CloseContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:close_contactors",
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     },
#     "CloseContactorsEnable": {
#         "actions": {
#             "onSet": [
#                 {
#                     "remap": [
#                         {
#                             "uri": "/assets/bms/summary:close_contactors@enabled"
#                         },
#                         {
#                             "uri": "/status/bms:CloseContactorsEnabled"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/bms:enable",
#         "expression": "not {1} and {2} and not {3}",
#         "note": "Enable Close Contactors UI control if DC Contactors are opened, the BMS is not faulted, and the BMS is in maintenance mode",
#         "numVars": 3,
#         "useExpr": true,
#         "value": false,
#         "variable1": "/status/bms:DCClosed",
#         "variable2": "/assets/bms/summary:maint_mode",
#         "variable3": "/status/bms:IsFaulted"
#     },
#     "OpenContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:open_contactors",
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     },
#     "OpenContactorsEnable": {
#         "actions": {
#             "onSet": [
#                 {
#                     "remap": [
#                         {
#                             "uri": "/assets/bms/summary:open_contactors@enabled"
#                         },
#                         {
#                             "uri": "/status/bms:OpenContactorsEnabled"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enable": "/config/bms:enable",
#         "expression": "{1} and not {2} and {3}",
#         "note": "Enable Open Contactors UI control if DC Contactors are closed and the BMS is in maintenance mode",
#         "numVars": 3,
#         "useExpr": true,
#         "value": false,
#         "variable1": "/status/bms:DCClosed",
#         "variable2": "/status/pcs:DCClosed",
#         "variable3": "/assets/bms/summary:maint_mode"
#     },
#     "VerifyCloseContactors": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "amap": "bms",
#                             "func": "HandleCmd"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enableAlert": false,
#         "expression": "{1}",
#         "numVars": 1,
#         "sendCmdTimeout": 10,
#         "useExpr": true,
#         "value": 0,
#         "variable1": "/status/bms:DCClosed"
#     },
#     "VerifyOpenContactors": {
#         "actions": {
#             "onSet": [
#                 {
#                     "func": [
#                         {
#                             "amap": "bms",
#                             "func": "HandleCmd"
#                         }
#                     ]
#                 }
#             ]
#         },
#         "enableAlert": false,
#         "expression": "not {1}",
#         "numVars": 1,
#         "sendCmdTimeout": 10,
#         "useExpr": true,
#         "value": 0,
#         "variable1": "/status/bms:DCClosed"
#     }
# } '


# # BMS controls variables
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms  '
# {
#     "CloseContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:close_contactors",
#         "statusVar": "/components/bms:status",
#         "ClosedStatusValue": 1,
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     },
#     "OpenContactors": {
#         "value": 0,
#         "cmdVar": "/components/bms:open_contactors",
#         "statusVar": "/components/bms:status",
#         "OpenStatusValue": 0,
#         "checkCmdTimeout": 3,
#         "sendCmdTimeout": 3,
#         "triggerCmd": false
#     }
# } '

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


read -n1 -p "Press any key to continue - Post [assets] | Pre [controls]";echo

# # BMS controls variables
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/schedule/wake_monitor/bms  '
# {
#     "/controls/bms:VerifyCloseContactors": {
#         "amap": "bms",
#         "func": "HandleCmd"
#     },
#     "/controls/bms:VerifyOpenContactors": {
#         "amap": "bms",
#         "func": "HandleCmd"
#     }
# } '


read -n1 -p "Press any key to continue - Post [sched] | Pre [config]";echo

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


read -n1 -p "Press any key to continue - Post [controls] | Pre [components]";echo

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

read -n1 -p "Press any key to continue - Post [components] | Pre [pcs]";echo

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


read -n1 -p "Press any key to continue - Pre [sched]";echo

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
    },
    "CloseContactorsEnable_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "CloseContactorsEnable"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "value": 1
    },
    "OpenContactorsEnable_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "OpenContactorsEnable"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "value": 1
    },
    "VerifyCloseContactors_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "VerifyCloseContactors"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "value": 1
    },
    "VerifyOpenContactors_bms": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "VerifyOpenContactors"
                        }
                    ]
                }
            ]
        },
        "enable": "/config/bms:enable",
        "value": 1
    }
} '

# read -n1 -p "Press any key to continue - Pre [CloseContactors /commands/run]";echo


# fims_send -m set -r /$$ -u /ess/system/commands/run '
# {
#     "value":0, 
#     "uri":"/sched/bms:CloseContactors",
#     "every":10,
#     "offset":0,
#     "debug":0
# }
# '

# read -n1 -p "Press any key to continue - Pre [OpenContactors /commands/run]";echo


# fims_send -m set -r /$$ -u /ess/system/commands/run '
# {
#     "value":0, 
#     "every":10,
#     "offset":0,
#     "debug":0
# }
# '


read -n1 -p "Press any key to continue - Pre [system/commands]";echo


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
            },
            {
                "aname": "bms",
                "value":1, 
                "uri":"/sched/bms:CloseContactorsEnable_bms",
                "every":0.1
            },
            {
                "aname": "bms",
                "value":1, 
                "uri":"/sched/bms:OpenContactorsEnable_bms",
                "every":0.1
            },
            {
                "aname": "bms",
                "value":1, 
                "uri":"/sched/bms:VerifyCloseContactors_bms",
                "every":0.1
            },
            {
                "aname": "bms",
                "value":1, 
                "uri":"/sched/bms:VerifyOpenContactors_bms",
                "every":0.1
            }
        ],
        "targav": "/system/commands:run",
        "value": true
    }
}
'



/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true
# /usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms/CloseContactorsEnable true

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors@enabled true

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true






    # "start": {
    #     "name": "Close DC Contactor",
    #     "value": false,
    #     "unit": "",
    #     "scaler": 0,
    #     "enabled": false,
    #     "ui_type": "control",
    #     "type": "enum_button",
    #     "actions":	{
    #         "onSet":	[{
    #             "remap":	[
    #                 {"inValue":true, "uri":	"/sched/bms:schedCloseContactors@endTime", "outValue": 0},
    #                 {"inValue":true, "uri":	"/sched/bms:schedCloseContactors", "outValue": "schedCloseContactors"}
    #             ]
    #         }]
    #     },

    #     "options": [
    #         {
    #             "name": "No",
    #             "return_value": false
    #         },
    #         {
    #             "name": "Yes",
    #             "return_value": true
    #         }
    #     ]
    # },

    # "stop": {
    #     "name": "Open DC Contactor",
    #     "value": false,
    #     "unit": "",
    #     "scaler": 0,
    #     "enabled": false,
    #     "ui_type": "control",
    #     "type": "enum_button",
    #     "actions":	{
    #         "onSet":	[{
    #             "remap":	[
    #                 {"inValue":true, "uri":	"/sched/bms:schedOpenContactors@endTime", "outValue": 0},
    #                 {"inValue":true, "uri":	"/sched/bms:schedOpenContactors", "outValue": "schedOpenContactors"}
    #             ]
    #         }]
    #     },

    #     "options": [
    #         {
    #             "name": "No",
    #             "return_value": false
    #         },
    #         {
    #             "name": "Yes",
    #             "return_value": true
    #         }
    #     ]
    # },
