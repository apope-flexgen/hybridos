#!/bin/sh
# p wilshire 02/04/2022
# demo script to set up enum (strings)  

echo
echo "first set up the disabled run command"
echo

fims_send -m set -r /$$ -u /ess/system/commands '
{
    "run":{
        "value":"test",
        "help": "send the time to a variable once or periodically ",
        "ifChanged":false, 
        "enabled":false, 
        "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}}
}'

echo
echo "now enable it"
echo
fims_send -m set -r /$$ -u /ess/system/commands/run@enabled true 

#Next we'll send the system time to "/test/run1" every 100mS

fims_send -m set -r /$$ -u /ess/system/commands/run '{"value":22, "uri":"/test/ess:run1", "every":0.01}'
#you could do this but you'll use 10% cpu
#fims_send -m set -r /$$ -u /ess/system/commands/run '{"value":22, "uri":"/test/ess:run2", "every":0.0001}'

fims_send -m set -r /$$ -u /ess/system/commands/run '{"value":22, "uri":"/test/ess:run2", "every":0.1}'
fims_send -m set -r /$$ -u /ess/system/commands/run '{"value":22, "uri":"/test/ess:run3", "every":0.01}'
sleep 0.5
fims_send -m get -r /$$ -u /ess/full/test/ess | jq

# sleep 2
# echo
# echo add a calculation 
# echo

# fims_send -m set -r /$$ -u /ess/test/bms '
# {
#     "BMSVoltage": 1356,
#     "BMSCurrent": 280,
#     "MaxBMSDischargeCurrentFilt": 295,
#     "MaxBMSDischargeCurrentEx": 25,

#     "MaxBMSDischargePowerEst": {
#             "value": 0,
#             "enabled": false,
#             "useExpr": true,
#             "numVars": 4,
#             "variable1": "/test/bms:BMSVoltage",
#             "variable2": "/test/bms:BMSCurrent",
#             "variable3": "/test/bms:MaxBMSDischargeCurrentFilt",
#             "variable4": "/test/bms:MaxBMSDischargeCurrentEx",
#             "expression": "{1} * ({2} + {3} - {4}) * 0.001 * 0.9492 + 91.519",
#             "actions": { 
#                 "onSet":[{
#                     "func":[
#                         {"amap":"ess","func":"CalculateVar"}
#                     ],
#                     "remap":[
#                         {"uri":"/status/bms:MaxBMSDischargePowerEstFiltIn"}
#                     ]
#                 }]
#             }
#         }
# }'
# now add some actions to one of these guys.
echo
echo add an enum
echo
fims_send -m set  -u /ess/components/pcs_registers_fast ' 
{
    "pcs_01_status": {
        "value": 0,
        "actions": {
            "onSet": [{
                "enum": [
                    {"shift": 0, "mask": 255, "inValue": 55, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Init"},
                    {"shift": 0, "mask": 255, "inValue": 22, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Idle"},
                    {"shift": 0, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Stop"},
                    {"shift": 0, "mask": 255, "inValue": 2, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Softcharge"},
                    {"shift": 0, "mask": 255, "inValue": 3, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Ready"},
                    {"shift": 0, "mask": 255, "inValue": 4, "uri": "/test/pcs/pcs_01:module_status", "outValue": "LCL"},
                    {"shift": 0, "mask": 255, "inValue": 5, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Main AC"},
                    {"shift": 0, "mask": 255, "inValue": 6, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Main AC Wait"},
                    {"shift": 0, "mask": 255, "inValue": 7, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Run"},
                    {"shift": 0, "mask": 255, "inValue": 8, "uri": "/test/pcs/pcs_01:module_status", "outValue": "nMain AC"},
                    {"shift": 0, "mask": 255, "inValue": 9, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Discharge"},
                    {"shift": 0, "mask": 255, "inValue": 10, "uri": "/test/pcs/pcs_01:module_status", "outValue": "LVRT"},
                    {"shift": 0, "mask": 255, "inValue": 11, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Trackers mode"},
                    {"shift": 0, "mask": 255, "inValue": 17, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Fault exit OV wait"},
                    {"shift": 0, "mask": 255, "inValue": 18, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Fault exit wait"},
                    {"shift": 0, "mask": 255, "inValue": 19, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Softcharge fault"},
                    {"shift": 0, "mask": 255, "inValue": 20, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Fault20"},
                    {"shift": 0, "mask": 255, "inValue": 159, "uri": "/test/pcs/pcs_01:module_status", "outValue": "Master FPGA comms error"}
                ]
            }]
        }
    },
    "pcs_01_status_str": {
        "value": "test",
        "actions": {
            "onSet": [{
                "remap": [
                    {"shift": 0, "mask": 255, "inValue": "Status01", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Init"},
                    {"shift": 0, "mask": 255, "inValue": "Status02", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Idle"},
                    {"shift": 0, "mask": 255, "inValue": "Status03", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Stop"},
                    {"shift": 0, "mask": 255, "inValue": "Status04", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Softcharge"},
                    {"shift": 0, "mask": 255, "inValue": "Status05", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Ready"},
                    {"shift": 0, "mask": 255, "inValue": "Status06", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "LCL"},
                    {"shift": 0, "mask": 255, "inValue": "Status07", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Main AC"},
                    {"shift": 0, "mask": 255, "inValue": "Status08", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Main AC Wait"},
                    {"shift": 0, "mask": 255, "inValue": "Status09", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Run"},
                    {"shift": 0, "mask": 255, "inValue": "Status10", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "nMain AC"},
                    {"shift": 0, "mask": 255, "inValue": "Status11", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Discharge"},
                    {"shift": 0, "mask": 255, "inValue": "Status12", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "LVRT"},
                    {"shift": 0, "mask": 255, "inValue": "Status13", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Trackers mode"},
                    {"shift": 0, "mask": 255, "inValue": "Status14", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Fault exit OV wait"},
                    {"shift": 0, "mask": 255, "inValue": "Status15", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Fault exit wait"},
                    {"shift": 0, "mask": 255, "inValue": "Status16", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Softcharge fault"},
                    {"debug":false, "shift": 0, "mask": 255, "inValue": "Status17", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Fault17"},
                    {"debug":false,"shift": 0, "mask": 255, "inValue": "Status18", "uri": "/test/pcs/pcs_01:module_status_str", "outValue": "Master FPGA comms error"}
                ]
            }]
        }
    },
    "pcs_01_i_o_status": {
        "value": 0,
        "actions": {
            "onSet": [{
                "enum": [
                    {"shift": 0, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:fan_status", "outValue": "Open"},
                    {"shift": 0, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:fan_status", "outValue": "Closed"},
                    {"shift": 1, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:ac_main_status", "outValue": "Open"},
                    {"shift": 1, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:ac_main_status", "outValue": "Closed"},
                    {"shift": 2, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:ac_softcharge_status", "outValue": "Open"},
                    {"shift": 2, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:ac_softcharge_status", "outValue": "Closed"},
                    {"shift": 3, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:lcl_status", "outValue": "Open"},
                    {"shift": 3, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:lcl_status", "outValue": "Closed"},
                    {"shift": 4, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:dc_switch_status", "outValue": "Open"},
                    {"shift": 4, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:dc_switch_status", "outValue": "Closed"},
                    {"shift": 6, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:thermal_lcl_status", "outValue": "Open"},
                    {"shift": 6, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:thermal_lcl_status", "outValue": "Closed"},
                    {"shift": 7, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:fan_feedback", "outValue": "Open"},
                    {"shift": 7, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:fan_feedback", "outValue": "Closed"},
                    {"shift": 8, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:ac_main_feedback", "outValue": "Open"},
                    {"shift": 8, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:ac_main_feedback", "outValue": "Closed"},
                    {"shift": 9, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:ac_softcharge_feedback", "outValue": "Open"},
                    {"shift": 9, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:ac_softcharge_feedback", "outValue": "Closed"},
                    {"shift": 10, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:lcl_feedback", "outValue": "Open"},
                    {"shift": 10, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:lcl_feedback", "outValue": "Closed"},
                    {"shift": 11, "mask": 255, "inValue": 0, "uri": "/test/pcs/pcs_01:dc_switch_feedback", "outValue": "Open"},
                    {"shift": 11, "mask": 255, "inValue": 1, "uri": "/test/pcs/pcs_01:dc_switch_feedback", "outValue": "Closed"}
                ]
            }]
        }
    },
    "pcs_01_current_r": 0,
    "pcs_01_current_s": 0,
    "pcs_01_current_t": 0,
    "pcs_01_dc_current": 0,
    "pcs_01_dc_voltage_p": 0,
    "pcs_01_dc_voltage_n": 0,
    "pcs_01_dc_voltage": 0,
    "pcs_01_p": 0,
    "pcs_01_q": 0,
    "pcs_01_ambient_temp": 0,
    "pcs_01_max_temp": 0,
    "pcs_01_hw_version": 0,
    "pcs_01_sw_version": 0,
    "pcs_01_temp_r1": 0,
    "pcs_01_temp_r2": 0,
    "pcs_01_temp_r3": 0,
    "pcs_01_temp_s1": 0,
    "pcs_01_temp_s2": 0,
    "pcs_01_temp_s3": 0,
    "pcs_01_temp_t1": 0,
    "pcs_01_temp_t2": 0,
    "pcs_01_temp_t3": 0,
    "current_fault_module": 0,
    "current_fault": 0
}'


fims_send -m set -r /$$ -u /ess/test/ess '
{
    "run2":{
        "value":1,
        "enabled":true,
        "actions":{"onSet":[{ "remap":[
            {"uri":"/test/ess:run2_action_1", "outTime":0},
            {"uri":"/test/ess:run2_action_2", "outTime":0},
            {"uri":"/components/pcs_registers_fast:pcs_01_status_str", "outValue":"Status17"},
            {"uri":"/test/ess:run2_action_3", "outTime":0},
            {"uri":"/test/ess:calc_time_2", "outTime":0}
            ]}]}

    },
    "run3":{
        "value":1,
        "enabled":true,
        "actions":{"onSet":[{ "remap":[
            {"uri":"/test/ess:run3_action_1", "outTime":0},
            {"uri":"/test/ess:run3_action_2", "outTime":0},
            {"uri":"/components/pcs_registers_fast:pcs_01_status", "outValue":20},
            {"uri":"/test/ess:run3_action_3", "outTime":0},
            {"uri":"/test/ess:calc_time_3", "outTime":0}
            ]}]}

    },
    "calc_time_2":{
            "value": 0,
            "enabled": true,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/test/ess:run2_action_2",
            "variable2": "/test/ess:run2_action_3",
            "expression": "({2} - {1}) * 1000.0 ",
            "actions": { 
                "onSet":[{
                    "func":[
                        {"amap":"ess","func":"CalculateVar"}
                    ],
                    "remap":[
                        {"uri":"/timings/ess:calc_result_enum_mS"}
                    ]
                }]
            }
    },
    "calc_time_3":{
            "value": 0,
            "enabled": true,
            "useExpr": true,
            "numVars": 2,
            "variable1": "/test/ess:run3_action_2",
            "variable2": "/test/ess:run3_action_3",
            "expression": "({2} - {1}) * 1000.0 ",
            "actions": { 
                "onSet":[{
                    "func":[
                        {"amap":"ess","func":"CalculateVar"}
                    ],
                    "remap":[
                        {"uri":"/timings/ess:calc_result_str_mS"}
                    ]
                }]
            }
    }
}'

sleep 1
echo "fims_send -m get -r /$$ -u /ess/full/timings/ess "
fims_send -m get -r /$$ -u /ess/full/timings/ess | jq

fims_send -m set -r /$$ -u /ess/components/pcs_registers_fast/pcs_01_i_o_status  254 | jq
 echo "fims_send -m get -r /$$ -u /ess/full/test/pcs "
fims_send -m get -r /$$ -u  /ess/full/test/pcs | jq



