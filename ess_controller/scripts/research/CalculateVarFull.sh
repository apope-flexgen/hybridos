#/bin/sh
# Calculatevar full demo 
# p. Wilshire 3-8-2022

# see doc in CalculateVar.md

ess_init='{
    "pname":"ess",
    "amname":"bms",
    "/controls/bms":{
        "enable":false,
        "start":{
            "value":false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue":true, "uri": "/control/bms:enable" }
                        ]
                    }
                ]}
        }
    },
    "/status/bms": {
        "Cap": 1,
        "ChargeRate": 0,
        "Max": 3000,
        "Min": 125,

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

        "CheckChargeCharging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} > 0 ",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true,  "uri": "/status/bms:DischargeCap@enabled",  "outValue":false            },
                        { "inValue": true,  "uri": "/status/bms:ChargeCap@enabled",     "outValue":true             },
                        { "inValue": true,  "uri": "/status/bms:Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Status", "fims":"set",   "outValue":  "Charging"    }
                        ]
                    }
                ]}
        },

        "CheckChargeDischarging": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} < 0 ",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",  "outValue":true             },
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Discharging"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Discharging"   }
                        ]
                    }
                ]}
        },

        "CheckChargeIdle": {
            "debug": true,
            "value": false,
            "useExpr": true,
            "numVars": 1,
            "variable1": "/status/bms:ChargeRate",
            "expression": "{1} == 0 ",
            "ifChanged": false,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",     "outValue":false            },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",  "outValue":false             },
                        { "inValue": true, "uri": "/status/bms:Status",                "outValue": "Idle"   },
                        { "inValue": true, "uri": "/status/bms:Status", "fims":"set",  "outValue": "Idle"   }
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
            "variable2": "/status/bms:ChargeRate",
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
            "variable2": "/status/bms:ChargeRate",
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
        "/status/bms:CheckChargeIdle":         { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeCharging":     { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:CheckChargeDischarging":  { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:ChargeCap":               { "amap": "bms", "func":"CalculateVar"},
        "/status/bms:DischargeCap":            { "amap": "bms", "func":"CalculateVar"}
    },

    "/system/commands": {
        "runMon":{
            "value":0,
            "aname":"bms",
            "debug":true,
            "monitor":"monitor",
            "enable":"/controls/bms:enable",
            "help": "load the wake monitor setup system",
            "actions":{"onSet":[{"func":[{"debug":true, "func":"RunMonitor","aname":"ess"}]}]}
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

echo $ess_init > /tmp/ess_init_full
fims_send -f /tmp/ess_init_full -m set  -u /ess/cfg/cfile/ess/ess_init 
sleep 1
fims_send  -m set  -u /ess/controls/bms/enable true 
