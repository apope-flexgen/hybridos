#!/bin/sh
# this script sets up the Power Electroncs Personality intto the PCS simulator

# char * tval = (char *)"/components/pcs_registers_fast:current_fault";
#         vm->setVal(vmap, "/links/pcs", "current_fault", tval);
#         tval = (char *)"/components/pcs_registers_fast:current_status";
#         vm->setVal(vmap, "/links/pcs", "current_status", tval);
#         tval = (char *)"/components/pcs_registers_fast:current_warning";
#         vm->setVal(vmap, "/links/pcs", "current_warning", tval);     
# so , once we get the time ticking , we will activate a remap for /components/bms_sim/bms_heartbeat

SIM=flex

/usr/local/bin/fims/fims_send -m set -r /$$  -u /$SIM/components/pcs_parameter_setting '{
 "ac_precharge":{"value":0},
 "active_islanding":{"value":0},
 "black_start_beat":{"value":0},
 "black_start_enabling":{"value":0},
 "bms_comms_exception_switch":{"value":0},
 "bms_comms_exception_time":{"value":0},
 "control_state":{"value":0},
 "dc_mode":{"value":0},
 "grid_mode_setting":{"value":0},
 "hearbeat_timeout_interval":{"value":0},
 "heartbeat":{"value":0},
 "hv_load_switch_remote_closed":{"value":0},
 "hv_load_switch_remote_open":{"value":0},
 "on_grid_const_ac_power":{"value":0},
 "on_grid_const_current":{"value":0},
 "on_grid_const_dc_power":{"value":0},
 "on_grid_const_volt_limit_current":{"value":0},
 "on_grid_const_voltage":{"value":0},
 "on_grid_mode":{"value":0},
 "parallel_operation_ctrl":{"value":0},
 "power_factor_setting":{"value":0},
 "power_priority":{"value":0},
 "power_soft_start":{"value":0},
 "qu_working_mode":{"value":0},
 "quick_dispatch":{"value":0},
 "reactive_power_adj_switch":{"value":0},
 "reactive_power_pct_setting":{"value":0},
 "reactive_power_soft_start":{"value":0},
 "start_stop":{"value":0},
 "weak_grid_mode":{"value":0},
 "zero_power_standby":{"value":0}
}
'

/usr/local/bin/fims/fims_send -m set -r /$$  -u /$SIM/components/pcs_running_info '{
 "active_power":{"value":0},
 "alarm_running_state_1":{"value":0},
 "alarm_running_state_2":{"value":0},
 "alarm_state":{"value":0},
 "ambient_temp":{"value":0},
 "charge_state":{"value":0},
 "dc_current":{"value":0},
 "dc_power":{"value":0},
 "dc_voltage":{"value":0},
 "fault_state":{"value":0},
 "fault_state_1":{"value":0},
 "fault_state_2":{"value":0},
 "grid_frequency":{"value":0},
 "grid_status":{"value":0},
 "grid_voltage_vab":{"value":0},
 "grid_voltage_vbc":{"value":0},
 "grid_voltage_vca":{"value":0},
 "heartbeat":{"value":0},
 "leakage_current":{"value":0},
 "max_capacitive_reactive_power":{"value":0},
 "max_charging_power":{"value":0},
 "max_discharging_power":{"value":0},
 "max_inductive_reactive_power":{"value":0},
 "module_temp_1":{"value":0},
 "module_temp_2":{"value":0},
 "module_temp_3":{"value":0},
 "mv_node_state":{"value":0},
 "mv_node_state_1":{"value":0},
 "mv_node_state_2":{"value":0},
 "neg_ground_impedence":{"value":0},
 "node_state":{"value":0},
 "nominal_output_power":{"value":0},
 "nominal_reactive_output_power":{"value":0},
 "num_black_start_beats":{"value":0},
 "operating_mode":{"value":0},
 "phase_a_current":{"value":0},
 "phase_b_current":{"value":0},
 "phase_c_current":{"value":0},
 "pos_ground_impedence":{"value":0},
 "power_factor":{"value":0},
 "reactive_power":{"value":0},
 "trans_oil_temp":{"value":0},
 "winding_temp":{"value":0},
 "work_state":{"value":0}
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/links/pcs '
 {
     "current_fault":   { "value":"/components/pcs_running_info:fault_state"},
     "current_warning": { "value":"/components/pcs_running_info:warning_state"},
     "current_status":  { "value":"/components/pcs_running_info:work_state"},
     "heartbeat":       { "value":"/components/pcs_running_info:heartbeat"},
     "active_power":    { "value":"/components/pcs_running_info:active_power"},
     "reactive_power":  { "value":"/components/pcs_running_info:reactive_power"},
     "PCSStartKeyCmd":  { "value":"/components/pcs_registers_fast:start"},
     "PCSStopKeyCmd":   { "value":"/components/pcs_registers_fast:stop" }
 }
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/configsim/pcs '
 {
     "num_modules": {"value":4}
 }
