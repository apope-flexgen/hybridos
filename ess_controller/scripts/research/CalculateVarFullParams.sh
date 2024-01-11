#/bin/sh
# Calculatevar full demo 
# p. Wilshire 3-8-2022

# see doc in CalculateVarParams.md

# we can use a few params to "tidy up" and "encapsulate" the Cap system.
# we cannot use a Param for the CurrentCap if we want to publish the data.
# we can use the Status as a filter for the Cap Calcs 

# problems ,, we overshoot the Max / Min values
#  "expression": "if (({1} > {4} and {5} == Discharging), ({1} + {2}), {1})",
# lets try 
#  "expression": "if (({1} > {4} and {5} == Discharging), ({1} + {2}), {4})",   Nice fixed it ....
#
#  Check{C|Disc}hargeCap not disabled after reaching max /min  fixed set ChargeRate to 0 when done. 

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
        "ChargeRate": 0,
        "Cap": {
            "value":1,
            "Max": 3000,
            "Min": 125,
            "Status":"Init",
            "CurrentCap":200,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "ifChanged":false, "inAv":"/status/bms:Cap@CurrentCap","fims":"set", "uri": "/status/bms:CurrentCap" }
                        ]
                    }
                ]}
        },

        "CurrentCap": {
            "value":200,
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "fims":"set", "uri": "/status/bms:CurrentCap" },
                        { "fims":"set", "uri": "/status/bms:Cap@CurrentCap" },
                        { "fims":"set", "uri": "/status/bms:Cap@Status","outVar":"/status/bms:Cap@Status" }
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
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",         "outValue": true             },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",      "outValue": false            },
                        { "inValue": true,  "uri": "/status/bms:Cap@Status",                "outValue":   "Charging"    },
                        { "inValue": true,  "uri": "/status/bms:Cap@Status", "fims":"set",   "outValue":  "Charging"    }
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
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",         "outValue": false           },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",      "outValue": true            },
                        { "inValue": true, "uri": "/status/bms:Cap@Status",                "outValue": "Discharging"   },
                        { "inValue": true, "uri": "/status/bms:Cap@Status", "fims":"set",  "outValue": "Discharging"   }
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
                        { "inValue": true, "uri": "/status/bms:ChargeCap@enabled",         "outValue": false    },
                        { "inValue": true, "uri": "/status/bms:DischargeCap@enabled",      "outValue": false    },
                        { "inValue": true, "uri": "/status/bms:Cap@Status",                "outValue": "Idle"   },
                        { "inValue": true, "uri": "/status/bms:Cal@Status", "fims":"set",  "outValue": "Idle"   }
                        ]
                    }
                ]}
        },
        "dischargeDone":false,
        "dischargeDone2":false,
        "DischargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 5,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:Cap@CurrentCap",
            "variable2": "/status/bms:ChargeRate",
            "variable3": "/status/bms:Cap@Max",
            "variable4": "/status/bms:Cap@Min",
            "variable5": "/status/bms:Cap@Status",
            "expression": "if ((({1} + {2}) > {4} and {5} == Discharging), ({1} + {2}), {4})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "uri": "/status/bms:Cap@CurrentCap" },
                        { "uri": "/status/bms:CurrentCap" },
                        { "inVar":"/status/bms:Cap@Min", "uri": "/status/bms:ChargeRate", "outValue":0 },
                        { "inVar":"/status/bms:Cap@Min", "uri": "/status/bms:DischargeCap@enabled", "outValue":false },
                        { "inVar":"/status/bms:Cap@Min", "uri": "/status/bms:dischargeDone", "outValue": true ,"fims":"pub"}
                        ]
                    }
                ]}
        },
        "ChargeCap": {
            "value": 0,
            "useExpr": true,
            "numVars": 5,
            "enabled":false,
            "debug":true,
            "variable1": "/status/bms:Cap@CurrentCap",
            "variable2": "/status/bms:ChargeRate",
            "variable3": "/status/bms:Cap@Max",
            "variable4": "/status/bms:Cap@Min",
            "variable5": "/status/bms:Cap@Status",
            "expression": "if ((({1} + {2}) < {3} and {5} == Charging), ({1} + {2}), {3})",
            "actions": { 
                "onSet":[
                    {
                    "remap": 
                        [
                        { "inVar":"/status/bms:Cap@Max", "uri": "/status/bms:ChargeRate", "outValue":0 },
                        { "inVar":"/status/bms:Cap@Max", "uri": "/status/bms:ChargeCap@enabled", "outValue":false },
                        { "uri": "/status/bms:CurrentCap" },
                        { "uri": "/status/bms:Cap@CurrentCap" }
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

echo $ess_init > /tmp/ess_init_full
fims_send -f /tmp/ess_init_full -m set  -u /ess/cfg/cfile/ess/ess_init 
sleep 1
fims_send  -m set  -u /ess/controls/bms/enable true 
