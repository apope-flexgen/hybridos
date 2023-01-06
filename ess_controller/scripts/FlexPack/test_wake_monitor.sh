#!/bin/sh
#
# The object of this script is to extend the CalculateVar operations.
# this is used schecule system checking  and other  calculations on variables from a config script.
# This is done by the wake_monitor system. Bit of a relic of the early days of the design but it works well
#    So we'll maintain it forward. 

# Here is the basic config component used to set this up 

#  "/schedule/wake_monitor/bms":{
#         "/components/catl_mbmu_summary_r:mbmu_max_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_mbmu_summary_r:mbmu_min_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_mbmu_summary_r:mbmu_max_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_mbmu_summary_r:mbmu_min_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_mbmu_summary_r:mbmu_soc": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_mbmu_summary_r:mbmu_soh": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_bms_ems_r:bms_heartbeat": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_bms_ems_r:num_hv_subsystem": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/status/bms:BMSPowerOn": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/status/bms:BMSPower": { "enable": true, "rate":0.1, "amap": "bms", "func":"CalculateVar"},
#         "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"},
#         "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
#     },
echo " this is the start of the bms monitoring sequence"
# These are run on a scheduled basis ( we should use rate but we dont yet) 
# The vars are also checked when received from fims.
# this list is for the scheduled items 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/schedule/wake_monitor/bms '{
        "/components/catl_mbmu_summary_r:mbmu_max_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_mbmu_summary_r:mbmu_min_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_mbmu_summary_r:mbmu_max_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_mbmu_summary_r:mbmu_min_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_mbmu_summary_r:mbmu_soc": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_mbmu_summary_r:mbmu_soh": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_bms_ems_r:bms_heartbeat": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/catl_bms_ems_r:num_hv_subsystem": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/status/bms:BMSPowerOn": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/status/bms:BMSPower": { "enable": true, "rate":0.1, "amap": "bms", "func":"CalculateVar"},
        "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"},
        "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
    }'

echo; echo " >> check wake_monitor results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/schedule/wake_monitor/bms | jq 

#exit
# next we'll set up some of those variables
# the action ensures that the CheckMonitorVar process is run whenever a pubor set is received.
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/catl_mbmu_summary_r '{
        "mbmu_max_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "mbmu_min_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMinValCheck": false, "MinAlarmThreshold": 25.4,"MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 2.5,"MinFaultTimeout": 5.5,"MinRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "mbmu_avg_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5, "MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "mbmu_max_cell_temperature": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMaxCellTemp"}]}]}
        },
        "mbmu_min_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMinValCheck": false, "MinAlarmThreshold": 25.4, "MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 2.5,"MinFaultTimeout": 5.5,"MinRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMinCellTemp"}]}]}
        },
        "mbmu_avg_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSAvgCellTemp"}]}]}
        },
        "mbmu_soc": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "mbmu_soh": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "mbmu_max_charge_current": {
            "value":0,
            "actions": { "onSet":[{"remap":[{"bit":0, "offset": 20000, "uri":"/status/bms:MaxBMSChargeCurrent"}]}]}
        },
        "mbmu_max_discharge_current": {
            "value":0,
            "actions": { "onSet":[{"remap":[{"bit":0, "offset": 20000,"uri":"/status/bms:MaxBMSDischargeCurrent"}]}]}
        }
    }'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/catl_bms_ems_r '{
    "bms_heartbeat":{
            "value": 0,"EnableStateCheck": true,"EnableCommsCheck": true,"Type":"int",
            "AlarmTimeout": 1,"FaultTimeout": 2,"RecoverTimeout": 1,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}}
    }'


# note that we need the following functions registered for the bms (or base flex) manager 
# "CheckMonitorVar"
# "CalculateVar"
# "CheckDbiVar"
# "CheckDbiResp"

# the different checks in place are state condition or limits

# the following code is used to set up the support structure for the wake_monitor system. 
# This is broken out into the LoadMonitor Command in FlexPack  
# am->vm->setMonitorList2(*am->vmap, "bms",    "wake_monitor");
# we have to register the command  to use  the  LoadMonitor command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"runMon":{"value":0,
                    "help": "load the wake monitor setup system",
                    "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"flex"}]}]}}}'

echo; echo " >>check system/commands results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/commands | jq 
#exit

# Next the system has to set up the schedule operations to cause this to happen.
# note the rate is ignored for now , hope to fix it for this release.

