#!/bin/sh
# script to start the dual risen load
# p wilshire 3/3/2022

# starts the load process
/usr/local/bin/ess_controller -x -s ":/metrics:/components:/site:" -n leo

In another  terminal.

set up the variables 
fims_send -m set -r /$$ -u /metrics/kW_set/active_power_command_mw 123
fims_send -m set -r /$$ -u /components/sel_735/voltage_l1_l2 456
fims_send -m set -r /$$ -u /site/operation/alarm_status_flag true

here you can get the values 
fims_send -m get -r /$$ -u /metrics/kW_set/active_power_command_mw
123
fims_send -m get -r /$$ -u /components/sel_735/voltage_l1_l2
456
fims_send -m get -r /$$ -u /site/operation/alarm_status_flag
true

I think the is exactly what you asked for .
