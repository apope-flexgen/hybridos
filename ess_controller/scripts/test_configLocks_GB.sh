#!/bin/bash

#G. Briggs 8/9/2022

echo "Load ESS locking configs and tables...."
/usr/local/bin/fims_send -m set -r /me -u /ess/locks/ess '{
    "/status/ess:NumRunningESS": {
            "value": false,
            "EnableMinValAlarm": true
        },
    "/measurements/ess": true
    }'
    
/usr/local/bin/fims_send -m set -r /me -u /ess/lockVars/ess '{
    "runLock": {
            "value": "test",
            "uri": "/locks/ess",
            "enabled": false,
            "ifChanged": false,
            "debug": true,
            "actions": {
                "onSet":[{"func":[{"amap": "ess", "func":"runAllLocks"}]}]
            }
        }
    }'

/usr/local/bin/fims_send -m set -r /me -u /ess/status/ess '{
    "NumRunningESS": {
        "value": 0,
        "EnableMinValAlarm": true
        }
    }'

/usr/local/bin/fims_send -m set -r /me -u /ess/measurements/ess '{
        "Voltage": 2,
        "Current": 2
    }'

echo "/locks/ess:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/locks/ess | jq
echo "/lockVars/ess:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/lockVars/ess | jq
echo "/status/ess:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/status/ess | jq
echo "/measurements/ess:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/measurements/ess | jq

read -n 1 -r -s -p "Press any key to load BMS items..."; echo

echo "Load BMS locking configs and tables"

/usr/local/bin/fims_send -m set -r /me -u /ess/locks/bms '{
    "/status/bms": {
            "value": true
        }
    }'

/usr/local/bin/fims_send -m set -r /me -u /ess/lockVars/bms '{
    "runLock": {
            "value": "test",
            "uri": "/locks/bms",
            "enabled": false,
            "ifChanged": false,
            "debug": true,
            "actions": {
                "onSet":[{"func":[{"amap": "bms", "func":"runAllLocks"}]}]
            }
        }
    }'

/usr/local/bin/fims_send -m set -r /me -u /ess/status/bms '{
    "Current": {
            "value": 0,
            "EnableMaxValCheck": false
        }
    }'

echo "/locks/bms:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/locks/bms | jq
echo "/lockVars/bms:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/lockVars/bms | jq
echo "/status/bms:"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/status/bms | jq

read -n 1 -r -s -p "Press any key to run locks..."; echo

/usr/local/bin/fims_send -m set -r /me -u /ess/lockVars/bms '{
    "runLock":{
            "value": "run bms locks",
            "enabled": true
        }
    }'

/usr/local/bin/fims_send -m set -r /me -u /ess/lockVars/ess '{
    "runLock":{
            "value": "run ess locks",
            "enabled": true
        }
    }'

echo "/lockVars/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/lockVars/bms | jq
echo "/lockVars/ess"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/lockVars/ess | jq

read -n 1 -r -s -p "Press any key to start lock tests..."; echo

echo "Changing value of variable /status/ess:NumRunningESS. Expect value to change"
/usr/local/bin/fims_send -m set -r /me -u /ess/status/ess/NumRunningESS 10

echo "Expect value 10"
/usr/local/bin/fims_send -m get -r /me -u /ess/status/ess/NumRunningESS

read -n 1 -r -s -p "Press any key for next test..."; echo
echo "Changing value of parameter /status/ess:NumRunningESS@EnableMinValAlarm. Expect value to not change"

/usr/local/bin/fims_send -m set -r /me -u /ess/status/ess/NumRunningESS/EnableMinValAlarm false

echo "Expect parameter EnableMinValAlarm to still be true" 
/usr/local/bin/fims_send -m get -r /me -u /ess/full/status/ess | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "Changing value of /measurements/ess:Current and :Voltage to 100. Expect neither to change"
/usr/local/bin/fims_send -m set -r /me -u /ess/measurements/ess/Current 100
/usr/local/bin/fims_send -m set -r /me -u /ess/measurements/ess/Voltage 100

echo "Expect Voltage: 2, Current: 2"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/measurements/ess | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "changing value of /status/bms:Current to 100. Changing value of /status/bms:Current@EnableMaxValCheck to true"
/usr/local/bin/fims_send -m set -r /me -u /ess/status/bms/Current 100
/usr/local/bin/fims_send -m set -r /me -u /ess/status/bms/Current/EnableMaxValCheck true

echo "Expect Current: 0, EnableMaxValCheck: false"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/status/bms | jq

read -n 1 -r -s -p "Press any key for next test..."; echo

echo "Unlock /status/bms. Test if values can be written"
/usr/local/bin/fims_send -m set -r /me -u /ess/locks/bms '{
    "/status/bms":{
        "value": false
        }
    }'

echo "Write to /lockVars/bms to unlock variables"
/usr/local/bin/fims_send -m set -r /me -u /ess/lockVars/bms '{
    "runLock": {
            "value": "run unlock test"
            }
        }'

echo "/locks/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/locks/bms | jq
echo "/lockVars/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/lockVars/bms | jq

echo "Try to change Current value from /status/bms"
/usr/local/bin/fims_send -m set -r /me -u /ess/status/bms/Current 20
echo "Expect Current to be value: 20"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/status/bms | jq