# Please see `README_for_setOutputOnUpdate_testing.txt` if you have not already done so.

# this script assumes we're using `/config/tx10/fleetmanager/adds_heartbeat_timestamp_connected_status`
# or the equivalent.

# TODO: the `/testing/debug_metrics.json` content in this PR should be copied to
# ../../config/fleet_manager/metrics/alvin/metrics.json before doing these tests

pkill metrics
sudo pkill mongod
pkill fims

# get rid of any old saved MDOs just so we start fresh every time
rm ../../config/fleet_manager/metrics/alvin/mdo_metrics.json
rm ../../config/fleet_manager/metrics/constants/mdo_metrics.json

sleep 3s
sudo mongod &
sleep 3s
/usr/local/bin/fims/fims_server &
sleep 3s
node ../src/metrics ../../config/fleet_manager/metrics/alvin/ &
sleep 3s
# TODO: comment or uncomment the following line to test
# node ../src/metrics ../../config/fleet_manager/metrics/constants/



# Before this PR's code change:
# if you run this script WITH the constants metrics above (together with the alvin metrics), you
# will get results on the following FIMS listen. if you COMMENT OUT constants, you will get no results.
# If you run this script and comment out constants, you can start and stop constants using the
# commented-out line to see results turn on and off with the following FIMS listen.

# TODO: copy-n-paste this FIMS listen to your terminal to test
# /usr/local/bin/fims/fims_listen | grep ' /sites/odessa/fr_OF_slew_override_flag' -A4



# After this PR's code change:
# if you COMMENT OUT the constants metrics above you WILL STILL GET results using the FIMS listen
# above. Those results will be updated any time there is a set to
# `/components/constants/FRRS_OF_slew_override_flag` - whether it is from the constants metrics
# process or a manually sent FIMS set like the following. As above, If you run this script and
# comment out constants, you can start and stop constants using the commented-out line and see
# that `/sites/odessa/fr_OF_slew_override_flag` is still set on every publish, whether or not
# the constants metrics is actually running.

# TODO: copy-n-paste this FIMS listen to your terminal to test
# /usr/local/bin/fims/fims_send -m set -u /components/constants/FRRS_OF_slew_override_flag '{"value": true}'
