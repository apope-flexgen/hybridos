#!/bin/sh 

# Tests CheckDbiVar function (checks if dbi config variable can be retrieved and set by ess controller)
#
# Example: In bms_manager.json: 
#          "/schedule/wake_monitor/bms": {
#              "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"},
#              "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
#          },
#          "/controls/bms": {
#              "DemoChargeCurrent": {
#                  "value": 0.0,
#                  "dbiStatus": "init",
#                  "actions": {
#                      "onSet": [
#                          {"func": [{ "func": "CheckVar", "amap": "bms" }]}
#                      ]
#                  }
#              }
#          }
#          "/dbi/controls/bms": {
#              "DemoChargeCurrent": {
#                  "value": 0.0,
#                  "dbiSet": false
#              }
#          }

fSend=/usr/local/bin/fims/fims_send

########################################################
#             Helper Functions
########################################################

terminate()
{
    echo
    echo "Quitting the program..."
    sh ~/ess_controller/scripts/run_scripts/ess_stop.sh
    exit
}

# Helper function that waits for user input before proceeding to next test case
commandPrompt()
{
    echo "Press 'q' to quit. Press any other key to continue"
    while [ true ] ; do
        read -n 1 k <&1
        if [ $k = q ]; then
            terminate ;
        else
            echo
            echo "Continuing...";
            echo
            break ;
        fi
    done
}

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_summary_r or /components/catl_mbmu_summary_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# Helper function for doing a fims get
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_summary_r or /components/catl_mbmu_summary_r/mbmu_max_cell_voltage)
fimsGet()
{
    $fSend -m get -u $1 -r /me | jq
}

# Checks the current status of the dbi and the ess controller assetVar that references the dbi
checkState()
{
    echo "Checking current status of dbi..."
    sleep 2
    echo -n "getting /controls/bms:DemoChargeCurrent from ess controller            " && fimsGet "/ess/full/controls/bms/DemoChargeCurrent"
    sleep 2
    echo -n "getting /dbi/controls/bms:DemoChargeCurrent from ess controller        " && fimsGet "/ess/full/dbi/controls/bms/DemoChargeCurrent"
    sleep 2
    echo -n "getting /dbi/ess_controller/controls/bms:DemoChargeCurrent from dbi    " && fimsGet "/dbi/ess_controller/controls/bms"
}

########################################################
#              Test Cases
########################################################

echo "Test: Check DBI variables"
sleep 2
echo "Upcoming test: Test 1 - Check DBI (without ESS Controller running)..."

# Read user input to continue or quit
commandPrompt

echo "Starting up mongod, influxd, fims_server, and dbi..."
sleep 2
# Start mongod, influxd, fims_server, dbi in background before continuing
(sh ~/ess_controller/scripts/run_scripts/ess_run_with_dbi.sh > /dev/null 2>&1)
echo "Startup complete."
sleep 2

echo "Test 1 - Check DBI (without ESS Controller running)..."
echo -e "Expectations: DBI variable should be retrievable.\n"
sleep 3
echo -n "removing /dbi/ess_controller/controls/bms:DemoChargeCurrent from dbi   " && $fSend -m del -u /dbi/ess_controller/controls/bms -r /me
sleep 2
echo -n "initializing /dbi/ess_controller/controls/bms:DemoChargeCurrent        " && fimsSet "/dbi/ess_controller/controls/bms" '{"DemoChargeCurrent":{"value":123,"UpdateTimeCfg":6,"resetChange":false,"actions":{"onSet":[{"func":[{"func":"CheckVar","amap":"bms"}]}]}}}' 
sleep 2
echo -n "getting /dbi/ess_controller/controls/bms:DemoChargeCurrent from dbi    " && fimsGet "/dbi/ess_controller/controls/bms"

sleep 2
echo "Test 1 complete"
sleep 1
echo

echo "Upcoming test: Test 2 - Check DBI (with ESS Controller running)..."
commandPrompt

echo "Starting ESS Controller..."
(~/ess_controller/build/release/ess_controller ~/ess_controller/configs/ess_controller/ > /dev/null 2>&1 &)
echo "ESS Controller is now running."
sleep 2

echo "Test 2 - Check DBI (with ESS Controller running)..."
echo -e "Expectations: The DBI variable should be the same as the ESS Controller variable referencing the DBI\n"
sleep 3
checkState
sleep 2
echo "Test 2 complete"
sleep 1
echo

echo "Upcoming test: Test 3 - Set assetVar value to update DBI..."
commandPrompt

