#!/bin/sh 

# Tests the HandleCmd function in test_jimmy.cpp
# Note: Run test_jimmy first before running this shell script

# Check state
echo "Test 1 - Check state of setpoint values..."
echo -e "Expectations: The command setpoint values should be retrievable in /controls/ess and /controls/bms.\n"
echo -n "getting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/On
echo -n "getting /controls/bms:OnCmd                                             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/OnCmd
echo -n "getting /controls/bms:Off                                               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Off
echo -n "getting /controls/bms:OffCmd                                            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/OffCmd
echo -n "getting /controls/bms:Standby                                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Standby
echo -n "getting /controls/bms:StandbyCmd                                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/StandbyCmd
echo -n "getting /controls/bms:AcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/AcContactor
echo -n "getting /controls/bms:DcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/DcContactor
echo -n "getting /controls/ess:EStop                                             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/EStop
echo
sleep 0.2

# Check if "Standby" is set to true and "StandbyCmd" is set to false if the "On" variable and "StandByCmd" are initially set to true

echo "Test 2 - Set \"OnCmd\", \"AcContactor\", and \"DcContactor\" to true and \"On\" to false..."
echo -e "Expectations: \"Standby\" should be set to true and \"StandbyCmd\" should be set to false. \n"
echo -n "setting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"On":false}'
echo -n "setting /controls/bms:OnCmd                                             " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"OnCmd":true}'
echo -n "setting /controls/bms:AcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"AcContactor":true}'
echo -n "setting /controls/bms:DcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"DcContactor":true}'
echo -n "setting /controls/bms:StandbyCmd                                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"StandbyCmd":true}'
sleep 5  # give enough time for any updates to be done
echo -n "getting /controls/bms:Standby                                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Standby
echo -n "getting /controls/bms:StandbyCmd                                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/StandbyCmd
echo
sleep 0.2

echo "Test 3 - Set \"OnCmd\", \"Standby\", \"AcContactor\", and \"DcContactor\" to true..."
echo -e "Expectations: \"Standby\" should be set to true and \"StandbyCmd\" should be set to false. \n"
echo -n "setting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"Standby":true}'
echo -n "setting /controls/bms:OnCmd                                             " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"OnCmd":true}'
echo -n "setting /controls/bms:AcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"AcContactor":true}'
echo -n "setting /controls/bms:DcContactor                                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"DcContactor":true}'
echo -n "setting /controls/bms:StandbyCmd                                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"StandbyCmd":true}'
sleep 5  # give enough time for any updates to be done
echo -n "getting /controls/bms:Standby                                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Standby
echo -n "getting /controls/bms:StandbyCmd                                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/StandbyCmd
echo
sleep 0.2

# Check if "Off" is set to true and "On", "OffCmd", and "Standby" are set to false if "On" and "OffCmd" are initially set to true
echo "Test 4 - Set \"On\" and \"OffCmd\" to true..."
echo -e "Expectations: \"Off\" should be set to true and \"On\", \"OffCmd\", and \"Standby\" should be set to false. \n"
echo -n "setting /controls/bms:OnCmd                                             " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"OnCmd":false}'
echo -n "setting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"On":true}'
echo -n "setting /controls/bms:OffCmd                                            " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/bms '{"OffCmd":true}'
echo -n "getting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/On
echo -n "getting /controls/bms:OffCmd                                            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/OffCmd
sleep 5  # give enough time for any updates to be done
echo -n "getting /controls/bms:On                                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/On
echo -n "getting /controls/bms:OffCmd                                            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/OffCmd
echo -n "getting /controls/bms:Standby                                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Standby
echo -n "getting /controls/bms:Off                                               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/bms/Off
echo
sleep 0.2