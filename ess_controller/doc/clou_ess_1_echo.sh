/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_high_speed -b '{
 "life":0,
 "start_stop":0,
 "run_mode":0,
 "on_off_grid_mode":0,
 "active_power_setpoint":0,
 "reactive_power_setpoint":0,
 "active_power_rising_slope":0,
 "active_power_droop_slope":0,
 "reactive_power_rising_slope":0,
 "reactive_power_droop_slope":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_high_speed -b '{
 "life_signal":0,
 "chargeable_energy":0,
 "dischargeable_energy":0,
 "chargeable_power":0,
 "dischargeable_power":0,
 "system_state":0,
 "com_status":0,
 "pcs_under_power":0,
 "pcs_over_power":0,
 "voltage_l1_l2":0,
 "voltage_l2_l3":0,
 "voltage_l3_l1":0,
 "current_l1":0,
 "current_l2":0,
 "current_l3":0,
 "frequency":0,
 "active_power":0,
 "reactive_power":0,
 "pf":0,
 "voltage_dc":0,
 "current_dc":0,
 "active_power_dc":0,
 "pcs_mode":0,
 "pcs_state":0,
 "pcs_state2":0,
 "faults":0,
 "alarms":0,
 "overall_maximum_charging_power_limit":0,
 "overall_maximum_discharging_power_limit":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "pcs1_voltage_l1_l2":0,
 "pcs1_voltage_l2_l3":0,
 "pcs1_voltage_l3_l1":0,
 "pcs1_current_l1":0,
 "pcs1_current_l2":0,
 "pcs1_current_l3":0,
 "pcs1_active_power":0,
 "psc1_reactive_power":0,
 "pcs1_voltage_dc":0,
 "pcs1_current_dc":0,
 "pcs1_active_power_dc":0,
 "pcs1_alarm":0,
 "pcs1_faults":0,
 "maximum_charging_power_limit_pcs1":0,
 "maximum_discharging_power_limit_pcs1":0,
 "pcs2_voltage_l1_l2":0,
 "pcs2_voltage_l2_l3":0,
 "pcs2_voltage_l3_l1":0,
 "pcs2_current_l1":0,
 "pcs2_current_l2":0,
 "pcs2_current_l3":0,
 "pcs2_active_power":0,
 "pcs2_reactive_power":0,
 "pcs2_voltage_dc":0,
 "pcs2_current_dc":0,
 "pcs2_active_power_dc":0,
 "pcs2_alarms":0,
 "pcs2_faults":0,
 "maximum_charging_power_limit_pcs2":0,
 "maximum_discharging_power_limit_pcs2":0,
 "bms_maximum_cell_voltage":0,
 "stackindex_of_the_bms_maximum_cell_voltage":0,
 "cluster_index_of_the_bms_maximum_cell_voltage":0,
 "bms_minimum_cell_voltage":0,
 "stackindex_of_the_bms_minimum_cell_voltage":0,
 "cluster_index_of_the_bms_minimum_cell_voltage":0,
 "bms_maximum_cell_temperature":0,
 "stackindex_of_the_bms_maximum_cell_temperature":0,
 "cluster_index_of_the_bms_maximum_cell_temperature":0,
 "bms_minimum_cell_temperature":0,
 "stackindex_of_the_bms_minimum_cell_temperature":0,
 "cluster_index_of_the_bms_minimum_cell_temperature":0,
 "bms_bus_voltage":0,
 "bms_bus_current":0,
 "bms_soc":0,
 "bms_power":0,
 "operation_status":0,
 "bms_current_chargeable_capacity":0,
 "bms_current_dischargeable_capacity":0,
 "circuit_breaker_control_word":0,
 "main_circuit_breaker_status":0,
 "control_circuit_breaker":0,
 "bms_maximum_charging_power_limit":0,
 "bms_maximum_discharging_power_limit":0,
 "power_limit_state":0,
 "bms_alarms":0,
 "bms_faults":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "cluster_no_1_battery_cluster_voltage":0,
 "cluster_no_2_battery_cluster_voltage":0,
 "cluster_no_3_battery_cluster_voltage":0,
 "cluster_no_4_battery_cluster_voltage":0,
 "cluster_no_5_battery_cluster_voltage":0,
 "cluster_no_6_battery_cluster_voltage":0,
 "cluster_no_7_battery_cluster_voltage":0,
 "cluster_no_8_battery_cluster_voltage":0,
 "cluster_no_9_battery_cluster_voltage":0,
 "cluster_no_10_battery_cluster_voltage":0,
 "cluster_no_11_battery_cluster_voltage":0,
 "cluster_no_1_battery_cluster_current":0,
 "cluster_no_2_battery_cluster_current":0,
 "cluster_no_3_battery_cluster_current":0,
 "cluster_no_4_battery_cluster_current":0,
 "cluster_no_5_battery_cluster_current":0,
 "cluster_no_6_battery_cluster_current":0,
 "cluster_no_7_battery_cluster_current":0,
 "cluster_no_8_battery_cluster_current":0,
 "cluster_no_9_battery_cluster_current":0,
 "cluster_no_10_battery_cluster_current":0,
 "cluster_no_11_battery_cluster_current":0,
 "cluster_no_1_max_cell_voltage":0,
 "cluster_no_1_max_cell_voltage_pack_num":0,
 "cluster_no_2_max_cell_voltage":0,
 "cluster_no_2_max_cell_voltage_pack_num":0,
 "cluster_no_3_max_cell_voltage":0,
 "cluster_no_3_max_cell_voltage_pack_num":0,
 "cluster_no_4_max_cell_voltage":0,
 "cluster_no_4_max_cell_voltage_pack_num":0,
 "cluster_no_5_max_cell_voltage":0,
 "cluster_no_5_max_cell_voltage_pack_num":0,
 "cluster_no_6_max_cell_voltage":0,
 "cluster_no_6_max_cell_voltage_pack_num":0,
 "cluster_no_7_max_cell_voltage":0,
 "cluster_no_7_max_cell_voltage_pack_num":0,
 "cluster_no_8_max_cell_voltage":0,
 "cluster_no_8_max_cell_voltage_pack_num":0,
 "cluster_no_9_max_cell_voltage":0,
 "cluster_no_9_max_cell_voltage_pack_num":0,
 "cluster_no_10_max_cell_voltage":0,
 "cluster_no_10_max_cell_voltage_pack_num":0,
 "cluster_no_11_max_cell_voltage":0,
 "cluster_no_11_max_cell_voltage_pack_num":0,
 "cluster_no_1_second_max_cell_voltage":0,
 "cluster_no_1_second_max_cell_voltage_pack_num":0,
 "cluster_no_2_second_max_cell_voltage":0,
 "cluster_no_2_second_max_cell_voltage_pack_num":0,
 "cluster_no_3_second_max_cell_voltage":0,
 "cluster_no_3_second_max_cell_voltage_pack_num":0,
 "cluster_no_4_second_max_cell_voltage":0,
 "cluster_no_4_second_max_cell_voltage_pack_num":0,
 "cluster_no_5_second_max_cell_voltage":0,
 "cluster_no_5_second_max_cell_voltage_pack_num":0,
 "cluster_no_6_second_max_cell_voltage":0,
 "cluster_no_6_second_max_cell_voltage_pack_num":0,
 "cluster_no_7_second_max_cell_voltage":0,
 "cluster_no_7_second_max_cell_voltage_pack_num":0,
 "cluster_no_8_second_max_cell_voltage":0,
 "cluster_no_8_second_max_cell_voltage_pack_num":0,
 "cluster_no_9_second_max_cell_voltage":0,
 "cluster_no_9_second_max_cell_voltage_pack_num":0,
 "cluster_no_10_second_max_cell_voltage":0,
 "cluster_no_10_second_max_cell_voltage_pack_num":0,
 "cluster_no_11_second_max_cell_voltage":0,
 "cluster_no_11_second_max_cell_voltage_pack_num":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "cluster_no_1_min_cell_voltage":0,
 "cluster_no_1_min_cell_voltage_pack_num":0,
 "cluster_no_2_min_cell_voltage":0,
 "cluster_no_2_min_cell_voltage_pack_num":0,
 "cluster_no_3_min_cell_voltage":0,
 "cluster_no_3_min_cell_voltage_pack_num":0,
 "cluster_no_4_min_cell_voltage":0,
 "cluster_no_4_min_cell_voltage_pack_num":0,
 "cluster_no_5_min_cell_voltage":0,
 "cluster_no_5_min_cell_voltage_pack_num":0,
 "cluster_no_6_min_cell_voltage":0,
 "cluster_no_6_min_cell_voltage_pack_num":0,
 "cluster_no_7_min_cell_voltage":0,
 "cluster_no_7_min_cell_voltage_pack_num":0,
 "cluster_no_8_min_cell_voltage":0,
 "cluster_no_8_min_cell_voltage_pack_num":0,
 "cluster_no_9_min_cell_voltage":0,
 "cluster_no_9_min_cell_voltage_pack_num":0,
 "cluster_no_10_min_cell_voltage":0,
 "cluster_no_10_min_cell_voltage_pack_num":0,
 "cluster_no_11_min_cell_voltage":0,
 "cluster_no_11_min_cell_voltage_pack_num":0,
 "cluster_no_1_second_min_cell_voltage":0,
 "cluster_no_1_second_min_cell_voltage_pack_num":0,
 "cluster_no_2_second_min_cell_voltage":0,
 "cluster_no_2_second_min_cell_voltage_pack_num":0,
 "cluster_no_3_second_min_cell_voltage":0,
 "cluster_no_3_second_min_cell_voltage_pack_num":0,
 "cluster_no_4_second_min_cell_voltage":0,
 "cluster_no_4_second_min_cell_voltage_pack_num":0,
 "cluster_no_5_second_min_cell_voltage":0,
 "cluster_no_5_second_min_cell_voltage_pack_num":0,
 "cluster_no_6_second_min_cell_voltage":0,
 "cluster_no_6_second_min_cell_voltage_pack_num":0,
 "cluster_no_7_second_min_cell_voltage":0,
 "cluster_no_7_second_min_cell_voltage_pack_num":0,
 "cluster_no_8_second_min_cell_voltage":0,
 "cluster_no_8_second_min_cell_voltage_pack_num":0,
 "cluster_no_9_second_min_cell_voltage":0,
 "cluster_no_9_second_min_cell_voltage_pack_num":0,
 "cluster_no_10_second_min_cell_voltage":0,
 "cluster_no_10_second_min_cell_voltage_pack_num":0,
 "cluster_no_11_second_min_cell_voltage":0,
 "cluster_no_11_second_min_cell_voltage_pack_num":0,
 "cluster_no_1_max_cell_temperature":0,
 "cluster_no_1_max_cell_temperature_pack_num":0,
 "cluster_no_2_max_cell_temperature":0,
 "cluster_no_2_max_cell_temperature_pack_num":0,
 "cluster_no_3_max_cell_temperature":0,
 "cluster_no_3_max_cell_temperature_pack_num":0,
 "cluster_no_4_max_cell_temperature":0,
 "cluster_no_4_max_cell_temperature_pack_num":0,
 "cluster_no_5_max_cell_temperature":0,
 "cluster_no_5_max_cell_temperature_pack_num":0,
 "cluster_no_6_max_cell_temperature":0,
 "cluster_no_6_max_cell_temperature_pack_num":0,
 "cluster_no_7_max_cell_temperature":0,
 "cluster_no_7_max_cell_temperature_pack_num":0,
 "cluster_no_8_max_cell_temperature":0,
 "cluster_no_8_max_cell_temperature_pack_num":0,
 "cluster_no_9_max_cell_temperature":0,
 "cluster_no_9_max_cell_temperature_pack_num":0,
 "cluster_no_10_max_cell_temperature":0,
 "cluster_no_10_max_cell_temperature_pack_num":0,
 "cluster_no_11_max_cell_temperature":0,
 "cluster_no_11_max_cell_temperature_pack_num":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "cluster_no_1_second_max_cell_temperature":0,
 "cluster_no_1_second_max_cell_temperature_pack_num":0,
 "cluster_no_2_second_max_cell_temperature":0,
 "cluster_no_2_second_max_cell_temperature_pack_num":0,
 "cluster_no_3_second_max_cell_temperature":0,
 "cluster_no_3_second_max_cell_temperature_pack_num":0,
 "cluster_no_4_second_max_cell_temperature":0,
 "cluster_no_4_second_max_cell_temperature_pack_num":0,
 "cluster_no_5_second_max_cell_temperature":0,
 "cluster_no_5_second_max_cell_temperature_pack_num":0,
 "cluster_no_6_second_max_cell_temperature":0,
 "cluster_no_6_second_max_cell_temperature_pack_num":0,
 "cluster_no_7_second_max_cell_temperature":0,
 "cluster_no_7_second_max_cell_temperature_pack_num":0,
 "cluster_no_8_second_max_cell_temperature":0,
 "cluster_no_8_second_max_cell_temperature_pack_num":0,
 "cluster_no_9_second_max_cell_temperature":0,
 "cluster_no_9_second_max_cell_temperature_pack_num":0,
 "cluster_no_10_second_max_cell_temperature":0,
 "cluster_no_10_second_max_cell_temperature_pack_num":0,
 "cluster_no_11_second_max_cell_temperature":0,
 "cluster_no_11_second_max_cell_temperature_pack_num":0,
 "cluster_no_1_min_cell_temperature":0,
 "cluster_no_1_min_cell_temperature_pack_num":0,
 "cluster_no_2_min_cell_temperature":0,
 "cluster_no_2_min_cell_temperature_pack_num":0,
 "cluster_no_3_min_cell_temperature":0,
 "cluster_no_3_min_cell_temperature_pack_num":0,
 "cluster_no_4_min_cell_temperature":0,
 "cluster_no_4_min_cell_temperature_pack_num":0,
 "cluster_no_5_min_cell_temperature":0,
 "cluster_no_5_min_cell_temperature_pack_num":0,
 "cluster_no_6_min_cell_temperature":0,
 "cluster_no_6_min_cell_temperature_pack_num":0,
 "cluster_no_7_min_cell_temperature":0,
 "cluster_no_7_min_cell_temperature_pack_num":0,
 "cluster_no_8_min_cell_temperature":0,
 "cluster_no_8_min_cell_temperature_pack_num":0,
 "cluster_no_9_min_cell_temperature":0,
 "cluster_no_9_min_cell_temperature_pack_num":0,
 "cluster_no_10_min_cell_temperature":0,
 "cluster_no_10_min_cell_temperature_pack_num":0,
 "cluster_no_11_min_cell_temperature":0,
 "cluster_no_11_min_cell_temperature_pack_num":0,
 "cluster_no_1_second_min_cell_temperature":0,
 "cluster_no_1_second_min_cell_temperature_pack_num":0,
 "cluster_no_2_second_min_cell_temperature":0,
 "cluster_no_2_second_min_cell_temperature_pack_num":0,
 "cluster_no_3_second_min_cell_temperature":0,
 "cluster_no_3_second_min_cell_temperature_pack_num":0,
 "cluster_no_4_second_min_cell_temperature":0,
 "cluster_no_4_second_min_cell_temperature_pack_num":0,
 "cluster_no_5_second_min_cell_temperature":0,
 "cluster_no_5_second_min_cell_temperature_pack_num":0,
 "cluster_no_6_second_min_cell_temperature":0,
 "cluster_no_6_second_min_cell_temperature_pack_num":0,
 "cluster_no_7_second_min_cell_temperature":0,
 "cluster_no_7_second_min_cell_temperature_pack_num":0,
 "cluster_no_8_second_min_cell_temperature":0,
 "cluster_no_8_second_min_cell_temperature_pack_num":0,
 "cluster_no_9_second_min_cell_temperature":0,
 "cluster_no_9_second_min_cell_temperature_pack_num":0,
 "cluster_no_10_second_min_cell_temperature":0,
 "cluster_no_10_second_min_cell_temperature_pack_num":0,
 "cluster_no_11_second_min_cell_temperature":0,
 "cluster_no_11_second_min_cell_temperature_pack_num":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "cluster_no_1_battery_cluster_average_voltage":0,
 "cluster_no_2_battery_cluster_average_voltage":0,
 "cluster_no_3_battery_cluster_average_voltage":0,
 "cluster_no_4_battery_cluster_average_voltage":0,
 "cluster_no_5_battery_cluster_average_voltage":0,
 "cluster_no_6_battery_cluster_average_voltage":0,
 "cluster_no_7_battery_cluster_average_voltage":0,
 "cluster_no_8_battery_cluster_average_voltage":0,
 "cluster_no_9_battery_cluster_average_voltage":0,
 "cluster_no_10_battery_cluster_average_voltage":0,
 "cluster_no_11_battery_cluster_average_voltage":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "stack_overall_average_cell_temperature":0,
 "average_ambient_temperature_inside_the_container":0,
 "average_ambient_humidity_inside_the_container":0,
 "maximum_cell_temperature":0,
 "stack_index_of_maximum_cell_temperature":0,
 "cluster_index_of_maximum_cell_temperature":0,
 "pack_index_of_maximum_cell_temperature":0,
 "minimum_cell_temperature":0,
 "stack_index_of_minimum_cell_temperature":0,
 "cluster_index_of_minimum_cell_temperature":0,
 "pack_index_of_minimum_cell_temperature":0,
 "offline_bams":0,
 "offline_air_conditioners":0,
 "offline_temperature_and_humidity_sensors":0,
 "meter_online_status":0,
 "gas_sensor_online_status":0,
 "number_of_online_bams":0,
 "number_of_online_air_conditioners":0,
 "number_of_online_temperature_and_humidity":0,
 "number_of_online_meters":0,
 "number_of_online_gas_sensors":0,
 "emmu_alarms":0,
 "emmu_faults":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "carbon_monoxide_concentration":0,
 "hydrogen_concentration":0,
 "oxygen_concentration":0,
 "carbon_monoxide_alarm_status":0,
 "hydrogen_alarm_status":0,
 "oxygen_alarm_status":0,
 "temperature_reading_of_temperature_and_humidty_sensor_no_1":0,
 "temperature_reading_of_temperature_and_humidty_sensor_no_2":0,
 "temperature_reading_of_temperature_and_humidty_sensor_no_3":0,
 "humidity_reading_of_temperature_and_humidty_sensor_no_1":0,
 "humidity_reading_of_temperature_and_humidty_sensor_no_2":0,
 "humidity_reading_of_temperature_and_humidty_sensor_no_3":0
}'&
/usr/local/bin/fims/fims_echo -u /components/clou_ess_1_low_speed -b '{
 "air_conditioner_temperature_limit_1":0,
 "air_conditioner_temperature_limit_2":0,
 "air_conditioner_temperature_limit_3":0,
 "air_conditioner_1_current_mode":0,
 "air_conditioner_1_temperature":0,
 "air_conditioner_1_humidity":0,
 "air_conditioner_2_current_mode":0,
 "air_conditioner_2_temperature":0,
 "air_conditioner_2_humidity":0,
 "air_conditioner_3_current_mode":0,
 "air_conditioner_3_temperature":0,
 "air_conditioner_3_humidity":0,
 "air_conditioner_4_current_mode":0,
 "air_conditioner_4_temperature":0,
 "air_conditioner_4_humidity":0,
 "air_conditioner_5_current_mode":0,
 "air_conditioner_5_temperature":0,
 "air_conditioner_5_humidity":0,
 "air_conditioner_6_current_mode":0,
 "air_conditioner_6_temperature":0,
 "air_conditioner_6_humidity":0,
 "air_conditioner_7_current_mode":0,
 "air_conditioner_7_temperature":0,
 "air_conditioner_7_humidity":0,
 "air_conditioner_8_current_mode":0,
 "air_conditioner_8_temperature":0,
 "air_conditioner_8_humidity":0,
 "air_conditioner_9_current_mode":0,
 "air_conditioner_9_temperature":0,
 "air_conditioner_9_humidity":0,
 "air_conditioner_10_current_mode":0,
 "air_conditioner_10_temperature":0,
 "air_conditioner_10_humidity":0,
 "air_conditioner_11_current_mode":0,
 "air_conditioner_11_temperature":0,
 "air_conditioner_11_humidity":0,
 "air_conditioner_12_current_mode":0,
 "air_conditioner_12_temperature":0,
 "air_conditioner_12_humidity":0,
 "air_conditioner_13_current_mode":0,
 "air_conditioner_13_temperature":0,
 "air_conditioner_13_humidity":0,
 "air_conditioner_14_current_mode":0,
 "air_conditioner_14_temperature":0,
 "air_conditioner_14_humidity":0,
 "air_conditioner_15_current_mode":0,
 "air_conditioner_15_temperature":0,
 "air_conditioner_15_humidity":0,
 "air_conditioner_16_current_mode":0,
 "air_conditioner_16_temperature":0,
 "air_conditioner_16_humidity":0
}'&
