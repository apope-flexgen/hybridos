#!/bin/sh
echo " set up  alarm and fault destinations"

fims_send -m set -r /$$ -u /flex/config/bms '
{
        "AlarmDestination": "/assets/bms/summary:alarms",
        "FaultDestination": "/assets/bms/summary:faults",
        "NoFaultMsg": "Normal",
        "NoAlarmMsg": "Normal"
}'

echo " set up variables to receive alarms"

fims_send -m set -r /$$ -u /flex/alarms/bms ' 
{
    "bms_internal_comms_failure": {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "bms"}]}]}},
    "racks_vol_diff_over_large":  {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "bms"}]}]}},
    "racks_curr_diff_over_large": {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "bms"}]}]}},
    "bau_emergency_stop":         {"value": "Normal", "type": "alarm", "actions": {"onSet": [{"func": [{"func": "process_sys_alarm", "amap": "bms"}]}]}}
}' | jq


echo " set up variables to display alarms"


fims_send -m set -r /$$ -u /flex/assets/bms/summary ' 
{

    "name": "BMS Manager",
    "alarms": {
        "name": "Alarms",
        "value": 0,
        "options": [],
        "unit": "",
        "scaler": 0,
        "enabled": true,
        "ui_type": "alarm",
        "type": "number"
    }
}' | jq

echo " look at the results"

fims_send -m get -r /$$ -u /flex/full/assets/bms/summary  | jq


 



