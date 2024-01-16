
#!/bin/sh 

# Tests the comms status in test_phil.cpp
# send  different time stamp to say bms_1 and /config/bms_1/TestComms will increment.
# after the initial increment the system will expect continual changes in the timestamp, wait too long and we'll get a CommsFail Status.


fSend=/usr/local/bin/fims/fims_send

# Check state

echo
echo "Testing ComsCheck with 4 bms units" `date`
echo
echo

echo "Test 0 - Check reload state on  ess/bms CheckComms... should all be 2"
echo -n "getting /reload/ess:CheckComms            " &&$fSend -r /me$$ -m get -u /ess/reload/ess/CheckComms
echo -n "getting /reload/bms_1:CheckAssetComms     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_1/CheckAssetComms
echo -n "getting /reload/bms_2:CheckAssetComms     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_2/CheckAssetComms
echo -n "getting /reload/bms_3:CheckAssetComms     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_3/CheckAssetComms
echo -n "getting /reload/bms_4:CheckAssetComms     " &&$fSend -r /me$$ -m get -u /ess/reload/bms_4/CheckAssetComms
sleep 1

echo
echo "Test 1 - Check state of  lastTimestamp values..."
echo -e "Expectations: They should all be  Empty\n"

echo -n "setting /config/ess:testCommsTimeout      " &&$fSend -r /me$$ -m set -u /config/ess '{"testCommsTimeout": 3.5}'
echo -n "getting /config/ess:testCommsTimeout 1    " &&$fSend -r /me$$ -m get -u /config/ess/testCommsTimeout
echo -n "getting /config/ess:testCommsTimeout 2    " &&$fSend -r /me$$ -m get -u /config/ess/testCommsTimeout
echo -n "setting /config/ess:maxCommsTimeout       " &&$fSend -r /me$$ -m set -u /config/ess '{"maxCommsTimeout": 5.5}'
echo -n "getting /config/ess:maxCommsTimeout 1     " &&$fSend -r /me$$ -m get -u /config/ess/maxCommsTimeout
echo -n "getting /config/ess:maxCommsTimeout 2     " &&$fSend -r /me$$ -m get -u /config/ess/maxCommsTimeout
echo -n "setting /config/ess:warnCommsTimeout      " &&$fSend -r /me$$ -m set -u /config/ess '{"warnCommsTimeout": 3.5}'
echo -n "getting /config/ess:warnommsTimeout 1     " &&$fSend -r /me$$ -m get -u /config/ess/warnCommsTimeout
# echo -n "getting /status/bms_1:lastTimestamp       " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp       " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp       " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp       " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/ess:CommsState            " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo
sleep 0.2
#exit 

# Set a new timestamp
echo "Test 2 - Change TimeStamp in each bms unit and check state..."
echo -e "Expectations: All bms_units will update lastTimestamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test2 value for bms_1"}'
echo -n "setting /components/bms_2:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test2 value for bms_2"}'
echo -n "setting /components/bms_3:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test2 value for bms_3"}'
echo -n "setting /components/bms_4:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test2 value for bms_4"}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/bms_1:CommsState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState         " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState
echo -n "getting /status/bms:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState

echo "sleep 1"
sleep 1  # wait till the next  timestamp cycle

# Set a new timestamp
echo "Test 3 - Change TimeStamp in each bms unit and check state again ..."
echo -e "Expectations: All bms_units will update lastTimestamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test3 for bms_1"}'
echo -n "setting /components/bms_2:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test3 for bms_2"}'
echo -n "setting /components/bms_3:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test3 for bms_3"}'
echo -n "setting /components/bms_4:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test3 for bms_4"}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState
sleep 1  # wait till the next  timestamp cycle
echo
echo "Test 3.1 - Change TimeStamp in each bms unit  and check state again ..."
echo -e "Expectations: All bms_units will update lastTimestamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test3.1 for bms_1"}'
echo -n "setting /components/bms_2:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test3.1 for bms_2"}'
echo -n "setting /components/bms_3:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test3.1 for bms_3"}'
echo -n "setting /components/bms_4:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test3.1 for bms_4"}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp      " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState

