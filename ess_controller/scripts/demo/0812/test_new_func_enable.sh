#!/bin/sh
# simple test for the enum with msb's
# 1<<12 = 4096
# 2<<12 = 8192
# 3<<12 = 12288
# 4<<12 = 16384
# 5<<12 = 20480
SYS=/flex

echo show functions available
/usr/local/bin/fims/fims_send -m get -u $SYS/naked/system/functions -r /$$| jq

echo
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
{
 "enablef":{"value":false},
 "enablet":{"value":true},
 "number2":{"value":2},
 "number3":{"value":3},
 "number4":{"value":4},
 "funAv":{
     "value":-1,
     "operation":"+",
     "numVars":3,
     "variable1": "/test/pcs:number2",
     "variable2": "/test/pcs:number3",
     "variable3": "/test/pcs:number4"
     }
 
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
{
 "test_func":{
     "value":0, 
     "amap":"ess",
     "enabled":true,
     "operation":"+",
     "numVars":2,
     "variable1": "/test/pcs:number2",
     "variable2": "/test/pcs:number3",
     "actions":{"onSet":[{    
                    "func": [
                        {"func": "CalculateVar"},
                        {"inValue":5,"func": "CalculateVar"},

                        {"enable":"/test/pcs:enablef","func": "CalculateVar"},
                        {"enable":"/test/pcs:enablet","func": "CalculateVar"},
                        {"func": "CalculateVar","funAv":"/test/pcs:nofunAv"},
                        {"func": "CalculateVar","funAv":"/test/pcs:funAv"}
            ]}]}}
}' | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_func":{"value":2}}'

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

