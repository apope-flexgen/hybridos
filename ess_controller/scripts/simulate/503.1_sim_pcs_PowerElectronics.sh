#!/bin/sh
# this script sets up the Power Electroncs Personality intto the PCS simulator

# char * tval = (char *)"/components/pcs_registers_fast:current_fault";
#         vm->setVal(vmap, "/links/pcs", "current_fault", tval);
#         tval = (char *)"/components/pcs_registers_fast:current_status";
#         vm->setVal(vmap, "/links/pcs", "current_status", tval);
#         tval = (char *)"/components/pcs_registers_fast:current_warning";
#         vm->setVal(vmap, "/links/pcs", "current_warning", tval);     
# so , once we get the time ticking , we will activate a remap for /components/bms_sim/bms_heartbeat


# Method:  set
# Uri:     /$SIM/components/pcs_registers_fast/vdc_bus_1
# ReplyTo: /me
# Body:    1400
# Timestamp:   2021-06-27 04:50:33.490270
# Method:  set
# Uri:     /$SIM/components/pcs_registers_fast
# ReplyTo: /me
# Body:    {"current_status":2}
# Timestamp:   2021-06-27 04:50:33.496606
# Method:  set
# Uri:     /$SIM/components/catl_bms_ems_r
# ReplyTo: /me
# Body:    {"bms_status":1}
# Timestamp:   2021-06-27 04:50:33.500361
# Method:  set
# Uri:     /$SIM/components/catl_bms_ems_r
# ReplyTo: /me
# Body:    {"bms_poweron":0}
# Timestamp:   2021-06-27 04:50:33.503108
# Method:  set
# Uri:     /$SIM/components/catl_bms_ems_r
# ReplyTo: /me
# Body:    {"bms_poweron":1}
# Timestamp:   2021-06-27 04:50:38.520480
# Method:  set
# Uri:     /$SIM/components/pcs_registers_fast
# ReplyTo: /me
# Body:    {"current_status":4}
# Timestamp:   2021-06-27 04:50:43.525648
# Method:  set
# Uri:     /$SIM/components/pcs_registers_fast
# ReplyTo: /me
# Body:    {"current_status":6}
# Timestamp:   2021-06-27 04:50:48.533247
# Method:  set
# Uri:     /$SIM/components/gpio/EStop
# ReplyTo: /me
# Body:    false
# Timestamp:   2021-06-27 04:50:54.542481
# Method:  set
# Uri:     /$SIM/components/pcs_registers_fast
# ReplyTo: /me
# Body:    {"current_status":4}
# Timestamp:   2021-06-27 04:50:59.546636
# this are some of the controls


# Uri:     /components/pcs_registers_slow
# ReplyTo: (null)

SIM=flex

