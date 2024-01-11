#!/bin/sh
# basic script to test the following actions.
#"limits"

echo "I am $0 and I am testing things"
echo "$0 tests running" >> tests.out

function check_resp()
{
    if [ "$2" = "$3" ] ; then
        echo "test $1 >>  it worked"
        echo "test $1 >>  worked" >> tests.out

    else
        echo "test $1 >> failed"
        echo "resp [$2]"
        echo "exp  [$3]"
        echo "test $1 >>  failed" >> tests.out
    fi 
}

# first test "limit"
#     "inValue", &inValue);
#            "uri", &uri);
#            "var", &var);
#            "outValue", &cjov);
# 
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/test/ess/limit_test" '{"low":-22, "high":22}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/ess '
{
    "test_limit_1":{
        "value":0,
        "debug":false,
        "enabled":true,
        "actions":{
            "onSet":[{
                "limits":[
                    {"low":  -25, "high": 25, "uri":"/system/limit:test"}
                ]
            }]
        }
    },
    "test_limit_2":{
        "value":0,
        "debug":false,
        "enabled":true,
        "actions":{
            "onSet":[{
                "limits":[
                    {"lowuri": "ess/test/ess/limit_test:low", "highuri": "ess/test/ess/limit_test:high", "uri":"/limit_test:test"}
                ]
            }]
        }
    }
}'

#response 
exp_resp='{"test_limit_1":{"value":0,"debug":false,"enabled":true,"actions":{"onSet":[{"limits":[{"high":25,"low":-25,"uri":"/system/limit:test"}]}]}}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_limit_1`
check_resp "setup action 1" $resp $exp_resp

exp_resp='{"test_limit_2":{"value":0,"debug":false,"enabled":true,"actions":{"onSet":[{"limits":[{"highuri":"ess/test/ess/limit_test:high","lowuri":"ess/test/ess/limit_test:low","uri":"/limit_test:test"}]}]}}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_limit_2`
check_resp "setup action 2" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set -u /ess/test/ess '{"test_limit_1":{"value":50}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_1`
exp_resp='25'
check_resp "send 1 value 50" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_1":{ "value":24}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_1`
exp_resp='24'
check_resp "send 1 value 24" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_1":{ "value":-50}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_1`
exp_resp='-25'
check_resp "send 1 value -50" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_1":{ "value":-24}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_1`
exp_resp='-24'
check_resp "send 1 value -24" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set -u /ess/test/ess '{"test_limit_2":{"value":50}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_2`
exp_resp='22'
check_resp "send 2 value 50" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_2":{ "value":20}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_2`
exp_resp='20'
check_resp "send 2 value 20" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_2":{ "value":-50}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_2`
exp_resp='-22'
check_resp "send 2 value -50" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_limit_2":{ "value":-20}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/test/ess/test_limit_2`
exp_resp='-20'
check_resp "send 2 value -20" $resp $exp_resp


echo "$0 tests passed" >> tests.out


