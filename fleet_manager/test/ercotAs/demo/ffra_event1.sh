echo ""
echo "-----PFR Up Event Demo-----"
fims_send -m get -u /fleet/sites/hoth -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_requirement_actual 30.5
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_responsibility_actual 4.6
echo "waiting for rebels to evacuate..."
sleep 5
fims_send -m get -u /fleet/sites/hoth -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    0         1     0       0          0  ]"
echo "                   [                    8                     ]"
echo ""
