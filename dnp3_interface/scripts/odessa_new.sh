#!/bin/sh
# odessa set val script

/usr/local/bin/fims/fims_send  -m pub -u /assets/ess/ess_1 "{
 \"alarms\": 0,
 \"faults\": 0,
 \"racks\": 0,
 \"soc\": 0,
 \"status\": 0
 }"
/usr/local/bin/fims/fims_send  -m pub -u /assets/ess/ess_2 "{
 \"alarms\": 0,
 \"faults\": 0,
 \"racks\": 0,
 \"soc\": 0,
 \"status\": 0
 }"
/usr/local/bin/fims/fims_send  -m pub -u /assets/ess/ess_3 "{
 \"alarms\": 0,
 \"faults\": 0,
 \"racks\": 0,
 \"soc\": 0,
 \"status\": 0
 }"
/usr/local/bin/fims/fims_send  -m pub -u /assets/ess/ess_4 "{
 \"alarms\": 0,
 \"faults\": 0,
 \"racks\": 0,
 \"soc\": 0,
 \"status\": 0
 }"
/usr/local/bin/fims/fims_send  -m pub -u /assets/ess/summary "{
 \"available_ess_num\":0,
 \"ess_average_soc\":0,
 \"ess_chargeable_power\":0,
 \"ess_dischargeable_power\":0,
 \"ess_total_active_power\":0,
 \"ess_total_apparent_power\":0,
 \"ess_total_reactive_power\":0,
 \"running_ess_num\":0
 }"

/usr/local/bin/fims/fims_send  -m pub -u /components/sel_3530_rtac "{

 \"active_power\":0,
 \"apparent_power\":0,
 \"frequency\":0,
 \"reactive_power\":0,
 \"voltage_l1\":0,
 \"voltage_l2\":0,
 \"voltage_l3\":0
 }"
/usr/local/bin/fims/fims_send  -m pub -u /components/sel_651r "{
\"breaker_status\":0
}"

/usr/local/bin/fims/fims_send  -m pub -u /features/active_power "{
    \"fr_OF_cooldown_status\":0,
 \"fr_OF_status_flag\":0,
 \"fr_UF_cooldown_status\":0,
 \"fr_UF_status_flag\":0
 }"

/usr/local/bin/fims/fims_send  -m pub -u /metrics/ess "{
    \"ess_chargeable_energy\":0,
    \"ess_dischargeable_energy\":0
}"

/usr/local/bin/fims/fims_send  -m pub -u /site/operation "{
    \"alarm_status_flag\":0,
     \"fault_status_flag\":0,
     \"heartbeat_counter\":0,
     \"running_status_flag\":0
     }"
