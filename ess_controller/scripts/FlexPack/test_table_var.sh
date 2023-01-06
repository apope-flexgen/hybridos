#!/bin/sh
# start testing soc table...

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/pSoc '{"value":0, "actions":{"onSet":[{"remap":[{"uri":"/status/bms:pSoc@prevSoc"}],"func":[{"func":"CheckTableVar","amap":"flex"}]}]}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar1":"bms_soc"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar2":"bms_temp"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableName":"soc_tbl.csv"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"value":14}}' | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq
