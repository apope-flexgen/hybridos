#!/bin/sh
# start testing soc table...

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/pSoc '
                     {"value":0, "actions":{"onSet":[{"remap":[{"uri":"/status/bms:pSoc@prevSoc"}],
                                                      "func":[{"func":"CheckTableVar","amap":"flex"}]}]}}
                     ' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar1":"bms_soc"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar2":"bms_val"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableName":"soc_tbl.csv"}}' | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/bms_soc 35 | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/bms_val 0 | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"value":14}}' | jq
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq

exit

1019  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"value":21}}' | jq
 1020  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar2":"bms_var"}}' | jq
 1021  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"value":20}}' | jq
 1022  /usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/ | jq
 1023  /usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq
 1024  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"value":20}}' | jq
 1025  /usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq
 1026  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar2":"bms_var"}}' | jq
 1027  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableVar1":"bms_soc"}}' | jq
 1028  /usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq
 1029  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/bms_soc 35 | jq
 1030  /usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/status | jq
 1031  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/bms_var 0 | jq
 1032  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '{"pSoc":{"tableName":"soc_tbl.csv"}}' | jq
 1033  /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms/pSoc '{"value":0, "actions":{"onSet":[{"remap":[{"uri":"/status/bms:pSoc@prevSoc"}],"func":[{"func":"CheckTableVar","amap":"flex"}]}]}}' | jq
