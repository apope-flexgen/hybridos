#!/bin/sh 

# Tests CheckMonitorVar function (check max limits like max cell voltage)
#
# Note:   Before running this test script, make sure setMonitorList2(*am->vmap, "bms","wake_monitor")
#         is added somewhere in src/ess_controller.cpp. Also, make sure "/schedule/wake_monitor/bms" is defined
#         somewhere in config file (ex.: bms manager)
#
# Example: In bms_manager.json: 
#          "/schedule/wake_monitor/bms": {
#              "/components/catl_mbmu_summary_r:mbmu_max_cell_voltage": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"}
#          },
#          "/links/bms": {
#              "mbmu_max_cell_voltage": {
#                  "value": "/components/catl_mbmu_summary_r:mbmu_max_cell_voltage"
#              }
#          },
#          "/components/catl_mbmu_summary_r": {
#              "mbmu_max_cell_voltage": {
#                  "value": 0,
#                  "EnableFaultCheck": true,
#                  "EnableMaxValCheck": false,
#                  "MaxAlarmThreshold": 25.4,
#                  "MaxFaultThreshold": 28,
#                  "MaxResetValue": 22.4,
#                  "MaxAlarmTimeout": 2.5,
#                  "MaxFaultTimeout": 5.5,
#                  "MaxRecoverTimeout": 1.4,
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

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_summary_r or /components/catl_mbmu_summary_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# Helper function for doing a fims get
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_summary_r or /components/catl_mbmu_summary_r/mbmu_max_cell_voltage)
fimsGet()
{
    $fSend -m get -u $1 -r /me | jq
}

########################################################
#             Initialization Functions
########################################################

setupMaxLimits()
{
    echo -e "Initializing max limits..."
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":0}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"EnableFaultCheck":false}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"EnableMaxValCheck":false}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxAlarmThreshold":25.4}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxFaultThreshold":28}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxResetValue":22.4}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxAlarmTimeout":2.5}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxFaultTimeout":5.5}}'
    fimsSet "/ess/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"MaxRecoverTimeout":1.4}}'
    fimsSet "/ess/reload/bms" '{"mbmu_max_cell_voltage_reload":0}'

    sleep 2 # give enough time for setpoints to be initialized

    echo -e "Initialization complete."

    echo
}

########################################################
#              Test Cases
########################################################

echo "Test: Monitor variable with max limit parameters"
sleep 2
echo "Upcoming test: Test 1 - Check Limits (Parameters related to Max Threshold)..."

# Read user input to continue or quit
commandPrompt

setupMaxLimits
echo -n "getting mbmu_max_cell_voltage_reload           " && fimsGet "/ess/reload/bms/mbmu_max_cell_voltage_reload"

echo "Test 1 - Check Limits (Parameters related to Max Threshold)..."
echo -e "Expectations: Variable should be retrievable. Parameters related to max threshold should be present.\n"
echo -n "getting mbmu_max_cell_voltage                  " && fimsGet "/ess/full/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
sleep 2
echo "Test 1 complete"
sleep 1
echo

echo "Upcoming test: Test 2 - Value > Threshold (fault disabled)..."
commandPrompt

echo "Test 2 - Value > Threshold (fault disabled)..."
echo -e "Expectations: An alarm should be sent if value > alarm threshold. Fault shouldn't occur even if value > fault threshold.\n"
echo -n "setting mbmu_max_cell_voltage params           " && fimsSet "/ess/full/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":22,"debug":20,"EnableMaxValCheck":true}}'
sleep 2
echo -n "setting mbmu_max_cell_voltage params           " && fimsSet "/ess/full/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":25.5,"debug":30}}'
sleep 3
echo -n "getting mbmu_max_cell_voltage after set        " && fimsGet "/ess/full/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 3
echo -n "setting mbmu_max_cell_voltage params           " && fimsSet "/ess/full/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":30,"debug":30}}'
sleep 6
echo -n "getting mbmu_max_cell_voltage after set        " && fimsGet "/ess/full/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 2 complete"
sleep 1
echo

echo "Upcoming test: Test 3 - Value > Threshold (fault enabled)..."
commandPrompt

echo "Test 3 - Value > Threshold (fault enabled)..."
echo -e "Expectations: Fault should occur if value > fault threshold.\n"
echo -n "setting mbmu_max_cell_voltage params           " && fimsSet "/ess/full/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":31,"debug":60,"EnableFaultCheck":true}}'
sleep 6
echo -n "getting mbmu_max_cell_voltage after set        " && fimsGet "/ess/full/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 3 complete"
sleep 1
echo

echo "Upcoming test: Test 4 - Value < Reset Val..."
commandPrompt

echo "Test 4 - Value < Reset Val..."
echo -e "Expectations: RecoverTime should be 0. seenReset should be true. seenFault and seenAlarm should be false.\n\n"
echo -n "setting mbmu_max_cell_voltage params           " && fimsSet "/ess/full/components/catl_mbmu_summary_r" '{"mbmu_max_cell_voltage":{"value":1,"debug":30}}'
sleep 3
echo -n "getting mbmu_max_cell_voltage after set        " && fimsGet "/ess/full/components/catl_mbmu_summary_r/mbmu_max_cell_voltage"
sleep 2
echo    " Check Alarm Log file                          " && cat run_logs/MonitorVarLog.txt
sleep 2
echo "Test 4 complete"
sleep 1
echo

echo "Upcoming test: Test 5 - Clear Alarms/Faults..."
commandPrompt

echo "Test 5 - Clear Alarms/Faults..."
echo -e "Expectations: Faults/Alarms should be cleared.\n"
echo -n "sending bms clear faults                       " && fimsSet "/assets/bms/summary" '{"clear_faults":true}'
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 3
echo -n "sending bms clear alarms                       " && fimsSet "/assets/bms/summary" '{"clear_alarms":true}'
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 2
echo "Test 5 complete"
sleep 1
echo

echo "Monitor variable with max limit parameters test complete" 

exit