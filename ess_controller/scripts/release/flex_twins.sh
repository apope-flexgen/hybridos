# flex_twins emulation layer
# p. wilshire
# 10/22/2021
# 10/27/2021
# run ess controller like this 
#   build/release/ess_controller -x -n flex
# the test script dots in this to run the config set up by 300.1
# it can be tricky to match exact responses so use grep -v to filter out stuff that changes regardless.
# also the responses will be in alpha order.
#
# basic script to test the following actions.
#"bitset"
#"enum"
#"remap"
#"func"
#"limits"

FimsDir=/usr/local/bin/
FimsName=/flex

#  send state 2,4,6  to /flex/fcomponents/bms_controls
# it passes them on to /components/bms_control/twins_state 
# 2 => 22
# 4 => 44
# 6 => 66

${FimsDir}fims_send -m set  -r /$$ -u ${FimsName}/full/fcomponents/bms_controls '
{
    "state":{ 
        "value":0,
        "debug":0,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"inValue": 2,"fims":"set", "uri": "/components/bms_control:twins_state","outValue": 22 },
                        {"inValue": 4,"fims":"set", "uri": "/components/bms_control:twins_state","outValue": 44 },
                        {"inValue": 6,"fims":"set", "uri": "/components/bms_control:twins_state","outValue": 66 }
                    ]                
            }]
        }
    }
}'
#  send state 202,404,606  to /flex/tcomponents/bms_controls
# it passes them on to /components/bms_control/ess_state 
# 202 => 2
# 404 => 4
# 606 => 6

${FimsDir}fims_send -m set  -r /$$ -u ${FimsName}/full/tcomponents/bms_controls '
{
    "state":{ 
        "value":0,
        "debug":0,
        "actions": {
            "onSet": [{
                "remap": 
                    [
                        {"inValue": 202,"fims":"set", "uri": "/components/bms_control:ess_state","outValue": 2 },
                        {"inValue": 404,"fims":"set", "uri": "/components/bms_control:ess_state","outValue": 4 },
                        {"inValue": 606,"fims":"set", "uri": "/components/bms_control:ess_state","outValue": 6 }
                    ]                
            }]
        }
    }
}'
