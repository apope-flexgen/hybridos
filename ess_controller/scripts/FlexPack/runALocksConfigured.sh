#!/usr/bin/sh

# Run this script with ESS controller running loaded with config files in ess_controller/config/ess_controller/test

read -rsn1 -p"Press any key to send lock tables to RunLocks";echo

/usr/local/bin/fims_send -m set -r /me -u /ess_t/system/commands/ESSLocks '{"value":"test"}'

#read -rsn1 -p"Press any key to run RunLocks";echo

#/usr/local/bin/fims_send -m set -r /me -u /ess_t/system/commands/RunLocks '{"value":"test"}'

read -rsn1 -p"Press any key to try to change a locked value";echo

/usr/local/bin/fims_send -m set -r /me -u /ess_t/status/bms_1/Current 30

echo "Did the value change?"
echo "Current = "
/usr/local/bin/fims_send -m get -r /me -u /ess_t/status/bms_1/Current

read -rsn1 -p"Press any key to try to change a locked parameter";echo

/usr/local/bin/fims_send -m set -r /me -u /ess_t/status/bms_1/Current@EnableMaxValCheck true

echo "Did the parameter value change?"
echo "Full var = "
/usr/local/bin/fims_send -m get -r /me -u /ess_t/full/status/bms_1/Current