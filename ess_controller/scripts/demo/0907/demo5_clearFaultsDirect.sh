#!/bin/sh


echo " remember  the alarms appeared on the local UI"

fims_send -m get -r /$$ -u /flex/full/assets  | jq

echo "set up a clear faults variable"

fims_send -m set -r /$$ -u /flex/alarms/bms '
 {
    "clear_alarms": {
        "value": "Normal",
        "type": "alarm",
        "actions": {
            "onSet": [{"func": [{"func": "process_sys_alarm", "amap": "bms"}]}]
        }
    }
}'  | jq

echo "send it a Clear command"

fims_send -m set -r /$$ -u /flex/alarms/bms '
 {
    "clear_alarms": {
        "value": "Clear"
    }
}' | jq


echo " the alarms are now gone  on the local UI"

fims_send -m get -r /$$ -u /flex/full/assets  | jq




