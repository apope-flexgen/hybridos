# echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'


read -n1 -p "Press any key to continue - 1";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/status/bms  '
{
    "DCClosed": {
        "value": false
    },
    "IsFaulted": {
        "value": false
    }
} '


read -n1 -p "Press any key to continue - 2";echo

# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary  '
{
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
        "value": false
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
        "uri":"/sched/bms:LocalStartBMS",
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
        "uri":"/sched/bms:LocalStopBMS",
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
    "enable": false
} '


read -n1 -p "Press any key to continue - 5";echo

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


read -n1 -p "Press any key to continue - 6";echo


# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms  '
{
    "LocalStartBMS": {
        "value":    "LocalStartBMS",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "LocalStartBMS"
                        }
                    ]
                }
            ]
        }
    },
    "LocalStopBMS": {
        "value":    "LocalStopBMS",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "bms",
                            "func": "LocalStopBMS"
                        }
                    ]
                }
            ]
        }
    }
} '


read -n1 -p "Press any key to continue - 7";echo


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
                "value":0, 
                "uri":"/sched/bms:LocalStartBMS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:LocalStopBMS",
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

read -n1 -p "Press any key to continue - 9 - fims_send -m set -r /$$ -u /ess/config/bms/enable true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true

read -n1 -p "Press any key to continue - 9 - fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true

read -n1 -p "Press any key to continue - 10 - fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true

read -n1 -p "Press any key to continue - 11 - fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/close_contactors true