#!/bin/sh 

# Tests the ESS Controller running at degraded if only one PCS module or one battery rack is faulted
#
# Note:   Before running this test script, make sure setMonitorList2(*am->vmap, "bms","wake_monitor")
#         is added somewhere in src/ess_controller.cpp. Also, make sure "/schedule/wake_monitor/bms" is defined
#         somewhere in config file (ex.: bms manager)
#
#         Also, in the ess controller config files, there may already be variables defined for monitoring that will
#         either populate alarm/fault messages or enter a fault state due to fault condition
#         For this test, we'll focus on the alarm/fault messages generated for num_hv_subsystem,
#         and we'll set FaultShutdown to false and check to make sure that's set to true when
#         num_hv_subsystem < fault threshold
#
# Example: In bms_manager.json: 
#          "/schedule/wake_monitor/bms": {
#              "/components/catl_bms_ems_r:num_hv_subsystem": { "enable": true, "rate":0.1, "func":"CheckMonitorVar"}
#          },
#          "/links/bms": {
#              "num_hv_subsystem": {
#                  "value": "/components/catl_bms_ems_r:num_hv_subsystem"
#              }
#          },
#          "/components/catl_bms_ems_r": {
#              "num_hv_subsystem": {
#                  "value": 0,
#                  "EnableFaultCheck": true,
#                  "EnableMinValCheck": true,
#                  "MinAlarmThreshold": 9,
#                  "MinFaultThreshold": 8,
#                  "MinResetValue": 8,
#                  "MinAlarmTimeout": 1,
#                  "MinFaultTimeout": 1,
#                  "MinRecoverTimeout": 1,
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
bin_dir=/usr/local/bin

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
            terminate
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

# Start running fims server and the ess controller before testing
startup()
{
    echo "Starting up fims server and ess controller..."
    ($bin_dir/fims/fims_server > /dev/null 2>&1 &)
    (sleep 3s; ~/ess_controller/build/release/ess_controller ~/ess_controller/configs/ess_controller/ > /dev/null 2>&1 &)
    echo "Startup complete."
}

# Shutdown running processes
terminate()
{
    echo
    echo "Quitting the program..."
    pkill ess_controller
    pkill fims
    exit
}

########################################################
#             Initialization Functions
########################################################

setupBatteryRacks()
{
    echo -e "Initializing # battery racks online..."
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":0}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"EnableFaultCheck":true}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"EnableMinValCheck":false}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinAlarmThreshold":9}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinFaultThreshold":8}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinResetValue":8}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinAlarmTimeout":1}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinFaultTimeout":1}}'
    fimsSet "/ess/components/catl_bms_ems_r" '{"num_hv_subsystem":{"MinRecoverTimeout":1}}'
    fimsSet "/ess/reload/bms" '{"num_hv_subsystem_reload":0}'

    sleep 2 # give enough time for setpoints to be initialized

    echo -e "Initialization complete."

    echo
}

########################################################
#              Test Cases
########################################################

echo "Test: ESS Controller running at degraded capacity"
sleep 2
echo "Upcoming test: Test 1 - Check Limits..."

# Read user input to continue or quit
commandPrompt

startup
setupBatteryRacks
sleep 5
fimsSet "/ess/status/ess/FaultShutdown" false

echo -n "getting num_hv_subsystem_reload                " && fimsGet "/ess/reload/bms/num_hv_subsystem_reload"

echo "Test 1 - Check Limits..."
echo -e "Expectations: The # of running instances (ex.: # battery racks online) and limit parameters should be retrievable.\n"
echo -n "getting num_hv_subsystem                       " && fimsGet "/ess/full/components/catl_bms_ems_r/num_hv_subsystem"
sleep 2
echo "Test 1 complete"
sleep 1
echo

echo "Upcoming test: Test 2 - Run at Degraded Capacity (# battery racks online < Alarm Threshold)..."
commandPrompt

echo "Test 2 - Run at Degraded Capacity (# batter racks online < Alarm Threshold)..."
echo -e "Expectations: An alarm should be sent if # battery racks < alarm threshold. The ESS Controller should not enter fault state and send shutdown commands.\n"
echo -n "setting num_hv_subsystem params                " && fimsSet "/ess/full/components/catl_bms_ems_r" '{"num_hv_subsystem":{"value":8,"debug":10,"EnableMinValCheck":true}}'
sleep 3
echo -n "getting num_hv_subsystem after set             " && fimsGet "/ess/full/components/catl_bms_ems_r/num_hv_subsystem"
sleep 3
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
sleep 3
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 3
echo -n "getting fault shutdown state                   " && fimsGet "/ess/status/ess/FaultShutdown"
sleep 3
echo "Test 2 complete"
sleep 1
echo

echo "Upcoming test: Test 3 - Enter Fault State (# battery racks online < Fault Threshold)..."
commandPrompt

echo "Test 3 - Enter Fault State (# battery racks online < Fault Threshold)..."
echo -e "Expectations: A fault should be sent if # battery racks < fault threshold. The ESS Controller should enter fault state and send shutdown commands.\n"
echo -n "setting num_hv_subsystem params                " && fimsSet "/ess/full/components/catl_bms_ems_r" '{"num_hv_subsystem":{"value":7,"debug":10}}'
sleep 3
echo -n "getting num_hv_subsystem after set             " && fimsGet "/ess/full/components/catl_bms_ems_r/num_hv_subsystem"
sleep 3
echo -n "getting /assets/bms/summary:alarms             " && fimsGet "/ess/full/assets/bms/summary/alarms"
sleep 3
echo -n "getting /assets/bms/summary:faults             " && fimsGet "/ess/full/assets/bms/summary/faults"
sleep 3
echo -n "getting fault shutdown state                   " && fimsGet "/ess/status/ess/FaultShutdown"
sleep 3
echo "Test 3 complete"
sleep 1
echo

echo "ESS Controller running at degraded capacity test complete" 
terminate