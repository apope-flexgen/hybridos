#!/bin/sh
#set up default data for test_dnp3 connection 

fims_send -m pub -u /site/operation '
{
    "test_int16": 123,
    "test_signed_int16": -456,
    "test_unsigned_int16":40000,
    "test_int32":32111,
    "test_signed_int32":-33111,
    "test_unsigned_int32": 12345,
    "test_Float32": 1.2345
}'

fims_send -m pub -u /features/active_power '
{
    "fr_baseload_kW_cmd":3456,
    "fr_inactive_kW_cmd":4567,
    "fr_UF_active_kW_cmd":6789,
    "fr_UF_slew_rate":10,
    "fr_UF_hz_trigger_percent":4.5,
    "fr_UF_hz_recover_percent":6.7,
    "fr_UF_hz_instant_recover_percent":8.9,
    "fr_UF_trigger_time":3.0,
    "fr_UF_recover_time":2.5,
    "fr_UF_cooldown_time":4.6,
    "fr_UF_droop_percent":4.5,
    "fr_OF_active_kW_cmd":4567,
    "fr_OF_slew_rate":11,
    "fr_OF_hz_trigger_percent":4.6,
    "fr_OF_hz_recover_percent":6.8,
    "fr_OF_hz_instant_recover_percent":8.8,
    "fr_OF_trigger_time":3.1,
    "fr_OF_recover_time":2.6,
    "fr_OF_cooldown_time":4.7,
    "fr_OF_droop_percent":4.6,
    "ess_total_active_power":4567,
    "ess_total_reactive_power":4532,
    "ess_total_apparent_power":3456
}'
fims_send -m pub -u /components/sel_735 '
{
    "active_power":20000,
    "reactive_power":3000,
    "apparent_power":23000,
    "frequency":60.12
}'
fims_send -m pub -u /components/sel_651r '
{

    "grid_voltage_l1_l2":480.23,
    "grid_voltage_l2_l3":480.12,
    "grid_voltage_l3_l1":479.2
}'
fims_send -m pub -u /assets/ess/summary '
{
    "ess_average_soc":67.8,
    "ess_chargeable_energy":5678,
    "ess_dischargeable_energy":2345,
    "ess_chargeable_power":4456,
    "ess_dischargeable_power":3356,
    "ess_total_active_power":4456,
    "ess_total_apparent_power":4456,
    "ess_total_reactive_power":4456,
    "available_ess_num" : 5,
    "running_ess_num":4
 
}'
fims_send -m pub -u /metrics/ess '
{
    "ess_chargeable_energy":5678,
    "ess_dischargeable_energy":2345,
    "ess_chargeable_power":4456,
    "ess_dischargeable_power":3356,
    "available_ess_num" : 5,
    "running_ess_num":4
}' 
fims_send -m pub -u /site/operation '
{
    "heartbeat_counter":3
}'

fims_send -m pub -u /assets/ess/ess_1 '
{
    "status":3,
    "soc":56.8,
    "racks":11
}'

fims_send -m pub -u /assets/ess/ess_2 '
{
    "status":3,
    "soc":56.8,
    "racks":11
}'

fims_send -m pub -u /assets/ess/ess_3 '
{
    "status":3,
    "soc":56.8,
    "racks":11
}'
fims_send -m pub -u /assets/ess/ess_4 '
{
    "status":3,
    "soc":56.8,
    "racks":11
}'
fims_send -m pub -u /components/sel_651r '
{
    "breaker_status":0,
    "running_status_flag":1
}'
fims_send -m pub -u /site/operation '
{
    "alarm_status_flag":0,
    "fault_status_flag":0
}'
fims_send -m pub -u /features/active_power '
{

    "fr_UF_status_flag":1,
    "fr_UF_cooldown_status":1,
    "fr_OF_status_flag":0,
    "fr_OF_cooldown_status":1
}'

fims_send -m pub -u /assets/ess/ess_1 '
{
    "faults":0,
    "alarms":0
}'

fims_send -m pub -u /assets/ess/ess_2 '
{
    "faults":0,
    "alarms":0
}'

fims_send -m pub -u /assets/ess/ess_3 '
{
    "faults":0,
    "alarms":0
}'
fims_send -m pub -u /assets/ess/ess_4 '
{
    "faults":0,
    "alarms":0
}'


fims_send -m pub -u /site/operation '
{

    "running_status_flag":1,
    "running_status_event":1,
    "remote_enable_flag":0,
    "remote_disable_flag":0,
    "remote_clear_faults":0
}'

fims_send -m pub -u /features/active_power '
{

    "fr_UF_enable_flag":1,
    "fr_UF_slew_override_flag":0,
    "fr_UF_droop_bypass_flag":0,
    "fr_UF_droop_limit_flag":1,
    "fr_OF_enable_flag":1,
    "fr_OF_slew_override_flag":0,
    "fr_OF_droop_bypass_flag":0,
    "fr_OF_droop_limit_flag":1
}'
fims_send -m pub -u /components/sel_735 '
{
    "active_power":20100,
    "reactive_power":3100,
    "apparent_power":23000,
    "frequency":60.12
}'

sleep 0.5
fims_send -m pub -u /site/operation '
{

    "running_status_flag":1,
    "running_status_event":0,
    "remote_enable_flag":0,
    "remote_disable_flag":0,
    "remote_clear_faults":0
}'
fims_send -m pub -u /components/sel_735 '
{
    "active_power":20200,
    "reactive_power":3200,
    "apparent_power":23000,
    "frequency":60.12
}'

sleep 0.5
fims_send -m pub -u /site/operation '
{

    "running_status_flag":1,
    "running_status_event":1,
    "remote_enable_flag":0,
    "remote_disable_flag":0,
    "remote_clear_faults":0
}'
fims_send -m pub -u /components/sel_735 '
{
    "active_power":20400,
    "reactive_power":3400,
    "apparent_power":23000,
    "frequency":60.12
}'

