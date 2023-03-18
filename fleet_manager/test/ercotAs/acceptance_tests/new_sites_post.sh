fims_send -m set -u /fleet/features -f features.json
fims_send -m set -u /fleet/sites -f sites.json
echo "---Add_New_Bad_JSON_Sites---"
echo "Current Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m post -u /fleet/features/ercotAs -f bad_sites.json
sleep 5
echo "All Sites"
fims_send -m get -u /fleet/features/ercotAs/sites -r /me | jq
echo "Old Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq

sleep 10

echo "---Add_New_Incorrectly_Formatted_Sites---"
echo "Current Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m post -u /fleet/features/ercotAs -f incorrect_sites.json
sleep 5
echo "All Sites"
fims_send -m get -u /fleet/features/ercotAs/sites -r /me | jq
echo "Old Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq

sleep 10

echo "---Add_Existing_Sites---"
echo "Current Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m post -u /fleet/features/ercotAs -f existing_sites.json
sleep 5
echo "All Sites"
fims_send -m get -u /fleet/features/ercotAs/sites -r /me | jq
echo "Old Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq

sleep 10

echo "---Add_New_Sites---"
echo "Current Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m post -u /fleet/features/ercotAs -f new_sites.json
sleep 5
echo "All Sites"
fims_send -m get -u /fleet/features/ercotAs/sites -r /me | jq
echo "All Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/mustafar -r /me | jq

sleep 10

echo "---Add_New_Sites_with_Default_Controls---"
echo "Current Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m post -u /fleet/features/ercotAs -f new_sites_with_defaults.json
sleep 5
echo "All Sites"
fims_send -m get -u /fleet/features/ercotAs/sites -r /me | jq
echo "All Sites' Feature Variables"
fims_send -m get -u /fleet/features/ercotAs/alderaan -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/endor -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/hoth -r /me | jq
fims_send -m get -u /fleet/features/ercotAs/tatooine -r /me | jq