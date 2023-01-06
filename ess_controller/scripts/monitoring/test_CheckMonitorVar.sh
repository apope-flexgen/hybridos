
#!/bin/sh 

# Tests CheckMonitorVar function ( this script needs updatind 01/20/2021)
# send  different time stamp to say bms_1 and /config/bms_1/TestComms will increment.
# after the initial increment the system will expect continual changes in the HeartBeat, wait too long and we'll get a CommsFail Status.


fSend=/usr/local/bin/fims/fims_send

# Check state
# /components/catl_mbmu_control_r:mbmu_max_cell_voltage":     { "enable": true, "rate":0.1, "func":"CheckMonitorVar"},
# Item                        Map Addr  Enable EnableMaxValCheck  EnableMinValCheck AlarmThresholdValue FaultThresholdValue ResetValue AlarmTimeout FaultTimeout RecoverTimeout Func
# mbmu_max_cell_voltage       36        true   false              false             25.4                28                   22.4      2.5           5.5         1.4
echo
echo "Testing CheckMonitorVar" `date`
echo
echo

echo "Test 0 - get the set up  for max_cell_voltage"
#echo -n "getting /reload/bms                           " &&$fSend -r /me$$ -m get -u /ess/reload/ess | jq
echo -n "getting /reload/bms                            " &&$fSend -r /me$$ -m get -u /ess/reload/bms  | jq
#echo -n "getting /reload/ess:CheckMonitorVar           " &&$fSend -r /me$$ -m get -u /ess/reload/pcs  | jq
echo -n "getting mbmu_max_cell_voltage                  " &&$fSend -r /me$$ -m get -u /ess/full/components/catl_mbmu_control_r/mbmu_max_cell_voltage | jq
echo -n "setting mbmu_max_cell_voltage to 22 with debug " &&$fSend -r /me$$ -m set -u /ess/full/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"value":-10,"debug":20,"EnableMaxValCheck":true}}'| jq
sleep 2
echo -n "setting mbmu_max_cell_voltage to 25.5          " &&$fSend -r /me$$ -m set -u /ess/full/components/catl_mbmu_control_r '{"mbmu_max_cell_voltage":{"value":25.5,"debug":30}}'| jq
sleep 5
echo -n "getting /reload/bms                            " &&$fSend -r /me$$ -m get -u /ess/reload/bms  | jq
echo -n "getting mbmu_max_cell_voltage after set        " &&$fSend -r /me$$ -m get -u /ess/full/components/catl_mbmu_control_r/mbmu_max_cell_voltage | jq
echo    " Check Alarm Log file                          " &&cat run_logs/MonitorVarLog.txt
echo -n "getting /assets/bms/summary:alarms             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/alarms  | jq
echo -n "getting /assets/bms/summary:faults             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/faults  | jq
echo -n "sending bms clear faults                       " &&$fSend -r /me$$ -m set -u /assets/bms/summary '{"clear_faults":true}'| jq
echo -n "getting /assets/bms/summary:alarms             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/alarms  | jq
echo -n "getting /assets/bms/summary:faults             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/faults  | jq
echo -n "sending bms clear alarms                       " &&$fSend -r /me$$ -m set -u /assets/bms/summary '{"clear_alarms":true}'| jq
echo -n "getting /assets/bms/summary:alarms             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/alarms  | jq
echo -n "getting /assets/bms/summary:faults             " &&$fSend -r /me$$ -m get -u /ess/full/assets/bms/summary/faults  | jq
#echo -n "getting /reload/bms_2:CheckAssetHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_2/CheckAssetHeartBeat
#echo -n "getting /reload/bms_3:CheckAssetHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_3/CheckAssetHeartBeat
#echo -n "getting /reload/bms_4:CheckAssetHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_4/CheckAssetHeartBeat
#sleep 1
exit 

echo
echo "Test 1 - Check state of  lastHeartBeat values..."
echo -e "Expectations: They should all be  Empty\n"

echo -n "setting /config/ess:testHeartBeatTimeout      " &&$fSend -r /me$$ -m set -u /config/ess '{"testHeartBeatTimeout": 3.5}'
echo -n "getting /config/ess:testHeartBeatTimeout 1    " &&$fSend -r /me$$ -m get -u /config/ess/testHeartBeatTimeout
echo -n "getting /config/ess:testHeartBeatTimeout 2    " &&$fSend -r /me$$ -m get -u /config/ess/testHeartBeatTimeout
echo -n "setting /config/ess:maxHeartBeatTimeout       " &&$fSend -r /me$$ -m set -u /config/ess '{"maxHeartBeatTimeout": 5.5}'
echo -n "getting /config/ess:maxHeartBeatTimeout 1     " &&$fSend -r /me$$ -m get -u /config/ess/maxHeartBeatTimeout
echo -n "getting /config/ess:maxHeartBeatTimeout 2     " &&$fSend -r /me$$ -m get -u /config/ess/maxHeartBeatTimeout
echo -n "setting /config/ess:warnHeartBeatTimeout      " &&$fSend -r /me$$ -m set -u /config/ess '{"warnHeartBeatTimeout": 3.5}'
echo -n "getting /config/ess:warnommsTimeout 1     " &&$fSend -r /me$$ -m get -u /config/ess/warnHeartBeatTimeout
# echo -n "getting /status/bms_1:lastHeartBeat       " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat       " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat       " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat       " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/ess:HeartBeatState            " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo
sleep 0.2
#exit 

