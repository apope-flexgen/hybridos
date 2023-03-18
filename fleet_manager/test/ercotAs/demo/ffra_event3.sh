echo ""
echo "-----PFR Up ONLY Event Demo-----"
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_requirement_actual 29.8
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_frrs_up_responsibility_actual 0
fims_send -m set -u /fleet/features/ercotAs/hoth/load_updated_basepoint_actual 0.7
sleep 5
fims_send -m get -u /fleet/sites/hoth -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    0         1     0       0          0  ]"
echo "                   [                    8                     ]"
echo ""
