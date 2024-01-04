#!/bin/sh 

# kill any old echo processes 
pkill fims_echo

# run them
sh /home/docker/configs/modbus/echo/gcom_test_echo.sh

# run server
/home/docker/git1/hybridos/modbus_interface/build/release/modbus_server /home/docker/configs/modbus/server/gcom_test_server.json 
