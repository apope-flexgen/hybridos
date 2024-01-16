#!/bin/sh
dest=$1
echo $dest
curl -G 'http://localhost:8086/query?db=brp_northfork' --data-urlencode 'q=SELECT sbmu_soc,sbmu_soh,sbmu_ambient_temp,sbmu_avg_cell_temp,sbmu_avg_cell_voltage,sbmu_balance_status,sbmu_current,sbmu_door_state,sbmu_fan_in_box,sbmu_master_negitive,sbmu_master_positive,sbmu_max_cell_temp,sbmu_max_cell_temp_position,sbmu_max_cell_voltage,sbmu_max_cell_voltage_position,sbmu_max_charge_current,sbmu_max_discharge_current,sbmu_min_cell_temp,sbmu_min_cell_temp_position,sbmu_min_cell_voltage,source FROM "components" order by time desc LIMIT 9' | jq

