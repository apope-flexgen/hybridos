cd /usr/local/bin/fims
./fims_send -m set -r /me -u /ess/assets/pcs/summary/clear_faults false
./fims_send -m set -r /me -u /ess/assets/bms/summary/clear_faults false
sleep 1
# ./fims_send -m set -r /me -u /ess/components/pcsm_dc_inputs/vdc_bus_1 '{"value":1300}'
echo "PCS Faulted, BMS On, Batteries normal"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":9}'
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_fault":56}'
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered off batteries normal
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
sleep 1
./fims_send -m get -r /me -u /ess/full/assets/pcs/summary/faults | jq
./fims_send -m get -r /me -u /ess/site/ess_ls/pcs1_faults2
#./fims_send -m set -r /me -u /ess/status/ess/EStop true
sleep 1
echo "Clear faults command"
./fims_send -m set -r /me -u /site/ess_hs/clear_faults 1
echo "Waiting 10 seconds"
sleep 5
./fims_send -m get -r /me -u /ess/full/assets/pcs/summary/faults | jq
./fims_send -m get -r /me -u /ess/site/ess_ls/pcs1_faults2
./fims_send -m get -r /me -u /ess/status/pcs/SystemFaultCleared
sleep 5
echo "PCS fault clearing"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_fault":0}'
sleep 1
./fims_send -m get -r /me -u /ess/full/assets/pcs/summary/faults | jq
./fims_send -m get -r /me -u /ess/site/ess_ls/pcs1_faults2
./fims_send -m get -r /me -u /ess/status/pcs/SystemFaultCleared
sleep 1
./fims_send -m set -r /me -u /ess/status/ess/EStop false
# sleep 5
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":9}'
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/clear_faults false
# echo "PCS Stop Command"
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop true
# echo "Waiting 7 seconds"
# sleep 7
# echo "PCS going to ready state"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":4}'
# echo "BMS Stop Command in 5 seconds"
# ./fims_send -m set -r /me -u /ess/assets/bms/summary/stop false
# sleep 5
# ./fims_send -m set -r /me -u /ess/assets/bms/summary/stop true
# echo "Waiting 10 seconds"
# sleep 10
# echo "BMS Turning Off"
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'                 # BMS is Off

