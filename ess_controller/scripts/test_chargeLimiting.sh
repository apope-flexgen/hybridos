
#!/bin/sh 

# Tests the HandlePower function (monitoring the state of the active current setpoint) in test_ess.cpp
# Note: Run test_ess first before running this shell script

# Initialize
echo "Initializing variables"
echo -n "setting /variables/bms_1:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_status":1}'
echo -n "setting /variables/bms_2:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_status":1}'
echo -n "setting /variables/bms_3:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_status":1}'
echo -n "setting /variables/bms_4:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_status":1}'
echo -n "setting /controls/ess:ActivePowerSetpoint                         " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActivePowerSetpoint":1350}'
echo -n "setting /variables/bms_1:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_max_p_discharge":400}'
echo -n "setting /variables/bms_2:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_max_p_discharge":340}'
echo -n "setting /variables/bms_3:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_max_p_discharge":380}'
echo -n "setting /variables/bms_4:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_max_p_discharge":360}'
echo -n "setting /variables/bms_1:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_max_p_charge":400}'
echo -n "setting /variables/bms_2:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_max_p_charge":340}'
echo -n "setting /variables/bms_3:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_max_p_charge":380}'
echo -n "setting /variables/bms_4:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_max_p_charge":360}'
echo
sleep 5  # give enough time for wakeups

# Check state
echo "Test 1 - Check variables"
echo -n "getting /controls/ess:ActivePowerSetpoint                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerSetpoint
echo -n "getting /variables/bms_1:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_max_p_discharge
echo -n "getting /variables/bms_2:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_max_p_discharge
echo -n "getting /variables/bms_3:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_max_p_discharge
echo -n "getting /variables/bms_4:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_max_p_discharge
echo -n "getting /status/bms:NumActiveBms                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/numActiveBms
echo
sleep 0.2

# Check state
echo "Test 2 - Check discharge command. Expect 1350"
echo -n "getting /controls/ess:ActivePowerCommand                          " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerCmd
echo
sleep 0.2

# Set a new active power value and check the state
echo "Test 3 - Discharge limited power command"
echo -e "Expectations: The active power command in /controls/ess will be limited to 1360 (4 * 340)."
echo -n "setting /controls/ess:ActivePowerSetpoint                         " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActivePowerSetpoint":1361}'
sleep 5  # give enough time for wakeups
echo -n "getting /controls/ess:ActivePowerCmd                              " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerCmd
echo
sleep 0.2

echo "Test 4 - Set bms_2 to faulted"
echo -e "Expecting 3 activeBms and ActivePowerCmd = 1080 (360 * 3)"
echo -n "setting /variables/bms_2:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_status":5}'
sleep 5  # give enough time for wakeups
echo -n "getting /status/bms:NumActiveBms                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/numActiveBms
echo -n "getting /controls/ess:ActivePowerCmd                              " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerCmd
echo

echo "Test 5 - Charge limited power command"
echo -e "Expecting ActivePowerCmd = -1080 -(360 * 3)"
echo -n "setting /controls/ess:ActivePowerSetpoint                         " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActivePowerSetpoint":-1081}'
sleep 5  # give enough time for wakeups
echo -n "getting /status/bms:NumActiveBms                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/numActiveBms
echo -n "getting /controls/ess:ActivePowerCmd                              " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerCmd
echo

echo "Test 6 - Charge command. Expect -1079"
echo -n "setting /controls/ess:ActivePowerSetpoint                         " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActivePowerSetpoint":-1079}'
sleep 5  # give enough time for wakeups
echo -n "getting /controls/ess:ActivePowerCmd                              " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerCmd