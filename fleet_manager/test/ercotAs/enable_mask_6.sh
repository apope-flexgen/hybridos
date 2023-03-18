echo ""
echo "-----FFR & FRRS Down Event-----"
fims_send -m get -u /fleet/sites/mustafar -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_resource_status_manual 21
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_rrs_requirement_actual 30.6
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_frrs_down_responsibility_actual 12.5
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_resource_status_override true
echo "waiting for Lord Vader..."
sleep 5
fims_send -m get -u /fleet/sites/mustafar -r /me | jq
echo "5-bit Enable Mask: [ PFR_Down  PFR_Up  FFR  FRRS_Down  FRRS_Up]"
echo "                   [    0         0     1       1          0  ]"
echo "                   [                    6                     ]"
echo ""
