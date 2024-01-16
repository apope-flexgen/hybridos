#!/bin/sh
echo "set up a variable called /components/bms_hs:remote_ac_on"
echo " this used an onSet action to set /system/ac1:state to running or stopped based on a 1 or 2 value from /components/bms_hs:remote_ac_on"
/usr/local/bin/fims/fims_send -m set -u /flex/components/bms_hs '
     {"remote_ac_on":{"value":0, 
             "actions":{"onSet":[{"enum": [
                                          {"shift": 0, "mask": 65535, "inValue": 1, "uri": "/system/ac1:state", "outValue": "running"},
                                          {"shift": 0, "mask": 65535, "inValue": 2, "uri": "/system/ac1:state", "outValue": "stopped"}            
                        ]
              }]}}
     }'

#        "func":[{"func":"TurnOnRemoteAc","amap":"flex"}],
#        {"shift": 0, "mask": 65535, "inValue": 1, "uri": "/system/ac1:state", "outValue": "running"},

echo check result

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_hs | jq

echo send value 1 to /components/bms_hs:remote_ac_on
/usr/local/bin/fims/fims_send -m pub  -u /components/bms_hs '{"remote_ac_on":{"value":1}}' 

echo ; echo 
echo check result
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

echo send value 2 to /components/bms_hs:remote_ac_on
/usr/local/bin/fims/fims_send -m pub  -u /components/bms_hs '{"remote_ac_on":{"value":2}}' 

echo ; echo 
echo check result
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq
