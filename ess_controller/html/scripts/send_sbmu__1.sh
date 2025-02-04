#!/bin/sh 
# send sbmu_9 test data
/usr/local/bin/fims/fims_send -m pub -u /components/catl_sbmu_9 '
{
"sbmu_warning_1":0,
"sbmu_warning_21":0,
"sbmu_warning_22":0,
"sbmu_warning_23":4096,
"sbmu_precharge_status":0,
"sbmu_master_positive":0,
"sbmu_master_negitive":0,
"sbmu_balance_status":0,
"sbmu_voltage":3.8,
"sbmu_current":2000,
"sbmu_soc":45,
"sbmu_soh":100,
"sbmu_max_cell_voltage":3.298,
"sbmu_min_cell_voltage":3.295,
"sbmu_avg_cell_voltage":3.295,
"sbmu_max_cell_temp":70,
"sbmu_min_cell_temp":69,
"sbmu_avg_cell_temp":69,
"sbmu_max_charge_current":1749.6,
"sbmu_max_discharge_current":2272.1,
"sbmu_max_cell_voltage_position":5,
"sbmu_min_cell_voltage_position":1,
"sbmu_max_cell_temp_position":27,
"sbmu_min_cell_temp_position":2,
"sbmu_sum_cells":13709,
"sbmu_tms_mode_command":0,
"sbmu_tms_temp_setting":60,
"sbmu_tms_real_mode":60,
"sbmu_ambient_temp":60,
"sbmu_tms_demand_power":0,
"sbmu_tms_fault_code":0,
"sbmu_door_state":0,
"sbmu_fan_in_box":0
}'
 
