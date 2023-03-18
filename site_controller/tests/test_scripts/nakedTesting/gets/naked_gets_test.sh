#!/bin/bash

proceed(){
  printf "\nPress enter to proceed.\n"
  read waiting
}

get2(){
  fims_send -m get -u /assets/$1 -r /me
}

get3(){
  fims_send -m get -u /assets/ess/$1 -r /me
}

get4(){
  fims_send -m get -u /assets/ess/ess_1/$1 -r /me
}

get4Summary(){
  fims_send -m get -u /assets/ess/summary/$1 -r /me
}

pretty(){
  jq . $1 > temp2.json && mv temp2.json $1
}

stars(){
  count=0
  cols=$( tput cols )
  printf "\n"
  while [[ $count -lt $cols ]]
  do
    printf "*"
    let "count=count+1"
  done
  
  printf "\n"
}

headingText(){
  stars
  printf "$1"
  stars
}

clear

headingText 'This test will walk from the lowest endpoint. (A single variable) All the way to a general /assets get.\nThe user will be responsible for visual verification of expected behavior.\nPretty JSON will be displayed to terminal.'

proceed

headingText 'Starting with /assets/ess/ess_1/x. Running through all possible xs.'

proceed

for endpoint in "active_power" "active_power_setpoint" "alarms" "apparent_power" "autobalancing_status" "bms_fault" "com_status" "component_connected" "current_l1" "current_l2" "current_l3" "dc_contactors_closed" "emmu_fault" "faults" "frequency" "frequency_setpoint" "grid_mode" "local_bms_status" "max_temp" "min_temp" "modbus_heartbeat" "reactive_power" "reactive_power_setpoint" "soc" "soh" "system_chargeable_energy" "system_chargeable_power" "system_dischargeable_energy" "system_dischargeable_power" "voltage_dc" "voltage_l1_l2" "voltage_l2_l3" "voltage_l3_l1" "voltage_max" "voltage_min" "voltage_setpoint" "maint_mode" "start" "stop" "enter_standby" "exit_standby" "clear_faults" "limits_override" "autobalancing_enable" "autobalancing_disable" "maint_active_power_setpoint" "maint_reactive_power_setpoint"
do
    get4 $endpoint > temp.json
    pretty temp.json
    headingText $endpoint
    cat temp.json
    read waiting
done

headingText 'Starting test for /assets/ess/summary/x.'

for endpoint in  "num_ess_available" "num_ess_running" "ess_total_active_power" "ess_total_reactive_power" "ess_total_apparent_power" "ess_average_soc" "ess_chargeable_power" "ess_dischargeable_power" "ess_chargeable_energy" "ess_dischargeable_energy" "ess_total_alarms" "ess_total_faults" "grid_forming_voltage_slew"
do
  get4Summary $endpoint > temp.json
  pretty temp.json
  headingText $endpoint
  cat temp.json
  read waiting
done

headingText 'Starting test for /assets/ess/x'
proceed

for endpoint in "summary" "ess_1" "ess_2"
do
  get3 $endpoint > temp.json  
  pretty temp.json
  headingText $endpoint
  proceed
  cat temp.json
  read waiting
done

headingText 'Starting test for /assets/x.\nIncluding feeders to show what a non-naked get looks like.'
proceed

for endpoint in "ess" "feeders"
do
  get2 $endpoint > temp.json  
  pretty temp.json
  headingText $endpoint
  proceed
  cat temp.json
  read waiting
done

headingText 'Now just /assets'
proceed

fims_send -m get -u /assets -r /me > temp.json
pretty temp.json
cat temp.json
read waiting
