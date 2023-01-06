
#!/bin/sh 

# Tests the HandlePower function (monitoring the state of the active current setpoint) in test_ess.cpp
# Note: Run test_ess first before running this shell script

# Initialize
echo "Initializing variables"
echo -n "setting /variables/bms_1:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_status":1}'
echo -n "setting /variables/bms_2:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_status":1}'
echo -n "setting /variables/bms_3:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_status":1}'
echo -n "setting /variables/bms_4:bmsStatus to running                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_status":1}'
echo -n "setting /variables/bms_1:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_max_p_discharge":400}'
echo -n "setting /variables/bms_2:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_max_p_discharge":350}'
echo -n "setting /variables/bms_3:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_max_p_discharge":380}'
echo -n "setting /variables/bms_4:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_max_p_discharge":360}'
echo -n "setting /variables/bms_1:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_max_p_charge":400}'
echo -n "setting /variables/bms_2:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_max_p_charge":380}'
echo -n "setting /variables/bms_3:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_max_p_charge":340}'
echo -n "setting /variables/bms_4:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_max_p_charge":350}'
echo -n "setting /variables/bms_1:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_charge_energy":172}'
echo -n "setting /variables/bms_2:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_charge_energy":172}'
echo -n "setting /variables/bms_3:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_charge_energy":172}'
echo -n "setting /variables/bms_4:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_charge_energy":172}'
echo -n "setting /variables/bms_1:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_1 '{"bms_discharge_energy":172}'
echo -n "setting /variables/bms_2:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_discharge_energy":172}'
echo -n "setting /variables/bms_3:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_discharge_energy":172}'
echo -n "setting /variables/bms_4:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_4 '{"bms_discharge_energy":172}'
echo -n "setting /variables/pcs:rated_active_power                         " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /params/pcs '{"rated_active_power":3000}'
echo -n "setting /variables/pcs:rated_reactive_power                       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /params/pcs '{"rated_reactive_power":1000}'
echo -n "setting /variables/pcs:pcs_p_limit_inst                           " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/pcs '{"pcs_p_limit_inst":100}'
echo -n "setting /variables/pcs:pcs_q_limit_inst                           " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/pcs '{"pcs_q_limit_inst":50}'
echo
sleep 0.2

# Check state
echo "Test 1 - Check variables"
echo -n "getting /variables/bms_1:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_status
echo -n "getting /variables/bms_2:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_status
echo -n "getting /variables/bms_3:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_status
echo -n "getting /variables/bms_4:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_status
echo -n "getting /variables/bms_1:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_max_p_discharge
echo -n "getting /variables/bms_2:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_max_p_discharge
echo -n "getting /variables/bms_3:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_max_p_discharge
echo -n "getting /variables/bms_4:bmsMaxDischargePower                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_max_p_discharge
echo -n "getting /variables/bms_1:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_max_p_charge
echo -n "getting /variables/bms_2:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_max_p_charge
echo -n "getting /variables/bms_3:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_max_p_charge
echo -n "getting /variables/bms_4:bmsMaxChargePower                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_max_p_charge
echo -n "getting /variables/bms_1:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_charge_energy
echo -n "getting /variables/bms_2:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_charge_energy
echo -n "getting /variables/bms_3:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_charge_energy
echo -n "getting /variables/bms_4:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_charge_energy
echo -n "getting /variables/bms_1:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_1/bms_discharge_energy
echo -n "getting /variables/bms_2:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_2/bms_discharge_energy
echo -n "getting /variables/bms_3:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_3/bms_discharge_energy
echo -n "getting /variables/bms_4:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/bms_4/bms_discharge_energy
echo -n "getting /variables/pcs:rated_active_power                         " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /params/pcs/rated_active_power
echo -n "getting /variables/pcs:rated_reactive_power                       " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /params/pcs/rated_reactive_power
echo -n "getting /status/bms:NumActiveBms                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/numActiveBms
echo -n "getting /variables/pcs:pcs_p_limit_inst                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/pcs/pcs_p_limit_inst
echo -n "getting /variables/pcs:pcs_q_limit_inst                           " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /variables/pcs/pcs_q_limit_inst
echo
sleep 0.2

# Check state
echo "Test 2 - Check Charge Power limit. Expect 340*4 = 1360"
echo -n "getting /controls/ess:ActivePowerChargeLimit                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerChargeLimit
echo
sleep 0.2

# Check state
echo "Test 3 - Check Discharge Power limit. Expect 350*4 = 1400"
echo -n "getting /controls/ess:ActivePowerDischargeLimit                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerDischargeLimit
echo
sleep 0.2

# Check state
echo "Test 4 - Check Reactive Power limit. Expect 0.5*1000 = 500"
echo -n "getting /controls/ess:ReactivePowerLimit                          " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ReactivePowerLimit
echo
sleep 0.2

# Set a new PCS active power limit and check the state
echo "Test 5 - Set PCS to limiting"
echo -n "setting /variables/pcs:pcs_p_limit_inst                           " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/pcs '{"pcs_p_limit_inst":45}'
echo "Expect 0.45*3000 = 1350 for Charge and Discharge power limit"
sleep 0.2
echo -n "getting /controls/ess:ActivePowerChargeLimit                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerChargeLimit
echo -n "getting /controls/ess:ActivePowerDischargeLimit                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerDischargeLimit
echo
sleep 0.2

echo "Test 6 - Set bms_2 to faulted"
echo -n "setting /variables/bms_2:bmsStatus                                " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_status":5}'
echo -e "Expecting 3 activeBms and Discharge Power limit = 360*3 = 1080, Charge Power Limit = 340*3 = 1020"
sleep 0.2  # give enough time for wakeups
echo -n "getting /status/bms:NumActiveBms                                  " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /status/bms/numActiveBms
echo -n "getting /controls/ess:ActivePowerChargeLimit                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerChargeLimit
echo -n "getting /controls/ess:ActivePowerDischargeLimit                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerDischargeLimit
echo

echo "Test 7 - Set Charge/Discharge energy remaining to 0"
echo -n "setting /variables/bms_2:bms_charge_energy                        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_2 '{"bms_charge_energy":0}'
echo -n "setting /variables/bms_3:bms_discharge_energy                     " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /variables/bms_3 '{"bms_discharge_energy":0}'
echo "Expect 0 for Discharge power limit, no change for Charge power limit"
sleep 0.2
echo -n "getting /controls/ess:ActivePowerChargeLimit                      " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerChargeLimit
echo -n "getting /controls/ess:ActivePowerDischargeLimit                   " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /controls/ess/ActivePowerDischargeLimit
echo
sleep 0.2