
# reset dnp3 test 
fims_send -m set -u /assets/ess/summary/ess_total_active_power   1234
fims_send -m set -u /assets/ess/summary/ess_total_apparent_power   1234
fims_send -m set -u /components/sel_735/active_power   1234
fims_send -m set -u /components/sel_735/reactive_power   1234
fims_send -m set -u /components/sel_735/apparent_power   1234
fims_send -m set -u /components/sel_735/frequency   50.6
fims_send -m set -u /components/sel_651r/grid_voltage_l1_l2  481
fims_send -m set -u /components/sel_651r/grid_voltage_l2_l3  483
fims_send -m set -u /components/sel_651r/grid_voltage_l3_l1  485

fims_send -m set -u /assets/ess/summary/ess_average_soc   44.5
fims_send -m set -u /metrics/ess/ess_chargeable_energy   44567
fims_send -m set -u /metrics/ess/ess_dischargeable_energy   34567

fims_send -m set -u /assets/ess/summary/ess_chargeable_power  8895
fims_send -m set -u /assets/ess/summary/ess_dischargeable_power  3395
fims_send -m set -u /assets/ess/summary/available_ess_num  5
fims_send -m set -u /assets/ess/summary/running_ess_num  4

fims_send -m set -u /site/operation/heartbeat_counter  6

fims_send -m set -u /assets/ess/ess_1/status  4
fims_send -m set -u /assets/ess/ess_1/soc  45.6
fims_send -m set -u /assets/ess/ess_1/racks  6

fims_send -m set -u /assets/ess/ess_2/status  4
fims_send -m set -u /assets/ess/ess_2/soc  45.6
fims_send -m set -u /assets/ess/ess_2/racks  6

fims_send -m set -u /assets/ess/ess_3/status  4
fims_send -m set -u /assets/ess/ess_3/soc  45.6
fims_send -m set -u /assets/ess/ess_3/racks  6

fims_send -m set -u /assets/ess/ess_4/status  4
fims_send -m set -u /assets/ess/ess_4/soc  45.6
fims_send -m set -u /assets/ess/ess_4/racks  6

fims_send -m set -u /components/sel_651r/breaker_status true
fims_send -m set -u /site/operation/running_status_flag true
fims_send -m set -u /site/operation/alarm_status_flag true
fims_send -m set -u /site/operation/fault_status_flag true
fims_send -m set -u /features/active_power/fr_UF_status_flag false
fims_send -m set -u /features/active_power/fr_UF_cooldown_status false
fims_send -m set -u /features/active_power/fr_OF_status_flag false
fims_send -m set -u /features/active_power/fr_OF_cooldown_status false

fims_send -m set -u /assets/ess/ess_1/faults  true
fims_send -m set -u /assets/ess/ess_1/alarms  true
fims_send -m set -u /assets/ess/ess_2/faults  true
fims_send -m set -u /assets/ess/ess_2/alarms  true
fims_send -m set -u /assets/ess/ess_3/faults  true
fims_send -m set -u /assets/ess/ess_3/alarms  true
fims_send -m set -u /assets/ess/ess_4/faults  true
fims_send -m set -u /assets/ess/ess_4/alarms  true
fims_send -m set -u /site/operation/running_status_event  true
