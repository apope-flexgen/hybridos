#!/bin/sh
pause=$#
#echo "pause = $pause"

#if [ "$pause" == "1" ] ; then
#   echo " pause was 1"
#fi

#if [ "$pause" == "1" ] ; then
#   echo -n " press enter to continue " && read in
#fi
wait_pause()
{
   if [ "$pause" == "1" ] ; then
      echo -n " press enter to continue " && read in
   fi
}

#
# The object of this script is to extend the CalculateVar operations.
# this is used schecule system checking  and other  calculations on variables from a config script.
# This is done by the wake_monitor system. Bit of a relic of the early days of the design but it works well
#    So we'll maintain it forward. 

# Here is the basic config component used to set this up 

#  "/schedule/wake_monitor/bms":{
#         "/components/bms_info:mbmu_max_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/bms_info:mbmu_min_cell_voltage": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/bms_info:mbmu_max_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/bms_info:mbmu_min_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/bms_info:mbmu_soc": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/bms_info:mbmu_soh": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_bms_ems_r:bms_heartbeat": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/components/catl_bms_ems_r:num_hv_subsystem": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/status/bms:BMSPowerOn": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
#         "/status/bms:BMSPower": { "enable": true, "rate":0.1, "amap": "bms", "func":"CalculateVar"},
#         "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"},
#         "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
#     },
echo " this is the start of the bms monitoring sequence"

echo "first set up some system test points"

echo '{
        "max_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": true,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "min_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMinValCheck": true, "MinAlarmThreshold": 25.4,"MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 2.5,"MinFaultTimeout": 5.5,"MinRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "avg_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5, "MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "max_cell_temperature": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMaxCellTemp"}]}]}
        },
        "min_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMinValCheck": false, "MinAlarmThreshold": 25.4, "MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 2.5,"MinFaultTimeout": 5.5,"MinRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMinCellTemp"}]}]}
        },
        "avg_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSAvgCellTemp"}]}]}
        },
        "soc": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "soh": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        }
    }'

wait_pause 
echo "configure the controller with this data (this is normally done in the setup configuration file)"
echo
echo

# the action ensures that the CheckMonitorVar process is run whenever a pubor set is received.
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_info '{
        "max_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": true,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 1.0,"MaxFaultTimeout": 2.0,"MaxRecoverTimeout": 1.0,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "min_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMinValCheck": true, "MinAlarmThreshold": 25.4,"MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 1.0,"MinFaultTimeout": 2.0,"MinRecoverTimeout": 1.0,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "avg_cell_voltage": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5, "MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "max_cell_temperature": {
            "value": 0, "EnableFaultCheck": true, "EnableMaxValCheck": false, "MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMaxCellTemp"}]}]}
        },
        "min_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMinValCheck": false, "MinAlarmThreshold": 25.4, "MinFaultThreshold": 10,"MinResetValue": 30,
            "MinAlarmTimeout": 2.5,"MinFaultTimeout": 5.5,"MinRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSMinCellTemp"}]}]}
        },
        "avg_cell_temperature": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"remap":[{"bit":0, "offset": 50,"uri":"/status/bms:BMSAvgCellTemp"}]}]}
        },
        "soc": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        },
        "soh": {
            "value": 0,"EnableFaultCheck": true,"EnableMaxValCheck": false,"MaxAlarmThreshold": 25.4,"MaxFaultThreshold": 28,"MaxResetValue": 22.4,
            "MaxAlarmTimeout": 2.5,"MaxFaultTimeout": 5.5,"MaxRecoverTimeout": 1.4,
            "actions": {"onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]}
        }
    }'
wait_pause
echo "examine the variables  Note details are missing  this time "
echo;echo;echo

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/components/bms_info

wait_pause

echo "examine the variables in full detail "
echo;echo;echo
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_info


wait_pause

echo "examine the variables in full detail but use jq to expand "
echo;echo;echo
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_info | jq

wait_pause

