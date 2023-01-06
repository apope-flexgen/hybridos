#!/bin/bash

# Test script for start, stop, and standby operation for Sungrow PCS

fSend=/usr/local/bin/fims_send

########################################################
#             Helper Functions
########################################################

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# Helper function for doing a fims pub
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsPub()
{
    $fSend -m pub -u $1 $2
}

# Helper function for doing a fims get
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
fimsGet()
{
    $fSend -m get -u $1 -r /me | jq
}

# Helper function that waits for user input before proceeding to next test case
commandPrompt()
{
    echo "Press 'q' to quit. Press any other key to continue"
    while [ true ] ; do
        read -n 1 k <&1
        if [ $k = q ]; then
            exit ;
        else
            echo
            echo "Continuing...";
            echo
            break ;
        fi
    done
}

########################################################
#             Test Functions
########################################################

startup=true
shutdown=true
standby=true

# Initialize data before testing
# Note: comment out function call below if raw data values should be read from the hardware unit
setup()
{
    fimsSet "/ess/assets/pcs/summary/start" false
    fimsSet "/ess/assets/bms/summary/start" false
    fimsSet "/ess/assets/pcs/summary/stop" false
    fimsSet "/ess/assets/bms/summary/stop" false
    fimsSet "/ess/components/gpio/EStop" false
    fimsSet "/ess/status/pcs/DCVoltage" 1400
    fimsSet "/ess/status/bms/BMSHeartbeat@EnableStateCheck" false
    fimsSet "/ess/status/pcs/PCSHeartbeat@EnableStateCheck" false
    fimsSet "/ess/status/bms/NumRacksInService@EnableFaultCheck" false
    fimsSet "/ess/status/bms/BMSMinCellTemp@EnableFaultCheck" false
    fimsSet "/ess/assets/bms/summary/maint_mode" true
    fimsSet "/ess/assets/pcs/summary/maint_mode" true
    fimsSet "/ess/status/pcs/MaxPCSActivePower" 100
    fimsSet "/ess/status/pcs/MaxPCSReactivePower" 100
    fimsSet "/ess/status/pcs/MaxPCSApparentPower" 100
    fimsSet "/ess/controls/ess/ActivePowerSetpoint" 0
    sleep 0.2
    fimsSet "/ess/site/ess_hs/clear_faults" 1
    sleep 0.2
    fimsSet "/ess/site/ess_hs/clear_faults" 0
}

# Test PCS startup operation
startup()
{
    if [ "$startup" = true ] ; then
        echo "PCS Off, BMS Off, Batteries normal"
        fimsSet "/ess/components/pcs_running_info" '{"work_state":8}'           # PCS is ready
        fimsSet "/ess/components/bms_info" '{"system_operation_state":0}'       # Batteries normal
        fimsSet "/ess/components/bms_info" '{"connection_status":0}'            # BMS is powered off 
        sleep 3
        echo "Starting BMS from UI...                                                        " && fimsSet "/ess/assets/bms/summary/start" true
        sleep 2
        echo "Setting /components/bms_running_info:connection_status to 1 (BMS is starting)  " && fimsSet "/ess/components/bms_info" '{"connection_status":1}'
        sleep 2
        echo "Setting /components/bms_running_info:connection_status to 2 (BMS is now on)    " && fimsSet "/ess/components/bms_info" '{"connection_status":2}'
        sleep 3
        echo "Starting PCS from UI...                                                        " && fimsSet "/ess/assets/pcs/summary/start" true
        sleep 3
        echo "Setting /components/pcs_running_info:work_state to 64 (PCS is starting)        " && fimsSet "/ess/components/pcs_running_info" '{"work_state":64}'
        sleep 3
        echo "Setting /components/pcs_running_info:work_state to 1 (PCS is now running)      " && fimsSet "/ess/components/pcs_running_info" '{"work_state":1}'
        sleep 3
    fi
}

# Test PCS shutdown operation
shutdown()
{
    if [ "$shutdown" = true ] ; then
        echo "PCS Running, BMS On, Batteries normal"
        fimsSet "/ess/components/bms_info" '{"system_operation_state":0}'        # Batteries normal
        fimsSet "/ess/components/bms_info" '{"connection_status":2}'             # BMS is powered on
        fimsSet "/ess/components/pcs_parameter_setting" '{"work_state":1}'       # PCS is running
        fimsSet "/ess/status/bms/BMSCurrentCheckStop" 100
        fimsSet "/ess/controls/ess/ActivePowerSetpoint" 3000
        sleep 3
        echo "Shutting down PCS from UI...                                                        " && fimsSet "/ess/assets/pcs/summary/shutdown" true
        sleep 5
        echo "Setting /status/bms:BMSCurrentCheckStop to 20 (BMS current going low)               " && fimsSet "/ess/status/bms/BMSCurrentCheckStop" 20
        sleep 5
        echo "Setting /components/pcs_running_info:work_state to 8 (Key stop; PCS Going to Stop)  " && fimsSet "/ess/components/pcs_running_info" '{"work_state":8}'
        sleep 3
        echo "Setting /components/bms_info:connection_status to 0 (BMS is now off)                " && fimsSet "/ess/components/pcs_running_info" '{"connection_status":0}'
        sleep 3
    fi
}

# Test PCS standby operation
standby()
{
    if [ "$standby" = true ] ; then
        echo "PCS Running, BMS On, Batteries normal"
        fimsSet "/ess/components/bms_info" '{"system_operation_state":0}'        # Batteries normal
        fimsSet "/ess/components/bms_info" '{"connection_status":1}'             # BMS is powered on
        fimsSet "/ess/components/pcs_running_info" '{"work_state":1}'            # PCS is running
        # ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 100
        sleep 3
        echo "Putting PCS to standby from UI...                                             " && fimsSet "/ess/assets/pcs/summary/stop" true
        sleep 5
        echo "Setting /components/pcs_running_info:work_state to 16 (PCS going to standby)  " && fimsSet "/ess/components/pcs_running_info" '{"work_state":16}'
        sleep 3
        # echo "BMS Current going low"
        # ./fims_send -m set -r /me -u /ess/status/bms/BMSCurrentCheckStop 20
        # sleep 1
    fi
}

########################################################
#              Test Cases
########################################################

echo "Test: Start/stop/standby operation for Sungrow PCS"
sleep 2
echo "Upcoming test: Test 1 - Start PCS..."

# Read user input to continue or quit
commandPrompt

setup

echo "Test 1 - Start PCS..."
startup
echo "Test 1 complete"
sleep 1
echo
echo "Upcoming test: Test 2 - Stop PCS..."

commandPrompt

echo "Test 2 - Stop PCS..."
shutdown
echo "Test 2 complete"
sleep 1
echo
echo "Upcoming test: Test 3 - PCS in Standby..."

commandPrompt

echo "Test 3 - PCS in Standby..."
standby
echo "Test 3 complete"
sleep 1
echo

echo "PCS start/stop/standby test cases complete"
exit
