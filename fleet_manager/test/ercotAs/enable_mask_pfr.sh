echo ""
echo "-----PFR Down  & FFR Event-----"
fims_send -m get -u /fleet/sites/hoth -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_resource_status_actual 21
fims_send -m set -u /fleet/features/ercotAs/hoth/load_responsive_reserve_requirement_actual 30.8
fims_send -m set -u /fleet/features/ercotAs/hoth/load_responsive_reserve_responsibility_actual 12.5
echo "waiting for the rebels to evacuate..."
sleep 5
fims_send -m get -u /fleet/sites/hoth -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    1         0     1       0          0  ]"
echo "                   [                   20                     ]"
echo ""