echo " setup registers slow"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_slow '
{
"vdc_low_start":1000,"vac_low_start":95,"vac_high_start":105,"start_delay":5,
"tracker_enable":0,"tracker_vac_out":480,"tracker_vac_min":80,
"tracker_time_start":60.5,"svr_enable":0,"svr_v1_threshold":105,"svr_v2_threshold":107,
"svr_v1_hysteresis":2,"svr_v2_hysetresis":2,"svr_grad_inject":0.3,"svr_grad_recover_q":1.7,
"svr_grad_reduction_p":0.3,"svr_grad_recover_p":1.7,"svr_cosfi_limit":0.85,
"vp_ref_power":0,"vp_rec_mode":0,"vp_enable":0,"vp_v1":110,"vp_v2":120,"vp_p1":100,"vp_p2":0,
"vp_hysteresis":0,"vp_delay_hysteresis":60,"vp_response_tiime":10,"active_bal_activate":0,
"bank_bal_activate":0,"bank_bal_reconnect_grad":100,"cvsb_activate":0,
"cvsb_time_to_enter":1200,"lv_enable":26,"lv_slow":90,"lv_delay_slow":0.3,
"lv_fast":20,"lv_delay_fast":1,"lv_very_fast":0,"lv_delay_very_fast":10,
"lv_interpolation":0,"hv_enable":24,"hv_slow":110,"hv_delay_slow":1,"hv_fast":130,
"hv_delay_fast":0.5,"hv_very_fast":130,"hv_delay_very_fast":0.01,"hv_interpolation":0,
"lf_enable":20,"lf_slow":58.8,"lf_delay_slow":30,"lf_fast":57,"lf_delay_fast":0.1,
"lf_very_fast":5.6,"lf_delay_very_fast":0.1,"hf_enable":20,"hf_slow":61.2,"hf_delay_slow":30,
"hf_fast":61.8,"hf_delay_fast":0.1,"hf_very_fast":61,"hf_delay_very_fast":0.01,
"unbalance_enable":3,"unbalance_i_threshold":30,"unbalance_i_delay":5,
"unbalance_v_threshold":25,"unbalance_v_delay":5,"unbalance_i_min":30,
"antiislanding_enable":2,"igbt_temp_thres_fit":125,"igbt_temp_hist":10,"module_temp_thres_fit":85,
"module_temp_hist":10,"internal_temp_thres_fit":85,"earth_current":30,"high_vdc_delay":30,
"du_overcurrent":400,"lv_fast_2":45,"lv_delay_fast_2":0.01,"lv_very_fast_2":35,
"lv_delay_very_fast_2":0.01,"hv_fast_2":125,"hv_delay_fast_2":0.01,"hv_very_fast_2":135,
"hv_delay_very_fast_2":0.01,"lf_slow_2":59.3,"lf_delay_slow_2":0.01,"lf_very_fast_2":55,
"lf_delay_very_fast_2":0.5,"hf_slow_2":60.4,"hf_delay_slow_2":0.01,
"hf_very_fast_2":61.5,"hf_delay_very_fast_2":0.5,"hvpl_enable":0,"hvpl_activation_level_5":70,
"hvpl_scale_1000":1000,"hvpl_fault_level_0":0,"hvpl_filter_hz_100_hz":0.1,
"hvpl_delay_fault_s_0_s":2,"hvpl_start_test":0,"hvpl_test_level_20":-20,"lv_lf_enable":0,
"lv_lf_V":0,"lv_lf_f":0,"lv_lf_t":0,"mv_manometer_fault":0,"mv_manometer_delay":10,
"lv_manometer_fault":0,"lv_manometer_delay":10,"hv_hf_enable":0,"hv_hf_V":120,
"hv_hf_f":62,"hv_hf_t":1,"lv_hf_enable":0,"lv_hf_V":80,"lv_hf_f":62,"lv_hf_t":1,
"hv_lf_enable":0,"hv_lf_V":120,"hv_lf_f":58,"hv_lf_t":1,"auto_reset_enable":1,
"number_of_retries":5,"delay_between_retries_s_s":3,"reset_retries_timeout":30,
"all_faults_enabled":1,"faults_allowed_number":0,"faults_allowed_1":0,"faults_allowed_2":0,
"faults_allowed_3":0,"faults_allowed_4":0,"faults_allowed_5":0,"faults_allowed_6":0,
"faults_allowed_7":0,"faults_allowed_8":0,"faults_allowed_9":0,"faults_allowed_10":0,
"faults_exception_number":1,"faults_exception_1":45,"faults_exception_2":0,
"faults_exception_3":0,"faults_exception_4":0,"faults_exception_5":0,
"faults_exception_6":0,"faults_exception_7":0,"faults_exception_8":0,"faults_exception_9":0,
"faults_exception_10":0,"leak_protection_type_1":1,"leak_imi_activation_1":1,
"leak_fault_threshold_1":5,"leak_fault_recover_threshold_1":0.0575,"leak_warning_theshold_1":50,
"leak_self_test_exec_period_1":1440,"leak_self_test_manual_command_1":0,
"leak_protection_type_2":0,"leak_imi_activation_2":0,"leak_fault_threshold_2":5,
"leak_fault_recover_threshold_2":0.0575,"leak_warning_theshold_2":50,
"leak_self_test_exec_period_2":1440,"leak_self_test_manual_command_2":0,"leak_protection_type_3":0,
"leak_imi_activation_3":0,"leak_fault_threshold_3":5,"leak_fault_recover_threshold_3":0.0575,
"leak_warning_theshold_3":50,"leak_self_test_exec_period_3":1440,
"leak_self_test_manual_command_3":0,"enabled_nec":1,"du_type":4,"working_mode":1,
"battery_module_model":0,"bms_banks_enabled":1,"insulation_status_1":2,"insulation_resistor_msb_1":25,
"insulation_resistor_lsb_1":58176,"insulation_measurement_started_time_1":3,
"insulation_self_test_started_time_1":0,"insulation_self_test_exec_countdown_1":294,
"insulation_status_update_1":0,"insulation_status_2":0,"insulation_resistor_msb_2":65535,
"insulation_resistor_lsb_2":65535,"insulation_measurement_started_time_2":0,
"insulation_self_test_started_time_2":0,"insulation_self_test_exec_countdown_2":0,
"insulation_status_update_2":0,"insulation_status_3":0,"insulation_resistor_msb_3":65535,
"insulation_resistor_lsb_3":65535,"insulation_measurement_started_time_3":0,
"insulation_self_test_started_time_3":0,"insulation_self_test_exec_countdown_3":0,
"insulation_status_update_3":0,"select_module":0,"daily_energy_msb":0,
"daily_energy_lsb":0,"yesterday_energy_msb":0,"yesterday_energy_lsb":0,"current_month_energy_msb":0,
"current_month_energy_lsb":525,"last_month_energy_msb":0,"last_month_energy_lsb":0,
"total_energy_msb":0,"total_energy_lsb":525,"select_module_reactive_energy":0,
"daily_reactive_energy_msb":0,"daily_reactive_energy_lsb":0,"yesterday_reactive_energy_msb":0,
"yesterday_reactive_energy_lsb":0,"current_month_reactive_energy_msb":0,
"current_month_reactive_energy_lsb":969,"last_month_reactive_energy_msb":0,
"last_month_reactive_energy_lsb":0,"total_reactive_energy_msb":0,"total_reactive_energy_lsb":969,
"total_run_time_years":0,"total_run_time_days":0,"total_run_time_hours":1,
"partial_run_time_years":0,"partial_run_time_days":0,"partial_run_time_hours":1,
"total_grid_connections":22,"partial_grid_connections":22,"consum_daily_energy_msb":0,
"consum_daily_energy_lsb":0,"consum_total_energy_msb":0,"consum_total_energy_lsb":957,
"last_fault":51,"last_fault_date":57877,"last_fault_time":2083,
"ninth_fault":191,"ninth_fault_date":55829,"ninth_fault_time":2572,"eighth_fault":51,
"eighth_fault_date":43541,"eighth_fault_time":5397,"seventh_fault":191,
"seventh_fault_date":43541,"seventh_fault_time":4907,"sixth_fault":150,"sixth_fault_date":43541,
"sixth_fault_time":4401,"fifth_fault":41,"fifth_fault_date":45461,"fifth_fault_time":4906,
"fourth_fault":44,"fourth_fault_date":8597,"fourth_fault_time":4615,"third_fault":41,
"third_fault_date":8597,"third_fault_time":4613,"second_fault":44,"second_fault_date":8597,
"second_fault_time":4613,"first_fault":41,"first_fault_date":8597,
"first_fault_time":4613,"last_warning":45,"last_warning_date":61973,
"last_warning_time":1298,"ninth_warning":45,"ninth_warning_date":61973,"ninth_warning_time":9,
"eighth_warning":45,"eighth_warning_date":61973,"eighth_warning_time":8,"seventh_warning":45,
"seventh_warning_date":61973,"seventh_warning_time":8,"sixth_warning":45,
"sixth_warning_date":61973,"sixth_warning_time":8,"fifth_warning":45,
"fifth_warning_date":61973,"fifth_warning_time":8,"fourth_warning":45,
"fourth_warning_date":61973,"fourth_warning_time":8,"third_warning":45,"third_warning_date":61973,
"third_warning_time":7,"second_warning":45,"second_warning_date":61973,"second_warning_time":7,
"first_warning":45,"first_warning_date":61973,"first_warning_time":7,"last_event":2,
"last_event_date":57877,"last_event_time":2337,"ninth_event":2,"ninth_event_date":57877,
"ninth_event_time":2335,"eighth_event":2,"eighth_event_date":57877,
"eighth_event_time":2087,"seventh_event":2,"seventh_event_date":55829,
"seventh_event_time":2317,"sixth_event":2,"sixth_event_date":55829,"sixth_event_time":2069,
"fifth_event":2,"fifth_event_date":53781,"fifth_event_time":2330,"fourth_event":2,
"fourth_event_date":53781,"fourth_event_time":1799,"third_event":2,
"third_event_date":53781,"third_event_time":1795,"second_event":2,"second_event_date":53781,
"second_event_time":1588,"first_event":2,"first_event_date":43541,"first_event_time":2837,
"selector_config":2,"selector_status":43541,"s_nominal":58500,"num_modules":6,
"du_feedback_status":181,"bms1_status":0,"bms1_strings_in_service":0,
"bms1_total_strings_count":0,"bms1_system_voltage":0,"bms1_system_current":0,"bms1_system_soc":0,
"bms2_status":0,"bms2_strings_in_service":0,"bms2_total_strings_count":0,
"bms2_system_voltage":0,"bms2_system_current":0,"bms2_system_soc":0,"bms3_status":0,
"bms3_strings_in_service":0,"bms3_total_strings_count":0,"bms3_system_voltage":0,
"bms3_system_current":0,"bms3_system_soc":0,"bms_select":0.1,"bms_sys_heartbeat":0,
"bms_strings_service":0,"bms_total_strings_count":0,"bms_service_voltage":0,
"bms_service_soc":0,"bms_voltage":0,"bms_current":0,"bms_soc":0,"bms_soh":0,"bms_mode":0,
"bms_max_v":0,"bms_min_v":0,"bms_max_temp":0,"bms_min_temp":0,"bms_min_partial_strings":0,
"bms_partial_voltage":0,"bms_config_status":0,"bms_protection_4":0,
"bms_protection_3":0,"bms_protection_2":0,"bms_protection_1":0,"bms_alarm_4":0,"bms_alarm_3":0,
"bms_alarm_2":0,"bms_alarm_1":0,"bms_dcl":0,"bms_ccl":0,"bms_watchdog_response":0,
"bms_dio_status":0,"bms_alarm_status":0,"bmsenc_door_contact":0,"bmsenc_fcp_alarm":0,
"bmsenc_fcp_predischarge":0,"bmsenc_fcp_release":0,"bmsenc_fcp_sup":0,
"bmsenc_dc_disconnected_1":0,"bmsenc_dc_disconnected_2":0,"bmsenc_emergency_stop":0,"bmsenc_ups_failure":0,
"bmsenc_ups_ok":0,"bmsenc_temp_humidity_t":0,"bmsenc_temp_humidity_h":0,
"bmsenc_temp_1":0,"bmsenc_temp_2":0,"bmsenc_temp_3":0,"bmsenc_temp_4":0,
"bmsenc_temp_5":0,"bmsenc_temp_6":0,"transformer_alarms":0,"hvac_signals_1":0,
"hvac_signals_2":0,"hvac_signals_3":0,"hvac_signals_4":0,"Timestamp":"04-30-2021 05:22:39.955138"
}'

