cd /usr/local/bin/fims
echo "PCS Off"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":2}'
read -n 1 k <&1
echo "Starting up"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":3}'
read -n 1 k <&1
echo "PCS Going to Ready"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":4}'
read -n 1 k <&1
echo "PCS Receives e-stop"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":9}'
read -n 1 k <&1
echo "Check fault enabled"
./fims_send -m get -r /me -u /ess/full/components/pcsm_dc_inputs/vdc_bus_1 | jq
read -n 1 k <&1
echo "Starting up"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":3}'
read -n 1 k <&1
echo "Set Vdc high"
./fims_send -m set -r /me -u /ess/components/pcsm_dc_inputs/vdc_bus_1 '{"value":1300}'
read -n 1 k <&1
echo "Running"
./fims_send -m set -r /me -u /ess/components/pcsm_internal_visualization '{"current_status":6}'
read -n 1 k <&1
echo "Set Vdc low"
./fims_send -m set -r /me -u /ess/components/pcsm_dc_inputs/vdc_bus_1 '{"value":1000}'
read -n 1 k <&1