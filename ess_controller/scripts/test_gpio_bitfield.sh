#!/bin/sh
# sctipt to test bitfield in gpio

echo  "look at the current gpio state"

#  "Disconnect Switch": {
#  "Door Latch": {
#  "EStop": {
#  "Fire Alarm": {
#  "Fuse Monitoring": {
#  "Surge Arrester": {


/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo  "turn them all off"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/EStop false
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/Disconnect\ Switch false
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/Door\ Latch false
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/Fire\ Alarm false
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/Fuse\ Monitoring false
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/Surge\ Arrester false
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on EStop"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 1
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on Disconnect Switch"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on Door Latch"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 4
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on Surge Arrester"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 8
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on Fire Alarm"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 16
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq

echo " turn on Fuse Monitoring"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/gpio/gpioBitfield 32
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/components/gpio | jq
