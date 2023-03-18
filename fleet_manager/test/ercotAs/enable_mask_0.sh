echo ""
echo "-----No Events-----"
fims_send -m get -u /fleet/sites/tatooine -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_resource_status_manual 0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_resource_status_actual 0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_responsive_reserve_requirement_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/load_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/load_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/load_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/tatooine/gen_resource_status_override false

fims_send -m get -u /fleet/sites/mustafar -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_resource_status_manual 0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_resource_status_actual 0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_responsive_reserve_requirement_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/load_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/load_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/load_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/mustafar/gen_resource_status_override false

fims_send -m get -u /fleet/sites/hoth -r /me | jq
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_resource_status_manual 0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_resource_status_actual 0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_requirement_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/load_frrs_up_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/load_frrs_down_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/load_responsive_reserve_responsibility_actual 0.0
fims_send -m set -u /fleet/features/ercotAs/hoth/gen_resource_status_override false