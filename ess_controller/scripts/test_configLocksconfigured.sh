#!/bin/bash

#G. Briggs 8/9/2022
#intended to be used with ess that has been configured by ess_t_file.
#Make sure to send configurations to dbi first!

read -n 1 -r -s -p "Press any key to start lock tests..."; echo

echo "Changing value of variable /status/ess:NumRunningESS. Expect value to change"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/status/ess/NumRunningESS 10

echo "Expect value 10"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/status/ess/NumRunningESS

read -n 1 -r -s -p "Press any key for next test..."; echo
echo "Changing value of parameter /status/ess:NumRunningESS@EnableMinValAlarm. Expect value to not change"

/usr/local/bin/fims_send -m set -r /me -u /ess_1/status/ess/NumRunningESS/EnableMinValAlarm false

echo "Expect parameter EnableMinValAlarm to still be true" 
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/status/ess | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "Changing value of /measurements/ess:Current and :Voltage to 100. Expect neither to change"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/measurements/ess/Current 100
/usr/local/bin/fims_send -m set -r /me -u /ess_1/measurements/ess/Voltage 100

echo "Expect Voltage: 2, Current: 1"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/measurements/ess | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "changing value of /status/bms:Current to 100. Changing value of /status/bms:Current@EnableMaxValCheck to true"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/status/bms/Current 100
/usr/local/bin/fims_send -m set -r /me -u /ess_1/status/bms/Current/EnableMaxValCheck true

echo "Expect Current: 0, EnableMaxValCheck: false"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/status/bms | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "Unlock /status/bms. Test if values can be written"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/locks/bms '{
    "/status/bms":{
        "value": false
        }
    }'

echo "Write to /lockVars/bms to unlock variables"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/lockVars/bms '{
    "runLock": {
            "value": "run unlock test"
            }
        }'

echo "/locks/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/locks/bms | jq
echo "/lockVars/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/lockVars/bms | jq

echo "Try to change Current value from /status/bms"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/status/bms/Current 20
echo "Expect Current to be value: 20"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/full/status/bms | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "Write to /lockVars/sbmu_1 to lock variables"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/lockVars/sbmu_1 '{
    "runLock": {
            "value": "run lock test"
            }
        }'

echo "Try to change RatedCurrent value from /config/sbmu_1"
/usr/local/bin/fims_send -m set -r /me -u /ess_1/config/sbmu_1/RatedCurrent 20
echo "Expect Current to be value: 100"
/usr/local/bin/fims_send -m get -r /me -u /ess_1/config/sbmu_1 | jq

