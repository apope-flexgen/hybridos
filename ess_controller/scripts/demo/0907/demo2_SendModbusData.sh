#!bin/sh

echo " look at the variable "
 
fims_send -m get -r /$$ -u /flex/full/components/bms_info/bau_alarm_state | jq

echo " send it some values "
fims_send -m set -r /$$ -u /flex/full/components/bms_info/bau_alarm_state 1

echo "look at the results"
fims_send -m get -r /$$ -u /flex/full/alarms/bms | jq
fims_send -m get -r /$$ -u /flex/full/site | jq

echo " and again"
fims_send -m set -r /$$ -u /flex/full/components/bms_info/bau_alarm_state 19

echo "look at the results again"
fims_send -m get -r /$$ -u /flex/full/alarms/bms | jq
fims_send -m get -r /$$ -u /flex/full/site | jq


echo " this passes on  alarms to the site controller but does not cause the alarms to appear on the local UI"
echo " We have to trigger the alarm processor"

 

