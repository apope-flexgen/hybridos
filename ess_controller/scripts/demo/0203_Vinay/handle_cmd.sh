#!/bin/bash

# This demo will cover the command handler. Specifically, how ESS Controller control variable can be configured to send/verify commands
# to/from to an asset (ex.: BMS/PCS) as well as how control sequences can be configured

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
    # Setup control variable
    fims_send -m set -u /ess/controls/ess '
    {
        "CloseContactors": {
            "value": 0,
            "cmdVar": "/components/catl_ems_bms_rw:ems_cmd",
            "actions": {
                "onSet": [{"func": [{"func": "HandleCmd", "amap": "ess"}]}]
            }
        }
    }'

    # Setup other variables
    fims_send -m set -u /ess/components/catl_ems_bms_rw/ems_cmd 0

    # # Set up a monitor command
    # fims_send  -m set -u /ess/full/system/commands '
    # {
    #     "runEssMon": {
    #         "value":"test",
    #         "help": "run a wake_monitor list",
    #         "ifChanged":false, 
    #         "aname":"ess", 
    #         "mname": "wake_monitor", 
    #         "enabled":false, 
    #         "runChild":false,
    #         "actions":{"onSet":[{"func":[{"func":"RunMonitorList"}]}]}
    #     }
    # }'
    
    # # Setup wake_monitor variables
    # fims_send  -m set -u /ess/full/schedule/wake_monitor/ess '
    # {
    #     "/controls/bms:CloseContactors": { "enabled": true, "rate": 0.1, "amap": "ess", "func": "HandleCmd"}
    # }'

    # # Start wake_monitor task
    # fims_send  -m set -u /ess/full/system/commands '
    # {
    #     "runEssMon": {
    #         "value":"test",
    #         "enabled":true
    #     }
    # }'
}

