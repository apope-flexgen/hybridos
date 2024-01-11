#!/bin/s
# load the bms_sim managers and template


# set up a test link
echo set component pcs_max_charge_current
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/test_aname/pcs '
                 {"pcs_max_charge_current":{"value":1234.5,"$aopts":true,
                  "$aname":":am:pcs001","$amap":":am:pcs001:MaxChargeCurrent"}}
                  '

# /usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
#                 {"value":"bms", "pname":"flex", "aname":"bms"}'


echo " check status" 

/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/test_aname/pcs   | jq
echo "amap result"
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/amap | jq
echo "amap pcs001 result"
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/amap/pcs001 | jq


