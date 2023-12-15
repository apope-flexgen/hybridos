# echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"bms","func":"RunSched"}]}]}}}'
stepThrough=false

$stepThrough && read -n1 -p "Press any key to continue - 1";echo

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


$stepThrough && read -n1 -p "Press any key to continue - 2";echo

# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary  '
{
    "maint_mode": {
        "value": false
    },
    "close_contactors": {
        "name": "Close DC Contactor",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": false,
        "note": "Enabled conditions: ![/status/bms/DCClosed] && [/assets/bms/summary/maint_mode] && ![/status/bms/IsFaulted]",
        "ui_type": "control",
        "type": "enum_button",   
        "aname": "bms",
        "uri":"/sched/bms:LocalStartBMS",
        "every":0.5,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        }
    },
    "open_contactors": {
        "name": "Open DC Contactor",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": false,
        "note": "Enabled conditions: [/status/bms/DCClosed] && ![/status/pcs/DCClosed] && [/assets/bms/summary/maint_mode]",
        "ui_type": "control",
        "type": "enum_button",
        "aname": "bms",
        "uri":"/sched/bms:LocalStopBMS",
        "every":0.5,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"inValue":true, "func": "RunSched"}
                ]
            }]
        }
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 3";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/bms  '
{
    "CloseContactors": {
        "value": 2,
        "cmdVar": "/components/catl_bms_1_controls:ems_cmd",
        "maxCmdTries": 2,
        "checkCmdTimeout": 2,
        "checkCmdHoldTimeout": 2
    },
    "OpenContactors": {
        "value": 3,
        "cmdVar": "/components/catl_bms_1_controls:ems_cmd",
        "maxCmdTries": 2,
        "checkCmdTimeout": 2,
        "checkCmdHoldTimeout": 2
    },
    "VerifyCloseContactors": {
        "value": 0,
        "enableAlert": false,
        "numVars": 1,
        "variable1": "/status/bms:DCClosed",
        "useExpr": true,
        "expression": "{1}",
        "sendCmdTimeout": 5
    },
    "VerifyOpenContactors": {
        "value": 0,
        "enableAlert": false,
        "numVars": 1,
        "variable1": "/status/bms:DCClosed",
        "useExpr": true,
        "expression": "not {1}",
        "sendCmdTimeout": 5
    }
}'

$stepThrough && read -n1 -p "Press any key to continue - 4";echo

# BMS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms  '
{
    "enable": false
} '


$stepThrough && read -n1 -p "Press any key to continue - 5";echo

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


$stepThrough && read -n1 -p "Press any key to continue - 6";echo

# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/catl_bms_1_info  '
{
    "bms_poweron":{
        "value":0,
        "actions":{ "onSet":[{ "enum":[ {"inValue":0, "outValue":false,  "uri": "/status/bms:DCClosed"},
                                        {"inValue":1, "outValue":true,  "uri": "/status/bms:DCClosed"}
                                        ]}]}
    }
} '

# BMS control variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/catl_bms_1_controls  '
{
    "ems_cmd":{
        "value":0
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 7";echo


# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/bms  '
{
    "LocalStartBMS": {
        "every": 0.5,
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
        "every": 0.5,
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
    },
    "CloseContactorsEnable": {
        "value":    "CloseContactorsEnable",
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
        }
    },
    "OpenContactorsEnable": {
        "value":    "OpenContactorsEnable",
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
        }
    }
} '


$stepThrough && read -n1 -p "Press any key to continue - 8";echo


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
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:CloseContactorsEnable",
                "every":0.1,
                "offset":0,
                "debug":0
            },
            {
                "aname": "bms",
                "value":0, 
                "uri":"/sched/bms:OpenContactorsEnable",
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

$stepThrough && read -n1 -p "Press any key to continue - 9 - fims_send -m set -r /$$ -u /ess/config/bms/enable true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true

$stepThrough && read -n1 -p "Press any key to continue - 11 - fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true