#!/bin/bash

BIN_DIR=/usr/local/bin
CONFIG_DIR=/home/config
LIB_DIR=/home/scripts/tests/lib
NUM_ESS=$(ls $CONFIG_DIR/modbus_client | grep ess | wc -l)

. $LIB_DIR/ess_controls.sh   # Load ESS control functions

enterMaintenanceMode
closeDCContactors
sleep 3s
exitMaintenanceMode