#!/bin/sh
# p. wilshire 3/23/2022
# run the modbus servers and the echo stuff for pcs testing


sh configs/modbus_server/bms_1_echo.sh
modbus_server configs/modbus_server/bms_1_modbus_server.json  &

sh configs/modbus_server/pcs_sungrow_echo.sh
modbus_server configs/modbus_server/pcs_sungrow_modbus_server.json &
