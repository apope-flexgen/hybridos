#!/usr/bin/sh

#Typical input from a BMS interface unit

#The BMS is required to periodically scan all the battery cells and 
#determine the max / min avg cell voltages.
#Batteries are arranged in clusters of cells
#In this example there are 20 clusters each with approx 1000 voltages 

#The system also has states, alarms and faults.
#/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":1234}}'  -r /me

BMS_TEST_APX1="{
    \"ems_test_appendixI\":{
        \"system_warning_summary\":{
            \"bit_field\": true,
            \"bit_strings\":[
                \"single_cell_over_voltage_normal\",
                \"single_cell_over_voltage_warning_1\",
                \"single_cell_over_voltage_warning_2\",
                \"single_cell_over_voltage_warning_3\",
                \"single_cell_under_voltage_normal\",
                \"single_cell_under_voltage_warning_1\",
                \"single_cell_under_voltage_warning_2\",
                \"single_cell_under_voltage_warning_3\",
                \"single_cell_over_temp_normal\",
                \"single_cell_over_temp_warning_1\",
                \"single_cell_over_temp_warning_2\",
                \"single_cell_over_temp_warning_3\",
                \"single_cell_under_temp_normal\",
                \"single_cell_under_temp_warning_1\",
                \"single_cell_under_temp_warning_2\",
                \"single_cell_under_temp_warning_3\",
                \"soc_low_normal\",
                \"soc_low_warning_1\",
                \"soc_low_warning_2\",
                \"soc_low_warning_3\"
            ]
        }
    }
}"

BMS_TEST_APX2="{
    \"ems_test_appendixII\":{
        \"system_warning_summary\":{
            \"bit_field\": true,
            \"bit_strings\":[
                \"current_over_limit_warning\",
                \"single_cell_over_voltage_warning\",
                \"single_cell_over_temp_warning\",
                \"system_overvoltge_warning\",
                \"system_undervoltge_warning\",
                \"inner_communication_warning\",
                \"cell_extreme_temp_warning\",
                \"cell_extreme_voltage_warning\",
                \"ems_heartbeat_fault\",
                \"reserve_apxII_09\",
                \"reserve_apxII_0A\",
                \"reserve_apxII_0B\",
                \"reserve_apxII_0C\",
                \"reserve_apxII_0D\",
                \"reserve_apxII_0E\",
                \"reserve_apxII_0F\",
                \"reserve_apxII_10\",
                \"reserve_apxII_11\",
                \"reserve_apxII_12\",
                \"reserve_apxII_13\",
                \"reserve_apxII_14\",
                \"reserve_apxII_15\",
                \"reserve_apxII_16\",
                \"reserve_apxII_17\",
                \"reserve_apxII_18\",
                \"reserve_apxII_19\",
                \"soc_difference_warning\",
                \"fire_fault_2_warning\",
                \"reserve_apxII_1C\",
                \"reserve_apxII_1D\",
                \"reserve_apxII_1E\",
                \"reserve_apxII_1F\"
            ]
        }
    }
}"
BMS_TEST_APX3="{
    \"ems_test_appendixIII\":{
        \"system_warning_summary\":{
            \"bit_field\": true,
            \"bit_strings\":[
                \"single_cell_over_voltage_normal\",
                \"single_cell_over_voltage_warning_1\",
                \"single_cell_over_voltage_warning_2\",
                \"single_cell_over_voltage_warning_3\",
                \"single_cell_under_voltage_normal\",
                \"single_cell_under_voltage_warning_1\",
                \"single_cell_under_voltage_warning_2\",
                \"single_cell_under_voltage_warning_3\",
                \"single_cell_over_temp_normal\",
                \"single_cell_over_temp_warning_1\",
                \"single_cell_over_temp_warning_2\",
                \"single_cell_over_temp_warning_3\",
                \"single_cell_under_temp_normal\",
                \"single_cell_under_temp_warning_1\",
                \"single_cell_under_temp_warning_2\",
                \"single_cell_under_temp_warning_3\",
                \"soc_low_normal\",
                \"soc_low_warning_1\",
                \"soc_low_warning_2\",
                \"soc_low_warning_3\",
                \"TMS_fault_normal\",
                \"TMS_fault_level_1\",
                \"TMS_fault_level_2\",
                \"TMS_fault_level_3\"
            ]
        }
    }
}"
BMS_TEST_APX4="{
    \"ems_test_appendixIV\":{
        \"system_warning_summary\":{
            \"bit_field\": true,
            \"bit_strings\":[
                \"charge_over_current_warning\",
                \"discharge_over_current_warning\",
                \"reserve_apxIV_02\",
                \"temp_diff_warning\",
                \"cell_extreme_temp_fault\",
                \"cell_extreme_voltage_fault\",
                \"extreme_total_voltage_fault\",
                \"main_positive_relay_close_fail\",
                \"main_negative_relay_close_fail\",
                \"power_loss_fault\",
                \"balance_function_fault\",
                \"reserve_apxIV_0D\",
                \"reserve_apxIV_0E\",
                \"inner_ccan_communication_fault\",
                \"inner_scan_communication_fault\"
            ]
        }
    }
}"
BMS_INPUT="{
    \"ems_input\":{
    \"system_warning_summary\":{\"value\":1234},
    \"system_warning_1\":{\"value\":1234},
    \"system_warning_2\":{\"value\":1234},
    \"system_warning_3\":{\"value\":1234},
    \"system_warning_1\":{\"value\":1234},
    \"system_voltage\":{\"value\":1234},
    \"system_current\":{\"value\":1234},
    \"soc\":{\"value\":1234},
    \"soh\":{\"value\":1234},
    \"max_cell_voltge\":{\"value\":1234},
    \"min_cell_voltge\":{\"value\":1234},
    \"avg_cell_voltge\":{\"value\":1234},
    \"max_cell_temperature\":{\"value\":1234},
    \"min_cell_temperature\":{\"value\":1234},
    \"avg_cell_temperature\":{\"value\":1234},
    \"max_charge_current_allowed\":{\"value\":1234},
    \"max_discharge_current_allowed\":{\"value\":1234},
    \"BMS_heartbeat\":{\"value\":1234},
    \"BMS_poweron\":{\"value\":1234},
    \"BMS_status\":{\"value\":1234},
    \"number_HV_connectd\":{\"value\":1234},
    \"system_remain_charge_energy\":{\"value\":1234},
    \"system_remain_discharge_energy\":{\"value\":1234},
    \"max_system_discharge_power_allowed\":{\"value\":1234},
    \"max_system_charge_power_allowed\":{\"value\":1234},
    \"BMS_limit_charge_hv\":{\"value\":1234},
    \"BMS_limit_discharge_hv\":{\"value\":1234}
    }}"