echo
echo -e "Command Handler Demo:"
echo -e "For this demo, we'll go over how to configure control variable to send/verify commands to/from an asset (ex.: BMS/PCS)"
echo
commandPrompt
setup
echo "Here is a control variable that uses the command handler:"
echo "fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors" && fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors | jq
echo
echo "Note the following fields we'll be focusing on for this demo:"
echo -e "\t - triggerCmd means whether the send command operation should run"
echo -e "\t - cmdVar means the command variable to send commands to"
echo -e "\t - useExpr means whether expression evaluation is enabled/disabled"
echo -e "\t - expression means the expression that contains the conditions to meet before sending a command value"
echo -e "\t - numVars means the number of variables to include for variable storage"
echo -e "\t - variable[id] means the name of the variable we are "
echo
echo "For more information on the parameters for the command handler, refer to https://github.com/flexgen-power/ess_controller/wiki/System-Command-Handler#Parameters"
echo
echo "In the example above, we are using /controls/ess:CloseContactors to send a value out to /components/catl_ems_bms_rw:ems_cmd, which refers to a modbus register used in the BMS, for example."
echo "/controls/ess:CloseContactors will run HandleCmd function in response to a value set, which can be triggered by a remap, enum, or other configuration actions defined in other variables."
echo
commandPrompt
echo "Let's say that ems_cmd needs to receive a value of 2 to close contactors."
echo "Let's trigger HandleCmd by setting /controls/ess:CloseContactors to 2, which should send the value of 2 to /components/catl_ems_bms_rw:ems_cmd:"
echo "fims_send -m set -u /ess/controls/ess/CloseContactors 2" && fims_send -m set -u /ess/controls/ess/CloseContactors 2
echo
echo "Let's check CloseContactors again:"
echo "fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors" && fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors | jq
echo
echo "And let's check ems_cmd:"
echo "fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd" && fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd
echo
echo "Notice that ems_cmd is still set to 0, even though CloseContactors is set to 0."
echo "We need an extra step. To update ems_cmd, or any variable defined in cmdVar, we'll need to first set /controls/ess:CloseContactors@triggerCmd to true so that the command handler knows when to start the send/verify command routine."
echo -e "\t - triggerCmd is used because the command handler uses send/check command timeouts, which allows the ESS Controller to wait before sending/verifying commands or determining that the value cannot be sent/verified"
echo -e "\t - Timeouts can be defined when the command handler variable is defined in a scheduler or a wake_monitor"
echo
echo "So, we'll first set triggerCmd parameter to true, then set the command handler variable again, and finally check if the variable defined in cmdVar now received an update:"
echo "fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true" && fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true
echo "fims_send -m set -u /ess/controls/ess/CloseContactors 2" && fims_send -m set -u /ess/controls/ess/CloseContactors 2
echo "fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd" && fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd
echo
echo "And we can see now that ems_cmd has received a value of 2"
echo
commandPrompt
echo "We've demonstrated one simple way of using the command handler, but let's add a bit more complexity."
fims_send -m set -u /ess/components/catl_ems_bms_rw/ems_cmd 0
echo "Let's say we want to send a value of 2 to /components/catl_ems_bms_rw:ems_cmd if:"
echo -e "\t - The BMS DC contactors are open (ex.: /status/bms:DCClosed == false)"
echo -e "\t - The PCS is off (ex.: /status/pcs:SystemState == Off)"
echo
echo "We'll define our conditions in an expression parameter and the condition variables (or operands) in the variable parameter(s)."
echo -e "\t - If the evaluated expression is true, then the command handler is clear to update the variable defined in cmdVar"
echo -e "\t - Otherwise, the command handler will wait until an elapsed time (defined in sendCmdTimeout) before determining an unsuccessful send command event"
echo
echo "So, let's define our condition variables in the ESS Controller, if they don't already exist:"
echo "fims_send -m set -u /ess/status/bms '{\"DCClosed\":false}'" && fims_send -m set -u /ess/status/bms '{"DCClosed":false}'
echo "fims_send -m set -u /ess/status/pcs '{\"SystemState\":\"Init\"}'" && fims_send -m set -u /ess/status/pcs '{"SystemState":"Init"}'
echo
echo "Then, let's update /controls/ess:CloseContactors to contain our condition variables and expression:"
echo "Setting /controls/ess:CloseContactors..."
fims_send -m set -r /me -u /ess/controls/ess '
{
    "CloseContactors": {
        "value": 0,
        "cmdVar": "/components/catl_ems_bms_rw:ems_cmd",
        "numVars": 2,
        "variable1": "/status/bms:DCClosed",
        "variable2": "/status/pcs:SystemState",
        "useExpr": true,
        "expression": "not {1} and {2} == Off",
        "actions": {
            "onSet": [{"func": [{"func": "HandleCmd", "amap": "ess"}]}]
        }
    }
}' | jq
echo
echo "Note the following:"
echo -e "\t - {1} = /status/bms:DCClosed"
echo -e "\t - {2} = /status/pcs:SystemState"
echo
commandPrompt
echo "Let's now try sending a value of 2 and see if ems_cmd receives a value of 2 again:"
echo "fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true" && fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true
echo "fims_send -m set -u /ess/controls/ess/CloseContactors 2" && fims_send -m set -u /ess/controls/ess/CloseContactors 2
echo "fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd" && fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd
echo
echo "ems_cmd is not updated to 2, which means our conditions are not met. So, we'll need to check our condition variables:"
echo "fims_send -m get -r /me -u /ess/status/bms/DCClosed" && fims_send -m get -r /me -u /ess/status/bms/DCClosed
echo "fims_send -m get -r /me -u /ess/status/pcs/SystemState" && fims_send -m get -r /me -u /ess/status/pcs/SystemState
echo "fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors | jq" && fims_send -m get -r /me -u /ess/full/controls/ess/CloseContactors | jq
echo
echo "As we can see here, /status/pcs:SystemState is Init, but our expression expects SystemState to be Off."
echo
echo "So, let's set SystemState to Off and try sending a value of 2 to ems_cmd:"
echo "fims_send -m set -u /ess/status/pcs '{\"SystemState\":\"Off\"}'" && fims_send -m set -u /ess/status/pcs '{"SystemState":"Off"}'
echo "fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true" && fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true
echo "fims_send -m set -u /ess/controls/ess/CloseContactors 2" && fims_send -m set -u /ess/controls/ess/CloseContactors 2
echo "fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd" && fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd
echo 
echo "And as we can see, ems_cmd now contains a value of 2"
commandPrompt
echo "Next, let's also require that we check if the BMS contactors did actually close after we have verified that ems_cmd received the command."
echo -e "\t - /status/bms:DCClosed == true"
echo -e "\t - If /status/bms:DCClosed == false, we'll send an alarm"
echo
echo "We'll define a new variable called /controls/ess:VerifyContactorsClosed in the same way /controls/ess:CloseContactors is defined in the previous example."
echo "Setting /controls/bms:VerifyContactorsClosed..."
fims_send -m set -r /me -u /ess/controls/ess '
{
    "VerifyContactorsClosed": {
        "value": 0,
        "numVars": 1,
        "variable1": "/status/bms:DCClosed",
        "useExpr": true,
        "expression": "{1}",
        "actions": {
            "onSet": [{"func": [{"func": "HandleCmd", "amap": "ess"}]}]
        }
    }
}' | jq
echo
echo "Notice that we don't have cmdVar defined since we don't need to send commands out to a variable. We just want to check for state."
echo
echo "We'll also define a validation variable called /status/ess:VerifyContactorsClosedSuccess"
echo "Setting /status/ess:VerifyContactorsClosedSuccess..."
fims_send -m set -r /me -u /ess/status/ess '
{
    "VerifyContactorsClosedSuccess": {
        "value": false,
        "actions": {
            "onSet": [{
                "remap": [
                    {"inValue": false, "ifChanged": false, "uri": "/status/ess:close_contactors_failure", "outValue": "Failed to close contactors for BMS"}
                ]
            }]
        }
    }
}' | jq
echo
echo "Note:"
echo -e "\t - The validation variable is defined in /status and contains the name of the command handler followed by the Success suffix"
echo -e "\t - The validation variable would be used to perform remaps, enums, or other actions"
echo -e "\t - We can use the validation variable to send our alarm if the command handler reports an unsuccessful send/verify command event, for example"
echo -e "\t - For this demo, we'll just remap to a variable that contains the alarm message"
echo
echo "And we'll define /status/ess:close_contactors_failure, which will contain our alarm message if the command handler fails:"
echo "fims_send -m set -u /ess/status/ess '{\"close_contactors_failure\":\"Normal\"}'" && fims_send -m set -u /ess/status/ess '{"close_contactors_failure":"Normal"}'
commandPrompt
echo "Lastly, let's define one more validation variable, this time for /controls/bms:CloseContactors."
echo "This validation variable will be /status/ess:CloseContactorsSuccess"
echo "Setting /status/ess:CloseContactorsSuccess..."
fims_send -m set -r /me -u /ess/status/ess '
{
    "CloseContactorsSuccess": {
        "value": false,
        "actions": {
            "onSet": [{
                "remap": [
                    {"inValue": true, "ifChanged": false, "uri": "/controls/ess:VerifyContactorsClosed@triggerCmd", "outValue": true},
                    {"inValue": true, "ifChanged": false, "uri": "/controls/ess:VerifyContactorsClosed", "outValue": 0}
                ]
            }]
        }
    }
}' | jq
echo
echo "Note:"
echo -e "\t - We'll use this variable to trigger the command handler for VerifyContactorsClosed if the command handler for CloseContactors report a successful send/verify command event"
echo
commandPrompt
echo "Now that we have our new control variable and a validation variables, let's observe what happens when we try to send a command value of 2:"
echo "fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true" && fims_send -m set -u /ess/controls/ess/CloseContactors@triggerCmd true
echo "fims_send -m set -u /ess/controls/ess/CloseContactors 2" && fims_send -m set -u /ess/controls/ess/CloseContactors 2
echo "fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd" && fims_send -m get -r /me -u /ess/components/catl_ems_bms_rw/ems_cmd
echo "fims_send -m get -r /me -u /ess/full/status/ess/CloseContactorsSuccess | jq" && fims_send -m get -r /me -u /ess/full/status/ess/CloseContactorsSuccess | jq
echo "fims_send -m get -r /me -u /ess/full/status/ess/VerifyContactorsClosedSuccess | jq" && fims_send -m get -r /me -u /ess/full/status/ess/VerifyContactorsClosedSuccess | jq
echo "fims_send -m get -r /me -u /ess/status/ess/close_contactors_failure" && fims_send -m get -r /me -u /ess/status/ess/close_contactors_failure
echo
echo "Notice that /status/ess:close_contactors_failure contains the alarm value we specified in the remap action in /status/ess:VerifyContactorsClosedSuccess"
echo "This means that our ems_cmd received a value of 2, but the BMS did not close contactors (/status/ess:DCClosed == false). Hence, an alarm was reported"
echo
commandPrompt
echo "Demo complete. For more details on how to use and configure command handler, refer to the following documents:"
echo -e "\t - https://github.com/flexgen-power/ess_controller/wiki/System-Command-Handler"
echo -e "\t - https://github.com/flexgen-power/config_ess/wiki/Configuration-Recipes#Controls"
exit 0