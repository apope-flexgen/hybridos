/usr/local/bin/fims_echo -u /components/sel_735 -b '
{ 
    "active_power":{"value":0}, 
    "reactive_power":{"value":0}, 
    "voltage_l1_l2":{"value":0},
    "kwh_delivered":{"value":0}, 
    "kwh_received":{"value":0}, 
     }'&

/usr/local/bin/fims_echo -u /metrics/curtailment_flag -b '
{ 
    "setpoint":{"value":0}
}'&

/usr/local/bin/fims_echo -u /features/active_power -b '
{ 
    "features_kW_mode_cmd":{"value":0},
    "features_kW_mode_cmd_local":{"value":0},
    "absolute_power_kW_cmd":{"value":0},
   "absolute_power_kW_cmd_local":{"value":0}

}'&

/usr/local/bin/fims_echo -u /assets/ess_summary -b '
{ 
    "setpoint":{"value":0}
}'&