echo ; echo " >>run RunMonitor for bms wake_monitor"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":22,"aname":"bms","monitor":"wake_monitor","debug":1}}'

#exit

echo " check results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq 


echo " wait  one sec for an alarm"
sleep 1.1
echo ; echo " >>run RunMonitor for bms wake_monitor again "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":22,"aname":"bms","monitor":"wake_monitor","debug":1}}'

echo " wait  another sec for a fault"
sleep 1.1
echo ; echo " >>run RunMonitor for bms wake_monitor again "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":22,"aname":"bms","monitor":"wake_monitor","debug":1}}'

echo " check  results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq 
# this does not work yet.
echo " check alarm results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq | grep -A 20 /assets/flex/summary

exit

#MYVAL=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex/sbmu_voltage `

#echo "myval = $MYVAL"
#(( MYVAL = MYVAL + 4 ))
#echo "myval 2 = $MYVAL"
#
# then we'll set up some values
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_voltage 123.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 123.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_power 0

#Lets look at what we have so far
echo 
echo " variables so far"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex  | jq

# next we'll set up a calculate action 

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex '
{
    "calc_power":{
        "aman":"/flex",
        "value": 0,
        "numVars": 2,
        "variable1": "sbmu_voltage",
        "variable2": "sbmu_current",
        "scale": 1,
        "operation": "*",
        "ifChanged":false, 
        "actions":{
            "onSet":[                     
                {
                    "func":[
                          {
                               "func":"CalculateVar",
                               "amap":"flex"
                         }],
                     "remap":[
                          {
                            "uri":"/status/flex:sbmu_power",
                            "amap":"flex"
                          }]
                }
            ]}            
        }
}'
echo 
echo " operations so far"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/operations/flex  | jq

# now when we set the value for calc_power we should perform the calculation
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_voltage 1356.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 100

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power 1356.4

echo " operations now"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/operations/flex  | jq

echo " variables now"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex  | jq

#echo " all done"
#exit

# but we need to use real input and output data 
#


# well in the real system things are done using wake monitor vars


echo "get links for current and voltage"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_current | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_voltage | jq

echo "reset link for current"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/links/flex/sbmu_current '{"value":"/components/sbmu:system_current"}' | jq

echo "get links for current and voltage"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_current | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_voltage | jq

echo "get function reloads"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/reload/flex | jq

echo "force a function reload"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/reload/flex/CalculateVar_calc_power 1 | jq

echo "get function reloads"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/reload/flex | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 101

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power '{"value":-135.4}'

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex | jq

echo " Now change the new linked var"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/components/sbmu/system_current  240  | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power 135.4

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex | jq

exit
echo "get status"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/flex | jq

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
# {
#   "SendDb":{"value":1,
#             "db":"pirates",
#             "measure":"test_meas",
#             "table":"/system/dbtest"
#            }
# }'



# "cooling_temp_setting":{"temp":0,
# "actions":{"onSet":[{
# "func":[{"func":"ChangeCoolingTemp", "amap":"flex"}]
# }]}},
# "cooling_temp_upper_limit":{"temp":0, 
# "actions":{"onSet":[{
# "func":[{"func":"SetTempUpperLimit", "amap":"flex"}]
# }]}},
# "ht_warning_setting":{"temperature":0,
# "actions":{"onSet":[{
# "func":[{"func":"CheckTemp", "amap":"flex"}],
# "enum":[
#    {"shift": 0, "mask": 65536, "inValue": 3, "uri": "/system/ac1:temp", "outValue": "too high"},
#    {"shift": 0, "mask": 65536, "inValue": 4, "uri": "/system/ac1:temp", "outValue": "okay"}
# ]
# }]}},
# "lt_warning_setting":{"temperature":0,
# "actions":{"onSet":[{
# "func":[{"func":"CheckTemp", "amap":"flex"}],
# "enum":[
#    {"shift": 0, "mask": 65537, "inValue": 4, "uri": "/system/ac1:temp", "outValue": "okay"},
#    {"shift": 0, "mask": 65537, "inValue": 5, "uri": "/system/ac1:temp", "outValue": "too low"}
# ]
# }]}}
# }'


#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":1}}' 
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":2}}' 
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"cooling_temp_setting":{"temp":435}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"cooling_temp_upper_limit":{"temp":75}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":1}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"ht_warning_setting":{"temperature":3}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_hs/ht_warning_setting | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"lt_warning_setting":{"temperature":5}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq
