#!/bin/sh

#fims_send -m pub -u /interfaces/SEL-2440/_bug '{"cdelay":2400}'
#fims_send -m pub -u /interfaces/SEL-2440/_bug '{"delay":2400}'
sleep 1.5
fims_send -m pub -u /interfaces/SEL-2440/_bug '{"exception":5}'
sleep 1.5
fims_send -m pub -u /interfaces/SEL-2440/_bug '{"exception":6}'
sleep 1.5
fims_send -m pub -u /interfaces/SEL-2440/_bug '{"exception":7}'
sleep 1.5
fims_send -m pub -u /interfaces/SEL-2440/_bug '{"exception":8}'

