
#!/bin/sh 

# Tests theheartbeat status in test_phil.cpp
# send  different time stamp to say bms_1 and /config/bms_1/TestComms will increment.
# after the initial increment the system will expect continual changes in the HeartBeat, wait too long and we'll get a CommsFail Status.

# Check state
echo "Test 1 - Check state of  lastHb values..."
echo -e "Expectations: They should all be  Empty\n"

echo -n "setting /components/bms:HeartbeatTimeout                    " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bss '{"HeartBeatTimeout": 3.5}'
echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /status/ess:AllCommsStatus                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/ess/allHeartBeatStatus
echo ; echo ; echo
sleep 0.2

# Set a new HeartBeat
echo "Test 2 - Change HeartBeat in each bms unit and check state..."
echo -e "Expectations: All bms_units will update  lastAssetHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"assetHeartBeat":1}'
echo -n "setting /components/bms_2:HeartBeat                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat":2}'
echo -n "setting /components/bms_3:HeartBeat                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat":3}'
echo -n "setting /components/bms_4:HeartBeat                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat":4}'
echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo ; echo ; echo
sleep 1  # wait till the next  HeartBeat cycle

# Set a new HeartBeat
echo "Test 3 - Change HeartBeat in each bms unit and check state again ..."
echo -e "Expectations: All bms_units will update  lastAssetHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"AssetHeartBeat": 11}'
echo -n "setting /components/bms_2:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat": 12 }'
echo -n "setting /components/bms_3:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat": 13 }'
echo -n "setting /components/bms_4:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat": 14 }'
echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /controls/bms:HeartBeatState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/HeartBeatState
echo ; echo ; echo
sleep 1  # wait till the next  HeartBeat cycle

echo "Test 3 - Change HeartBeat in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will update  lastAssetHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"AssetHeartBeat": 1}'
echo -n "setting /components/bms_2:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat": 2}'
echo -n "setting /components/bms_3:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat": 3}'
echo -n "setting /components/bms_4:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat": 4}'
echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /status/bms:HeartBeatErrors                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatErrors
echo -n "getting /status/bms:HeartBeatState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatState
echo -n "getting /status/bms_1:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/HeartBeatState

echo ; echo ; echo
sleep 3  # wait till nearly timed out

echo "Test 3 - Change HeartBeat in each bms unit except bms_2 and check state again ..."
echo -e "Expectations: All bms_units will except bms_2 update  lastAssetHeartBeat indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"AssetHeartBeat": 101}'
#echo -n "setting /components/bms_2:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat": 02}'
echo -n "setting /components/bms_3:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat": 103}'
echo -n "setting /components/bms_4:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat": 104}'

echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /status/bms:HeartBeatState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatState
echo -n "getting /status/bms:HeartBeatErrors                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatErrors
echo -n "getting /status/bms_1:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/HeartBeatState

echo ; echo ; echo
sleep 1  # wait till bms_2 times out

echo "Test 4 - Change active current setpoint in /controls/ess and check state..."
echo -e "HeartBeatState should indicate a failure in bms_2.\n"
echo -n "setting /components/bms_1:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"AssetHeartBeat": 111}'
#echo -n "setting /components/bms_2:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat": 02}'
echo -n "setting /components/bms_3:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat": 113}'
echo -n "setting /components/bms_4:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat": 114}'

echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /status/bms:HeartBeatState                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatState
echo -n "getting /status/bms:HeartBeatErrors                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatErrors
echo -n "getting /status/bms_1:HeartBeatState                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/HeartBeatState
echo ; echo ; echo
sleep 1

echo "Test 5 - Change HeartBeat in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will online with changed HeartBeat Values.\n"
echo -n "setting /components/bms_1:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"AssetHeartBeat": 1}'
echo -n "setting /components/bms_2:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"AssetHeartBeat": 200}'
echo -n "setting /components/bms_3:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"AssetHeartBeat": 3}'
echo -n "setting /components/bms_4:HeartBeat                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"AssetHeartBeat": 4}'
sleep 0.5
echo -n "getting /status/bms_1: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/lastAssetHeartBeat
echo -n "getting /status/bms_2: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/lastAssetHeartBeat
echo -n "getting /status/bms_3: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/lastAssetHeartBeat
echo -n "getting /status/bms_4: lastAssetHeartBeat                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/lastAssetHeartBeat
echo -n "getting /status/bms:HeartBeatState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatState
echo -n "getting /status/bms:HeartBeatErrors                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/HeartBeatErrors
echo -n "getting /status/bms_1:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_1/HeartBeatState
echo -n "getting /status/bms_2:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_2/HeartBeatState
echo -n "getting /status/bms_3:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_3/HeartBeatState
echo -n "getting /status/bms_4:HeartBeatState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms_4/HeartBeatState

echo ; echo ; echo

