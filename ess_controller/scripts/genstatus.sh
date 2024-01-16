#!/bin/sh
# script to generate the sbmu status arrays
total_arrays=6
total_racks=8
total_bats=12
offset=1152
first_addr=1024
hex_offset=0x400
sbmu_num=9
i=0
j=$total_racks
k=$total_bats
echo "{"
for (( i=1; i<=$(( $sbmu_num  )); i++ ))
do 
base_addr=$(($first_addr * ${i}))
base_addr=$(($base_addr + 0))
echo -n "{\"id\":\"catl_sbmu_${i}_warning_r\","
echo -n "\"frequency\":2000,"
echo -n "\"offset_time\":$((${i}+1))00,"
echo -n "\"registers\":[ {"
echo -n "\"type\":\"Holding Registers\","
echo -n "\"starting_offset\":${base_addr},"
echo -n "\"hex_offset\":\"${hex_offset}\","
#echo -n "\"number_of_registers\":$(( $total_racks * $total_bats)),"
echo -n "\"number_of_registers\": 5,"
echo -n "\"map\":[ "
echo -n "{\"id\": \"sbmu_warning_1\",\"offset\":       $(( $base_addr + 0)),\"name\": \"Warning 1\"},"
echo -n "{\"id\": \"sbmu_warning_21\",\"offset\":      $(( $base_addr + 1)),\"name\": \"Warning 2.1\"},"
echo -n "{\"id\": \"sbmu_warning_22\",\"offset\":      $(( $base_addr + 2)),\"name\": \"Warning 2.2\"}"

echo -n "]}]}"
#if [ $i -lt $(( $sbmu_num )) ] ; then
echo -n ","
#fi
echo
done
#echo "},"

hex_offset=0x410
#echo "{"
for (( i=1; i<=$(( $sbmu_num  )); i++ ))
do 
base_addr=$(($first_addr * ${i}))
base_addr=$(($base_addr + 16))
echo -n "{\"id\":\"catl_sbmu_${i}_status_r\","
echo -n "\"frequency\":2000,"
echo -n "\"offset_time\":$((${i}+1))00,"
echo -n "\"registers\":[ {"
echo -n "\"type\":\"Holding Registers\","
echo -n "\"starting_offset\":${base_addr},"
echo -n "\"hex_offset\":\"${hex_offset}\","
#echo -n "\"number_of_registers\":$(( $total_racks * $total_bats)),"
echo -n "\"number_of_registers\": 5,"
echo -n "\"map\":[ "
echo -n "{\"id\": \"sbmu_precharge_status\",\"offset\":  $(( $base_addr + 0)),\"name\": \"Precharge Status\"},"
echo -n "{\"id\": \"sbmu_master_positive\",\"offset\":   $(( $base_addr + 1)),\"name\": \"Master positive\"},"
echo -n "{\"id\": \"sbmu_master_negitive\",\"offset\":   $(( $base_addr + 2)),\"name\": \"Master negtive\"},"
echo -n "{\"id\": \"sbmu_balance_status\",\"offset\":    $(( $base_addr + 4)),\"name\": \"Balance status\"}"

echo -n "]}]}"
#if [ $i -lt $(( $sbmu_num )) ] ; then
echo -n ","
#fi
echo
done
#echo "},"

