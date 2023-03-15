#/bin/sh
pkill fims_echo
/usr/local/bin/fims/fims_echo -u /interfaces/outstation -b '{
"Controllable_Load_Emercency_Down_Ramp_Rate":1,
"Controllable_Load_Emercency_Up_Ramp_Rate":2,
"Controllable_Load_Normal_Down_Ramp_Rate":3,
"Controllable_Load_Normal_Up_Ramp_Rate":4,
"Controllable_Load_Scheduled_Power_Consumption":5,
"Controllable_Load_Scheduled_Power_Consumption_+2_hours":6,
"Controllable_Load_Virtual_Breaker_Status_-_PSEUDO_LOAD":7,
"Generation_Emercency_Down_Ramp_Rate":8,
"Generation_Emergency_Up_Ramp_Rate":9,
"Generation_Maximum_Operating_Charge_Power_Limit":10,
"Generation_Maximum_Operating_Discharge_Power_Limit":11,
"Generation_Maximum_Operating_State_of_Charge":12,
"Generation_Minimum_Operating_State_of_Charge":13,
"Generation_Normal_Down_Ramp_Rate":14,
"Generation_Normal_Up_Ramp_Rate":15,
"Generation_Plant_Auxiliary_Load_MVAR":16,
"Generation_Plant_Auxiliary_Load_MW":17,
"Generation_Resource_Breaker_Status":18,
"Generation_Resource_Switches_":19,
"Generation_State_of_Charge":20,
"Generation_Virtual_Breaker_Status_-_PSEUDO_GEN":21,
"Generation_Voltage":22
                 }' &

/usr/local/bin/fims/fims_echo -u /metrics/misc -b '{
"gross_gen_mvar":100,
"gross_gen_mw":101,
"net_gen_mvar":102,
"net_gen_mw":103,
"net_gen_mwb":true,
"net_load_mvar":104,
"net_load_mw":105
                 }' &