'
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/links/bms '
#  {
#      "current_fault": { "value":"/components/pcs_registers_fast:current_fault"},
#      "current_warning": { "value":"/components/pcs_registers_fast:current_warning"},
#      "current_status": { "value":"/components/pcs_registers_fast:current_status"}
#  }
# '
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/reload/pcs '
 {
     "SimHandlePcs": { "value":0}
 }
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsFastPub":{
        "value":"dummy",
         "table":"/components/pcs_running_info",
         "enabled":false
         }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ 
    "addSchedItem":{
        "value":"PcsFastPub",
        "var":"/sched/pcs:pcsFastPub",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:PcsFastPub", 
        "fcn":"FastPub",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":1.000,
        "endTime":0
}}
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsSlowPub":{
        "value":"dummy",
         "table":"/components/pcs_parameter_setting",
         "enabled":false
         }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/schedule/ess '
{ 
    "addSchedItem":{
        "value":"PcsFastPub",
        "var":"/sched/pcs:pcsFastPub",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:PcsFastPub", 
        "fcn":"FastPub",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":2.000,
        "endTime":0
}}
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/sched/pcs '
{ 
    "PcsFastPub":{
        "value":"dummy",
        "table":"/components/pcs_running_info",
        "enabled":true,
        "endTime":1
        },
    "PcsSlowPub":{
        "value":"dummy",
        "table":"/components/pcs_parameter_setting",
        "enabled":false,
        "endTime":1
         }
}'

exit



/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/schedule/wake_monitor/bms | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/schedule/wake_monitor/bms '
 {
   "/components/catl_bms_ems_r:bms_timestamp": {
     "value": false,
     "amap": "bms",
     "enable": true,
     "func": "CheckMonitorVar",
     "rate": 0.1
   }
 }
'
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/schedule/wake_monitor/bms | jq

#exit

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/ess '
# { "addSchedItem":{
#     "value":"SimHeartBeat",
#     "uri":"/sched/ess:SimHeartBeat", 
#     "fcn":"SimHandleHeartbeat","refTime":0.200,"runTime":0.200,"repTime":1.000,"endTime":0
# }}
# '
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/SimHeartBeat | jq
# {
#   "SimHeartBeat": {
#     "value": 27.367977000016253,
#     "active": true,
#     "enabled": true,
#     "endTime": 0,
#     "fcn": "SimHandleHeartbeat",
#     "refTime": 0.2,
#     "repTime": 1,
#     "runCnt": 332,
#     "runEnd": 0,
#     "runTime": 360.2
#   }
# }
echo " set configsim"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/configsim/ess '
{ 
  "SimBmsComms": {
    "value": true
  },
  "SimBmsHB": {
    "value": true
  },
  "SimPcsComms": {
    "value": true
  },
  "SimPcsHB": {
    "value": true
  }
}'

echo " get configsim"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/configsim/ess | jq
# {
#   "HeartbeatMax": {
#     "value": 255
#   },
#   "HeartbeatPeriod": {
#     "value": 1
#   },
#   "SimBmsComms": {
#     "value": false
#   },
#   "SimBmsHB": {
#     "value": false
#   },
#   "SimPcsComms": {
#     "value": false
#   },
#   "SimPcsHB": {
#     "value": false
#   }
# }
sleep 2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/pcs_sim | jq
# {
#   "pcs_heartbeat": {
#     "value": 11
#   },
#   "pcs_timestamp": {
#     "value": "the new time is 16.218826"
#   }
# }
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/bms_sim | jq
# {
#   "bms_heartbeat": {
#     "value": 58
#   },
#   "bms_timestamp": {
#     "value": "the new time is 40.218825"
#   }
# }

echo "map the bms heartbeat and time stamp" 
/usr/local/bin/fims/fims_send -m set -r /$$  -u /ess/components/bms_sim '
{
    "bms_heartbeat":{ 
        "value":0,
        "debug":1,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"uri": "/components/catl_bms_ems_r:bms_heartbeat"}
                    ]                
            }]
        }
    },
    "bms_timestamp":{ 
        "value":"some text",
        "debug":1,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"uri": "/components/catl_bms_ems_r:bms_timestamp"}
                    ]                
            }]
        }
    }
}'

echo "inspect bms 1"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/catl_bms_ems_r/bms_heartbeat | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/catl_bms_ems_r/bms_timestamp | jq

sleep 2
echo "inspect bms 2"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/catl_bms_ems_r/bms_heartbeat | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/components/catl_bms_ems_r/bms_timestamp | jq

exit

# script to set up and test the BMS Heasrtbeat
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:01"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  1
sleep 1
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  2

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:02"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 3
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  3
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:03"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 4
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  4
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:04"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 5
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:05"'
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  5
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:06"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:07"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 6
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  6
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:08"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 7
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  7
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:09"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 8
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  8
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:10"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 9
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  9
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:11"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 10
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  10

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:12"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 11
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  11
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12
echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/full/components/pcs_registers_fast/seconds | jq

echo "# trigger alarm"
sleep 3   
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:14"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12

echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/full/components/pcs_registers_fast/seconds | jq


echo " # trigger fault"
sleep 7
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:14"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12
echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/full/components/pcs_registers_fast/seconds | jq

echo "# recover"
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:01"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  1
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:02"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  2
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:03"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 3
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  3
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:04"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 4
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  4
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:05"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 5
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  5
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:06"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 6
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  6
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:07"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 7
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  7

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:08"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 8
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  8
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:09"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 9
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  9

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:10"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 10
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  10

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:11"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 11
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  11

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:12"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  55

/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/full/components/pcs_registers_fast/seconds | jq

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  56

/usr/local/bin/fims/fims_send -m get -r/$$ -u /ess/full/components/pcs_registers_fast/seconds | jq
