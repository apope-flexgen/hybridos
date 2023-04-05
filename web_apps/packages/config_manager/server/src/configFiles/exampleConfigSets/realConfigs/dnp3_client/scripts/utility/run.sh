#!/bin/bash

BIN_DIR=/usr/local/bin
CONFIG_DIR=/home/config

# Start fims_server
$BIN_DIR/fims_server &

$BIN_DIR/dnp3_client  "$CONFIG_DIR/dnp3_client/apx_client.json" &

bash