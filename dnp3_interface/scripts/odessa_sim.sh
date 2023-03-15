i="0"
imax=1000
gv_l1=67000

while [ $i -lt $imax ]
do

echo 'send ' $i 'imax ' $imax
i=$[$i+1]
gv_l1=$[$gv_l1+1]

/usr/local/bin/fims/fims_send  -m pub -u /components/sel_651r "{
\"breaker_control\":0,
\"battery_voltage\":14.1,
\"battery_temperature\":45,
\"frequency\":60.01,
\"ldpf3\":0,
\"power_factor\":0.93,
\"current_a\":0,
\"current_b\":0,
\"current_c\":0,
\"current_n\":0,
\"voltage_l1\":32,
\"voltage_l2\":16,
\"voltage_l3\":25,
\"grid_voltage_l1\":$gv_l1,
\"grid_voltage_l2\":7300,
\"grid_voltage_l3\":7398,
\"xgrid_voltage_l1\":6394,
\"grid_voltage_l2\":6300,
\"grid_voltage_l3\":6398,
\"active_power\":0,
\"apparent_power\":1,
\"reactive_power\":0,
\"instantaneous_overcurrent_status\":[],
\"time_overcurrent_status\":[
   {\"value\":2,\"string\":\"Reserved Address:30 Bit:2\"},
   {\"value\":6,\"string\":\"Reserved Address:30 Bit:6\"},
   {\"value\":18,\"string\":\"Reserved Address:31 Bit:2\"},
   {\"value\":22,\"string\":\"Unknown\"}],
\"ground_fault_status\":[
   {\"value\":2,\"string\":\"Reserved Address:32 Bit:2\"},
   {\"value\":6,\"string\":\"Unknown\"}],
\"definite_time_overcurrent\":[],
\"single_pole_open\":false,
\"breaker_closed\":true,
\"phase_c_status\":false,
\"phase_b_status\":false,
\"phase_a_status\":false,
\"relay_row_2\":[],
\"relay_row_22\":[],
\"relay_row_23\":[],
\"relay_row_24\":[],
\"relay_row_26\":[],
\"relay_row_27\":[],
\"relay_row_28\":[],
\"relay_row_34\":[],
\"relay_row_42\":[],
\"relay_row_43\":[],
\"relay_row_85\":[],
\"relay_row_87\":[],
\"relay_row_93\":[],
\"Timestamp\":\"07-07-2020 07:12:42.56616\"}"

sleep 0.1
done
echo 'all done'



