#!/bin/bash

bin_dir=/usr/local/bin
cfg_dir=$(readlink -f "$1")
echo -e "$cfg_dir"

# abort if no config directory is passed
if [ "$#" -ne 1 ]; then
    echo -e "[echo.sh] you have died of dysentery, try again."
    exit 1
fi

# generate modbus_client echo files
if [ -d "$cfg_dir/modbus_client" ]
then
    files=($(ls "$cfg_dir/"))
    for i in "${folders[@]}"; do
        echo -e "generating echo files for $cfg_dir/modbus_client/$file..."
        $bin_dir/echo -config="$cfg_dir/modbus_client/$file" -mode=modbus
    done
    # move new configs to appropriate locations
    mkdir -p "$cfg_dir/modbus_server"
    mkdir -p "$cfg_dir/echo"
    mv "$cfg_dir"/modbus_client/*_server.json "$cfg_dir"/modbus_server/
    mv "$cfg_dir"/modbus_client/*_echo.json "$cfg_dir"/echo/
else
    echo -e "skipping modbus_client, no configs exist."
fi

# generate dnp3_client echo files
if [ -d "$cfg_dir/dnp3_client" ]
then
    files=($(ls "$cfg_dir/"))
    for i in "${folders[@]}"; do
        echo -e "generating echo files for $cfg_dir/dnp3_client/$file..."
        $bin_dir/echo -config="$cfg_dir/dnp3_client/$file" -mode=dnp3
    done
    # move new configs to appropriate locations
    mkdir -p "$cfg_dir/modbus_server"
    mkdir -p "$cfg_dir/echo"
    mv "$cfg_dir"/dnp3_client/*_server.json "$cfg_dir"/modbus_server/
    mv "$cfg_dir"/dnp3_client/*_echo.json "$cfg_dir"/echo/
else
    echo -e "skipping dnp3_client, no configs exist."
fi
