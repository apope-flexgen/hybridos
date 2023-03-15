#!/bin/sh
# client config/dnp3_scale_sign_test_client.json
# server  config/dnp3_scale_sign_test_server.json
# run this on the server
#
# on the client run
# watch '/usr/local/bin/fims/fims_send -m get -u /local_client/assets/ess/test -r /$$ | jq'

VAL=1234.678
if [ $# > 1 ] ; then
  VAL=$1
fi

echo sending $VAL



/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_binary_allscale_neg_1  "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_binary_clscale_neg_1   "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_binary_svscale_neg_1   "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_binary_scale_1         "{\"value\":$VAL}"



