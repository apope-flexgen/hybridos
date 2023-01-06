#!/bin/sh 

# Tests the SetCmd Function

# Note: Run test_phil first before running this shell script

# Check state
echo "Test 1 - Check state of PCRCmd  value..."
echo -e "Expectations: The command setpoint values should be retrievable in /controls/ess and /controls/bms.\n"
echo -n "setting /components/ess/PCRCmd    to 2345                               " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/ess '{"PCRCmd": 234}'
echo -n "setting /components/ess/PCRCmd    to 2345                               " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/ess '{"PCRCmd": 2345}'

echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus
echo -n "setting /components/ess/PCRCmd    to 2345                               " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/ess '{"PCRCmd": 234}'
echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus
sleep 1
echo "note time should not have changed"
#echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus
echo -n "setting /components/ess/PCRCmd    to 2345                               " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/ess '{"PCRCmd": 2345}'
echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus
echo "note time should have changed due to different value"
#echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus
echo -n "setting /components/ess/PCRCmd    to 234500                               " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/ess '{"PCRCmd": 234500}'
echo -n "getting /ess/status/ess/PCRCmdStatus                                    " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/PCRCmdStatus