echo " setup registers to be moved from fast"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_slow '
{
"minutes":23,"hours":5,"day":30,"month":4,"year":2021,"utc":0,
"access":37358,"user":4,
"night_mode_enable":0,"night_mode_q_ref":0,"night_mode_vbus_ref":1125,"night_mode_activate":0,
"start_grad_p_enable":1,"start_grad_p":633.3,
"start_grad_q_enable":1,"start_grad_q":633.3,
"rise_grad_p_enable":1,"rise_grad_p":633.3,
"rise_grad_q_enable":1,"rise_grad_q":633.3,
"drop_grad_p_enable":1,"drop_grad_p":633.3,
"drop_grad_q_enable":1,"drop_grad_q":633.3,
"stop_grad_p_enable":1,"stop_grad_p":633.3,
"stop_grad_q_enable":1,"stop_grad_q":633.3,
"svr_enable":0,"svr_v1_threshold":105,"svr_v2_threshold":107,
"svr_v1_hysteresis":2,"svr_v2_hysetresis":2,
"svr_grad_inject":0.3,"svr_grad_recover_q":1.7,
"svr_grad_reduction_p":0.3,"svr_grad_recover_p":1.7,
"svr_cosfi_limit":0.85,
"lv_enable":26,
"lv_slow":90,"lv_delay_slow":0.3,
"lv_fast":20,"lv_delay_fast":1,
"lv_very_fast":0,"lv_delay_very_fast":10,
"lv_interpolation":0,
"hv_enable":24,"hv_slow":110,"hv_delay_slow":1,
"hv_fast":130,"hv_delay_fast":0.5,"hv_very_fast":130,
"hv_delay_very_fast":0.01,
"hv_interpolation":0,
"lf_enable":20,"lf_slow":58.8,"lf_delay_slow":30,
"lf_fast":57,"lf_delay_fast":0.1,
"lf_very_fast":5.6,"lf_delay_very_fast":0.1,
"hf_enable":20,"hf_slow":61.2,"hf_delay_slow":30,
"hf_fast":61.8,"hf_delay_fast":0.1,
"hf_very_fast":61,"hf_delay_very_fast":0.01,
"unbalance_enable":3,
"unbalance_i_threshold":30,"unbalance_i_delay":5,
"unbalance_v_threshold":25,"unbalance_v_delay":5,
"unbalance_i_min":30,
"antiislanding_enable":2,
"igbt_temp_thres_fit":125,"igbt_temp_hist":10,
"module_temp_thres_fit":85,"module_temp_hist":10,
"internal_temp_thres_fit":85,
"lv_fast_2":45,"lv_delay_fast_2":0.01,
"lv_very_fast_2":35,"lv_delay_very_fast_2":0.01,
"hv_fast_2":125,"hv_delay_fast_2":0.01,
"hv_very_fast_2":135,"hv_delay_very_fast_2":0.01,
"lf_slow_2":59.3,"lf_delay_slow_2":0.01,
"lf_very_fast_2":55,"lf_delay_very_fast_2":0.5,
"hf_slow_2":60.4,"hf_delay_slow_2":0.01,
"hf_very_fast_2":61.5,"hf_delay_very_fast_2":0.5,
"hvpl_enable":0,
"hvpl_activation_level_5":70,"hvpl_scale_1000":1000,"hvpl_fault_level_0":0,
"hvpl_filter_hz_100_hz":0.1,
"hvpl_delay_fault_s_0_s":2,
"hvpl_start_test":0,"hvpl_test_level_20":-20,
"lv_lf_enable":0,"lv_lf_V":0,"lv_lf_f":0,"lv_lf_t":0,
"mv_manometer_fault":0,"mv_manometer_delay":10,
"lv_manometer_fault":0,"lv_manometer_delay":10,
"hv_hf_enable":0,"hv_hf_V":120,"hv_hf_f":62,"hv_hf_t":1,
"lv_hf_enable":0,"lv_hf_V":80,"lv_hf_f":62,"lv_hf_t":1,
"hv_lf_enable":0,"hv_lf_V":120,"hv_lf_f":58,"hv_lf_t":1,
"display_baudrate":5,
"modbus_address":150,
"modbus_baudrate":3,"modbus_parity":0,"modbus_stop_bits":0,
"modbus_master":1,
"automatic_ip":0,
"assigned_ip_a":192,"assigned_ip_b":168,"assigned_ip_c":112,"assigned_ip_d":13,
"assigned_subnet_a":255,"assigned_subnet_b":255,"assigned_subnet_c":255,"assigned_subnet_d":0,
"assigned_gateway_a":0,"assigned_gateway_b":0,"assigned_gateway_c":0,"assigned_gateway_d":1,
"ip_address_a":192,"ip_address_b":168,"ip_address_c":112,"ip_address_d":13,
"subnet_a":255,"subnet_b":255,"subnet_c":255,"subnet_d":0,
"gateway_a":0,"gateway_b":0,"gateway_c":0,"gateway_d":1,
"mac_a":112,"mac_b":179,"mac_c":213,"mac_d":189,"mac_e":176,"mac_f":17,
"ppc_watchdog_ref":33,
"reset_refs_time_s":4,
"comms_fault_time_s_s":60,
"home_p_ref_0_0":0,
"home_q_ref_0_0":0,
"module_1_hw_version":0,"module_1_sw_version":17236997,
"module_2_hw_version":0,"module_2_sw_version":17236997,
"module_3_hw_version":0,"module_3_sw_version":17236997,
"module_4_hw_version":0,"module_4_sw_version":17236997,
"module_5_hw_version":0,"module_5_sw_version":17236997,
"module_6_hw_version":0,"module_6_sw_version":17236997,
}'

