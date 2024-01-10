#!/bin/sh
# builder for the modbus_dump stuff
mkdir -p build/mdump
g++ -g -std=c++17 -o build/mdump/md -I ./utils ./utils/modbus_map.cpp ./utils/modbus_dump.cpp -lcjson
#build/release/md ~/config_ess/install/configs/modbus_client/bms_modbus_client.json  bms_server.json