echo " now we set up the list of variables to be monitored by the alarm / fault system : note rate is a (near) future option "
echo  '{
        "/components/bms_info:max_cell_voltage":     { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:min_cell_voltage":     { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:max_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:min_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:soc":                  { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:soh":                  { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"}
    }'

wait_pause

# These are run on a scheduled basis ( we should use rate but we dont yet) 
# The vars are also checked when received from fims.
# this list is for the scheduled items 
echo " add the list to the system"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/schedule/wake_monitor/bms '{
        "/components/bms_info:max_cell_voltage":     { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:min_cell_voltage":     { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:max_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:min_cell_temperature": { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:soc":                  { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"},
        "/components/bms_info:soh":                  { "enable": true, "rate":0.1, "amap": "bms","func":"CheckMonitorVar"}
    }'

echo; echo " >> check wake_monitor_bms  results"
echo;echo;echo
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/schedule/wake_monitor/bms | jq 

#exit
#

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

wait_pause
echo "next we have to set up a trigger to run the above list "

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"runMon":{"value":0,
                    "help": "load the wake monitor setup system",
                    "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"RunMonitor","aname":"flex"}]}]}}}'

echo; echo " >>check system/commands results"

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/commands | jq 
wait_pause

#exit

# Next the system has to set up the schedule operations to cause this to happen.
# note the rate is ignored for now , hope to fix it for this release.

echo ; echo " >>run RunMonitor for bms wake_monitor"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":22,"aname":"bms","monitor":"wake_monitor","debug":1}}'

wait_pause
#exit

#echo " check results"
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq 

echo "set up bad values for max_cell_voltage 26.4 "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_info '{"max_cell_voltage": {"value":26.4}}'

echo " wait  one second for an alarm"
sleep 1.1

echo ; echo " >>run RunMonitor for bms wake_monitor this time triggers the time elasped alarm "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":22,"aname":"bms","monitor":"wake_monitor","debug":1}}'
wait_pause

echo " check specific alarm results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_info/max_cell_voltage | jq 
wait_pause

echo " check alarm results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq | grep -A 50 /assets/flex/summary | grep -B 5 -A 15 alarms

echo "set up bad values for max_cell_voltage 28.4 "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_info '{"max_cell_voltage": {"value":28.4}}'

echo " wait  another second  for a fault"
sleep 1.1
echo ; echo " >>run RunMonitor for bms wake_monitor again "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":23,"aname":"bms","monitor":"wake_monitor","debug":1}}'
wait_pause

echo " check faults results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq | grep -A 50 /assets/flex/summary | grep -B 5 -A 15 faults


echo " check specific alarm results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_info/max_cell_voltage | jq 
wait_pause

echo "set up good values for max_cell_voltage 22.4 must be below the max reset value for the reset time "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_info '{"max_cell_voltage": {"value":22.3}}'

echo " wait  two seconds  for a fault reset"
sleep 2.2
echo ; echo " >>run RunMonitor for bms wake_monitor again "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands '{"runMon": {"value":24,"aname":"bms","monitor":"wake_monitor","debug":1}}'

wait_pause

echo " check alarm results for a third time"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq | grep -A 20 /assets/flex/summary

echo " check specific alarm results"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_info/max_cell_voltage | jq 
wait_pause

echo " check the fault shutdown controls"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/flex | jq
wait_pause

echo " Notes: "
echo " Something called a vlist enables on or more variables to  receive the alarm/fault  info."
echo " Its already hard coded into the original ess_controller, I just have to activate if from a config action.."

echo
echo " TODO list: "
echo " Show how to schedule the monitor list checking...."
echo " Something like "
echo "/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run "
echo "                    {\"value\":0,\"uri\":\"/system/commands:runMon\",\"every\":0.1,\"offset\":0,\"debug\":0}"
echo " Fix the destination to be bms or pcs controllers rather than just the \"flex\" system controller  ...."
echo " Fix the routing of the error messages using vLinks ...."

