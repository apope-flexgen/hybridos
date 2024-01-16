#!/bin/sh 
# this script tests the opertion of a LoadRequest
# we set the maxLoadRequest param 
/usr/local/bin/fims/fims_send -r /$$ -m set -u /params/bms_1 '{"maxLoadRequest":25000}'
# we get the load Request and tate
echo -n "getting /controls/bms_1:LoadRequest " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/LoadRequest
echo -n "getting /status/bms_1:LoadState     " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/LoadState

sleep 0.2

# send in a valid LoadRequest and check the State
echo -n "setting /controls/bms_1:LoadRequest " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms_1 '{"lastLoadRequest":0,"LoadRequest":20000}'
echo -n "getting /controls/bms_1:LoadRequest " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/LoadRequest
echo -n "getting /status/bms_1:LoadState     " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/LoadState
sleep 0.2

# send in a invalid LoadRequest and check the State
echo -n "setting /controls/bms_1:LoadRequest " &&/usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms_1 '{"LoadRequest":26000}'
echo -n "getting /controls/bms_1:LoadRequest " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/LoadRequest
echo -n "getting /status/bms_1:LoadState" &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/LoadState
sleep 0.2

# send in a fault reset  and check the State
echo -n "setting /controls/bms_1:StateReset  " &&/usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms_1 '{"StateReset":true}'
sleep 0.2

# check the State
echo -n "getting /controls/bms_1:LoadRequest " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/LoadRequest
echo -n "getting /status/bms_1:LoadState     " &&/usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/LoadState
sleep 0.2

