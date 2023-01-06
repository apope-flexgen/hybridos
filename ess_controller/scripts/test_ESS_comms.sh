
#!/bin/sh 

# Tests the comms status in test_phil.cpp
# send  different time stamp to say bms_1 and /config/bms_1/TestComms will increment.
# after the initial increment the system will expect continual changes in the timestamp, wait too long and we'll get a CommsFail Status.

# Check state
echo "Test 1 - Check state of  lastTimeStamp values..."
echo -e "Expectations: They should all be  Empty\n"

echo -n "setting /components/bms:CommsTimeout                    " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bss '{"CommsTimeout": 3.5}'
echo -n "getting /controls/bss_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_1/lastTimeStamp
echo -n "getting /controls/bss_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_1/lastTimeStamp
echo -n "getting /controls/bss_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_1/lastTimeStamp
echo -n "getting /controls/bss_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_1/lastTimeStamp
echo -n "getting /status/ess:AllCommsStatus                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/ess/allCommsStatus
echo
sleep 0.2

# Set a new timestamp
echo "Test 2 - Change TimeStamp in each bms unit and check state..."
echo -e "Expectations: All bms_units will update lastTimeStamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 1"}'
echo -n "setting /components/bms_2:Timestamp                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 2"}'
echo -n "setting /components/bms_3:Timestamp                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 3"}'
echo -n "setting /components/bms_4:Timestamp                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 4"}'
echo -n "getting /controls/bss_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_1/lastTimeStamp
echo -n "getting /controls/bss_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_2/lastTimeStamp
echo -n "getting /controls/bss_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_3/lastTimeStamp
echo -n "getting /controls/bss_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bss_4/lastTimeStamp
sleep 1  # wait till the next  timestamp cycle

# Set a new timestamp
echo "Test 3 - Change TimeStamp in each bms unit and check state again ..."
echo -e "Expectations: All bms_units will update lastTimeStamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 11"}'
echo -n "setting /components/bms_2:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 12"}'
echo -n "setting /components/bms_3:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 13"}'
echo -n "setting /components/bms_4:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 14"}'
echo -n "getting /controls/bms_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/lastTimeStamp
echo -n "getting /controls/bms_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/lastTimeStamp
echo -n "getting /controls/bms_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/lastTimeStamp
echo -n "getting /controls/bms_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/lastTimeStamp
echo -n "getting /controls/bms:CommsState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/CommsState
echo -n "getting /controls/bms_1:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/CommsState
echo -n "getting /controls/bms_2:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/CommsState
echo -n "getting /controls/bms_3:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/CommsState
echo -n "getting /controls/bms_4:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/CommsState
sleep 1  # wait till the next  timestamp cycle

echo "Test 3 - Change TimeStamp in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will update lastTimeStamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 1}'
echo -n "setting /components/bms_2:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 2}'
echo -n "setting /components/bms_3:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 3}'
echo -n "setting /components/bms_4:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 4}'
echo -n "getting /controls/bms_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/lastTimeStamp
echo -n "getting /controls/bms_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/lastTimeStamp
echo -n "getting /controls/bms_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/lastTimeStamp
echo -n "getting /controls/bms_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/lastTimeStamp
echo -n "getting /controls/bms:CommsState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/CommsState
echo -n "getting /controls/bms_1:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/CommsState
echo -n "getting /controls/bms_2:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/CommsState
echo -n "getting /controls/bms_3:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/CommsState
echo -n "getting /controls/bms_4:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/CommsState

sleep 3  # wait till nearly timed out

echo "Test 3 - Change TimeStamp in each bms unit except bms_2 and check state again ..."
echo -e "Expectations: All bms_units will except bms_2 update lastTimeStamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 01}'
#echo -n "setting /components/bms_2:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 02}'
echo -n "setting /components/bms_3:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 03}'
echo -n "setting /components/bms_4:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 04}'

echo -n "getting /controls/bms_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/lastTimeStamp
echo -n "getting /controls/bms_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/lastTimeStamp
echo -n "getting /controls/bms_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/lastTimeStamp
echo -n "getting /controls/bms_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/lastTimeStamp
echo -n "getting /controls/bms:CommsState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/CommsState
echo -n "getting /controls/bms_1:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/CommsState
echo -n "getting /controls/bms_2:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/CommsState
echo -n "getting /controls/bms_3:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/CommsState
echo -n "getting /controls/bms_4:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/CommsState

sleep 1  # wait till bms_2 times out

echo "Test 4 - Change active current setpoint in /controls/ess and check state..."
echo -e "CommsState should indicate a failure in bms_2.\n"
echo -n "setting /components/bms_1:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 01}'
#echo -n "setting /components/bms_2:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 02}'
echo -n "setting /components/bms_3:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 03}'
echo -n "setting /components/bms_4:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 04}'

echo -n "getting /controls/bms_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/lastTimeStamp
echo -n "getting /controls/bms_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/lastTimeStamp
echo -n "getting /controls/bms_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/lastTimeStamp
echo -n "getting /controls/bms_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/lastTimeStamp
echo -n "getting /controls/bms:CommsState                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/CommsState
echo -n "getting /controls/bms_1:CommsState                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/CommsState
echo -n "getting /controls/bms_2:CommsState                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/CommsState
echo -n "getting /controls/bms_3:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/CommsState
echo -n "getting /controls/bms_4:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/CommsState
sleep 1

echo "Test 5 - Change TimeStamp in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will online with changed Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_1 '{"Timestamp":"nice value for 1}'
echo -n "setting /components/bms_2:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_2 '{"Timestamp":"nice value for 2}'
echo -n "setting /components/bms_3:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_3 '{"Timestamp":"nice value for 3}'
echo -n "setting /components/bms_4:Timestamp                          " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/bms_4 '{"Timestamp":"nice value for 4}'
echo -n "getting /controls/bms_1:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/lastTimeStamp
echo -n "getting /controls/bms_2:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/lastTimeStamp
echo -n "getting /controls/bms_3:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/lastTimeStamp
echo -n "getting /controls/bms_4:lastTimeStamp                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/lastTimeStamp
echo -n "getting /controls/bms:CommsState                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/CommsState
echo -n "getting /controls/bms_1:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_1/CommsState
echo -n "getting /controls/bms_2:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_2/CommsState
echo -n "getting /controls/bms_3:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_3/CommsState
echo -n "getting /controls/bms_4:CommsState                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms_4/CommsState


