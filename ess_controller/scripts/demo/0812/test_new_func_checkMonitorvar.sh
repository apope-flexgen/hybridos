#!/bin/sh
# test for checkMonitorVar function
# 1<<12 = 4096
# 2<<12 = 8192
# 3<<12 = 12288
# 4<<12 = 16384
# 5<<12 = 20480
SYS=/ess

echo show functions available
/usr/local/bin/fims/fims_send -m get -u $SYS/naked/system/functions -r /$$| jq

echo set up an assetVar to monitor called funAv2
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
{
 "funAv2":{
     "value":1234
     }
 
}' | jq

#  "funAv":{
#      "value":-1,
#      "operation":"+",
#      "numVars":3,
#      "variable1": "/test/pcs:number2",
#      "variable2": "/test/pcs:number3",
#      "variable3": "/test/pcs:number4"
#      }

# "enablef":{"value":false},
# "enablet":{"value":true},
## "number2":{"value":2},
# "number3":{"value":3},
# "number4":{"value":4},

echo set up a trigger for a function to check the value of funAv2
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
{
 "test_func":{
     "value":0, 
     "actions":{"onSet":[{    
                    "func": [
                        {"func": "CheckMonitorVar","inAv":"/test/pcs:funAv2"}
            ]}]}}
}' | jq

#                        ,{"enable":"/test/pcs:enablef","func": "CheckMonitorVar","funAv":"/test/pcs:funAvf"}
#                        ,{"enable":"/test/pcs:enablet","func": "CheckMonitorVar","funAv":"/test/pcs:funAvt"}

echo take a look at the monitored assetVar
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs/funAv2 | jq

echo now trigger the function to run the checks
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_func":{"value":2}}'


echo take another look at the monitored assetVar
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs/funAv2 | jq

echo look at the /test/pcs/  assetVars
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs | jq

# expected output
# {
#   "/talarms/module_1": {
#     "sys_warn": {
#       "value": "(HVAC) High input voltage"
#     }
#   },
#   "/talarms/pcs": {
#     "sys_warn": {
#       "value": "(HVAC) High output temp"
#     }
#   }
# }

