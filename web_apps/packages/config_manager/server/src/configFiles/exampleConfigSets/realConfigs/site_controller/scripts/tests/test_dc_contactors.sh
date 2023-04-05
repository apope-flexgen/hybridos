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
}

function teardown() {
    banner "Teardown"
    exitMaintenanceMode
}

function checkDCContactorsClosed() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo -n "    Checking if DC contactors are closed for ESS $id: " && $BIN_DIR/fims_send -m get -r /me -u /assets/ess/ess_$id/dc_contactors_closed | grep -Po '(?<="value":).*?(?=,)'
    done
    echo
    sleep 2s
}

function closeDCContactorsAndCheck() {
    banner "Close DC Contactors"
    closeDCContactors
    checkDCContactorsClosed
}

function openDCContactorsAndCheck() {
    banner "Open DC Contactors"
    openDCContactors
    checkDCContactorsClosed
}

brief "ESS Maintenance Mode Commands Test: Open/Close DC Contactors"
setup
closeDCContactorsAndCheck
openDCContactorsAndCheck
teardown