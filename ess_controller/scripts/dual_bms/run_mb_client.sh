#!/bin/sh 
# p. wilshire 3/23/2022
# run modbus_clients for bms-pcs testing 

modbus_client configs/modbus_client/bms_1_modbus_client.json&
modbus_client configs/modbus_client/pcs_sungrow_modbus_client.json&


