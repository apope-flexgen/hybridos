#!/bin/sh 
# script to setup  test_dnp3_server.config

/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 0
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 0
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 0
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 0
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/running_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/heartbeat_counter 1
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/fault_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /site/operation/alarm_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /metrics/ess/ess_dischargeable_energy 1
/usr/local/bin//fims_send -m set -r /$$ -u /metrics/ess/ess_chargeable_energy 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_4/soc 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_4/status 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_4/racks 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_4/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_4/alarms 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_3/soc 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/soc 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/soc 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/status 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/status 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_3/status 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_3/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/faults 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/alarms 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/alarms 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_3/alarms 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_3/racks 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_2/racks 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/ess_1/racks 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_average_soc 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/available_ess_num 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_chargeable_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_dischargeable_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_total_active_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_total_reactive_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/ess_total_apparent_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /assets/ess/summary/running_ess_num 1
/usr/local/bin//fims_send -m set -r /$$ -u /features/active_power/fr_OF_cooldown_status 1
/usr/local/bin//fims_send -m set -r /$$ -u /features/active_power/fr_OF_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /features/active_power/fr_UF_cooldown_status 1
/usr/local/bin//fims_send -m set -r /$$ -u /features/active_power/fr_UF_status_flag 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_735/reactive_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_735/active_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_735/apparent_power 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_735/frequency 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_651r/grid_voltage_l1_l2 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_651r/grid_voltage_l2_l3 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_651r/grid_voltage_l3_l1 1
/usr/local/bin//fims_send -m set -r /$$ -u /components/sel_651r/breaker_status 1
