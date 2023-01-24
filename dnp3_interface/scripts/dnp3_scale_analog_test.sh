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

/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_noscale "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_noscale "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_clscale_2.5 $"{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_clscale_2.5 "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_svscale_2.5 "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_noscale "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_default_noscale "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_noscale "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_clscale_2.5 "{\"value\":$VAL}"
/usr/local/bin/fims/fims_send -m pub -u /assets/ess/test/test_30Var5_svscale_2.5 "{\"value\":$VAL}"