##// i056
first_addr=1024
hex_offset=0x420
#echo "{"
for (( i=1; i<=$(( $sbmu_num  )); i++ ))
do 
base_addr=$(($first_addr * ${i} + 32))
echo -n "{\"id\":\"catl_sbmu_${i}_summary_r\","
echo -n "\"frequency\":2000,"
echo -n "\"offset_time\":$((${i}+1))00,"
echo -n "\"registers\":[ {"
echo -n "\"type\":\"Holding Registers\","
echo -n "\"starting_offset\":${base_addr},"
echo -n "\"hex_offset\":\"${hex_offset}\","
#echo -n "\"number_of_registers\":$(( $total_racks * $total_bats)),"
echo -n "\"number_of_registers\": 50,"
echo -n "\"map\":[ "
echo -n "{\"id\": \"sbmu_voltage\",                    \"offset\": $(( $base_addr + 0  )),\"name\": \"Battery Subsystem Voltage\"},"
echo -n "{\"id\": \"sbmu_current\",                    \"offset\": $(( $base_addr + 2  )),\"name\": \"Battery Subsystem Current\"},"
echo -n "{\"id\": \"sbmu_soc\",                        \"offset\": $(( $base_addr + 3  )),\"name\": \"Battery Subsystem SOC\"},"
echo -n "{\"id\": \"sbmu_soh\",                        \"offset\": $(( $base_addr + 4  )),\"name\": \"Battery Subsystem SOH\"},"
echo -n "{\"id\": \"sbmu_max_cell_voltage\",           \"offset\": $(( $base_addr + 5  )),\"name\": \"Battery Subsystem Max Cell Voltage\"},"
echo -n "{\"id\": \"sbmu_min_cell_voltage\",           \"offset\": $(( $base_addr + 6  )),\"name\": \"Battery Subsystem Min Cell Voltage\"},"
echo -n "{\"id\": \"sbmu_avg_cell_voltage\",           \"offset\": $(( $base_addr + 7  )),\"name\": \"Battery Subsystem Avg Cell Voltage\"},"
echo -n "{\"id\": \"sbmu_max_cell_temp\",              \"offset\": $(( $base_addr + 8  )),\"name\": \"Battery Subsystem Max Cell Temp\"},"
echo -n "{\"id\": \"sbmu_min_cell_temp\",              \"offset\": $(( $base_addr + 9  )),\"name\": \"Battery Subsystem Min Cell Temp\"},"
echo -n "{\"id\": \"sbmu_avg_cell_temp\",              \"offset\": $(( $base_addr + 10 )),\"name\": \"Battery Subsystem Avg Cell Temp\"},"
echo -n "{\"id\": \"sbmu_max_charge_current\",         \"offset\": $(( $base_addr + 11 )),\"name\": \"Battery Subsystem Max Charge\"},"
echo -n "{\"id\": \"sbmu_max_discharge_current\",      \"offset\": $(( $base_addr + 12 )),\"name\": \"Battery Subsystem Max Discharge\"},"
echo -n "{\"id\": \"sbmu_max_cell_voltage_position\",  \"offset\": $(( $base_addr + 16 )),\"name\": \"Battery Subsystem Max Cell Voltage Positions\"},"
echo -n "{\"id\": \"sbmu_min_cell_voltage_position\",  \"offset\": $(( $base_addr + 17 )),\"name\": \"Battery Subsystem Min Cell Voltage Positions\"},"
echo -n "{\"id\": \"sbmu_max_cell_temp_position\",     \"offset\": $(( $base_addr + 18 )),\"name\": \"Battery Subsystem Max Cell Temp Positions\"},"
echo -n "{\"id\": \"sbmu_min_cell_temp_position\",     \"offset\": $(( $base_addr + 19 )),\"name\": \"Battery Subsystem Min Cell Temp Positions\"},"
echo -n "{\"id\": \"sbmu_sum_cells\",                  \"offset\": $(( $base_addr + 23 )),\"name\": \"Battery Subsystem Sum Cell\"},"
echo -n "{\"id\": \"sbmu_tms_mode_command\",           \"offset\": $(( $base_addr + 42 )),\"name\": \"Battery Subsystem TMS Mode Cmd\"},"
echo -n "{\"id\": \"sbmu_tms_temp_setting\",           \"offset\": $(( $base_addr + 43 )),\"name\": \"Battery Subsystem TMS Temp Setting\"},"
echo -n "{\"id\": \"sbmu_tms_real_mode\",              \"offset\": $(( $base_addr + 44 )),\"name\": \"Battery Subsystem TMS Real Mode\"},"
echo -n "{\"id\": \"sbmu_ambient_temp\",               \"offset\": $(( $base_addr + 45 )),\"name\": \"Battery Subsystem Ambient Temp\"},"
echo -n "{\"id\": \"sbmu_tms_demand_power\",           \"offset\": $(( $base_addr + 46 )),\"name\": \"Battery Subsystem TMS Demand Power\"},"
echo -n "{\"id\": \"sbmu_tms_fault_code\",             \"offset\": $(( $base_addr + 47 )),\"name\": \"Battery Subsystem TMS Fault Code\"},"
echo -n "{\"id\": \"sbmu_door_state\",                 \"offset\": $(( $base_addr + 48 )),\"name\": \"Battery Subsystem Door State\"},"
echo -n "{\"id\": \"sbmu_fan_in_box\",                 \"offset\": $(( $base_addr + 49 )),\"name\": \"Battery Subsystem Fan In Box\"}"

echo -n "]}]}"
if [ $i -lt $(( $sbmu_num )) ] ; then
echo -n ","
fi
echo
done
echo "}"
#               "id": "sbmu1_sum_cells",
#               "id": "sbmu1_tms_mode_command",
#               "id": "sbmu1_tms_temp_setting",
#               "id": "sbmu1_tms_real_mode",
#               "id": "sbmu1_rack_inlet_temperature",
#               "id": "sbmu1_rack_outlet_temperature",
#               "id": "sbmu1_ambient_temp",
#               "id": "sbmu1_tms_demand_power",
#               "id": "sbmu1_tms_fault_code",
#               "id": "sbmu1_door_state",
#               "id": "sbmu1_fan_in_box",



