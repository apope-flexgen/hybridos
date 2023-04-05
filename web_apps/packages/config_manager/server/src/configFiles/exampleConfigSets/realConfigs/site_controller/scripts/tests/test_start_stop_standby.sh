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
}

function teardown() {
    banner "Teardown"
    openDCContactors
    exitMaintenanceMode
}

function checkESSStatus() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo -n "    Checking status for ESS $id: " && $BIN_DIR/fims_send -m get -r /me -u /assets/ess/ess_$id/status | grep -Po '(?<="value":).*?(?=,)'
    done
    echo
    sleep 2s
}

function startESSAndCheck() {
    banner "Start ESSs"
    startESS
    checkESSStatus
}

function stopESSAndCheck() {
    banner "Stop ESSs"
    stopESS
    checkESSStatus
}

brief "ESS Maintenance Mode Commands Test: Start, Stop, and Standby"
setup
startESSAndCheck
stopESSAndCheck
teardown