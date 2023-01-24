#!/bin/sh
# this script populates the basic test_tls_server  
fims_send -m pub  -u /local_server/site/operation/running_status_flag 2
fims_send -m pub  -u /local_server/site/operation/alarm_status_flag 3
fims_send -m pub  -u /local_server/site/operation/fault_status_flag 4
fims_send -m pub  -u /local_server/site/operation/heartbeat_counter 5

fims_send -m pub  -u /local_server/features/active_power/fr_OF_cooldown_status 6
fims_send -m pub  -u /local_server/features/active_power/fr_OF_status_flag 6

fims_send -m pub  -u /local_server/features/active_power/fr_UF_cooldown_status 6
fims_send -m pub  -u /local_server/features/active_power/fr_UF_status_flag 6

fims_send -m pub  -u /local_server/components/sel_735/active_power 6
fims_send -m pub  -u /local_server/components/sel_735/apparent_power 6
fims_send -m pub  -u /local_server/components/sel_735/frequency 60
fims_send -m pub  -u /local_server/components/sel_735/reactive_power 6


fims_send -m pub  -u /local_server/components/sel_651r/breaker_status 7
fims_send -m pub  -u /local_server/components/sel_651r/grid_voltage_l1_l2 480
fims_send -m pub  -u /local_server/components/sel_651r/grid_voltage_l2_l3 480
fims_send -m pub  -u /local_server/components/sel_651r/grid_voltage_l3_l1 480

fims_send -m pub  -u /local_server/metrics/ess/ess_chargeable_energy 4800
fims_send -m pub  -u /local_server/metrics/ess/ess_dischargeable_energy 4800

fims_send -m pub  -u /local_server/assets/ess/summary/available_ess_num 4
fims_send -m pub  -u /local_server/assets/ess/summary/ess_average_soc  50.2
fims_send -m pub  -u /local_server/assets/ess/summary/ess_chargeable_power 600
fims_send -m pub  -u /local_server/assets/ess/summary/ess_dischargeable_power 600
fims_send -m pub  -u /local_server/assets/ess/summary/ess_total_active_power 700
fims_send -m pub  -u /local_server/assets/ess/summary/ess_total_apparent_power 700
fims_send -m pub  -u /local_server/assets/ess/summary/ess_total_reactive_power 700
fims_send -m pub  -u /local_server/assets/ess/summary/running_ess_num 4

fims_send -m pub  -u /local_server/assets/ess/ess_1/alarms    0
fims_send -m pub  -u /local_server/assets/ess/ess_1/racks    9
fims_send -m pub  -u /local_server/assets/ess/ess_1/faults    0
fims_send -m pub  -u /local_server/assets/ess/ess_1/soc    45.6
fims_send -m pub  -u /local_server/assets/ess/ess_1/status    1

fims_send -m pub  -u /local_server/assets/ess/ess_2/alarms    0
fims_send -m pub  -u /local_server/assets/ess/ess_2/racks    9
fims_send -m pub  -u /local_server/assets/ess/ess_2/faults    0
fims_send -m pub  -u /local_server/assets/ess/ess_2/soc    45.6
fims_send -m pub  -u /local_server/assets/ess/ess_2/status    1

fims_send -m pub  -u /local_server/assets/ess/ess_3/alarms    0
fims_send -m pub  -u /local_server/assets/ess/ess_3/racks    9
fims_send -m pub  -u /local_server/assets/ess/ess_3/faults    0
fims_send -m pub  -u /local_server/assets/ess/ess_3/soc    45.6
fims_send -m pub  -u /local_server/assets/ess/ess_3/status    1

fims_send -m pub  -u /local_server/assets/ess/ess_4/alarms    0
fims_send -m pub  -u /local_server/assets/ess/ess_4/racks    9
fims_send -m pub  -u /local_server/assets/ess/ess_4/faults    0
fims_send -m pub  -u /local_server/assets/ess/ess_4/soc    45.6
fims_send -m pub  -u /local_server/assets/ess/ess_4/status    1