# for (( j=0; j<=$(( $total_racks -1 )); j++ ))
# do 
# for (( k=0; k<=$(( $total_bats -1 )); k++ ))
# do 
#     echo -n "  {"
#     echo -n "   \"id\": \"array_${i}_rack_${j}_cell_${k}_volts\",  "
#     echo  -n "    \"offset\": $offset  "
#     echo -n "  }"
#     if [ $j -eq $(( $total_racks -1)) ] ; then
#       if [ $k -eq $(( $total_bats -1)) ] ; then
#         echo  -n ""
#       else
#         echo -n ","
#       fi
#     else
#       echo -n ","
#     fi
#     #echo
#     offset=`expr $offset + 1`
# done
# done


# "id": "catl_sbmu_warn_r",
#               "id": "sbmu1_system_warning_1",
#               "id": "sbmu1_system_warning_2",
#               "id": "sbmu1_system_warning_3",
#       "id": "catl_sbmu_status_r",
#               "id": "sbmu1_precharge",
#               "id": "sbmu1_master_positive",
#               "id": "sbmu1_master_negative",
#               "id": "sbmu1_balance_status",
#       "id": "catl_sbmu_summary_r",
#               "id": "sbmu1_voltage",
#               "id": "sbmu1_current",
#               "id": "sbmu1_soc",
#               "id": "sbmu1_soh",
#               "id": "sbmu1_max_cell_voltage",
#               "id": "sbmu1_min_cell_voltage",
#               "id": "sbmu1_avg_cell_voltage",
#               "id": "sbmu1_max_cell_temp",
#               "id": "sbmu1_min_cell_temp",
#               "id": "sbmu1_avg_cell_temp",
#               "id": "sbmu1_max_charge_current",
#               "id": "sbmu1_max_discharge_current",
#               "id": "sbmu1_max_cell_voltage_positions",
#               "id": "sbmu1_min_cell_voltage_positions",
#               "id": "sbmu1_max_cell_temp_positions",
#               "id": "sbmu1_min_cell_temp_positions",
#               "id": "sbmu1_sum_cells",
#               "id": "sbmu1_tms_mode_command",
#               "id": "sbmu1_tms_temp_setting",
#               "id": "sbmu1_tms_real_mode",
#               "id": "sbmu1_rack_inlet_temperature",
#               "id": "sbmu1_rack_outlet_temperature",
#               "id": "sbmu1_ambient_temp",
#               "id": "sbmu1_tms_demand_power",
#               "id": "sbmu1_tms_fault_code",
#               "id": "sbmu1_door_state",
#               "id": "sbmu1_fan_in_box",

