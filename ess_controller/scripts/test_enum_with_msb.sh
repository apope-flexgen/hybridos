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
 "test_enum":{"value":0, 
     "amap":"ess",
     "actions":{"onSet":[{    
                    "enum": [
                        {"shift": 12, "mask": 15, "inValue": 0, "uri": "/test/pcs:aname",  "outValue": "/talarms/pcs"},
                        {"shift": 12, "mask": 15, "inValue": 1, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_1"},
                        {"shift": 12, "mask": 15, "inValue": 2, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_2"},
                        {"shift": 12, "mask": 15, "inValue": 3, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_3"},
                        {"shift": 12, "mask": 15, "inValue": 4, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_4"},
                        {"shift": 12, "mask": 15, "inValue": 5, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_5"},
                        {"shift": 12, "mask": 15, "inValue": 6, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_6"},
                        {"shift": 12, "mask": 15, "inValue": 7, "uri": "/test/pcs:aname",  "outValue": "/talarms/module_7"},
                        {"shift": 0, "mask": 4095, "inValue": 1, "uri":"/test/pcs:aname", "ruri": "sys_warn",  "outValue": "(HVAC) High input voltage"},                 
                        {"shift": 0, "mask": 4095, "inValue": 2, "uri":"/test/pcs:aname", "ruri": "sys_warn",  "outValue": "(HVAC) High output temp"},                 
                        {"shift": 0, "mask": 4095, "inValue": 3, "uri":"/test/pcs:aname", "ruri": "sys_warn",  "outValue": "(HVAC) High value3 "}                 
            ]}]}}
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_enum":{"value":2}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_enum":{"value":4096}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/pcs '
      {"test_enum":{"value":2}}'
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs | jq
echo just get aname
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs/aname | jq
echo get talarms
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/talarms| jq

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

