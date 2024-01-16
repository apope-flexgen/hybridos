echo pub operations
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control/pubs  | jq 

echo set up a test var

#wait_pause
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/site/flex_ls/testvar 1  | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/site/flex_hs/testvar 1  | jq
#{
#  "testvar": 1
#}
#wait_pause
