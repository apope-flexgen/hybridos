/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
{
  "SendDb":{"value":0,
    "ifChanged":false, 
    "actions":{"onSet":[{                     
         "func":[{"func":"SendDb","amap":"flex"}]
            }]}}
}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/dbtest '
{
  "int1": 1,
  "float1": 2.3,
  "string1": "four"
}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
{
  "SendDb":{"value":1,
            "db":"pirates",
            "measure":"test_meas",
            "table":"/system/dbtest"
           }
}'



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

echo "I got here"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/dbtest | jq

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
