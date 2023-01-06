
#!/bin/sh 

# Tests the HandlePower pcs function (observe how the active power command changes) in test_preston.cpp
# Note: Run ess_controller first before running this shell script

echo -e "Initializing variables based on lab readings..."
/usr/local/bin/fims/fims_send -m set -u /ess/controls/ess '{"ActivePowerSetpoint":0}'
/usr/local/bin/fims/fims_send -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":0}'
/usr/local/bin/fims/fims_send -m set -u /ess/controls/ess '{"PowerPriority":"value":"p"}}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/bms '{"MaxBMSChargeCurrent":-2335}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/bms '{"MaxBMSDischargeCurrent":2334}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/bms '{"BMSVoltage":1436.2}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/pcs '{"MaxPCSActivePower":100}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/pcs '{"MaxPCSReactivePower":100}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/pcs '{"MaxPCSApparentPower":100}'
/usr/local/bin/fims/fims_send -m set -u /ess/status/pcs '{"NumRunningModules":6}'
sleep 0.2 # give enough time for setpoints to be initialize

# Check state
echo "Test 1 - Check state of setpoint values..."
echo -e "Expectations: Variables in /controls/pcs, /variables/pcs, and /variables/bms should be retrievable.\n"
echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
echo -n "getting /status/ess:MaxChargePower               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxChargePower
echo -n "getting /status/ess:MaxDischargePower            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxDischargePower
echo -n "getting /status/ess:MaxReactivePower             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxReactivePower
echo -n "getting /status/ess:MaxApparentPower             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxApparentPower
echo
sleep 1

# Set a new active power setpoint value (+ && > maxDischargePower) and check the state
echo "Test 2 - Active power setpoint 1000 kW"
echo -e "Expectations: Active power command should be set to 1000kW - 28.49 as % of Srated (3510kVA).\n"
echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":1000}'
sleep 0.2
echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
echo
sleep 0.5

echo "Test 2 - Active power setpoint 2600 kW"
echo -e "Expectations: Active power command should be limited to 2500kW - 71.23 as % of Srated (3510kVA).\n"
echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":2600}'
sleep 0.2
echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
echo
sleep 0.5

echo "Test 2 - Active power setpoint -2550 kW"
echo -e "Expectations: Active power command should be limited to -2500kW - -71.23 as % of Srated (3510kVA).\n"
echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":-2550}'
sleep 0.2
echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
echo
sleep 0.5

# echo "Test 3 - Reactive power setpoint 40 kW"
# echo -e "Expectations: Reactive power command should be 40kVA - 9.09% of Srated\n" #limited by MaxApparentPower of 110kVA to 45.8kVAr - 10.41 as % of Srated.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":40}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 4 - Reactive power setpoint 60 kW"
# echo -e "Expectations: Qcmd limited to sqrt(110^2-100^2) = 45.82kVAr - 10.41% of Srated\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":60}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 5 - Change priority to Q"
# echo -e "Expectations: Reactive power command should be 60kVAr - 13.64 as % of Srated, Pcmd limited to sqrt(110^2-60^2) = 92.2kVAr - 20.95 as % of Srated.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"PowerPriority":{"value":"q"}}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo -n "getting /status/ess:MaxChargePower               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxChargePower
# echo -n "getting /status/ess:MaxDischargePower            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxDischargePower
# echo
# sleep 0.5

# echo "Test 6 - Negate P setpoint"
# echo -e "Expectations: Same results with Pcmd negative.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":-100}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 7 - Negate Q setpoint"
# echo -e "Expectations: Same results with Qcmd negative.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":-60}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 8 - Negate P setpoint"
# echo -e "Expectations: Same results with Pcmd positive.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":100}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 9 - Pset and Qset greater than Smax"
# echo -e "Expectations: Qcmd = 25 %, Pcmd = 0.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":130}'
# echo -n "setting /controls/pcs:ReactivePowerSetpoint      " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":120}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 10 - Change priority to P"
# echo -e "Expectations: Pcmd = 25 %, Qcmd = 0.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"PowerPriority":{"value":"p"}}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 11 - Change Pset and Qset to negative"
# echo -e "Expectations: Pcmd = -25 %, Qcmd = 0.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":-130}'
# echo -n "setting /controls/pcs:ReactivePowerSetpoint      " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":-120}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 12 - Change priority to Q"
# echo -e "Expectations: Pcmd = 0 %, Qcmd = -25.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"PowerPriority":{"value":"q"}}'
# sleep 0.2
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 11 - Limit via Discharge current limits"
# echo -e "Expectations: Pcmd limited by MaxDischargePower = 90 - 20.45 as % of Srated.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"PowerPriority":{"value":"p"}}'
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":100}'
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ReactivePowerSetpoint":-60}'
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/status/bms '{"BMSVoltage":1}'
# echo -n "setting /ess/status:MaxBMSDischargeCurrent       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/status/bms '{"MaxBMSDischargeCurrent":90}'
# sleep 0.2
# echo -n "getting /status/ess:MaxChargePower               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxChargePower
# echo -n "getting /status/ess:MaxDischargePower            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxDischargePower
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5

# echo "Test 12 - Limit via Charge current limits"
# echo -e "Expectations: Pcmd limited by MaxChargePower = -80 - -18.18 as % of Srated.\n"
# echo -n "setting /controls/pcs:ActivePowerSetpoint        " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/controls/ess '{"ActivePowerSetpoint":-100}'
# echo -n "setting /ess/status:MaxBMSDischargeCurrent       " && /usr/local/bin/fims/fims_send -r /$$ -m set -u /ess/status/bms '{"MaxBMSChargeCurrent":-80}'
# sleep 0.2
# echo -n "getting /status/ess:MaxChargePower               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxChargePower
# echo -n "getting /status/ess:MaxDischargePower            " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/ess/MaxDischargePower
# echo -n "getting /status/pcs:ActivePowerCmd               " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ActivePowerCmd
# echo -n "getting /status/pcs:ReactivePowerCmd             " && /usr/local/bin/fims/fims_send -r /$$ -m get -u /ess/status/pcs/ReactivePowerCmd
# echo
# sleep 0.5




