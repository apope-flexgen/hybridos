#!/bin/sh 
# p wilshire 02-16-2022
# Script to demo using SchedItemOpt to send multiple values to the same assetVar

echo " Set up commands"

fims_send -m set -r/$$ -u /ess/system/commands  '
{
    "run":{
        "value":"test",
        "help": " send time to a selected var at defined interval",
        "ifChanged":false,
        "enabled": true,
        "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}
    },
    "stop":{
        "value":"test",
        "help": " stop sending time to a selected var",
        "ifChanged":false,
        "enabled": true,
        "actions":{"onSet":[{"func":[{"func":"StopSched"}]}]}
    }
}
'
#exit 0

fims_send -m set -r/$$ -u /ess/controls/pcs  '
{
    "sched_pubs":{
        "value":false,
        "debug": false,
        "ifChanged":false,
        "enabled": false,
        "targav": "/system/commands:run",
        "new_options":[
            {"value":22,"uri":"/controls/pcs:pubPcsHighSpeed","every":0.5, "offset":0,"debug":false},
            {"value":23,"uri":"/controls/pcs:pubPcsLowSpeed","every":2.0, "offset":0.1,"debug":false}
        ],
        "actions":{
            "onSet": [{
                "func":[{"debug":true,"func":"SchedItemOpts","amap":"ess"}]
            }]
        }
    }
}
'

#exit 0

#exit 0


fims_send -m set -r/$$ -u /ess/controls/pcs  '
{
    "pubPcsHighSpeed": {
        "value":1,
        "enabled":false,
        "mode":"naked",
        "debug":false,
        "table":"/status/pcs_hs",
        "actions":{
            "onSet":[{"func":[{"debug":false,"amap":"ess","func":"RunPub"}]}]
        }
    },
    "pubPcsLowSpeed": {
        "value":1,
        "enabled":false,
        "mode":"naked",
        "debug":false,
        "table":"/status/pcs_ls",
        "actions":{
            "onSet":[{"func":[{"debug":false,"amap":"ess","func":"RunPub"}]}]
        }
    }
}
'
sleep 0.1

fims_send -m get -r/$$ -u /ess/full/controls/pcs | jq

fims_send -m set -r/$$ -u /ess/controls/pcs  '
{
    "sched_pubs":{
        "value":true,
        "enabled": true
        }
}
'

fims_send -m set -r/$$ -u /ess/status/pcs_hs  '
{
    "ChargeCurrent": 345, "DischargeCurrent":-334,"Voltage":1345,"SOC":56.7
}
'
fims_send -m set -r/$$ -u /ess/status/pcs_ls  '
{
    "Temp": 34.5, "MaxChargeCurrent":334.5,"MaVoltage":1345
}
'

timeout 10 fims_listen

