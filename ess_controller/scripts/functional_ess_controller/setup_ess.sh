# echo -e "\n\nset up run command."
fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"bms","func":"RunSched"}]}]}}}'

fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"pcs","func":"RunSched"}]}]}}}'

fims_send -m set -r /$$ -u /ess/system/commands '{"run":{"value":"test","ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'
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
    },
    "CurrentBeforeStopIsOK": {
        "value": true
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 2";echo
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

$stepThrough && read -n1 -p "Press any key to continue - 3";echo

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

$stepThrough && read -n1 -p "Press any key to continue - 4";echo
# BMS UI variables
/usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary  '
{
    "maint_mode": {
        "value": false
    },
    "start": {
        "name": "Start PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": false,
        "note": "Enabled conditions: [/status/bms/DCClosed] && [/assets/pcs/summary/maint_mode] && ![/status/pcs/IsFaulted]",
        "ui_type": "control",
        "type": "enum_button",   
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStartPCS",
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
    "stop": {
        "name": "Stop PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": false,
        "note": "Enabled conditions: [/assets/pcs/summary/maint_mode] && [/status/pcs/SystemState != Stop]",
        "ui_type": "control",
        "type": "enum_button",
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStopPCS",
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
    "standby": {
        "name": "Standby PCS",
        "value": false,
        "unit": "",
        "scaler": 0,
        "enabled": false,
        "note": "Enabled conditions: [/status/bms/DCClosed] && [/assets/pcs/summary/maint_mode] && [/status/pcs/SystemState == (Stop || Run)]",
        "ui_type": "control",
        "type": "enum_button",
        "aname": "pcs",
        "uri":"/sched/pcs:LocalStandbyPCS",
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


$stepThrough && read -n1 -p "Press any key to continue - 5";echo
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

$stepThrough && read -n1 -p "Press any key to continue - 6";echo
/usr/local/bin/fims_send -m set -r /$$ -u /ess/controls/pcs  '
{
    "Start": {
        "value": 7,
        "cmdVar": "/components/sungrow_pcs_controls:state_cmd",
        "maxCmdTries": 2,
        "checkCmdTimeout": 2,
        "checkCmdHoldTimeout": 2
    },
    "Stop": {
        "value": 5,
        "cmdVar": "/components/sungrow_pcs_controls:state_cmd",
        "maxCmdTries": 2,
        "checkCmdTimeout": 2,
        "checkCmdHoldTimeout": 2
    },
    "Standby": {
        "value": 6,
        "cmdVar": "/components/sungrow_pcs_controls:state_cmd",
        "maxCmdTries": 2,
        "checkCmdTimeout": 2,
        "checkCmdHoldTimeout": 2
    },
    "VerifyStart": {
        "value": 0,
        "enableAlert": false,
        "numVars": 1,
        "variable1": "/status/pcs:SystemState",
        "useExpr": true,
        "expression": "{1} == Run",
        "sendCmdTimeout": 5
    },
    "VerifyStop": {
        "value": 0,
        "enableAlert": false,
        "numVars": 1,
        "variable1": "/status/pcs:SystemState",
        "useExpr": true,
        "expression": "{1} == Stop",
        "sendCmdTimeout": 5
    },
    "VerifyStandby": {
        "value": 0,
        "enableAlert": false,
        "numVars": 1,
        "variable1": "/status/pcs:SystemState",
        "useExpr": true,
        "expression": "{1} == Standby",
        "sendCmdTimeout": 5
    }
}'


$stepThrough && read -n1 -p "Press any key to continue - 7";echo
# BMS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms  '
{
    "enable": false
} '

$stepThrough && read -n1 -p "Press any key to continue - 8";echo
# BMS config variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/pcs  '
{
    "enable": false
} '


$stepThrough && read -n1 -p "Press any key to continue - 9";echo
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

$stepThrough && read -n1 -p "Press any key to continue - 10";echo
# BMS control variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/catl_bms_1_controls  '
{
    "ems_cmd":{
        "value":0
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 11";echo
# BMS status variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/sungrow_pcs_info  '
{
    "pcs_state":{
        "value":0,
        "actions":{ "onSet":[{ "enum":[ {"inValue": 1, "outValue": "Stop",  "uri": "/status/pcs:SystemState"},
                                        {"inValue": 2, "outValue": "Standby",  "uri": "/status/pcs:SystemState"},
                                        {"inValue": 3, "outValue": "Run",  "uri": "/status/pcs:SystemState"}
                                        ]}]}
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 12";echo
# BMS control variables
/usr/local/bin/fims_send -m set -r /$$ -u /ess/components/sungrow_pcs_controls  '
{
    "state_cmd":{
        "value":0
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 13";echo
# Site variables
/usr/local/bin/fims_send -m set -r /$$ -u /site/ess  '
{
    "site_status_control_command": {

        "enable": "/config/pcs:enable",
        "ifChanged": false,
        "name": "Start/Stop/Standby Command",
        "note": "Initiates ESS start, standby, or stop command routines",
        "size": 1,
        "type": "control",
        "value": -1,
        "note": "0->start | 1->stop | 2->standby",

        "aname": "ess",
        "uri":"/sched/ess:SiteRunCmd",
        "every":0.5,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"func": "RunSched"}
                ]
            }]
        }
    },

    "bms_dc_contactor_control": {

        "enable": "/config/pcs:enable",
        "ifChanged": false,
        "name": "BMS Contactor Control Command",
        "note": "Initiates BMS close and open contactors",
        "size": 1,
        "type": "control",
        "value": 0,
        "note": "2->close contactors | 3->open contactors",

        "aname": "ess",
        "uri":"/sched/ess:SiteBMSContactorControl",
        "every":0.5,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"func": "RunSched"}
                ]
            }]
        }
    },

    "start_stop_standby_command": {

        "enable": "/config/pcs:enable",
        "ifChanged": false,
        "name": "Start/Stop/Standby Command",
        "note": "Initiates PCS start, standby, or stop command routines",
        "size": 1,
        "type": "control",
        "value": -1,
        "note": "0->stop | 1->start | 2->standby",

        "aname": "ess",
        "uri":"/sched/ess:SitePCSStatusControl",
        "every":0.5,
        "offset":0,
        "debug":0,
        "actions":	{
            "onSet":	[{
                "func":	[
                    {"func": "RunSched"}
                ]
            }]
        }
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 14";echo
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

$stepThrough && read -n1 -p "Press any key to continue - 15";echo
# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/pcs  '
{
    "LocalStartPCS": {
        "every": 0.5,
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
        "every": 0.5,
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
        "every": 0.5,
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
    },
    "StartEnable": {
        "value":    "StartEnable",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "StartEnable"
                        }
                    ]
                }
            ]
        }
    },
    "StopEnable": {
        "value":    "StopEnable",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "StopEnable"
                        }
                    ]
                }
            ]
        }
    },
    "StandbyEnable": {
        "value":    "StandbyEnable",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "pcs",
                            "func": "StandbyEnable"
                        }
                    ]
                }
            ]
        }
    }
} '

