#!/bin/bash

# run this from hybridos/sandbox/asset_controls/config/nonvolatile_ess_data/scripts

scripts_dir=local
bin_dir=/usr/local/bin

pkill dbi
pkill mongod
pkill ess_controller

if [ $# -gt 0 ]
then
    exit
fi

sleep 1s

## start fims_server
if ! pgrep -af fims_server
then
fims_server &
fi

## start mongod
if ! pgrep -af mongod
then
    echo "launching mongodb..."
    mongod --config /etc/mongod.conf &
    sleep 1s
fi

## start dbi
if ! pgrep -af dbi
then
    echo "Starting DBI"
    dbi &
fi
sh ../../../scripts/dbi.sh ../config ess_controller


cp ../config/ess_controller/ess_file.json /usr/local/etc/config/ess_controller

ess_controller -f ess_file