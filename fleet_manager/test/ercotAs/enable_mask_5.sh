echo ""
echo "-----FFR & FRRS Up Event-----"
fims_send -m get -u /fleet/sites/tatooine -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_resource_status_manual 21
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_responsive_reserve_requirement_actual 30.5
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_frrs_up_responsibility_actual 12.5
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_resource_status_override true
echo "waiting for sandstorms to fade..."
sleep 5
fims_send -m get -u /fleet/sites/tatooine -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    0         0     1       0          1  ]"
echo "                   [                    5                     ]"
echo ""
