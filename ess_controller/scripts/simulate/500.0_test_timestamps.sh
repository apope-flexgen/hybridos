#!/bin/sh
# this part of the script sets up the time stamp si bit it only populates
# /components/pcs_sim /components/bms_sim  pcs{bms}_heartbeat pcs{bms}_timestamp 
# On the pcs system we need the pcs_heartbeat mapped to /components/pcsm_general:seconds 
# On the bms system we need the bms_heartbeat mapped to /components/pcsm_general:seconds 
# /components/catl_bms_ems_r/bms_heartbeat 
# /components/catl_bms_ems_r/bms_timestamp 

# so , once we get the time ticking , we will activate a remap for /components/bms_sim/bms_heartbeat
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/schedule/wake_monitor/bms '
 {
   "/components/catl_bms_ems_r:bms_heartbeat": {
     "value": false,
     "amap": "bms",
     "enable": true,
     "func": "CheckMonitorVar",
     "rate": 0.1
   }
 }
'
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

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/ess '
{ "addSchedItem":{
    "value":"SimHeartBeat",
    "uri":"/sched/ess:SimHeartBeat", 
    "fcn":"SimHandleHeartbeat","refTime":0.200,"runTime":0.200,"repTime":1.000,"endTime":0
}}
'
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/SimHeartBeat | jq
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
