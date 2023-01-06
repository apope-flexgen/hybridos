#!/bin/sh
# send some data
fsend='/usr/local/bin/fims/fims_send -m pub -u'

$fsend /components/catl_mbmu_summary_r '{"mbmu_voltage":1317.2,"mbmu_current":21402,"mbmu_soc":59.3,"mbmu_soh":100,"mbmu_max_cell_voltage":3.189,"mbmu_min_cell_voltage":3.148,"mbmu_avg_cell_voltage":3.172,"mbmu_max_cell_temperature":72,"mbmu_min_cell_temperature":70,"mbmu_avg_cell_temperature":70,"mbmu_max_charge_current":17474,"mbmu_max_discharge_current":22546,"Timestamp":"04-30-2021 09:18:22.486902"}'

$fsend /components/catl_mbmu_stat_r '{"mbmu_status":1,"Timestamp":"04-30-2021 09:18:22.571984"}'

$fsend /components/catl_mbmu_summary_r '{"mbmu_voltage":1317.2,"mbmu_current":21402,"mbmu_soc":59.3,"mbmu_soh":100,"mbmu_max_cell_voltage":3.189,"mbmu_min_cell_voltage":3.148,"mbmu_avg_cell_voltage":3.172,"mbmu_max_cell_temperature":72,"mbmu_min_cell_temperature":70,"mbmu_avg_cell_temperature":70,"mbmu_max_charge_current":17472,"mbmu_max_discharge_current":22546,"Timestamp":"04-30-2021 09:18:22.581919"}'

$fsend /components/catl_ems_bms_rw '{"ems_heartbeat":124,"ems_cmd":[{"value":1,"string":"Stay Status"}],"ems_status":0,"ems_rtc_year":2021,"ems_rtc_month":4,"ems_rtc_day":30,"ems_rtc_hour":1,"ems_rtc_minute":1,"ems_rtc_second":0,"fault_clear_cmd":0,"Timestamp":"04-30-2021 09:18:22.701811"}'

$fsend /components/catl_bms_ems_r '{"bms_heartbeat":157,"bms_poweron":1,"bms_status":1,"num_hv_subsystem":9,"bms_remain_charge_energy":1368,"bms_remain_discharge_energy":1985.4,"bms_max_discharge_allowed":22546,"bms_max_charge_allowed":17472,"bms_limit_charge_hv":15808,"bms_limit_discharge_hv":10816,"Timestamp":"04-30-2021 09:18:22.867015"}'


