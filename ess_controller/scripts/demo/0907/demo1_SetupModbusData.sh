#!/bin/sh
echo "set up a bms_info modbus varaible "
  
fims_send -m set -r /$$ -u /flex/full/components/bms_info '
{
    "bau_alarm_state": {
        "value": 0,
        "actions": {
            "onSet": [{
                "enum": [
                    { "shift": 0,"mask": 1,"inValue": 0,"uri": "/alarms/bms:bms_internal_comms_failure", "outValue": "Normal"},
                    { "shift": 0,"mask": 1,"inValue": 1,"uri": "/alarms/bms:bms_internal_comms_failure", "outValue": "Alarm"},
                    { "shift": 4,"mask": 1,"inValue": 0,"uri": "/alarms/bms:racks_vol_diff_over_large",  "outValue": "Normal"},
                    { "shift": 4,"mask": 1,"inValue": 1,"uri": "/alarms/bms:racks_vol_diff_over_large",  "outValue": "Alarm"},
                    { "shift": 5,"mask": 1,"inValue": 0,"uri": "/alarms/bms:racks_curr_diff_over_large", "outValue": "Normal"},
                    { "shift": 5,"mask": 1,"inValue": 1,"uri": "/alarms/bms:racks_curr_diff_over_large", "outValue": "Alarm"},
                    { "shift": 6,"mask": 1,"inValue": 0,"uri": "/alarms/bms:bau_emergency_stop",         "outValue": "Normal"},
                    { "shift": 6,"mask": 1,"inValue": 1,"uri": "/alarms/bms:bau_emergency_stop",         "outValue": "Triggered"},

                    { "shift": 0,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[0]", "outValue": true},
                    { "shift": 4,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[1]", "outValue": true},
                    { "shift": 5,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[2]", "outValue": true},
                    { "shift": 6,"mask": 1,"inValue": 1,"uri": "/site/ess_ls:bms_alarms[3]", "outValue": true}
                ]
            }]
        }
    }
}

}' | jq

echo "this sets up the incoming data to trigger different alarm variables and pass those alarms to the site controller as bits in a message

" 
