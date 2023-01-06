#!/bin/sh

echo "Notes on BmsPowerOn, resetChange and  ifChanged"
echo "In the current config, BmsPowerOn defaults in this state."

cd /usr/local/bin/fims

./fims_send -m get -r /$$ -u /ess/full/status/bms/BMSPowerOn | jq                      {
#   "BMSPowerOn": {
#     "value": "Off Ready",
#     "AlarmTime": 5,
#     "AlarmTimeout": 5,
#     "EnableAlert": true,
#     "EnableConditionCheck": false,
#     "FaultTime": 7,
#     "FaultTimeout": 7,
#     "RecoverTime": 4,
#     "RecoverTimeout": 4,
#     "Type": "string",
#     "conditionType1": "string",
#     "conditionVal1_1": "Ready",
#     "conditionVal2_1": "Running",
#     "conditionVar1": "SystemState",
#     "expectedVal1": "On Ready",
#     "numConditionVars": 1,
#     "numConditions1": 2,
#     "numExpectedVals": 1,
#     "resetChange": false,
#     "seenAlarm": false,
#     "seenFault": false,
#     "seenReset": false,
#     "tLast": 13.721296000003349,
#     "actions": {
#       "onSet": [
#             {
#             "func": [
#                 {"amap": "ess","func": "LogInfo"},
#                 {"amap": "ess","func": "LogIt","inValue": "Off Ready"},
#                 {"amap": "ess","func": "LogIt","inValue": "On Fault"},
#                 {"amap": "ess","func": "LogIt","inValue": "Off Fault"}
#                 ]
#             },
#             {
#             "xfunc": [
#                 {"amap": "bms","func": "CheckMonitorVar"}
#           ]
#         }
#       ]
#     }
#   }
# }

echo "( you can ignore the disabled 'xfunc')"

# As is, this operation will cause the following functions to Run
#                 {"amap": "ess","func": "LogInfo"},
#                 {"amap": "ess","func": "LogIt","inValue": "Off Ready"},
#                 {"amap": "ess","func": "LogIt","inValue": "On Fault"},
#                 {"amap": "ess","func": "LogIt","inValue": "Off Fault"}

# the "resetChanged:false" flag is used to tell the system to run these functions every time 
# we get an input on /status/bms/BMSPowerOn

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}'
./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}'

#produces  two outputs 

#./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}
# runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
# fims_send, Socket 5, thread 1: closed or error reading.
# Exit thread 1

# ./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
# fims_send, Socket 6, thread 1: closed or error reading.
# Exit thread 1

echo " Given that the 'resetChanged' flag has been defined we have to turn on the ifChanged logic"

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"ifChanged": true}}'

echo "We also have to reenable the resetChange operation"
echo "" to cause the change , or lack of change,  to be detected in future writes to BMSPowerOn."

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"resetChange": true}}'


echo "Now repeated sends of the same value do Not cause the onSet functions to trigger after the first one 

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}'
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
#  runActFuncfromCj >> NOT running Function  [LogIt->0x7f7361cb1c20] for an asset manager please fix [ess]
# fims_send, Socket 6, thread 1: closed or error reading.
# Exit thread 1

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}'
# fims_send, Socket 5, thread 1: closed or error reading.
# Exit thread 1

./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSPowerOn":{"value":"On Ready"}}'
# fims_send, Socket 6, thread 1: closed or error reading.
# Exit thread 1