echo " setup registers fast"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_fast '
{
"start":0,"stop":0,"reset":0,"e_stop":0,"user_pw":0,"
seconds":10,"minutes":23,"hours":5,"day":30,"month":4,"year":2021,"utc":0,
"access":37358,"user":4,
"p_limit":100,"q_limit":100,"p_q_priority":1,
"s_limit":100,"p_algo_pri":0,
"lvrt_ref":10000,"ovrt_ref":10000,
"night_mode_enable":0,"night_mode_q_ref":0,"night_mode_vbus_ref":1125,"night_mode_activate":0,
"start_grad_p_enable":1,"start_grad_p":633.3,
"start_grad_q_enable":1,"start_grad_q":633.3,
"rise_grad_p_enable":1,"rise_grad_p":633.3,
"rise_grad_q_enable":1,"rise_grad_q":633.3,
"drop_grad_p_enable":1,"drop_grad_p":633.3,
"drop_grad_q_enable":1,"drop_grad_q":633.3,
"stop_grad_p_enable":1,"stop_grad_p":633.3,
"stop_grad_q_enable":1,"stop_grad_q":633.3,
"tracker_enable":0,"tracker_vac_out":480,"tracker_vac_min":80,"tracker_time_start":0,
"svr_enable":0,"svr_v1_threshold":105,"svr_v2_threshold":107,
"svr_v1_hysteresis":2,"svr_v2_hysetresis":2,
"svr_grad_inject":0.3,"svr_grad_recover_q":1.7,
"svr_grad_reduction_p":0.3,"svr_grad_recover_p":1.7,
"svr_cosfi_limit":0.85,
"vp_ref_power":0,"vp_rec_mode":0,"vp_enable":0,
"vp_v1":110,"vp_v2":120,"vp_p1":100,"vp_p2":0,
"vp_hysteresis":0,"vp_delay_hysteresis":60,
"vp_response_tiime":10,"active_bal_activate":0,
"bank_bal_activate":0,"bank_bal_reconnect_grad":100,
"cvsb_activate":0,"cvsb_time_to_enter":1200,
"lv_enable":26,
"lv_slow":90,"lv_delay_slow":0.3,
"lv_fast":20,"lv_delay_fast":1,
"lv_very_fast":0,"lv_delay_very_fast":10,
"lv_interpolation":0,
"hv_enable":24,"hv_slow":110,"hv_delay_slow":1,
"hv_fast":130,"hv_delay_fast":0.5,"hv_very_fast":130,
"hv_delay_very_fast":0.01,
"hv_interpolation":0,
"lf_enable":20,"lf_slow":58.8,"lf_delay_slow":30,
"lf_fast":57,"lf_delay_fast":0.1,
"lf_very_fast":5.6,"lf_delay_very_fast":0.1,
"hf_enable":20,"hf_slow":61.2,"hf_delay_slow":30,
"hf_fast":61.8,"hf_delay_fast":0.1,
"hf_very_fast":61,"hf_delay_very_fast":0.01,
"unbalance_enable":3,
"unbalance_i_threshold":30,"unbalance_i_delay":5,
"unbalance_v_threshold":25,"unbalance_v_delay":5,
"unbalance_i_min":30,
"antiislanding_enable":2,
"igbt_temp_thres_fit":125,"igbt_temp_hist":10,
"module_temp_thres_fit":85,"module_temp_hist":10,
"internal_temp_thres_fit":85,
"earth_current":30,
"high_vdc_delay":30,
"du_overcurrent":400,
"lv_fast_2":45,"lv_delay_fast_2":0.01,
"lv_very_fast_2":35,"lv_delay_very_fast_2":0.01,
"hv_fast_2":125,"hv_delay_fast_2":0.01,
"hv_very_fast_2":135,"hv_delay_very_fast_2":0.01,
"lf_slow_2":59.3,"lf_delay_slow_2":0.01,
"lf_very_fast_2":55,"lf_delay_very_fast_2":0.5,
"hf_slow_2":60.4,"hf_delay_slow_2":0.01,
"hf_very_fast_2":61.5,"hf_delay_very_fast_2":0.5,
"hvpl_enable":0,
"hvpl_activation_level_5":70,"hvpl_scale_1000":1000,"hvpl_fault_level_0":0,
"hvpl_filter_hz_100_hz":0.1,
"hvpl_delay_fault_s_0_s":2,
"hvpl_start_test":0,"hvpl_test_level_20":-20,
"lv_lf_enable":0,"lv_lf_V":0,"lv_lf_f":0,"lv_lf_t":0,
"mv_manometer_fault":0,"mv_manometer_delay":10,
"lv_manometer_fault":0,"lv_manometer_delay":10,
"hv_hf_enable":0,"hv_hf_V":120,"hv_hf_f":62,"hv_hf_t":1,
"lv_hf_enable":0,"lv_hf_V":80,"lv_hf_f":62,"lv_hf_t":1,
"hv_lf_enable":0,"hv_lf_V":120,"hv_lf_f":58,"hv_lf_t":1,
"display_baudrate":5,
"modbus_address":150,
"modbus_baudrate":3,"modbus_parity":0,"modbus_stop_bits":0,
"modbus_master":1,
"automatic_ip":0,
"assigned_ip_a":192,"assigned_ip_b":168,"assigned_ip_c":112,"assigned_ip_d":13,
"assigned_subnet_a":255,"assigned_subnet_b":255,"assigned_subnet_c":255,"assigned_subnet_d":0,
"assigned_gateway_a":0,"assigned_gateway_b":0,"assigned_gateway_c":0,"assigned_gateway_d":1,
"ip_address_a":192,"ip_address_b":168,"ip_address_c":112,"ip_address_d":13,
"subnet_a":255,"subnet_b":255,"subnet_c":255,"subnet_d":0,
"gateway_a":0,"gateway_b":0,"gateway_c":0,"gateway_d":1,
"mac_a":112,"mac_b":179,"mac_c":213,"mac_d":189,"mac_e":176,"mac_f":17,
"ppc_watchdog_ref":33,
"reset_refs_time_s":4,
"comms_fault_time_s_s":60,
"home_p_ref_0_0":0,
"home_q_ref_0_0":0,
"p_control_mode":2,"q_control_mode":1,
"p_p_reference":0,"p_id_reference":100,"q_cosphi_reference":1,"q_iq_reference":0,
"q_q_reference":0,"excitation":0,"q_v_reference":100,
"vq_v1":90,"vq_v2":92,"vq_v3":108,"vq_v4":110,
"vq_q1":48.43,"vq_q2":0,"vq_q3":0,"vq_q4":48.43,
"vq_hysterisis":1,
"vq_p_activation":20,"vq_p_deactivation":5,
"vq_response_delay":0.3,"vq_response_time":5,
"grid_voltage_rs":34300,"grid_voltage_st":34300,"grid_voltage_tr":34300,
"grid_current_1":0,"grid_current_2":0,"grid_current_3":0,
"active_power":0,"reactive_power":0,"apparent_power":0,
"cosphi":0,
"grid_frequency":59.96,
"poi_v_est":343,"vdc_last_linp":0,"dc_p_input":0,"dc_total_i_input":0,
"vdc_bus_1":0,"vdc_bus_2":0,"vdc_bus_3":0,
"bess_1_v":5.3,"bess_2_v":0,"bess_3_v":0,
"bess_1_p_input":0,"bess_2_p_input":0,"bess_3_p_input":0,
"dc_1_i_input":0,"dc_2_i_input":0,"dc_3_i_input":0,
"bess_1_idcref":0,"bess_2_idcref":0,"bess_3_idcref":0,
"current_fault":56,"current_warning":0,
"current_status":9,
"internal_temperature":66,
"max_igbts_temperature":34,
"max_modules_temperature":41,
"internal_input_status":373,
"internal_output_status":4,
"ac_module_admission_temperature":51,"current_normative":0,"current_fault_module":0,
"internal_humidity":6,
"lv_manometer":0,
"power_source_1":25.3,"power_source_2":0,
"p_limit_inst":0,"q_limit_inst":0,
"r_mohm":5,
"l_uh":140,"c_uf":61.5,"lc_uh":16.2,
"start_conditions_all":0,"start_conditions_v":1,"start_conditions_f":1,
"start_conditions_vdc":0,"start_conditions_p":1,"start_conditions_soc_no":1,
"nominal_ac_voltage":660,"max_ac_current":529,
"nominal_bus_voltage":1500,"max_dc_current":670,
"nominal_s":585,
"number_of_modules":6,
"number_of_mpps":1,
"number_of_bus":1,
"inverter_type":3,
"inductive_i_limit":492,"capacitive_i_limit":440,
"max_p_limit":100,"max_q_limit":100,"max_s_limit":100,"running_modules":0,
"num_running_modules":0,
"next_starting_module":3,
"modules_start_stop":0,"modules_enable_reg":0,"selected_module":0,
"module_status":1,"module_current_r":0,"module_current_s":0,"module_current_t":0,
"module_dc_current":0,"module_dc_voltage_p":0,"module_dc_voltage_n":0,
"module_dc_voltage":0,"module_p":0,"module_q":1.4,"module_ambient_temperature":41,
"module_max_temperature":34,"module_i_o_status":192,"module_temperature_r1":33,
"module_temperature_r2":33,"module_temperature_r3":32,"module_temperature_s1":32,
"module_temperature_s2":34,"module_temperature_s3":32,"module_temperature_t1":32,
"module_temperature_t2":34,"module_temperature_t3":33,"module_1_status":1,
"module_1_current_r":0,
"module_1_current_s":0,"module_1_current_t":0,"module_1_dc_current":0,
"module_1_dc_voltage_p":0,"module_1_dc_voltage_n":0,"module_1_dc_voltage":0,"module_1_p":0,
"module_1_q":1.4,"module_1_ambient_temp":41,"module_1_max_temp":34,"module_1_i_o_status":192,
"module_1_hw_version":0,"module_1_sw_version":17236997,
"module_1_temp_r1":33,"module_1_temp_r2":33,"module_1_temp_r3":32,
"module_1_temp_s1":32,"module_1_temp_s2":34,"module_1_temp_s3":32,
"module_1_temp_t1":32,"module_1_temp_t2":34,"module_1_temp_t3":33,
"module_2_status":1,
"module_2_current_r":0,"module_2_current_s":0,"module_2_current_t":0,
"module_2_dc_current":0,"module_2_dc_voltage_p":0,
"module_2_dc_voltage_n":0,"module_2_dc_voltage":0,"module_2_p":0,"module_2_q":1.4,
"module_2_ambient_temp":39,"module_2_max_temp":34,"module_2_i_o_status":192,
"module_2_hw_version":0,"module_2_sw_version":17236997,
"module_2_temp_r1":33,"module_2_temp_r2":34,"module_2_temp_r3":33,
"module_2_temp_s1":34,"module_2_temp_s2":34,"module_2_temp_s3":34,
"module_2_temp_t1":33,"module_2_temp_t2":34,"module_2_temp_t3":33,
"module_3_status":1,
"module_3_current_r":0,"module_3_current_s":0,"module_3_current_t":0,
"module_3_dc_current":0,"module_3_dc_voltage_p":0,"module_3_dc_voltage_n":0,
"module_3_dc_voltage":0,"module_3_p":0,"module_3_q":1.4,"module_3_ambient_temp":38,
"module_3_max_temp":32,"module_3_i_o_status":192,
"module_3_hw_version":0,"module_3_sw_version":17236997,
"module_3_temp_r1":31,"module_3_temp_r2":32,"module_3_temp_r3":30,
"module_3_temp_s1":31,"module_3_temp_s2":31,"module_3_temp_s3":30,
"module_3_temp_t1":31,"module_3_temp_t2":31,"module_3_temp_t3":31,

"module_4_status":1,
"module_4_current_r":0,
"module_4_current_s":0,"module_4_current_t":0,"module_4_dc_current":0,
"module_4_dc_voltage_p":0,"module_4_dc_voltage_n":0,"module_4_dc_voltage":0,
"module_4_p":0,
"module_4_q":1.4,"module_4_ambient_temp":35,"module_4_max_temp":31,
"module_4_i_o_status":192,
"module_4_hw_version":0,"module_4_sw_version":17236997,
"module_4_temp_r1":30,"module_4_temp_r2":31,"module_4_temp_r3":30,
"module_4_temp_s1":30,"module_4_temp_s2":30,"module_4_temp_s3":30,
"module_4_temp_t1":30,"module_4_temp_t2":31,"module_4_temp_t3":29,

"module_5_status":1,
"module_5_current_r":0,"module_5_current_s":0,"module_5_current_t":0,
"module_5_dc_current":0,"module_5_dc_voltage_p":0,
"module_5_dc_voltage_n":0,"module_5_dc_voltage":0,"module_5_p":0,"module_5_q":1.4,
"module_5_ambient_temp":36,"module_5_max_temp":31,
"module_5_i_o_status":192,
"module_5_hw_version":0,"module_5_sw_version":17236997,
"module_5_temp_r1":30,"module_5_temp_r2":31,"module_5_temp_r3":30,
"module_5_temp_s1":30,"module_5_temp_s2":31,"module_5_temp_s3":30,
"module_5_temp_t1":30,"module_5_temp_t2":31,"module_5_temp_t3":30,

"module_6_status":1,
"module_6_current_r":0,"module_6_current_s":0,"module_6_current_t":0,
"module_6_dc_current":0,"module_6_dc_voltage_p":0,"module_6_dc_voltage_n":0,
"module_6_dc_voltage":0,"module_6_p":0,"module_6_q":1.4,"module_6_ambient_temp":33,
"module_6_max_temp":30,
"module_6_i_o_status":192,
"module_6_hw_version":0,"module_6_sw_version":17236997,
"module_6_temp_r1":29,"module_6_temp_r2":30,"module_6_temp_r3":29,
"module_6_temp_s1":29,"module_6_temp_s2":30,
"module_6_temp_s3":30,"module_6_temp_t1":29,
"module_6_temp_t2":30,"module_6_temp_t3":29,
"Timestamp":"04-30-2021 05:22:34.856035"
}'

