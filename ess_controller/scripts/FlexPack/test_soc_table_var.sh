#!/bin/sh 

# Tests tablevar 
#
# Note:   Before running this test script, make sure setMonitorList2(*am->vmap, "bms","wake_monitor")
#         is added somewhere in src/ess_controller.cpp. Also, make sure "/schedule/wake_monitor/bms" is defined
#         somewhere in config file (ex.: bms manager)
#
# Example: In bms_manager.json: 
#          "/schedule/wake_monitor/bms": {
#              "/status/bms:pChargeLimit": { "enable": true, "rate":0.1, "func":"CheckTableVar"}
#          },
#          "/status/bms": {
#              "pChargeLimit": {
#                  "value": 0,
#                  "tableName": "p_charge_tbl.csv",
#                  "tableVar1": "mbmu_soc",
#                  "tableVar2": "mbmu_avg_cell_temperature"
#               }
#          }

fSend=/usr/local/bin/fims/fims_send
bin_dir=/usr/local/bin

########################################################
#             Helper Functions
########################################################

# Helper function that waits for user input before proceeding to next test case
commandPrompt()
{
    echo "Press 'q' to quit. Press any other key to continue"
    while [ true ] ; do
        read -n 1 k <&1
        if [ $k = q ]; then
            terminate
            exit ;
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

# Start running fims server and the ess controller before testing
startup()
{
    echo "Starting up fims server and ess controller..."
    #($bin_dir/fims/fims_server > /dev/null 2>&1 &)
    #(sleep 3s; ~/ess_controller/build/release/FlexPack ~/ess_controller/configs/flex_controller/ > /dev/null 2>&1 &)
    echo "Startup complete."
}

# Shutdown running processes
terminate()
{
    echo
    echo "Quitting the program..."
    #pkill FlexPack
    #pkill fims
    exit
}

########################################################
#             Initialization Functions
########################################################

setupTableVar()
{
    echo -e "Initializing table variable for soc test..."
    fimsSet "/status/bms": '{
              "pChargeLimit": {
                  "value": 0,
                  "actions":["onSet":{
                      "remap":[{"uri":"/status/bms:pChargeLimit@inputVal"}]},
                      "func":[{"func":"CheckTableVar","amap":flex"}]}
                  ]
               }
          }'
    fimsSet "/flex/status/bms" '{"pChargeLimit":0}'
    fimsSet "/flex/status/bms" '{"pChargeLimit":{"tableName":"p_charge_tbl.csv"}}'
    fimsSet "/flex/status/bms" '{"pChargeLimit":{"tableVar1":"mbmu_soc"}}'
    fimsSet "/flex/status/bms" '{"pChargeLimit":{"tableVar2":"mbmu_avg_cell_temperature"}}'
    fimsSet "/flex/reload/bms" '{"CheckTableVar_pChargeLimit":0}'

    sleep 2 # give enough time for setpoints to be initialized

    echo -e "Initialization complete."

    echo
}

########################################################
#              Test Cases
########################################################

echo "Test: Power limitation based on current values (ex.: soc and temperature)"
sleep 2
echo "Upcoming test: Test 1 - Check Power Limitation Table Variable..."

# Read user input to continue or quit
commandPrompt

startup
setupTableVar
sleep 2

echo -n "getting /reload/bms:CheckTableVar_pChargeLimit " && fimsGet "/flex/reload/bms/CheckTableVar_pChargeLimit"

echo "Test 1 - Test 1 - Check Power Limitation Table Variable..."
echo -e "Expectations: The power limitation table variable should be retrievable.\n"
echo -n "getting pChargeLimit                           " && fimsGet "/flex/full/status/bms/pChargeLimit"
sleep 2
echo "Test 1 complete"
sleep 1
echo
terminate
echo "Upcoming test: Test 2 - Update Power Limitation Table Variable..."
commandPrompt

echo "Test 2 - Test 2 - Update Power Limitation Table Variable..."
echo -e "Expectations: Table variable should be updated to a table value depending on current values (ex.: soc and temperature).\n"
echo -n "setting soc to 4%                              " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_soc":4}'
sleep 1
echo -n "setting temperature to -5 degrees Celsius      " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_avg_cell_temperature":-5}'
sleep 3
echo -n "getting pChargeLimit after set (expect: 0)     " && fimsGet "/flex/full/status/bms/pChargeLimit"
sleep 3
echo -n "setting soc to 13%                             " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_soc":13}'
sleep 1
echo -n "setting temperature to 18 degrees Celsius      " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_avg_cell_temperature":18}'
sleep 3
echo -n "getting pChargeLimit after set (expect: 0.5)   " && fimsGet "/flex/full/status/bms/pChargeLimit"
sleep 3
echo -n "setting soc to 95%                             " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_soc":95}'
sleep 1
echo -n "setting temperature to 25 degrees Celsius      " && fimsSet "/flex/components/catl_mbmu_summary_r" '{"mbmu_avg_cell_temperature":25}'
sleep 3
echo -n "getting pChargeLimit after set (expect: 1)     " && fimsGet "/flex/full/status/bms/pChargeLimit"
echo "Test 2 complete"
sleep 1
echo

echo "Power limitation based on current values (ex.: soc and temperature) test complete" 
terminate