#!/bin/sh
# simple demo of a battery charger using the ess_controller
# p wilshire 03-27-2022
#
#  Enter a _v or -ve charge current and the system will charge up to Max capacity Create a simple battery charger using the ess_controller
#  we dont yet have start, stop , standby commands
#  we do have a capacity, 
# we dont have an soc
#  we will allow charge / discharge power.

# set up a base config that does just about all we need.

ess_init='{
    "pname":"ess",
    "amname":"bms",
    "/controls/bms":{
        "enable":true
    },
    "/status/bms": {
        "Cap": 1,
        "CurrentCap": {
            "value":200,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "fims":"set", "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },
        "Charge": 0,
        "Max": 3000,
        "Min": 125,

        "CheckCharge": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:Charge",
            "expression": "{1} > 0",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:DischargeCap@enabled",  "outValue":false            },
                        { "inValue": true,  "uri": "/status/bms:ChargeCap@enabled",     "outValue":true             },
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    },
                        { "inValue": false, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": false, "uri": "/status/bms:DischargeCap@enabled",  "outValue":true             },
                        { "inValue": false, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": false, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
        },

        "DischargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:Charge",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} > {4}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },
        "ChargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 4,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:CurrentCap",
            "variable2": "/status/bms:Charge",
            "variable3": "/status/bms:Max",
            "variable4": "/status/bms:Min",
            "expression": "if (({1} < {3}), ({1} + {2}), {1})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        }
    },

    "/schedule/monitor/bms": {
        "/status/bms:CheckCharge":            { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:ChargeCap":              { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:DischargeCap":           { "amap": "bms", "func":"CalculateVar"}
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "debug":false,
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"debug":false, "func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
}'

echo $ess_init > /tmp/ess_init
fims_send -f /tmp/ess_init -m set  -u /ess/cfg/cfile/ess/ess_init 


exit 0

# first set up a basic system 
fims_send -m set  -u /ess/cfg/cfile/ess/bms_init '{
    "pname":"ess",
    "amname":"bms",
    "/status/bms": {
        "MaxCapacity":3000,
        "CurrentCapacity":0,
        "Status":"Init",
        "Soc":0,
        "Voltage":0,
        "Power":0
    },
    "/config/bms": {
        "MaxCapacity":3000,
        "MaxChargeCurrent":-2000,
        "MaxDischargeCurrent":2000,
        "Voltage":0,
        "Power":0
    },
    "/controls/bms": {
        "Start":false,
        "Stop":false,
        "Standby":false,
        "ChargeCurrent":0,
        "DischargeCurrent":0
    },

    "/assets/bms":{
        "CurrentState":"init",
        "SOC": 0,
        "Voltage":0,
        "Current":0,
        "Power":0
    }
}'

#
# next set up some reactions to the start / stop commands
#
fims_send -m set  -u /ess/cfg/cfile/ess/bms_controls '{
    "/status/bms": {
        "test2": {
            "value":2,
            "actions":{
                "onSet":[
                {
                    "xremap":
                    [
                        {"fims":"set","uri":"/status/bms:test2"}
                    ]
                }
            ]}
        },
        "test3": 12,
        "test": {
            "value":0,
            "debug":true,
            "useExpr": false,
            "includeCurrVal":true,
            "numVars": 2,
            "variable1": "/status/bms:test2",
            "variable2": "/status/bms:test3",
            "expression": "{1} + {2}",
            "operation":"+",

            "actions":{
                "onSet":[
                {
                    "remap":
                    [
                        {"enable":false, "fims":"set","uri":"/status/bms:test"},
                        {"enable":false, "uri":"/status/bms:test2"}
                    ]
                },
                {
                    "func":
                    [
                        {"debug":false, "func":"CalculateVar"}
                    ]
                }
            ]
            }

        },

        "StartCount": {
            "value":0,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:Status",
            "variable2": "/status/bms:StartCount",
            "expression": "(({1} == Starting) and ({2} < 10)) {2} + 1 ",
            "actions":{"onSet":[
                {"remap":
                    [
                        {"enable":"/status/bms:startComplete","uri":"/status/bms:Status","outValue":"Running"},
                        {"fims":"set","uri":"/status/bms:StartCount"}
                    ]
                }
                ]
            }
        }
    },
    "/controls/bms": {
        "enable":false,
        "Start": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Starting"}]}]}
        },
        "Stop": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Stopping"}]}]}
        },
        "Standby": {
            "value":false,
            "actions":{"onSet":[{"remap":[{"uri":"/status/bms:Status","outValue":"Shutting_Down"}]}]}
        }

    }
}'

fims_send -m get -r /$$  -u /ess/naked/config/cfile | jq

#
# now we set up some dynamic operations.
fims_send -m set  -u /ess/cfg/cfile/ess/bms_monitor '{
    "pname":"ess",
    "amname":"bms",

    "/schedule/monitor/bms": {
        "/status/bms:StartCount":          { "enable": false,  "amap": "bms", "func":"CalculateVar"}
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"ess"}]}]}
        },
        "run":{
            "value":"test",
            "uri":"/system/commands:runMon",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"xRunSched"}]}]}
        },
        "runTest":{
            "value":"test",
            "uri":"/status/bms:test",
            "every":1,
            "help": "run the runMon schedule var",
            "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
        }
    }
}'