# map incoming Modbus values from ess_controller to the simulator 
echo " setup actions on registers fast"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_fast '
{
  "grid_current_1":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_current_1"}]}]}
      },
  "grid_current_2":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_current_2"}]}]}
      },
  "grid_current_3":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_current_3"}]}]}
      },
  "grid_voltage_rs":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_voltage_rs"}]}]}
      },
  "grid_voltage_st":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_voltage_st"}]}]}
      },
  "grid_voltage_tr":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_voltage_tr"}]}]}
      },
  "reactive_power":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:reactive_power"}]}]}
      },
  "active_power":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:active_power"}]}]}
      },
  "apparent_power":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:apparent_power"}]}]}
      }
}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/components/pcs_registers_fast '
{
  "vdc_bus_1":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:vdc_bus_1"}]}]}
      },

  "current_status":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:current_status"}]}]}
      },
  "selector_status":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:selector_status"}]}]}
      },
  "selector_config":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:selector_config"}]}]}
      },
  "stop":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:stop"}]}]}
      },
  "e_stop":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:e_stop"}]}]}
      },

  "drop_grad_p_enable":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:drop_grad_p_enable"}]}]}
      },
  "drop_grad_q_enable":{
      "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:drop_grad_q_enable"}]}]}
      },
  "start_grad_p_enable":{
    "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:start_grad_p_enable"}]}]}
    },
  "start_grad_q_enable":{
    "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:start_grad_q_enable"}]}]}
    },
  "stop_grad_p_enable":{
    "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:stop_grad_p_enable"}]}]}
    },
  "stop_grad_q_enable":{
    "value":0,
      "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:stop_grad_q_enable"}]}]}
    }
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/links/pcs '
 {
     "current_fault": { "value":"/components/pcs_registers_fast:current_fault"},
     "current_warning": { "value":"/components/pcs_registers_fast:current_warning"},
     "current_status": { "value":"/components/pcs_registers_fast:current_status"}
 }
