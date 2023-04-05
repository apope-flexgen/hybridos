#!/bin/bash

BIN_DIR=/usr/local/bin

function getID() {
    local id=$1
    if [[ $i -lt 10 ]]; then
        id="0$i"
    fi
    echo $id
}

function enterMaintenanceMode() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Entering maintenance mode for ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/maint_mode true
    done
    echo
}

function exitMaintenanceMode() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Exiting maintenance mode for ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/maint_mode false
    done
    echo
}

function closeDCContactors() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Closing DC contactors for ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/close_dc_contactors true
    done
    echo
    sleep 5s
}

function openDCContactors() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Opening DC contactors for ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/open_dc_contactors true
    done
    echo
    sleep 5s
}

function startESS() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Starting ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/start true
    done
    echo
    sleep 5s
}

function stopESS() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Stopping ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/stop true
    done
    echo
    sleep 5s
}

function sendActivePowerCmd() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Sending $1 kW to ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/maint_active_power_setpoint "{\"value\":$1}"
    done
    echo
    sleep 3s
}

function sendReactivePowerCmd() {
    for (( i=1; i<=$NUM_ESS; i++ )); do
        id=$(getID $i)
        echo "    Sending $1 kVAR to ESS $id..." && $BIN_DIR/fims_send -m set -u /assets/ess/ess_$id/maint_reactive_power_setpoint "{\"value\":$1}"
    done
    echo
    sleep 3s
}