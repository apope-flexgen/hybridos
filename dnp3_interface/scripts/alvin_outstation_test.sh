#!/bin/sh
i="0"
j=0
echo " hi there $i"
while true
do
  /usr/local/bin/fims/fims_send -m set -u /metrics/alvin/gen_ds1_status $j
  /usr/local/bin/fims/fims_send -m set -u /site/operation/remote_enable_flag $j
  /usr/local/bin/fims/fims_send -m set -u /site/operation/remote_enable_flag LATCH_ON
  /usr/local/bin/fims/fims_send -m set -u /metrics/alvin/gen_net_mw $i
  /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_total_active_ower $j
  /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_average_soc $i
#  /usr/local/bin/fims/fims_send -m set -r /me -u /metrics/alvin/gen_ds1_status $j
#  /usr/local/bin/fims_send -m set -r /me -u /metrics/alvin/gen_net_mw $i
  i=$[$i+1]
  j=$((1-j))
  echo " hi there i $i j $j"
  sleep 1
done

exit 0

#uri [/assets/ess/summary] num vars 8
# ..                            [available_ess_num]
# ..                            [ess_average_soc]
# ..                            [ess_chargeable_power]
# ..                            [ess_dischargeable_power]
# ..                            [ess_total_active_power]
# ..                            [ess_total_apparent_power]
# ..                            [ess_total_reactive_power]
# ..                            [running_ess_num]

#idx [analog.0] ->name :[ess_total_active_power] offset : [0]  vIdx [0]===>
#idx [analog.1] ->name :[ess_total_reactive_power] offset : [1]  vIdx [1]===>



i="0"
j=0
echo "val $i"
while true
do
  /usr/local/bin/fims/fims_send -m set -r /me -u /metrics/alvin/gen_ds1_status $j
  /usr/local/bin/fims_send -m set -r /me -u /metrics/alvin/gen_net_mw $i
  i=$[$i+1]
  j=$((1-j))
done