'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/configsim/pcs '
 {
     "num_modules": {"value":6}
 }
'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/reload/pcs '
 {
     "SimHandlePcs": {"value":0}
 }
'


exit

/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/schedule/wake_monitor/bms | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/full/schedule/wake_monitor/bms '
 {
   "/components/catl_bms_ems_r:bms_timestamp": {
     "value": false,
     "amap": "bms",
     "enable": true,
     "func": "CheckMonitorVar",
     "rate": 0.1
   }
 }
'
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/schedule/wake_monitor/bms | jq

#exit

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ "addSchedItem":{
    "value":"SimHeartBeat",
    "uri":"/sched/ess:SimHeartBeat", 
    "fcn":"SimHandleHeartbeat","refTime":0.200,"runTime":0.200,"repTime":1.000,"endTime":0
}}
'
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/sched/ess/SimHeartBeat | jq
# {
#   "SimHeartBeat": {
#     "value": 27.367977000016253,
#     "active": true,
#     "enabled": true,
#     "endTime": 0,
#     "fcn": "SimHandleHeartbeat",
#     "refTime": 0.2,
#     "repTime": 1,
#     "runCnt": 332,
#     "runEnd": 0,
#     "runTime": 360.2
#   }
# }
echo " set configsim"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/full/configsim/ess '
{ 
  "SimBmsComms": {
    "value": true
  },
  "SimBmsHB": {
    "value": true
  },
  "SimPcsComms": {
    "value": true
  },
  "SimPcsHB": {
    "value": true
  }
}'

