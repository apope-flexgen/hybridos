#!/bin/sh
# p. wilshire 03-09-2022
# test modbus data
# $1 is the value to send 

echo "sending $1"


fims_send -m pub -u /components/ess/pcs_running_info/faults $1


echo "current status"

fims_send -m get -r /$$  -u /ess/naked/status/pcs_enum | jq




