#!/bin/sh
# script to produce server and echo files for brazoria

files="
acuvim
apc_ups
clou_ess_1
clou_ess_2
clou_ess_3
sel_735
sungrow_ess_1
sungrow_ess_2
sungrow_ess_3
sungrow_ess_4
brainbox_ed538
clou_ess_4
sel_3530
sel_651r
"
xfiles="
acuvim
"
run="build/release/modbus_server"
mkdir -p /var/log/modbus_server 
#mkdir -p configs/braz/server 
#mkdir -p configs/braz/echo
#export MBPORT=500
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:

for f in $files 
do 
  echo $f
  #cp configs/braz/${f}.json configs/braz/client
  sh configs/braz/echo/${f}.sh
  $run configs/braz/server/${f}_server.json > /var/log/modbus_server/${f}.log 2>&1 &
  sleep 1
  #pkill fims_echo
  #export MBPORT=$((MBPORT+1))
done 

