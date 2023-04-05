#!/bin/bash

BIN_DIR=/usr/local/bin
CONFIG_DIR=/home/config
LIB_DIR=/home/scripts/tests/lib
NUM_ESS=$(ls $CONFIG_DIR/modbus_client | grep ess | wc -l)

. $LIB_DIR/common.sh         # Load common test utility functions
. $LIB_DIR/ess_controls.sh   # Load ESS control functions

function setup() {
    banner "Setup"
    enterMaintenanceMode
    closeDCContactors
    startESS
}

function teardown() {
    banner "Teardown"
    stopESS
    openDCContactors
    exitMaintenanceMode
}

function checkActivePower() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo -n "    Active power for ESS $id: " && $BIN_DIR/fims_send -m get -r /me -u /assets/ess/ess_$id/active_power | grep -Po '(?<="value":).*?(?=,)'
    done
    echo
    sleep 2s
}

function checkReactivePower() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo -n "    Reactive power for ESS $id: " && $BIN_DIR/fims_send -m get -r /me -u /assets/ess/ess_$id/reactive_power | grep -Po '(?<="value":).*?(?=,)'
    done
    echo
    sleep 2s
}

function testActivePowerCmd() {
    banner "Test Active Power Command (kW): $1"
    sendActivePowerCmd $1
    checkActivePower
    
}

function testReactivePowerCmd() {
    banner "Test Reactive Power Command (kVAR): $1"
    sendReactivePowerCmd $1
    checkReactivePower
}

brief "ESS Maintenance Mode Commands Test: Active and Reactive Power Commands"
setup
testActivePowerCmd 200
testActivePowerCmd 1000
testActivePowerCmd 2500
testActivePowerCmd 3000
testActivePowerCmd -200
testActivePowerCmd -1000
testActivePowerCmd -2500
testActivePowerCmd -3000
testActivePowerCmd 0
testReactivePowerCmd 200
testReactivePowerCmd 1000
testReactivePowerCmd 2500
testReactivePowerCmd 3000
testReactivePowerCmd -200
testReactivePowerCmd -1000
testReactivePowerCmd -2500
testReactivePowerCmd -3000
testReactivePowerCmd 0
teardown