echo " get configsim"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/configsim/ess | jq
# {
#   "HeartbeatMax": {
#     "value": 255
#   },
#   "HeartbeatPeriod": {
#     "value": 1
#   },
#   "SimBmsComms": {
#     "value": false
#   },
#   "SimBmsHB": {
#     "value": false
#   },
#   "SimPcsComms": {
#     "value": false
#   },
#   "SimPcsHB": {
#     "value": false
#   }
# }
sleep 2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/pcs_sim | jq
# {
#   "pcs_heartbeat": {
#     "value": 11
#   },
#   "pcs_timestamp": {
#     "value": "the new time is 16.218826"
#   }
# }
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/bms_sim | jq
# {
#   "bms_heartbeat": {
#     "value": 58
#   },
#   "bms_timestamp": {
#     "value": "the new time is 40.218825"
#   }
# }

echo "map the bms heartbeat and time stamp" 
/usr/local/bin/fims/fims_send -m set -r /$$  -u /$SIM/components/bms_sim '
{
    "bms_heartbeat":{ 
        "value":0,
        "debug":1,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"uri": "/components/catl_bms_ems_r:bms_heartbeat"}
                    ]                
            }]
        }
    },
    "bms_timestamp":{ 
        "value":"some text",
        "debug":1,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"uri": "/components/catl_bms_ems_r:bms_timestamp"}
                    ]                
            }]
        }
    }
}'

