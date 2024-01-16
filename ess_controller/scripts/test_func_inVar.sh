#!/bin/sh 
# test indirect calculate


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
 {
     "MaxBMSChargeCurent": { "value": 2000},
     "MaintModeEnabled": { "value": 1},
     "DCClosed": { "value": 1}
}'

echo open pcs dc contators
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/pcs '
 {
     "DCClosed": { "value": 0}
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/assets/bms/summary '
 {
     "stop": { 
            "value": true,
            "debug":true
            }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
 {
	    "OpenContactorsEnable": {
            "value": 0,
            "useExpr": true,
            "debug":true,
            "numVars": 3,
            "variable1": "/status/bms:DCClosed",
            "variable2": "/status/pcs:DCClosed",
            "variable3": "/status/bms:MaintModeEnabled",
            "expression": "{1} and not {2} and {3}",
            "ifChanged":false,
            "actions": {
                "onSet": [{
                    "remap": [
                        {"inValue":0, "uri":"/assets/bms/summary:stop@enabled", "outValue":false},
                        {"inValue":1, "uri":"/assets/bms/summary:stop@enabled", "outValue":true}
                    ]}]}}
	}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
{ "CheckVars100mS": {
            "value": 0,
            "debug":true,
            "actions": {
                "onSet": [{ "func": [
                                  {"inVar":"/status/bms:OpenContactorsEnable", "func": "CalculateVar", "amap": "bms"}

                    ]
                }]}}
}'


echo run CheckVars100mS

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
{ 
    "CheckVars100mS": {"value": 100.56}
}'

echo check results

#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/bms | jq 

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/assets/bms/summary | jq 

echo close pcs dc contactor
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/pcs '
 {
     "DCClosed": { "value": 1}
}'

echo run CheckVars100mS
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
{ "CheckVars100mS": {"value": 101.56}
}'

echo check results 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/assets/bms/summary | jq 