$stepThrough && read -n1 -p "Press any key to continue - 16";echo
# Set up on scheduler
/usr/local/bin/fims_send -m set -r /$$ -u /ess/sched/ess  '
{
    "SiteRunCmd": {
        "every": 0.5,
        "value":    "SiteRunCmd",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "ess",
                            "func": "SiteRunCmd"
                        }
                    ]
                }
            ]
        }
    },
    "SiteBMSContactorControl": {
        "every": 0.5,
        "value":    "SiteBMSContactorControl",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "ess",
                            "func": "SiteBMSContactorControl"
                        }
                    ]
                }
            ]
        }
    },
    "SitePCSStatusControl": {
        "every": 0.5,
        "value":    "SitePCSStatusControl",
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "ess",
                            "func": "SitePCSStatusControl"
                        }
                    ]
                }
            ]
        }
    }
} '


$stepThrough && read -n1 -p "Press any key to continue - 17";echo
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
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:LocalStartPCS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:LocalStopPCS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:LocalStandbyPCS",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:StartEnable",
                "every":0.1,
                "offset":0,
                "debug":0
            },
            {
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:StopEnable",
                "every":0.1,
                "offset":0,
                "debug":0
            },
            {
                "aname": "pcs",
                "value":0, 
                "uri":"/sched/pcs:StandbyEnable",
                "every":0.1,
                "offset":0,
                "debug":0
            }
        ],
        "targav": "/system/commands:run",
        "value": true
    },
    "runOpts_ess": {
        "actions": {
            "onSet": [
                {
                    "func": [
                        {
                            "amap": "ess",
                            "func": "SchedItemOpts"
                        }
                    ]
                }
            ]
        },
        "enabled": true,
        "note1": "Sets schedule variables (defined in /sched/ess) to /system/commands:run",
        "note2": "This allows the ESS Controller to kick off scheduled tasks based on the provided variables",
        "options": [
            {
                "aname": "ess",
                "value":0, 
                "uri":"/sched/ess:SiteRunCmd",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "ess",
                "value":0, 
                "uri":"/sched/ess:SiteBMSContactorControl",
                "every":0,
                "offset":0,
                "debug":0
            },
            {
                "aname": "ess",
                "value":0, 
                "uri":"/sched/ess:SitePCSStatusControl",
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



$stepThrough && read -n1 -p "Press any key to continue - 18";echo


$stepThrough && read -n1 -p "Press any key to continue - 19 - fims_send -m set -r /$$ -u /ess/config/bms/enable true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/bms/enable true

# $stepThrough && read -n1 -p "Press any key to continue - 20 - fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true";echo

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/bms/summary/maint_mode true

$stepThrough && read -n1 -p "Press any key to continue - 21 - fims_send -m set -r /$$ -u /ess/config/pcs/enable true";echo

/usr/local/bin/fims_send -m set -r /$$ -u /ess/config/pcs/enable true

# $stepThrough && read -n1 -p "Press any key to continue - 22 - fims_send -m set -r /$$ -u /assets/pcs/summary/maint_mode true";echo

# /usr/local/bin/fims_send -m set -r /$$ -u /assets/pcs/summary/maint_mode true


/usr/local/bin/fims_send -m set -r /$$ -u /mecho/accept_commands true
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/bms_conditions_met true
/usr/local/bin/fims_send -m set -r /$$ -u /mecho/pcs_conditions_met true