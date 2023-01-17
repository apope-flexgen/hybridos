#!/bin/sh
# builder for the modbus_dump stuff

g++ -g -std=c++17 -o md -I ./include ./modbus_map.cpp ./modbus_dump.cpp -lcjson -lfims
#build/release/md ~/config_ess/install/configs/modbus_client/bms_modbus_client.json  bms_server.json
