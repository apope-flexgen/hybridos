#!/usr/bin/sh

#Typical input from a BMS interface unit

#The BMS is required to periodically scan all the battery cells and 
#determine the max / min avg cell voltages.
#Batteries are arranged in clusters of cells
#In this example there are 20 clusters each with approx 1000 voltages 

#The system also has states, alarms and faults.
#/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 '{"soc":{"value":1234}}'  -r /me
FOO="{
\"bms_maximum_cell_voltage\":{\"value\":3.299},
\"stackindex_of_the_bms_maximum_cell_voltage\":{\"value\":3},
\"cluster_index_of_the_bms_maximum_cell_voltage\":{\"value\":3.299},
\"bms_minimum_cell_voltage\":{\"value\":3.099},
\"stackindex_of_the_bms_minimum_cell_voltage\":{\"value\":24},
\"cluster_index_of_the_bms_minimum_cell_voltage\":{\"value\":2},
\"bms_maximum_cell_temperature\":{\"value\":27},
\"stackindex_of_the_bms_maximum_cell_temperature\":{\"value\":3},
\"cluster_index_of_the_bms_maximum_cell_temperature\":{\"value\":7},
\"bms_minimum_cell_temperature\":{\"value\":3},
\"stackindex_of_the_bms_minimum_cell_temperature\":{\"value\":18},
\"cluster_index_of_the_bms_minimum_cell_temperature\":{\"value\":2},
\"bms_bus_voltage\":{\"value\":1066.7},
\"bms_bus_current\":{\"value\":0.2},
\"bms_soc\":{\"value\":35},
\"bms_power\":{\"value\":0},
\"bms_current_chargeable_capacity\":{\"value\":1830.328},
\"bms_current_dischargeable_capacity\":{\"value\":985.559},
\"circuit_breaker_control_word\":{\"value\":2047},
\"main_circuit_breaker_status\":{\"value\":1},
\"control_circuit_breaker\":{\"value\":0},
\"bms_maximum_charging_power_limit\":{\"value\":120},
\"bms_maximum_discharging_power_limit\":{\"value\":0},
\"cluster_no_20_battery_cluster_voltage\":{\"value\":0},
\"cluster_no_1_battery_cluster_current\":{\"value\":0},
\"cluster_no_1_max_cell_voltage\":{\"value\":3.298},
\"cluster_no_1_max_cell_voltage_pack_num\":{\"value\":18},
\"cluster_no_1_second_max_cell_voltage\":{\"value\":3.297},
\"cluster_no_1_second_max_cell_voltage_pack_num\":{\"value\":9},
\"cluster_no_1_min_cell_voltage\":{\"value\":3.291},
\"cluster_no_1_min_cell_voltage_pack_num\":{\"value\":7},
\"cluster_no_1_second_min_cell_voltage\":{\"value\":3.292},
\"cluster_no_1_second_min_cell_voltage_pack_num\":{\"value\":5},
\"cluster_no_1_max_cell_temperature_pack_num\":{\"value\":20},
\"cluster_no_2_max_cell_temperature\":{\"value\":26},
\"cluster_no_1_second_max_cell_temperature\":{\"value\":34},
\"cluster_no_1_second_max_cell_temperature_pack_num\":{\"value\":21},
\"cluster_no_1_min_cell_temperature\":{\"value\":22},
\"cluster_no_1_min_cell_temperature_pack_num\":{\"value\":1},
\"cluster_no_1_second_min_cell_temperature\":{\"value\":22},
\"cluster_no_1_second_min_cell_temperature_pack_num\":{\"value\":16},
\"cluster_no_1_battery_cluster_average_voltage\":{\"value\":3.294},
\"cluster_no_2_battery_cluster_average_voltage\":{\"value\":3.294},
\"stack_overall_average_cell_temperature\":{\"value\":24.3},
\"average_ambient_temperature_inside_the_container\":{\"value\":26.6},
\"average_ambient_humidity_inside_the_container\":{\"value\":40.4},
\"maximum_cell_temperature\":{\"value\":27},
\"stack_index_of_maximum_cell_temperature\":{\"value\":1},
\"cluster_index_of_maximum_cell_temperature\":{\"value\":7},
\"pack_index_of_maximum_cell_temperature\":{\"value\":3},
\"minimum_cell_temperature\":{\"value\":21},
\"stack_index_of_minimum_cell_temperature\":{\"value\":1},
\"cluster_index_of_minimum_cell_temperature\":{\"value\":2},
\"pack_index_of_minimum_cell_temperature\":{\"value\":24}
}"

#echo $FOO | jq

/usr/local/bin/fims/fims_send -m set -u /assets/bms/bms_1 $FOO

    