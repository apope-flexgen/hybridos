#!/bin/sh
# script to run a dnp3 server
# copy libraries
# 
sudo touch /var/log/dnp3_server.log
sudo chmod a+rw  /var/log/dnp3_server.log
 
sudo cp /tmp/targ/usr/local/lib64/* /usr/local/lib64
sudo cp /tmp/targ/usr/local/lib/* /usr/local/lib
sudo cp /tmp/targ/usr/local/bin/* /usr/local/bin

sudo LD_LIBRARY_PATH=/usr/local/lib64: /usr/local/bin/dnp3_server /tmp/targ/home/hybrios/config/test_dnp3_server.json> /var/log/dnp3_server.log 2>&1&
 

