#!/bin/bash

# This demo will cover DBI variable storage. Specifically, how ESS Controller variables can be stored
# in the database through DBI

# Helper function that waits for user input before proceeding
function commandPrompt()
{
    echo "Press 'q' to quit. Press any other key to continue"
    while [ true ] ; do
        read -n 1 k <&1
        if [ $k = q ]; then
            echo
            echo "Quitting the program..."
            exit ;
        else
            clear
            break ;
        fi
    done
}

function setup()
{
    fims_send -m del -u /dbi/ess_controller/bms_saved_variables_demo
    fims_send -m set -u /ess/components/bms_info '
    {
        "accumulated_charge_energy": 0,
        "accumulated_discharge_energy": 0
    }'

    fims_send -m set -u /ess/dbi/bms '
    {
        "UpdateChargeEnergy": {
            "value": 0,
            "document": "bms_saved_variables_demo",
            "numVars": 2,
            "variable1": "/components/bms_info:accumulated_charge_energy",
            "variable2": "/components/bms_info:accumulated_discharge_energy",
            "actions": {
                "onSet": [{"func": [{"func": "UpdateToDbi","amap": "bms"}]}]
            } 
        }
    }'
}

echo
echo -e "Update Variable(s) to DBI Demo:"
echo -e "For this demo, we'll go over how to update variable(s) to the database through DBI"
echo
commandPrompt
setup
echo "Here is a variable responsible for updating one or more variables to the database:" 
echo "fims_send -m get -r /me -u /ess/full/dbi/bms/UpdateChargeEnergy | jq" && fims_send -m get -r /me -u /ess/full/dbi/bms/UpdateChargeEnergy | jq
echo
echo "Note the following fields:"
echo -e "\t - document means we are storing our variables in this document in /dbi/ess_controller"
echo -e "\t - includeMetaData means we want to include the value, parameters, and actions for variable storage. This is default to false, meaning we're only including the value"
echo -e "\t - updateTimeout means the maximum amount of time to wait before sending an update request. This time is useful when used with a wake_monitor"
echo -e "\t - currUpdateTime means the remaining amount of time before sending an update request. This time is useful when used with a wake_monitor"
echo -e "\t - numVars means the number of variables to include for variable storage"
echo -e "\t - variable[id] means the name of the variable we are updating to the database"
echo
echo "In the example above, we are using /dbi/bms:UpdateChargeEnergy to update accumulated_charge_energy and accumulated_discharge_energy to the database."
echo "/dbi/bms:UpdateChargeEnergy will run UpdateToDbi function in response to a value set, which can be triggered by a remap, enum, or other configuration actions defined in other variables."
echo
commandPrompt
echo "Let's take a look at the current documents in the database:"
echo "fims_send -m get -r /me -u /dbi/ess_controller/show_documents | jq" && fims_send -m get -r /me -u /dbi/ess_controller/show_documents | jq
echo
echo "Notice here that we have a bms_saved_variables_demo document. This document will contain both accumulated_charge_energy and accumulated_discharge_energy in #components#bms_info"
echo -e "\t - We are using # as the placeholder for / since keys should not contain /. By doing so, we are able to update individual variables under a uri"
echo -e "\t - On system startup, we can load the document data the same way config files are loaded. Load instructions can be defined in a config load file (10.1)."
echo -e "\t - Any placeholder (#) will be replaced with / on config load"
echo
commandPrompt
echo "Let's check what is in the document:"
echo "fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq" && fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq
echo
echo "And what we see here are our two variables: accumulated_charge_energy and accumulated_discharge_energy."
echo
commandPrompt
echo "Let's demonstrate variable storage again. First, let's update accumulated_charge_energy and accumulated_discharge_energy:"
echo "fims_send -m set -u /ess/components/bms_info/accumulated_charge_energy 21"    && fims_send -m set -u /ess/components/bms_info/accumulated_charge_energy 21
echo "fims_send -m set -u /ess/components/bms_info/accumulated_discharge_energy 50" && fims_send -m set -u /ess/components/bms_info/accumulated_discharge_energy 50
echo
commandPrompt
echo "To update our variables to the database, we'll set to /dbi/bms:UpdateChargeEnergy so that we run UpdateToDbi function."
echo "fims_send -m set -u /ess/dbi/bms/UpdateChargeEnergy 0" && fims_send -m set -u /ess/dbi/bms/UpdateChargeEnergy 0
echo
echo "Now, let's check what is in the document now:"
echo "fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq" && fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq
echo
echo "And what we see here are our updated variables."
echo
commandPrompt
echo "Now, let's update another variable to the same document. This time, we'll include the variable's value, parameters, and actions."
fims_send -m set -u /ess/status/bms '
{
    "SOC": {
        "value": 0,
        "actions": {
            "onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms"}]}]
        } 
    }
}' | jq
fims_send -m set -u /ess/status/bms/SOC 0

fims_send -m set -u /ess/dbi/bms '
{
    "UpdateSOC": {
        "value": 0,
        "document": "bms_saved_variables_demo",
        "numVars": 1,
        "variable1": "/status/bms:SOC",
        "actions": {
            "onSet": [{"func": [{"func": "UpdateToDbi","amap": "bms"}]}]
        } 
    }
}' | jq

echo
echo "Here is the variable we want to update to the database:"
echo "fims_send -m get -r /me -u /ess/full/status/bms/SOC | jq" && fims_send -m get -r /me -u /ess/full/status/bms/SOC | jq
echo
echo "Here is a new variable responsible for sending update requests to DBI:" 
echo "fims_send -m get -r /me -u /ess/full/dbi/bms/UpdateSOC | jq" && fims_send -m get -r /me -u /ess/full/dbi/bms/UpdateSOC | jq
echo
echo "And here is the document data:"
echo "fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq" && fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq
echo
echo "Notice that our SOC variable is in the document, but it only contains the value"
echo
commandPrompt
echo "We'll set /dbi/bms:UpdateSOC@includeMetaData to true and then set /dbi/bms:UpdateSOC so that /status/bms:SOC's parameters and actions are included for variable storage."
echo "fims_send -m set -u /ess/dbi/bms/UpdateSOC@includeMetaData true" && fims_send -m set -u /ess/dbi/bms/UpdateSOC@includeMetaData true
echo "fims_send -m set -u /ess/dbi/bms/UpdateSOC 0" && fims_send -m set -u /ess/dbi/bms/UpdateSOC 0
echo
commandPrompt
echo "Let's check what is in the document again:"
echo "fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq" && fims_send -m get -r /me -u /dbi/ess_controller/bms_saved_variables_demo | jq
echo
echo "And what we see here are our updated variables."
echo
echo "Demo complete"
exit 0
