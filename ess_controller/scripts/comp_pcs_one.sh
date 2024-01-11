
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"bess_voltage_bus_1\":{\"value\":3344}}"
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"grid_voltage_rs\":{\"value\":224}}"
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"grid_voltage_st\":{\"value\":10224}}"
sleep 1
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"bess_voltage_bus_1\":{\"value\":33}}"
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"grid_voltage_rs\":{\"value\":22}}"
/usr/local/bin/fims/fims_send  -m pub -u /comp_pcs/component_one "{\"grid_voltage_st\":{\"value\":102}}"
# /usr/local/bin/fims/fims_send -r/me -m pub -u /comp_pcs/component_one \
# '{\
#     "bess_voltage_bus_1":{"value":0},\
#     "current_fault":{"value":0},  \
#     "current_warning":{"value":0},  \
#     "dc_input_current_bess1":{"value":0},\
#     "dc_input_power_bess1":{"value":0},  \
#     "grid_voltage_rs":{"value":0},  \
#     "grid_voltage_st":{"value":0},  \
#     "grid_voltage_tr":{"value":0},  \
#     "inverter_status":{"value":0},  \
#     "pt100_l1_temperature":{"value":0},  \
#     "pt100_l2_temperature":{"value":0},  \
#     "pt100_l3_temperature":{"value":0},  \
#     "reactive_power":{"value":0},  \
#     "vdc_bus":{"value":0},  \
#     "enable_main_selector":{"value":0},  \
#     "p_reference":{"value":55670},  \
#     "q_reference":{"value":45500} \

# }'