echo "Test 3 - Set assetVar value to update DBI..."
echo -e "Expectations: Changing the value of the assetVar should trigger a fims set and update the DBI\n"
sleep 3
echo -n "setting /controls/bms:DemoChargeCurrent to 35                          " && fimsSet "/ess/full/controls/bms" '{"DemoChargeCurrent":{"value":35}}'
sleep 2
checkState
sleep 2
echo "Test 3 complete"
sleep 1
echo

echo "Upcoming test: Test 4 - Set to DBI through ESS Controller..."
commandPrompt

echo "Test 4 - Set to DBI through ESS Controller..."
echo -e "Expectations: Setting to DBI through ESS Controller should update the DBI and update the assetVar referencing the DBI\n"
sleep 3
echo -n "setting /dbi/controls/bms:DemoChargeCurrent to 13                      " && fimsSet "/ess/dbi/controls/bms" '{"DemoChargeCurrent":{"value":13}}'
sleep 2
checkState
sleep 2
echo "Test 4 complete"
sleep 1
echo

echo "Upcoming test: Test 5 - Set assetVar value and param to update DBI..."
commandPrompt

echo "Test 5 - Set assetVar value and param to update DBI..."
echo -e "Expectations: Changing the value and parameter(s) of the assetVar should trigger a fims set and update the DBI\n"
sleep 3
echo -n "setting /dbi/controls/bms:DemoChargeCurrent value and params           " && fimsSet "/ess/dbi/controls/bms" '{"DemoChargeCurrent":{"value":123,"UpdateTimeCfg":3,"NewParam":"NewVal"}}'
sleep 2
checkState
sleep 2
echo "Test 5 complete"
sleep 1
echo

echo "Upcoming test: Test 6 - Set assetVar param to update DBI without setting dbiSet to true..."
commandPrompt

echo "Test 6 - Set assetVar param to update DBI without setting dbiSet to true..."
echo -e "Expectations: Changing the parameter(s) of the assetVar should not trigger a fims set and update the DBI if dbiSet is false\n"
sleep 3
echo -n "setting /dbi/controls/bms:DemoChargeCurrent params           " && fimsSet "/ess/dbi/controls/bms" '{"DemoChargeCurrent":{"value":123,"UpdateTimeCfg":6,"AnotherNewParam":"AnotherNewVal"}}'
sleep 2
checkState
sleep 2
echo "Test 6 complete"
sleep 1
echo

echo "Upcoming test: Test 7 - Set assetVar param to update DBI while setting dbiSet to true..."
commandPrompt

echo "Test 7 - Set assetVar param to update DBI while setting dbiSet to true..."
echo -e "Expectations: Changing the parameter(s) of the assetVar should not trigger a fims set and update the DBI if dbiSet is true\n"
sleep 3
echo -n "setting /dbi/controls/bms:DemoChargeCurrent params           " && fimsSet "/ess/dbi/controls/bms" '{"DemoChargeCurrent":{"value":123,"UpdateTimeCfg":6,"AnotherNewParam":"AnotherNewVal","dbiSet":true}}'
sleep 2
checkState
sleep 2
echo "Test 7 complete"
sleep 1
echo

echo "Upcoming test: Test 8 - Toggle dbi update..."
commandPrompt

echo "Test 8 - Toggle dbi update..."
echo -e "Expectations: The dbi containing the assetVar data should no longer be updated if EnableDbiUpdate is false. If EnableDbiUpdate is true, dbi will continue to be updated.\n"
sleep 3
echo -n "disable dbi update for /controls/bms:DemoChargeCurrent       " && fimsSet "/ess/controls/bms" '{"DemoChargeCurrent":{"value":123,"EnableDbiUpdate":false}}'
sleep 2
checkState
sleep 2
commandPrompt
echo -n "setting /dbi/controls/bms:DemoChargeCurrent params           " && fimsSet "/ess/dbi/controls/bms" '{"DemoChargeCurrent":{"value":12,"UpdateTimeCfg":8,"dbiSet":true}}'
sleep 2
checkState
sleep 2
commandPrompt
echo -n "enable dbi update for /controls/bms:DemoChargeCurrent        " && fimsSet "/ess/controls/bms" '{"DemoChargeCurrent":{"value":12,"EnableDbiUpdate":true}}'
sleep 2
checkState
sleep 2
echo "Test 8 complete"
sleep 1
echo

echo "dbi test cases complete."

terminate