cd /usr/local/bin/fims
mkdir -p /tmp/ess_controller/logs

#./fims_send -m get -r /$$ -u /ess/full/amap/bms | jq

./fims_send -m set -r /me -u /ess/assets/pcs/summary/start false
./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
# ./fims_send -m set -r /me -u /ess/components/pcsm_dc_inputs/vdc_bus_1 '{"value":1300}'
echo "PCS Off, BMS Off, Batteries normal"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'                 # PCS is ready
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered off batteries normal
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'
echo "BMS Start command"
./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
echo "Waiting 7 seconds"
sleep 7
echo "BMS Turning On"
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
echo "Snapshot Bms"
./fims_send -m get -r /$$ -u /ess/full/amap/bms | jq > /tmp/ess_controller/logs/StartStopLog.txt
echo "Waiting 7 seconds"
sleep 7
# echo "PCS Start command"
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
# echo "Waiting 10 seconds for PCS to respond"
# sleep 10
# echo "PCS Precharging for 5 seconds"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":3}'
# sleep 5

echo "PCS going to ready"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":4}'
echo "PCS Start command"
./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
echo "Waiting 10 seconds"
sleep 10
echo "Snapshot Pcs"
./fims_send -m get -r /$$ -u /ess/full/amap/pcs | jq >> /tmp/ess_controller/logs/StartStopLog.txt
echo "PCS Turning On"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'                 # PCS is On
sleep 1
./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop false

# echo "PCS Stop Command"
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/stop true
# echo "Waiting 7 seconds"
# sleep 7
# echo "PCS going to ready state"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":4}'
echo "BMS Stop Command in 5 seconds"
./fims_send -m set -r /me -u /ess/assets/bms/summary/stop false
sleep 5
echo "Snapshot Bms"
./fims_send -m get -r /$$ -u /ess/full/amap/bms | jq >> /tmp/ess_controller/logs/StartStopLog.txt
echo "Snapshot Pcs"
./fims_send -m get -r /$$ -u /ess/full/amap/pcs | jq >> /tmp/ess_controller/logs/StartStopLog.txt

./fims_send -m set -r /me -u /ess/assets/bms/summary/stop true
echo "Waiting 10 seconds"
sleep 10
echo "BMS Turning Off"
./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'                 # BMS is Off

# ./fims_send -m set -r /me -u /ess/assets/bms/summary/start false
# echo "PCS Off, BMS Off, Batteries normal"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}' && ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}' && ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'
# echo "Starting up"
# ./fims_send -m set -r /me -u /ess/assets/bms/summary/start true
# echo "Waiting 25 seconds (should fault at 24)"
# sleep 25

# echo "PCS Off, BMS Off, Batteries normal"
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'                 # BMS is Off
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'                 # PCS is ready
# echo "PCS Start command"
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true
# echo "Waiting 10 seconds for PCS to respond"
# sleep 10
# echo "PCS Turning On"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'                 # PCS is On
# sleep 1
# echo "BMS Stop Command"
# ./fims_send -m set -r /me -u /ess/assets/bms/summary/stop true
# echo "Waiting 10 seconds"
# sleep 10
# echo "BMS Turning Off"
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'                 # BMS is Off
# echo "Waiting 10 seconds"
# sleep 10
# echo "PCS Start command"
# ./fims_send -m set -r /me -u /ess/assets/pcs/summary/start true

# ./fims_send -m set -r /me -u /ess/status/ess '{"UiStartup":false}'
# echo "PCS Off, BMS On, Batteries normal"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'                 # PCS is ready
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered on batteries normal
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
# echo "Starting up"
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiStartup":true}'
# echo "Waiting 6 seconds for PCS to respond"
# sleep 6
# echo "PCS Turning On"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'                 # PCS is On
# sleep 5
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiStartup":false}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"FaultShutdown":false}'
# echo "Fault Shutdown Command"
# ./fims_send -m set -r /me -u /ess/status/ess '{"FaultShutdown":true}'
# echo "Waiting 7 seconds"
# sleep 7
# echo "PCS going to fault state"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":9}'
# echo "Waiting 5 seconds"
# sleep 5

# echo "PCS and BMS Turning On"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'                 # PCS is On
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered on batteries normal
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'
# echo "Waiting 5 seconds"
# sleep 5
# echo "Shutdown Command"
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiShutdown":false}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiShutdown":true}'
# echo "Waiting 30 seconds - first stop cmds then e-stop"
# sleep 30
# echo "PCS going to fault state"
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":9}'
# echo "Waiting 7 seconds"
# sleep 7
# echo "BMS Turning Off"
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":0}'                 # BMS is Off


# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'                 # PCS is On
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_status":1}'   # BMS is powered on batteries normal
# ./fims_send -m set -r /me -u /ess/components/catl_bms_ems_r '{"bms_poweron":1}'

# ./fims_send -m set -r /me -u /ess/status/pcs '{"maxPCSKeyCmdOnTime":1}'   # BMS is powered on batteries normal
# ./fims_send -m set -r /me -u /ess/status/pcs '{"maxPCSKeyCmdTime":2}'
# ./fims_send -m set -r /me -u /ess/status/pcs '{"maxPCSShutdownTime":26}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiShutdown":false}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiShutdown":true}'
# sleep 25
# ./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiStartup":false}'
# ./fims_send -m set -r /me -u /ess/status/ess '{"UiStartup":true}'
