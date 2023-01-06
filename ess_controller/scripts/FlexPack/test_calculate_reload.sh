#!/bin/sh
#
# this object of this script is to demonstrate the CalculateVar operation.
# This is used when you need to run calculations on variables from a config script.
# Normally this is done by the wake_monitor system. Bit of a relic of the early days of the design but it works well
#    So we'll maintain it forward. 
@
# In this case we are going to calculate power as a product of volts * amps  
#
# first we create our two variables  and our power result 


#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex '
#{
#    "sbmu_current": 120,
#    "sbmu_voltage": 1350,
#    "sbmu_power": 0
#
#}'


#MYVAL=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex/sbmu_voltage `

#echo "myval = $MYVAL"
#(( MYVAL = MYVAL + 4 ))
#echo "myval 2 = $MYVAL"
#
# then we'll set up some values
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_voltage 123.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 123.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_power 0

#Lets look at what we have so far
echo 
echo " variables so far"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex  | jq

# next we'll set up a calculate action 

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex '
{
    "calc_power":{
        "aman":"/flex",
        "value": 0,
        "numVars": 2,
        "variable1": "sbmu_voltage",
        "variable2": "sbmu_current",
        "scale": 1,
        "operation": "*",
        "ifChanged":false, 
        "actions":{
            "onSet":[                     
                {
                    "func":[
                          {
                               "func":"CalculateVar",
                               "amap":"flex"
                         }],
                     "remap":[
                          {
                            "uri":"/status/flex:sbmu_power",
                            "amap":"flex"
                          }]
                }
            ]}            
        }
}'
echo 
echo " operations so far"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/operations/flex  | jq

# now when we set the value for calc_power we should perform the calculation
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_voltage 1356.4
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 100

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power 1356.4

echo " operations now"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/operations/flex  | jq

echo " variables now"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex  | jq

#echo " all done"
#exit

# but we need to use real input and output data 
#


# well in the real system things are done using wake monitor vars


echo "get links for current and voltage"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_current | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_voltage | jq

echo "reset link for current"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/links/flex/sbmu_current '{"value":"/components/sbmu:system_current"}' | jq

echo "get links for current and voltage"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_current | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/links/flex/sbmu_voltage | jq

echo "get function reloads"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/reload/flex | jq

echo "force a function reload"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/reload/flex/CalculateVar_calc_power 1 | jq

echo "get function reloads"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/reload/flex | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/flex/sbmu_current 101

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power '{"value":-135.4}'

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex | jq

echo " Now change the new linked var"

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/components/sbmu/system_current  240  | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/operations/flex/calc_power 135.4

/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/status/flex | jq

exit
echo "get status"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status/flex | jq

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
# {
#   "SendDb":{"value":1,
#             "db":"pirates",
#             "measure":"test_meas",
#             "table":"/system/dbtest"
#            }
# }'



# "cooling_temp_setting":{"temp":0,
# "actions":{"onSet":[{
# "func":[{"func":"ChangeCoolingTemp", "amap":"flex"}]
# }]}},
# "cooling_temp_upper_limit":{"temp":0, 
# "actions":{"onSet":[{
# "func":[{"func":"SetTempUpperLimit", "amap":"flex"}]
# }]}},
# "ht_warning_setting":{"temperature":0,
# "actions":{"onSet":[{
# "func":[{"func":"CheckTemp", "amap":"flex"}],
# "enum":[
#    {"shift": 0, "mask": 65536, "inValue": 3, "uri": "/system/ac1:temp", "outValue": "too high"},
#    {"shift": 0, "mask": 65536, "inValue": 4, "uri": "/system/ac1:temp", "outValue": "okay"}
# ]
# }]}},
# "lt_warning_setting":{"temperature":0,
# "actions":{"onSet":[{
# "func":[{"func":"CheckTemp", "amap":"flex"}],
# "enum":[
#    {"shift": 0, "mask": 65537, "inValue": 4, "uri": "/system/ac1:temp", "outValue": "okay"},
#    {"shift": 0, "mask": 65537, "inValue": 5, "uri": "/system/ac1:temp", "outValue": "too low"}
# ]
# }]}}
# }'


#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":1}}' 
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":2}}' 
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"cooling_temp_setting":{"temp":435}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"cooling_temp_upper_limit":{"temp":75}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"remote_ac_on":{"value":1}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"ht_warning_setting":{"temperature":3}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/components/bms_hs/ht_warning_setting | jq

#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/components/bms_hs '{"lt_warning_setting":{"temperature":5}}'
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/ac1 | jq