EMS_OUTPUT="{
    \"ems_output\":{
    \"EMS_heartbeat\":{\"value\":1234},
    \"EMS_cmd\":{\"value\":1234},
    \"EMS_status\":{\"value\":1234},
    \"EMS_RTC_year\":{\"value\":1234},
    \"EMS_RTC_month\":{\"value\":1234},
    \"EMS_RTC_day\":{\"value\":1234},
    \"EMS_RTC_hour\":{\"value\":1234},
    \"EMS_RTC_second\":{\"value\":1234},
    \"fault_clear_command\":{\"value\":1234}
    }}"
SBMU_WARN="{
    \"sbmu_warn\":{
    \"system_warning_summary\":{\"value\":1234},
    \"system_warning_1\":{\"value\":1234},
    \"system_warning_2\":{\"value\":1234}
    }}"
SBMU_STATUS="{
    \"sbmu_status\":{
    \"precharge_relay_status\":{\"value\":1234},
    \"master_positive_relay_status\":{\"value\":1234},
    \"master_negative_relay_status\":{\"value\":1234},
    \"balance_status\":{\"value\":1234}
    }}"
SBMU_SUMMARY="{
    \"sbmu_summary\":{
    \"battery_subsystem_voltage\":{\"value\":1234},
    \"battery_subsystem_current\":{\"value\":1234},
    \"SOC\":{\"value\":1234},
    \"SOH\":{\"value\":1234},
    \"max_single_cell_voltage\":{\"value\":1234},
    \"min_single_cell_voltage\":{\"value\":1234},
    \"avg_single_cell_voltage\":{\"value\":1234},
    \"max_single_cell_temp\":{\"value\":1234},
    \"min_single_cell_temp\":{\"value\":1234},
    \"avg_single_cell_temp\":{\"value\":1234},
    \"max_allowed_charge_current\":{\"value\":1234},
    \"max_allowed_discharge_current\":{\"value\":1234},
    \"max_single_cell_voltage_position\":{\"value\":1234},
    \"min_single_cell_voltage_position\":{\"value\":1234},
    \"max_single_cell_temp_position\":{\"value\":1234},
    \"min_single_cell_temp_position\":{\"value\":1234},
    \"sum_of_cell_voltage\":{\"value\":1234},
    \"TMS_mode_command_by_BMS\":{\"value\":1234},
    \"TMS_temp_set_by_BMS\":{\"value\":1234},
    \"TMS_real_mode\":{\"value\":1234},
    \"rack_inlet_temperature\":{\"value\":1234},
    \"rack_outlet_temperature\":{\"value\":1234},
    \"environment_temperature\":{\"value\":1234},
    \"TMS_demand_power\":{\"value\":1234},
    \"TMS_fault_code\":{\"value\":1234},
    \"door_state\":{\"value\":1234},
    \"control_box_fan_state\":{\"value\":1234}
    }}"
# SBMS_<cell>_<CSC>
SBMS_DETAIL="{
    \"sbms_detail\":{
    \"single_cell_voltage_01_01\":{\"value\":1234},
    \"single_cell_temp_01_01\":{\"value\":1234},
    \"balance_information_01_01\":{\"value\":1234}
   }}"


echo $FOO | jq

#/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_BMS_INPUT="{
echo $EMS_OUTPUT | jq
echo $BMS_INPUT | jq
echo $SBMU_WARN | jq
echo $SBMU_STATUS | jq
echo $SBMU_SUMMARY| jq
echo $SBMS_DETAIL | jq
echo $BMS_TEST_APX1 | jq
echo $BMS_TEST_APX2 | jq
echo $BMS_TEST_APX3 | jq
echo $BMS_TEST_APX4 | jq

    