
#!/bin/sh 

# Tests the HandleCurrent function (monitoring the state of the active/reactive current setpoint) in test_jimmy.cpp
# Note: Run test_jimmy first before running this shell script

# Check state
echo "Test 1 - Check state of setpoint values..."
echo -e "Expectations: The active/reactive current setpoint values should be retrievable in /controls/ess and /components/pcs.\n"
echo -n "getting /controls/ess:ActiveCurrentSetpoint                             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActiveCurrentSetpoint
echo -n "getting /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint
echo -n "getting /components/pcs:ActiveCurrent                                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ActiveCurrent
echo -n "getting /controls/ess:ReactiveCurrentSetpoint                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ReactiveCurrentSetpoint
echo -n "getting /variables/ess:lastReactiveCurrentSetpoint                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint
echo -n "getting /components/pcs:ReactiveCurrent                                 " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ReactiveCurrent
echo
sleep 0.2

# Set a new active current setpoint value and check the state
echo "Test 2 - Change active current setpoint in /controls/ess and check state..."
echo -e "Expectations: The active current setpoint in /controls/ess will be sent to /components/pcs, indicating a change in current.\n"
echo -n "setting /controls/ess:ActiveCurrentSetpoint                             " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActiveCurrentSetpoint":1000}'
echo -n "setting /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/ess '{"lastActiveCurrentSetpoint":0}'
echo -n "getting /controls/ess:ActiveCurrentSetpoint                             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActiveCurrentSetpoint
echo -n "getting /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint
sleep 5  # give enough time for any updates to active current in /components/pcs
echo -n "getting /components/pcs:ActiveCurrent                                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ActiveCurrent
echo
sleep 0.2

# Set a new active current setpoint equal to last current setpoint - should not trigger a change to active current value in /components/pcs
echo "Test 3 - Set the same active current values in /controls/ess, /variables/ess, and /components/pcs and check state..."
echo -e "Expectations: The active current in /components/pcs should not change when the active current setpoint in /controls/ess remains unchanged.\n"
echo -n "check 1 /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint
echo -n "setting /components/pcs:ActiveCurrent                                   " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/pcs '{"ActiveCurrent":2000}'
echo -n "setting /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/ess '{"lastActiveCurrentSetpoint":2000}'
echo -n "check 2 /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint
echo -n "setting /controls/ess:ActiveCurrentSetpoint                             " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ActiveCurrentSetpoint":2000}'
echo -n "getting /controls/ess:ActiveCurrentSetpoint                             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActiveCurrentSetpoint
echo -n "getting /variables/ess:lastActiveCurrentSetpoint                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastActiveCurrentSetpoint
sleep 5  # give enough time for any updates to current setpoint in /components/pcs
echo -n "getting /components/pcs:ActiveCurrent                                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ActiveCurrent
echo
sleep 0.2

# Set a new reactive current setpoint value and check the state
echo "Test 4 - Change reactive current setpoint in /controls/ess and check state..."
echo -e "Expectations: The reactive current setpoint in /controls/ess will be sent to /components/pcs, indicating a change in current.\n"
echo -n "setting /controls/ess:ReactiveCurrentSetpoint                            " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ReactiveCurrentSetpoint":1000}'
echo -n "setting /variables/ess:lastReactiveCurrentSetpoint                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/ess '{"lastReactiveCurrentSetpoint":0}'
echo -n "getting /controls/ess:ReactiveCurrentSetpoint                            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ReactiveCurrentSetpoint
echo -n "getting /variables/ess:lastReactiveCurrentSetpoint                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint
sleep 5  # give enough time for any updates to reactive current in /components/pcs
echo -n "getting /components/pcs:ReactiveCurrent                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ReactiveCurrent
echo
sleep 0.2

# Set a new reactive current setpoint equal to last reactive current setpoint - should not trigger a change to reactive current value in /components/pcs
echo "Test 5 - Set the same reactive current values in /controls/ess, /variables/ess, and /components/pcs and check state..."
echo -e "Expectations: The reactive current in /components/pcs should not change when the reactive current setpoint in /controls/ess remains unchanged.\n"
echo -n "check 1 /variables/ess:lastReactiveCurrentSetpoint                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint
echo -n "setting /components/pcs:ReactiveCurrent                                 " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /components/pcs '{"ReactiveCurrent":2000}'
echo -n "setting /variables/ess:lastReactiveCurrentSetpoint                      " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/ess '{"lastReactiveCurrentSetpoint":2000}'
echo -n "check 2 /variables/ess:lastReactiveCurrentSetpoint                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint
echo -n "setting /controls/ess:ReactiveCurrentSetpoint                           " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /controls/ess '{"ReactiveCurrentSetpoint":2000}'
echo -n "getting /controls/ess:ReactiveCurrentSetpoint                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ReactiveCurrentSetpoint
echo -n "getting /variables/ess:lastReactiveCurrentSetpoint                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/ess/lastReactiveCurrentSetpoint
sleep 5  # give enough time for any updates to current setpoint in /components/pcs
echo -n "getting /components/pcs:ReactiveCurrent                                 " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /components/pcs/ReactiveCurrent
echo
sleep 0.2