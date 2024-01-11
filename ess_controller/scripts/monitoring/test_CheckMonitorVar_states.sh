#!/bin/sh 

# Tests CheckMonitorVar function (check states like Heartbeat and Timestamp)
#
# Note:   Before running this test script, make sure setMonitorList2(*am->vmap, "bms","wake_monitor")
#         is added somewhere in src/ess_controller.cpp. Also, make sure "/schedule/wake_monitor/bms" is defined
#         somewhere in config file (ex.: bms manager)
#
# Example: In bms_manager.json: 
#          "/schedule/wake_monitor/bms": {
#              "/components/catl_bms_ems_r:bms_heartbeat": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"}
#          },
#          "/links/bms": {
#              "bms_heartbeat": {
#                  "value": "/components/catl_mbmu_control_r:bms_heartbeat"
#              }
#          },
#          "/components/catl_mbmu_control_r": {
#              "bms_heartbeat": {
#                  "value": 0,
#                  "EnableStateCheck": false,
#                  "EnableCommsCheck": true,
#                  "Type": "int",
#                  "AlarmTimeout": 5,
#                  "FaultTimeout": 10,
#                  "RecoverTimeout": 1,
#                  "actions": {
#                      "onSet": [
#                          {
#                              "func": [
#                                  {
#                                      "func": "CheckMonitorVar",
#                                      "amap": "bms"
#                                  }
#                              ]
#                          }
#                      ]
#                  }
#              }
#          }

fSend=/usr/local/bin/fims/fims_send

########################################################
#             Helper Functions
########################################################

# Helper function that waits for user input before proceeding to next test case
commandPrompt()
{
    echo "Press 'q' to quit. Press any other key to continue"
    while [ true ] ; do
        read -n 1 k <&1
        if [ $k = q ]; then
            echo
            echo "Quitting the program..."
            exit ;
        else
            echo
            echo "Continuing...";
            echo
            break ;
        fi
    done
}

# Helper function for repeatedly sending out heartbeats to a specific uri
#
# $1 = uri and variable name (ex.: /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = max number of iterations
simulateHeartbeat()
{ 
    currCnt=0
    maxCnt=$2
    while [ $currCnt -lt $maxCnt ]
    do
        fimsSet $1 $currCnt
        sleep 0.1
        ((currCnt++))
    done
}

# Helper function for repeatedly sending out timestamps to a specific uri
#
# $1 = uri and variable name (ex.: /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = max number of iterations
simulateTimestamp()
{ 
    currCnt=0
    maxCnt=$2
    while [ $currCnt -lt $maxCnt ]
    do
        timestamp="Timestamp ${currCnt}"
        echo $timestamp
        $fSend -m set -u $1 "\"$timestamp\"" -r /me | jq
        sleep 0.1
        ((currCnt++))
    done
}

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# Helper function for doing a fims get
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
fimsGet()
{
    $fSend -m get -u $1 -r /me | jq
}

########################################################
#             Initialization Functions
########################################################

setupStates_int()
{
    echo -e "Initializing states (type int)..."
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"value":0}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"EnableStateCheck":false}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"Type":"int"}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"AlarmTimeout":2}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"FaultTimeout":5}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_heartbeat":{"RecoverTimeout":1}}'
    fimsSet "/ess/reload/bms" '{"bms_heartbeat_reload":0}'
}

setupStates_string()
{
    echo -e "Initializing states (type string)..."
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"value":"Initial Timestamp"}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"EnableStateCheck":false}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"Type":"string"}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"AlarmTimeout":2}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"FaultTimeout":5}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"bms_timestamp":{"RecoverTimeout":1}}'
    fimsSet "/ess/reload/bms" '{"bms_timestamp":0}'
}

########################################################
#              Test Cases
########################################################

echo "Test: Monitor variable with state parameters"
sleep 2
echo "Upcoming test: Test 1 - Check States (type int)..."

# Read user input to continue or quit
commandPrompt

