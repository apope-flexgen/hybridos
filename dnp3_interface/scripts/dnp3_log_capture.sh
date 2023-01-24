#!/bin/sh 
# p. wilshire
#  12/21/2021
# simple file to capture dnp3_interface logs
#
mkdir -p /var/log/dnp3_interface
mkdir -p /var/log/dnp3_log
FILE=/var/log/dnp3_intereface/top_dnp3_mem.txt
while [ 1 ] ; do
date >> $FILE
top -b -n 1 | grep -e dnp3 -e PID >> $FILE
sleep 580
tar cvzf /var/log/dnp3_log/dnp3_logs.tar.gz /var/log/dnp3_interface
done


