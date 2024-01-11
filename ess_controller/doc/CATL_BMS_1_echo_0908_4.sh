/usr/local/bin/fims/fims_echo -u /components/catl_ems_bms_rw -b '{
 "ems_cmd":{"value":0},
 "ems_heartbeat":{"value":0},
 "ems_rtc_day":{"value":0},
 "ems_rtc_hour":{"value":0},
 "ems_rtc_minute":{"value":0},
 "ems_rtc_second":{"value":0},
 "ems_rtc_year":{"value":0},
 "ems_status":{"value":0},
 "fault_clear_cmd":{"value":0}
}'&
/usr/local/bin/fims/fims_echo -u /components/catl_mbmu_control_r -b '{
 "mbmu_avg_cell_temperature":{"value":0},
 "mbmu_avg_cell_voltage":{"value":0},
 "mbmu_current":{"value":0},
 "mbmu_max_cell_temperature":{"value":0},
 "mbmu_max_cell_voltage":{"value":0},
 "mbmu_max_charge_current":{"value":0},
 "mbmu_max_dischare_current":{"value":0},
 "mbmu_min_cell_temperature":{"value":0},
 "mbmu_min_cell_voltage":{"value":0},
 "mbmu_soc":{"value":0},
 "mbmu_soh":{"value":0},
 "mbmu_voltage":{"value":0}
}'&
/usr/local/bin/fims/fims_echo -u /components/catl_mbmu_stat_r -b '{
 "mbmu_status":{"value":0}
}'&
/usr/local/bin/fims/fims_echo -u /components/catl_mbmu_sum_r -b '{
 "mbmu_warning_1":{"value":0},
 "mbmu_warning_2":{"value":0}
}'&
