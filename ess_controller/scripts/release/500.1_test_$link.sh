#!/bin/s
# load the bms_sim managers and template
#echo setup Link command
# /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
#         {"link":{"value":"test",
#                    "help": "give a single assetvar multiple names",
#                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunLinks"}]}]}}}'


# set up a test link
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/test/pcs '
                 {"pcs_val":{"value":1234.5,"$aopts":true, "$link":"/status/pcs:pcs_data"}}'

# /usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
#                 {"value":"bms", "pname":"flex", "aname":"bms"}'


echo set component and check status 

#/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/test/bms_val/bms_val 223344.5
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/test/pcs   | jq
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/status/pcs | jq 
echo "set value"
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/test/pcs/pcs_val 44.5
echo "result"
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/test/pcs | jq
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/status/pcs | jq 

# echo set status and check component
# /usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/status/bms/bms_soc  5432.1
# /usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/components/bms_info/bms_soc 

# echo check amap
# /usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/amap/bms   | jq
 


