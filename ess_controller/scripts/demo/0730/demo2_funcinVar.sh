#!/bin/sh 
# test indirect calculate

clear
echo close bms dc contators enable maint mode

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
 {
     "MaintModeEnabled": { "value": 1},
     "DCClosed": { "value": 1}
}' | jq

echo -n " press enter to continue " && read in

echo open pcs dc contators
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/pcs '
 {
     "DCClosed": { "value": 0}
}' | jq

echo set up UI stop control
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/assets/bms/summary '
{
     "stop": { 
            "value": true,
            "debug":true
            }
}' | jq

echo -n " press enter to continue " && read in

clear
echo review config
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status | jq

echo -n " press enter to continue " && read in


echo set up OpenContactorsEnable control
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/control/bms '
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
	}' | jq


echo -n " press enter to continue " && read in

echo set up trigger function to evaluate OpenContactorsEnable  

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/trigger/bms '
{ "CheckVars100mS": {
            "value": 0,
            "debug":true,
            "actions": {
                "onSet": [{ "func": [
                                  {"inVar":"/control/bms:OpenContactorsEnable", "func": "CalculateVar", "amap": "bms"}
                    ]
                }]}}
}' | jq

echo -n " press enter to continue " && read in

clear

echo run CheckVars100mS to trigger action.
echo
echo -n " press enter to continue " && read in

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/trigger/bms '
{ 
    "CheckVars100mS": {"value": 100.56}
}'

echo check results

#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/bms | jq 

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/assets/bms | jq 


echo -n " press enter to continue " && read in

clear
echo 
echo now close pcs dc contactor
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/pcs '
 {
     "DCClosed": { "value": 1}
}'

echo run CheckVars100mS
echo
echo -n " press enter to continue " && read in

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/trigger/bms '
{ 
    "CheckVars100mS": {"value": 101.56}
}'

echo check results in assets/bms 
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/assets/bms | jq 

echo -n " press enter to continue " && read in
clear

echo review status
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq

echo review control
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control | jq

echo review trigger
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/trigger | jq