# "id": "catl_sbmu_summary_r",
#       "frequency": 1000,
#       "offset_time": 200,
#       "registers": [
#         {
#           "type": "Holding Registers",
#           "starting_offset": 1056,
#           "hex offset":"0x0420",
#           "number_of_registers": 50,
#           "note": "SBMU summary message(read)",
#           "map": [
#             {
#               "id": "sbmu1_voltage",
#               "offset": 1056,
#               "scale": 10,
#               "name": "Battery Subsystem Voltage"
#             },
#             {
#               "id": "sbmu1_current",
#               "offset": 1058,
#               "scale": 10,
#               "name": "Battery Subsystem Current"
#             },
#             {
#               "id": "sbmu1_soc",
#               "offset": 1059,
#               "scale": 10,
#               "name": "SOC Battery system"
#             },
#             {
#               "id": "sbmu1_soh",
#               "offset": 1060,
#               "scale": 10,
#               "name": "SOH Battery system"
#             },
#             {
#               "id": "sbmu1_max_cell_voltage",
#               "offset": 1061,
#               "scale": 1000,
#               "name": "Max single cell voltage"
#             },
#             {
#               "id": "sbmu1_min_cell_voltage",
#               "offset": 1062,
#               "scale": 1000,
#               "name": "Min single cell voltage"
#             },
#             {
#               "id": "sbmu1_avg_cell_voltage",
#               "offset": 1063,
#               "scale": 1000,
#               "name": "Avg. cell voltage"
#             },
#             {
#               "id": "sbmu1_max_cell_temp",
#               "offset": 1064,
#               "name": "Max single cell temperature"
#             },
#             {
#               "id": "sbmu1_min_cell_temp",
#               "offset": 1065,
#               "name": "Min single cell temperature"
#             },
#             {
#               "id": "sbmu1_avg_cell_temp",
#               "offset": 1066,
#               "name": "Avg. single cell temperature"
#             },
#             {
#               "id": "sbmu1_max_charge_current",
#               "offset": 1067,
#               "scale": 10,
#               "name": "Max allowed charge current"
#             },
#             {
#               "id": "sbmu1_max_discharge_current",
#               "offset": 1068,
#               "scale": 10,
#               "name": "Max allowed discharge current"
#             },
#             {
#               "id": "sbmu1_max_cell_voltage_positions",
#               "offset": 1072,
#               "name": "Max single cell voltage position"
#             },
#             {
#               "id": "sbmu1_min_cell_voltage_positions",
#               "offset": 1073,
#               "name": "Min single cell voltage position"
#             },
#             {
#               "id": "sbmu1_max_cell_temp_positions",
#               "offset": 1074,
#               "name": "Max single cell temperature position"
#             },
#             {
#               "id": "sbmu1_min_cell_temp_positions",
#               "offset": 1075,
#               "name": "Min single cell temperature position"
#             },
#             {
#               "id": "sbmu1_sum_cells",
#               "offset": 1079,
#               "scale": 10,
#               "name": "Sum of cell voltage"
#             },
#             {
#               "id": "sbmu1_tms_mode_command",
#               "offset": 1096,
#               "name": "TMS mode command by BMS",
#               "enum": true,
#               "bit_strings": [
#                 "Power off",
#                 "Cooling Mode",
#                 "Heating Mode",
#                 "Self Circulating Mode"
#               ]
#             },
#             {
#               "id": "sbmu1_tms_temp_setting",
#               "offset": 1097,
#               "name": "TMS temperature value set by BMS"
#             },
#             {
#               "id": "sbmu1_tms_real_mode",
#               "offset": 1098,
#               "name": "TMS real mode",
#               "enum": true,
#               "bit_strings": [
#                 "Power off",
#                 "Cooling Mode",
#                 "Heating Mode",
#                 "Self Circulating Mode"
#               ]
#             },
#             {
#               "id": "sbmu1_rack_inlet_temperature",
#               "offset": 1099,
#               "name": "Rack inlet temperature"
#             },
#             {
#               "id": "sbmu1_rack_outlet_temperature",
#               "offset": 1100,
#               "name": "Rack outlet temperature"
#             },
#             {
#               "id": "sbmu1_ambient_temp",
#               "offset": 1101,
#               "name": "Environment temperature"
#             },
#             {
#               "id": "sbmu1_tms_demand_power",
#               "offset": 1102,
#               "name": "TMS demand power"
#             },
#             {
#               "id": "sbmu1_tms_fault_code",
#               "offset": 1103,
#               "name": "TMS fault code"
#             },
#             {
#               "id": "sbmu1_door_state",
#               "offset": 1104,
#               "name": "Door state"
#             },
#             {
#               "id": "sbmu1_fan_in_box",
#               "offset": 1105,
#               "name": "Fan state in control box"
#             }
# "id": "catl_bms_1_sbmu_status_r",
#             "frequency": 1000,
#             "offset_time": 200,
#             "registers": 
#             [
#                 {
#                     "type": "Holding Registers",
#                     "starting_offset": 1040,
#                     "hex offset": "0x0400",
#                     "number_of_registers": 5,
#                     "note": "SBMU status message(read)",
#                     "map":
#                     [
#                         {
#                             "id": "sbmu1_precharge",
#                             "offset": 1040,
#                             "name": "Precharge"
#                         },
#                         {
#                             "id": "sbmu1_master_positive",
#                             "offset": 1041,
#                             "name": "Master positive"
#                         },
#                         {
#                             "id": "sbmu1_master_negative",
#                             "offset": 1042,
#                             "name": "Master negative"
#                         },
#                         {
#                             "id": "sbmu1_balance_status",
#                             "offset": 1044,
#                             "name": "Balance status"
#                         }
#                     ]
#                 }
#             ]
#         },
#         {
#             "id": "catl_bms_1_sbmu_summary_r",
#             "frequency": 1000,
#             "offset_time": 200,
#             "registers": 
#             [
#                 {
#                     "type": "Holding Registers",
#                     "starting_offset": 1056,
#                     "hex offset": "0x0420",
#                     "number_of_registers": 50,
#                     "note": "SBMU summary message(read)",
#                     "map":
#                     [
#                         {
#                             "id": "sbmu1_voltage",
#                             "offset": 1056,
#                             "name": "Battery Subsystem Voltage"
#                         },
#                         {
#                             "id": "sbmu1_current",
#                             "offset": 1058,
#                             "name": "Battery Subsystem Current"
#                         },
#                         {
#                             "id": "sbmu1_soc",
#                             "offset": 1059,
#                             "name": "SOC Battery system"
#                         },
#                         {
#                             "id": "sbmu1_soh",
#                             "offset": 1060,
#                             "name": "SOH Battery system"
#                         },
#                         {
#                             "id": "sbmu1_max_cell_voltage",
#                             "offset": 1061,
#                             "name": "Max single cell voltage"
#                         },
#                         {
#                             "id": "sbmu1_min_cell_voltage",
#                             "offset": 1062,
#                             "name": "Min single cell voltage"
#                         },
#                         {
#                             "id": "sbmu1_avg_cell_voltage",
#                             "offset": 1063,
#                             "name": "Avg. cell voltage"
#                         },
#                         {
#                             "id": "sbmu1_max_cell_temp",
#                             "offset": 1064,
#                             "name": "Max single cell temperature"
#                         },
#                         {
#                             "id": "sbmu1_min_cell_temp",
#                             "offset": 1065,
#                             "name": "Min single cell temperature"
#                         },
#                         {
#                             "id": "sbmu1_avg_cell_temp",
#                             "offset": 1066,
#                             "name": "Avg. single cell temperature"
#                         },
#                         {
#                             "id": "sbmu1_max_charge_current",
#                             "offset": 1067,
#                             "name": "Max allowed charge current"
#                         },
#                         {
#                             "id": "sbmu1_max_discharge_current",
#                             "offset": 1068,
#                             "name": "Max allowed discharge current"
#                         },
#                         {
#                             "id": "sbmu1_max_cell_voltage_positions",
#                             "offset": 1072,
#                             "name": "Max single cell voltage position"
#                         },
#                         {
#                             "id": "sbmu1_min_cell_voltage_positions",
#                             "offset": 1073,
#                             "name": "Min single cell voltage position"
#                         },
#                         {
#                             "id": "sbmu1_max_cell_temp_positions",
#                             "offset": 1074,
#                             "name": "Max single cell temperature position"
#                         },
#                         {
#                             "id": "sbmu1_min_cell_temp_positions",
#                             "offset": 1075,
#                             "name": "Min single cell temperature position"
#                         },
#                         {
#                             "id": "sbmu1_sum_cells",
#                             "offset": 1079,
#                             "name": "Sum of cell voltage"
#                         },
#                         {
#                             "id": "sbmu1_tms_mode_command",
#                             "offset": 1096,
#                             "name": "TMS mode command by BMS",
#                             "xxenum": true,
#                             "xxbit_strings": [
#                                 "Power off",
#                                 "Cooling Mode",
#                                 "Heating Mode",
#                                 "Self Circulating Mode"
#                             ]
#                         },
#                         {
#                             "id": "sbmu1_tms_temp_setting",
#                             "offset": 1097,
#                             "name": "TMS temperature value set by BMS"
#                         },
#                         {
#                             "id": "sbmu1_tms_real_mode",
#                             "offset": 1098,
#                             "name": "TMS real mode",
#                             "xxenum": true,
#                             "xxbit_strings": [
#                                 "Power off",
#                                 "Cooling Mode",
#                                 "Heating Mode",
#                                 "Self Circulating Mode"
#                             ]
#                         },
#                         {
#                             "id": "sbmu1_rack_inlet_temperature",
#                             "offset": 1099,
#                             "name": "Rack inlet temperature"
#                         },
#                         {
#                             "id": "sbmu1_rack_outlet_temperature",
#                             "offset": 1100,
#                             "name": "Rack outlet temperature"
#                         },
#                         {
#                             "id": "sbmu1_ambient_temp",
#                             "offset": 1101,
#                             "name": "Environment temperature"
#                         },
#                         {
#                             "id": "sbmu1_tms_demand_power",
#                             "offset": 1102,
#                             "name": "TMS demand power"
#                         },
#                         {
#                             "id": "sbmu1_tms_fault_code",
#                             "offset": 1103,
#                             "name": "TMS fault code"
#                         },
#                         {
#                             "id": "sbmu1_door_state",
#                             "offset": 1104,
#                             "name": "Door state"
#                         },
#                         {
#                             "id": "sbmu1_fan_in_boxs",
#                             "offset": 1105,
#                             "name": "Fan state in control box"
#                         }
#                     ]
#                 }
#             ]
#         },