setupStates_int
echo -n "getting /reload/bms/bms_heartbeat_reload       " && fimsGet "/ess/reload/bms/bms_heartbeat_reload"
echo -n "getting /reload/bms/bms_timestamp_reload       " && fimsGet "/ess/reload/bms/bms_timestamp_reload"

echo "Test 1 - Check States (type int)..."
echo -e "Expectations: Variable should be retrievable. Parameters related to state of type int should be present.\n"
echo -n "getting bms_heartbeat                          " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_heartbeat"
sleep 2
echo "Test 1 complete"
sleep 1
echo

echo "Upcoming test: Test 2 - Value == lastVal (type int)..."
commandPrompt

echo "Test 2 - Value == lastVal (type int)..."
echo -e "Expectations: Alarm/fault should be sent if value == lastVal (type int).\n"
echo -n "setting bms_heartbeat params                   " && fimsSet "/ess/full/components/catl_bms_ems_r" '{"bms_heartbeat":{"debug":40,"EnableStateCheck":true}}'
# simulate heartbeat
simulateHeartbeat "/ess/components/catl_bms_ems_r/bms_heartbeat" 30
sleep 6
echo -n "getting bms_heartbeat after set                " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_heartbeat"
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 2 complete"
sleep 1
echo

echo "Upcoming test: Test 3 - Value != lastVal (type int)..."
commandPrompt

echo "Test 3 - Value != lastVal (type int)..."
echo -e "Expectations: RecoverTime should be 0. seenReset should be true. seenFault and seenAlarm should be false.\n"
# simulate heartbeat
simulateHeartbeat "/ess/components/catl_bms_ems_r/bms_heartbeat" 30
echo -n "getting bms_heartbeat after set                " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_heartbeat"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 3 complete"
sleep 1
echo

echo "Upcoming test: Test 4 - Check States (type string)..."
commandPrompt

echo -n "setting bms_heartbeat params                   " && fimsSet "/ess/full/components/catl_bms_ems_r" '{"bms_heartbeat":{"EnableStateCheck":false}}'
setupStates_string

echo "Test 4 - Check States (type string)..."
echo -e "Expectations: Variable should be retrievable. Parameters related to state of type int should be present.\n"
echo -n "getting bms_timestamp                          " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_timestamp"
sleep 2
echo "Test 4 complete"
sleep 1
echo

echo "Upcoming test: Test 5 - Value == lastVal (type string)..."
commandPrompt

echo "Test 5 - Value == lastVal (type string)..."
echo -e "Expectations: Alarm/fault should be sent if value == lastVal (type string).\n"
echo -n "setting bms_timestamp params                   " && fimsSet "/ess/full/components/catl_bms_ems_r" '{"bms_timestamp":{"debug":40,"EnableStateCheck":true}}'
# simulate timestamp
simulateTimestamp "/ess/components/catl_bms_ems_r/bms_timestamp" 30
sleep 6
echo -n "getting bms_timestamp after set                " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_timestamp"
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 5 complete"
sleep 1
echo

echo "Upcoming test: Test 6 - Value != lastVal (type string)..."
commandPrompt

echo "Test 6 - Value != lastVal (type string)..."
echo -e "Expectations: RecoverTime should be 0. seenReset should be true. seenFault and seenAlarm should be false.\n"
# simulate timestamp
simulateTimestamp "/ess/components/catl_bms_ems_r/bms_timestamp" 30
echo -n "getting bms_timestamp after set                " && fimsGet "/ess/full/components/catl_bms_ems_r/bms_timestamp"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 6 complete"
sleep 1
echo

echo "Upcoming test: Test 7 - Clear Alarms/Faults..."
commandPrompt

echo "Test 7 - Clear Alarms/Faults..."
echo -e "Expectations: Faults/Alarms should be cleared.\n"
echo -n "sending bms clear faults                       " && fimsSet "/assets/bms/summary" '{"clear_faults":true}'
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 3
echo -n "sending bms clear alarms                       " && fimsSet "/assets/bms/summary" '{"clear_alarms":true}'
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo "Test 7 complete"
sleep 1
echo

echo "Monitor variable with state parameters test complete" 

exit