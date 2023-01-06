# TODO break this down into a proper fims command for each sbmu

# Method:  pub
# Uri:     /components/catl_sbmu_9
# ReplyTo: (null)
# Body:    {"sbmu_warning_1":0,"sbmu_warning_21":0,"sbmu_warning_22":0,"sbmu_warnin
# g_23":4096,"sbmu_precharge_status":0,"sbmu_master_positive":0,"sbmu_master_negiti
# ve":0,"sbmu_balance_status":0,"sbmu_voltage":3.8,"sbmu_current":2000,"sbmu_soc":4
# 5,"sbmu_soh":100,"sbmu_max_cell_voltage":3.298,"sbmu_min_cell_voltage":3.295,"sbm
# u_avg_cell_voltage":3.295,"sbmu_max_cell_temp":70,"sbmu_min_cell_temp":69,"sbmu_a
# vg_cell_temp":69,"sbmu_max_charge_current":1749.6,"sbmu_max_discharge_current":22
# 72.1,"sbmu_max_cell_voltage_position":5,"sbmu_min_cell_voltage_position":1,"sbmu_
# max_cell_temp_position":27,"sbmu_min_cell_temp_position":2,"sbmu_sum_cells":13709
# ,"sbmu_tms_mode_command":0,"sbmu_tms_temp_setting":60,"sbmu_tms_real_mode":60,"sb
# mu_ambient_temp":60,"sbmu_tms_demand_power":0,"sbmu_tms_fault_code":0,"sbmu_door_
# state":0,"sbmu_fan_in_box":0,"Timestamp":"04-30-2021 05:22:34.769024"}
# Timestamp:   2021-04-30 05:22:34.769707
# Method:  pub
# Uri:     /components/catl_mbmu_summary_r
# ReplyTo: (null)
# Body:    {"mbmu_voltage":0,"mbmu_current":20000,"mbmu_soc":102.3,"mbmu_soh":100,"
# mbmu_max_cell_voltage":3.298,"mbmu_min_cell_voltage":3.293,"mbmu_avg_cell_voltage
# ":3.293,"mbmu_max_cell_temperature":70,"mbmu_min_cell_temperature":69,"mbmu_avg_c
# ell_temperature":69,"mbmu_max_charge_current":20000,"mbmu_max_discharge_current":
# 20000,"Timestamp":"04-30-2021 05:22:34.779004"}
# Timestamp:   2021-04-30 05:22:34.779441
# Method:  pub
# Uri:     /components/catl_mbmu_stat_r
# ReplyTo: (null)
# Body:    {"mbmu_status":4,"Timestamp":"04-30-2021 05:22:34.789011"}
# Timestamp:   2021-04-30 05:22:34.789441
# 
# TODO add the rest of the variables
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_sbmu_9 '
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
    "sbmu_fan_in_box":0,
    "Timestamp":"04-30-2021 05:22:34.769024"
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_stat_r '
{ 
    "mbmu_status":4,
    "Timestamp":"04-30-2021 05:22:34.789011"
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_summary_r '
{ 
    "mbmu_voltage":0,
    "mbmu_current":20000,
    "mbmu_soc":102.3,
    "mbmu_soh":100,
    "mbmu_max_cell_voltage":3.298,
    "mbmu_min_cell_voltage":3.293,
    "mbmu_avg_cell_voltage":3.293,
    "mbmu_max_cell_temperature":70,
    "mbmu_min_cell_temperature":69,
    "mbmu_avg_cell_temperature":69,
    "mbmu_max_charge_current":20000,
    "mbmu_max_discharge_current":20000,
    "Timestamp":"04-30-2021 05:22:34.779004"
}'

# TODO for each of these too
# Uri:     /components/catl_bms_ems_r
# ReplyTo: (null)
# Body:    {"bms_heartbeat":155,"bms_poweron":0,"bms_status":4,"num_hv_subsystem":0
# ,"bms_remain_charge_energy":0,"bms_remain_discharge_energy":0,"bms_max_discharge_
# allowed":20000,"bms_max_charge_allowed":20000,"bms_limit_charge_hv":15808,"bms_li
# mit_discharge_hv":10816,"Timestamp":"04-30-2021 05:22:34.949159"}
# Timestamp:   2021-04-30 05:22:34.949598
# Method:  pub

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_bms_ems_r '
{
    "bms_heartbeat":155,
    "bms_poweron":0,
    "bms_status":4,
    "num_hv_subsystem":0,
    "bms_remain_charge_energy":0,
    "bms_remain_discharge_energy":0,
    "bms_max_discharge_allowed":20000,
    "bms_max_charge_allowed":20000,
    "bms_limit_charge_hv":15808,
    "bms_limit_dischage_hv":10816,
    "Timestamp":"04-30-2021 05:22:34.949159"
}'

# Method:  pub
# Uri:     /components/catl_mbmu_stat_r
# ReplyTo: (null)
# Body:    {"mbmu_status":4,"Timestamp":"04-30-2021 05:22:34.854050"}
# Timestamp:   2021-04-30 05:22:34.854491

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_stat_r '
{
    "mbmu_status":4,
    "Timestamp": "04-30-2021 05:22:34.854050"
}'

# Method:  pub
# Uri:     /components/catl_mbmu_sum_r
# ReplyTo: (null)
# Body:    {"mbmu_warning_1":7168,"mbmu_warning_21":0,"mbmu_warning_22":0,"Timestam
# p":"04-30-2021 05:22:34.798938"}
# Timestamp:   2021-04-30 05:22:34.799389
# Method:  pub
# Uri:     /components/catl_ems_bms_rw
# ReplyTo: (null)
# Body:    {"ems_heartbeat":200,"ems_cmd":[{"value":1,"string":"Stay Status"}],"ems
# _status":0,"ems_rtc_year":2021,"ems_rtc_month":4,"ems_rtc_day":30,"ems_rtc_hour":
# 1,"ems_rtc_minute":1,"ems_rtc_second":0,"fault_clear_cmd":0,"Timestamp":"04-30-20
# 21 05:22:34.809068"}
# Timestamp:   2021-04-30 05:22:34.809522

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_mbmu_sum_r '
{
    "mbmu_warning_1":7168,
    "mbmu_warning_21":21,
    "mbmu_warning_22":0,
    "Timestamp":"04-30-2021 05:22:34.798938"
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/catl_ems_bms_rw '
{
    "ems_heartbeat":200,
    "ems_cmd":[{
        "value":1, "string":"Stay Status"
    }],
    "ems_status":0,
    "ems_rtc_year":2021,
    "ems_rtc_month":4,
    "emc_rtc_day":30,
    "ems_rtc_hour":1,
    "ems_rtc_minute":1, 
    "ems_rtc_second":0,
    "fault_clear_cmd":0,
    "Timestamp":"04-30-3021 05:22:34.809068"
}'

#TODO remap the incoming data into the simulator 
# echo " setup registers fast"
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/pcs_registers_fast '
# {
#   "grid_current_1":{
#       "value":0,
#       "actions": {"onSet": [{"remap": [{"uri": "/configsim/pcs:grid_current_1"}]}]}
#       },
# }'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/links/bms '
{
    "current_fault": {"value":"/components"}
}'

# TODO set up the links
# note thses are copied from the pcs links so rework them  for the bms
# if an incoming register is used in the simulator it needs to be linked here
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/links/bms '
#  {
#      "current_fault": { "value":"/components/pcs_registers_fast:current_fault"},
#      "current_warning": { "value":"/components/pcs_registers_fast:current_warning"},
#      "current_status": { "value":"/components/pcs_registers_fast:current_status"}
#  }
# '
# set up the number of sbmu's
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/configsim/bms '
#  {
#      "num_modules": {"value":9}
#  }
# '
# this forces the bms to rework its links
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/reload/bms '
#  {
#      "SimHandleBms": {"value":0}
#  }
# '

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/ess '
{ 
    "addSchedItem":{
        "value":"SimBms",
        "debug":1,
        "amap":"bms",
        "uri":"/sched/bms:SimBms", 
        "fcn":"SimHandleBms","refTime":0.200,"runTime":0.200,"repTime":1.000,"endTime":0
}}
'
