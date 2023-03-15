#!/bin/sh
j=1
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_1/alarms $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_1/faults $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_1/racks $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_1/soc $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_1/status $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_2/alarms $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_2/faults $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_2/racks $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_2/soc $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_2/status $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_3/alarms $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_3/faults $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_3/racks $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_3/soc $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_3/status $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_4/alarms $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_4/faults $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_4/racks $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_4/soc $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/ess_4/status $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/available_ess_num $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_average_soc $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_chargeable_power $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_dischargeable_power $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_total_active_power $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_total_apparent_power $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/ess_total_reactive_power $j
 /usr/local/bin/fims/fims_send -m pub -u /assets/ess/summary/running_ess_num $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_651r/breaker_status $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_651r/grid_voltage_l1_l2 $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_651r/grid_voltage_l2_l3 $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_651r/grid_voltage_l3_l1 $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_735/active_power $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_735/apparent_power $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_735/frequency $j
 /usr/local/bin/fims/fims_send -m pub -u /components/sel_735/reactive_power $j
 /usr/local/bin/fims/fims_send -m pub -u /features/active_power/fr_OF_cooldown_status $j
 /usr/local/bin/fims/fims_send -m pub -u /features/active_power/fr_OF_status_flag $j
 /usr/local/bin/fims/fims_send -m pub -u /features/active_power/fr_UF_cooldown_status $j
 /usr/local/bin/fims/fims_send -m pub -u /features/active_power/fr_UF_status_flag $j
 /usr/local/bin/fims/fims_send -m pub -u /metrics/ess/ess_chargeable_energy $j
 /usr/local/bin/fims/fims_send -m pub -u /metrics/ess/ess_dischargeable_energy $j
 /usr/local/bin/fims/fims_send -m pub -u /site/operation/alarm_status_flag $j
 /usr/local/bin/fims/fims_send -m pub -u /site/operation/fault_status_flag $j
 /usr/local/bin/fims/fims_send -m pub -u /site/operation/heartbeat_counter $j
 /usr/local/bin/fims/fims_send -m pub -u /site/operation/running_status_flag $j
exit 0

#uri [/assets/ess/summary $j num vars 8
# ..                            [available_ess_num $j
# ..                            [ess_average_soc $j
# ..                            [ess_chargeable_power $j
# ..                            [ess_dischargeable_power $j
# ..                            [ess_total_active_power $j
# ..                            [ess_total_apparent_power $j
# ..                            [ess_total_reactive_power $j
# ..                            [running_ess_num $j

#idx [analog.0 $j ->name :[ess_total_active_power $j offset : [0 $j  vIdx [0 $j===>
#idx [analog.1 $j ->name :[ess_total_reactive_power $j offset : [1 $j  vIdx [1 $j===>



i="0"
j=0
echo "val $i"
while true
do
  /usr/local/bin/fims/fims_send -m set -r /me -u /metrics/alvin/gen_ds1_status  $j
  /usr/local/bin/fims_send -m set -r /me -u /metrics/alvin/gen_net_mw $i
  i=$[$i+1 $j
  j=$((1-j))
done
