#!/bin/sh
# simple test for the enum with msb's
# 1<<12 = 4096
# 2<<12 = 8192
# 3<<12 = 12288
# 4<<12 = 16384
# 5<<12 = 20480
SYS=/flex
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
{
 "test_remap":{"value":0, 
     "amap":"ess",
     "actions":{"onSet":[{    
                    "remap": [
                        {"uri": "/test/pcs:remap1"},
                        {"ignoreValue":234,"uri": "/test/pcs:remap1.1"},
                        {"inValue":234,"uri": "/test/pcs:remap2"},
                        {"uri": "/test/pcs:remap3",  "outValue": "ItWorked"},
                        {"uri": "/test/pcs:remap4",  "outTime": 2.0}
            ]}]}}
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_remap":{"value":2}}'

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