# Set a new HeartBeat
echo "Test 2 - Change HeartBeat in each bms unit and check state..."
echo -e "Expectations: All bms_units will update lastHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":{"value":2}}'
echo -n "setting /components/bms_2:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":2}}'
echo -n "setting /components/bms_3:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":2}}'
echo -n "setting /components/bms_4:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":2}}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/bms_1:HeartBeatState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState
echo -n "getting /status/bms:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState

echo "sleep 1"
sleep 1  # wait till the next  HeartBeat cycle

# Set a new HeartBeat
echo "Test 3 - Change HeartBeat in each bms unit and check state again ..."
echo -e "Expectations: All bms_units will update lastHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":{"value":3}}'
echo -n "setting /components/bms_2:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":3}}'
echo -n "setting /components/bms_3:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":3}}'
echo -n "setting /components/bms_4:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":3}}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState
sleep 1  # wait till the next  HeartBeat cycle
echo
echo "Test 3.1 - Change HeartBeat in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will update lastHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":{"value":31}}'
echo -n "setting /components/bms_2:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":31}}'
echo -n "setting /components/bms_3:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":31}}'
echo -n "setting /components/bms_4:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":31}}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState

echo "sleep 3.5"
sleep 3.5  # wait till nearly timed out
echo
echo "Test 3.2 - Change HeartBeat in each bms unit except bms_2 and check state again ..."
echo -e "Expectations: All bms_units will except bms_2 update lastHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":32}'
#echo -n "setting /components/bms_2:HeartBeat        " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":32}}'
echo -n "setting /components/bms_3:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":32}}'
echo -n "setting /components/bms_4:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":32}}'
echo -n "getting /status/ess:HeartBeatErrors                  " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatErrors
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState
echo "sleep 0.2"
sleep 0.2

# echo -n "getting /status/bms_1:lastHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat     " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState

echo -n "getting /status/ess:HeartBeatErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatErrors
echo -n "getting /status/bms:HeartBeatErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatErrors
echo -n "getting /status/ess:HeartBeatWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatWarns
echo -n "getting /status/bms:HeartBeatWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatWarns
echo "sleep 1"
sleep 1  # wait till bms_2 times out
echo
echo "Test 4 - Change active current setpoint in /controls/ess and check state..."
echo -e "HeartBeatState should indicate a failure in bms_2.\n"
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":{"value":4}}'
#echo -n "setting /components/bms_2:HeartBeat       " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":4}}'
echo -n "setting /components/bms_3:HeartBeat        " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":4}}'
echo -n "setting /components/bms_4:HeartBeat         " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":4}}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/ess:HeartBeatErrors                  " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatErrors
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState

echo -n "getting /status/ess:HeartBeatErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatErrors
echo -n "getting /status/bms:HeartBeatErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatErrors
echo -n "getting /status/ess:HeartBeatWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatWarns
echo -n "getting /status/bms:HeartBeatWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatWarns

echo " sleep 1"
sleep 1
echo "Test 5 - Check State then change HeartBeat in each bms unit, wait 0.2 then check state again ..."
echo -e "Expectations: All bms_units will online with changed Time Stamp Values.\n"
# echo -n "getting /status/bms_1:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "setting /components/bms_1:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"HeartBeat":{"value":5}}'
echo -n "setting /components/bms_2:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"HeartBeat":{"value":5}}'
echo -n "setting /components/bms_3:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"HeartBeat":{"value":5}}'
echo -n "setting /components/bms_4:HeartBeat      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"HeartBeat":{"value":5}}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastHeartBeat
# echo -n "getting /status/bms_2:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastHeartBeat
# echo -n "getting /status/bms_3:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastHeartBeat
# echo -n "getting /status/bms_4:lastHeartBeat  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastHeartBeat
echo -n "getting /status/ess:HeartBeatState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/HeartBeatState
echo -n "getting /status/bms:HeartBeatState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/HeartBeatState

echo -n "getting /status/ess:HeartBeatErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatErrors
echo -n "getting /status/bms:HeartBeatErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatErrors
echo -n "getting /status/ess:HeartBeatWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essHeartBeatWarns
echo -n "getting /status/bms:HeartBeatWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/HeartBeatWarns