echo "inspect bms 1"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/catl_bms_ems_r/bms_heartbeat | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/catl_bms_ems_r/bms_timestamp | jq

sleep 2
echo "inspect bms 2"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/catl_bms_ems_r/bms_heartbeat | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /$SIM/full/components/catl_bms_ems_r/bms_timestamp | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/schedule/ess '
{ 
    "addSchedItem":{
        "value":"SimPcsFastPub",
        "var":"/sched/pcs:simPcsFastPub",
        "debug":1,
        "amap":"pcs",
        "uri":"/sched/pcs:SimPcsFastPub", 
        "fcn":"FastPub",
        "refTime":0.200,
        "runTime":0.200,
        "repTime":1.000,
        "endTime":0
}}
'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /$SIM/full/sched/pcs ' {
 "SimPcsFastPub":{"value": 351.01897800000006,
 "table":"/components/pcs_registers_fast"
}}'

exit

# script to set up and test the BMS Heasrtbeat
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:01"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  1
sleep 1
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  2

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:02"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 3
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  3
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:03"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 4
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  4
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:04"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 5
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:05"'
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  5
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:06"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:07"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 6
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  6
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:08"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 7
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  7
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:09"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 8
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  8
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:10"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 9
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  9
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:11"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 10
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  10

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:12"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 11
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  11
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12
echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/full/components/pcs_registers_fast/seconds | jq

echo "# trigger alarm"
sleep 3   
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:14"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12

echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/full/components/pcs_registers_fast/seconds | jq


echo " # trigger fault"
sleep 7
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:14"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12
echo " Check Stuff"
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/components/catl_bms_ems_r | jq
/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/full/components/pcs_registers_fast/seconds | jq

echo "# recover"
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:01"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  1
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:02"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  2
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:03"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 3
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  3
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:04"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 4
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  4
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:05"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 5
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  5
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:06"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 6
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  6
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:07"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 7
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  7

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:08"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 8
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  8
sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:09"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 9
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  9

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:10"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 10
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  10

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:11"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 11
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  11

/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:12"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 12
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  12

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 1
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  55

/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/full/components/pcs_registers_fast/seconds | jq

sleep 1
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_timestamp '"10/23/2020 21:13"'
/usr/local/bin/fims/fims_send -m pub  -u /components/catl_bms_ems_r/bms_heartbeat 2
/usr/local/bin/fims/fims_send -m pub  -u /components/pcs_registers_fast/seconds  56

/usr/local/bin/fims/fims_send -m get -r/$$ -u /$SIM/full/components/pcs_registers_fast/seconds | jq
