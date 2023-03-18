fims_send -m set -u /fleet/features -f ~/git/fleet_manager/test/demo/features.json
echo ""
echo "-----Configuration Demo-----"

IMWMIN=$(fims_send -m get -u /fleet/features/ercotAs/hoth/inactive_cmd_min_limit_mw -r /me)
echo "Hoth Inactive MW Command Minimum Limit: $IMWMIN MW"

IMWMIN=$(fims_send -m get -u /fleet/features/ercotAs/mustafar/inactive_cmd_min_limit_mw -r /me)
echo "Mustafar Inactive MW Command Minimum Limit: $IMWMIN MW"

NDRROVRR=$(fims_send -m get -u /fleet/features/ercotAs/hoth/gen_normal_down_ramp_rate_override -r /me)
echo "Hoth Generation Normal Down Ramp Rate Override: $NDRROVRR"

NDRROVRR=$(fims_send -m get -u /fleet/features/ercotAs/mustafar/gen_normal_down_ramp_rate_override -r /me)
echo "Mustafar Generation Normal Down Ramp Rate Override: $NDRROVRR"
echo ""