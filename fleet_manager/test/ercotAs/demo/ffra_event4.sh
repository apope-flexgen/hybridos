echo ""
echo "-----PFR Up and FFR Event Demo-----"
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_requirement_actual 12.6
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_resource_status_actual 21
sleep 5
fims_send -m get -u /fleet/sites/hoth -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    0         1     1       0          0  ]"
echo "                   [                   12                     ]"
echo ""