echo "sleep 3.5"
sleep 3.5  # wait till nearly timed out
echo
echo "Test 3.2 - Change TimeStamp in each bms unit except bms_2 and check state again ..."
echo -e "Expectations: All bms_units will except bms_2 update lastTimestamp indicating a change in  Time Stamp Values.\n"
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test 3.2 value for bms_1"}'
#echo -n "setting /components/bms_2:Timestamp        " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test 3.2 value for bms_2"}'
echo -n "setting /components/bms_3:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test 3.2 value for bms_3"}'
echo -n "setting /components/bms_4:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test 3.2 value for bms_4"}'
echo -n "getting /status/ess:CommsErrors                  " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsErrors
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState
echo "sleep 0.2"
sleep 0.2

# echo -n "getting /status/bms_1:lastTimestamp     " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp     " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp     " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp     " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState

echo -n "getting /status/ess:CommsErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsErrors
echo -n "getting /status/bms:CommsErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsErrors
echo -n "getting /status/ess:CommsWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsWarns
echo -n "getting /status/bms:CommsWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsWarns
echo "sleep 1"
sleep 1  # wait till bms_2 times out
echo
echo "Test 4 - Change active current setpoint in /controls/ess and check state..."
echo -e "CommsState should indicate a failure in bms_2.\n"
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test 4 value for bms_1"}'
#echo -n "setting /components/bms_2:Timestamp       " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test 4 value for bms_2"}'
echo -n "setting /components/bms_3:Timestamp        " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test 4 value for bms_3"}'
echo -n "setting /components/bms_4:Timestamp         " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test 4 value for bms_4"}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/ess:CommsErrors                  " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsErrors
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState

echo -n "getting /status/ess:CommsErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsErrors
echo -n "getting /status/bms:CommsErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsErrors
echo -n "getting /status/ess:CommsWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsWarns
echo -n "getting /status/bms:CommsWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsWarns

echo " sleep 1"
sleep 1
echo "Test 5 - Check State then change TimeStamp in each bms unit, wait 0.2 then check state again ..."
echo -e "Expectations: All bms_units will online with changed Time Stamp Values.\n"
# echo -n "getting /status/bms_1:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "setting /components/bms_1:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_1 '{"Timestamp":"test 5 value for bms_1"}'
echo -n "setting /components/bms_2:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_2 '{"Timestamp":"test 5 value for bms_2"}'
echo -n "setting /components/bms_3:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_3 '{"Timestamp":"test 5 value for bms_3"}'
echo -n "setting /components/bms_4:Timestamp      " &&$fSend -r /me$$ -m set -u /components/bms_4 '{"Timestamp":"test 5 value for bms_4"}'
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo "sleep 0.2"
sleep 0.2
# echo -n "getting /status/bms_1:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/lastTimestamp
# echo -n "getting /status/bms_2:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/lastTimestamp
# echo -n "getting /status/bms_3:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/lastTimestamp
# echo -n "getting /status/bms_4:lastTimestamp  " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/lastTimestamp
echo -n "getting /status/ess:CommsState           " &&$fSend -r /me$$ -m get -u /ess/status/ess/CommsState
echo -n "getting /status/bms:CommsState               " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsState
echo -n "getting /status/bms_1:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_1/CommsState
echo -n "getting /status/bms_2:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_2/CommsState
echo -n "getting /status/bms_3:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_3/CommsState
echo -n "getting /status/bms_4:CommsState                " &&$fSend -r /me$$ -m get -u /ess/status/bms_4/CommsState

echo -n "getting /status/ess:CommsErrors             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsErrors
echo -n "getting /status/bms:CommsErrors                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsErrors
echo -n "getting /status/ess:CommsWarns             " &&$fSend -r /me$$ -m get -u /ess/status/ess/essCommsWarns
echo -n "getting /status/bms:CommsWarns                 " &&$fSend -r /me$$ -m get -u /ess/status/bms/CommsWarns

