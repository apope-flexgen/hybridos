#!/bin/sh
# p wilshire 02-07-2022
# send /components/sel_735/active_power every second


while [ 1 ] ; do
   #fims_send -m pub -u /components/sel_735/active_power  123
   #sleep 0.1
   fims_send -m pub -u /components/sel_735 '{"active_power":  123}'
   fims_send -m pub -u /components/sel_735 '{"reactive_power":  321}'
   fims_send -m pub -u /components/sel_651r '{"breaker_status":  true}'
   fims_send -m pub -u /site/operation '{"running_status_flag":  true}'
   fims_send -m pub -u /site/operation '{"alarm_status_flag":  true}'
   sleep 0.1
   fims_send -m pub -u /components/sel_735 '{"active_power":  124}'
   fims_send -m pub -u /components/sel_735 '{"reactive_power":  421}'
   fims_send -m pub -u /components/sel_651r '{"breaker_status":  false}'
   fims_send -m pub -u /site/operation '{"running_status_flag":  false}'
   fims_send -m pub -u /site/operation '{"alarm_status_flag":  false}'
   sleep 0.2
   fims_send -m pub -u /components/sel_735 '{"active_power":  {"value":125}}'
   fims_send -m pub -u /components/sel_735 '{"reactive_power":  521}'
   fims_send -m pub -u /components/sel_651r '{"breaker_status":  true}'
   fims_send -m pub -u /site/operation '{"running_status_flag":  true}'
   fims_send -m pub -u /site/operation '{"alarm_status_flag":  true}'
   sleep 0.5
   fims_send -m pub -u /components/sel_735 '{"active_power":  {"value":125}}'
   fims_send -m pub -u /components/sel_735 '{"reactive_power":  521}'
   fims_send -m pub -u /components/sel_651r '{"breaker_status":  false}'
   fims_send -m pub -u /site/operation '{"running_status_flag":  false}'
   fims_send -m pub -u /site/operation '{"alarm_status_flag":  false}'
   sleep 0.5
done

