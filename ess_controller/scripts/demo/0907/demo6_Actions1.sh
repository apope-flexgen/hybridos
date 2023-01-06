#!/bin/sh


echo " adding actions to Modbus inputs"

echo
echo 

echo " Make grid_mode_setting trigger a function, also make it enable / disable subsequent operations"
echo " first attempt with no enables but with a simple command decoder"


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/pcs_parameter_setting ' 
{
    "grid_mode_setting": {
            "value":85, 
            "actions": {
                    "onSet": 
                        [
                            {"func": [
                            {"note" : ">>>>>>>>>>>>>  Function not used in this demo",
                               "func": "SimPcsHandleCmd", "amap": "pcs"}
                                    ]
                            },
                            {"remap": [
                            { "note" : ">>>>>>>>>>>>>  preset output to Fault"},
                            { "uri":"/status/pcs_info:command_decoder",  "outValue": "Command Fault"},
                            { "uri":"/status/pcs_info:work_state",  "outValue": 0},
                            { "note" : ">>>>>>>>>>>>>  Set up for Grid Following command"},
                            {"inValue":85, "uri":"/status/pcs_info:work_state[10]",  "outValue": true},
                            {"inValue":85, "uri":"/status/pcs_info:work_state[9]",   "outValue": false},
                            {"inValue":85, "uri":"/status/pcs_info:command_decoder", "outValue": "set Grid Following Mode"},
                            { "note" : ">>>>>>>>>>>>>  Set up for Grid Forming command"},
                            {"inValue":170, "uri":"/status/pcs_info:work_state[10]", "outValue": false},
                            {"inValue":170, "uri":"/status/pcs_info:work_state[9]",  "outValue": true},
                            {"inValue":170, "uri":"/status/pcs_info:command_decoder", "outValue": "set Grid Forming Mode"},
                            { "note" : ">>>>>>>>>>>>>  transfer command decode status to output"},
                            {"inAv":"/status/pcs_info:command_decoder", "uri":"/status/pcs_info:command_response"},
                            {"inAv":"/status/pcs_info:work_state", "uri":"/components/pcs_running_info:work_state"}
                                    ]
                            }
                        ]
            }
    }
}' | jq

echo " see story so far"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/pcs_info | jq 
echo " break it"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/pcs_parameter_setting ' 
{
    "grid_mode_setting": {
            "value":95
    }
}'
echo
echo " see it broken"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/pcs_info | jq 

echo " make it better"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/pcs_parameter_setting ' 
{
    "grid_mode_setting": {
            "value":170
    }
}'
echo
echo " see it all better now"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/pcs_info | jq 

exit


echo
echo "                                    -> freq setpoint"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/pcs_parameter_setting ' 
{
    "off_grid_frequency_setpoint": {
            "value":600, 
            "actions": {
                    "onSet": 
                        [
                            {"func": [
                                        {"func": "SimPcsHandleSetpoint", "amap": "pcs"}
                                    ]
                            }
                        ]
            }
    }
}'

echo
echo "                                    -> voltage setpoint"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/pcs_parameter_setting ' 
{
    "off_grid_voltage_setpoint": {
            "value":480.0, 
            "actions": {
                    "onSet": 
                        [
                            {"func": [
                                        {"func": "SimPcsHandleSetpoint", "amap": "pcs"}
                                    ]
                            }
                        ]
            }
    }    
}'



