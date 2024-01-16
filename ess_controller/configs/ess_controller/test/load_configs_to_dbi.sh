#!/bin/bash

BIN_DIR=/usr/local/bin
DBI_CONFIG_DIR=/home/docker/ess_controller/configs/ess_controller/test

# Load ESS configs to dbi
shopt -s nullglob
FILES=$(find $DBI_CONFIG_DIR -type f -name '*.json')

for f in $FILES
do
    fName=$(basename ${f})
    dir=$(dirname ${f})
    echo "Loading $fName from $dir to database..."
    $BIN_DIR/fims_send -m set -u /dbi/ess_controller/configs_${fName%.*} -f $f
done
shopt -u nullglob

