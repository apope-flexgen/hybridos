#!/bin/sh
# builder for the modbus_dump stuff

g++ -std=c++11 -o build/release/md -I ./include test/modbus_map.cpp test/modbus_dump.cpp -lcjson -lfims
#build/release/md ~/config_ess/install/configs/modbus_client/bms_modbus_client.json  bms_server.json
