#!/bin/sh
# basic script to test the following actions.
#"bitset"
#"enum"
#"remap"
#"func"
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

# first test "bitset"
#     "inValue", &inValue);
#            "uri", &uri);
#            "var", &var);
#            "outValue", &cjov);
# 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/ess '
{
    "test_bitset_1":{ 
        "value":false,
        "debug":1,
        "actions": {
            "onSet": [{
                "bitset": 
                    [
                        {"bit": 1,"uri": "/controls/pcs_test","var":"ctrlword_qmode","soloBit": true}
                    ]
            }]
        }
    },
    "test_bitset_2":{ 
        "value":false,
        "debug":1,
        "actions": {
            "onSet": [{
                "bitset": 
                    [
                        {"bit": 2,"uri": "/controls/pcs_test","var":"ctrlword_qmode","soloBit": true}
                    ]
            }]
        }
    }

}'
#response 
exp_resp='{"test_bitset_1":{"value":false,"debug":1,"actions":{"onSet":[{"bitset":[{"bit":1,"soloBit":true,"uri":"/controls/pcs_test","var":"ctrlword_qmode"}]}]}}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_bitset_1`
check_resp "setup action 1" $resp $exp_resp


exp_resp='{"test_bitset_2":{"value":false,"debug":1,"actions":{"onSet":[{"bitset":[{"bit":2,"soloBit":true,"uri":"/controls/pcs_test","var":"ctrlword_qmode"}]}]}}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_bitset_2`
check_resp "setup action 2" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_bitset_1":{ "value":true}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs_test/ctrlword_qmode`
exp_resp='{"ctrlword_qmode":{"value":1}}'
check_resp "send value 1 true" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_bitset_1":{ "value":false}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs_test/ctrlword_qmode`
exp_resp='{"ctrlword_qmode":{"value":0}}'
check_resp "send value 1 false" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_bitset_2":{ "value":true}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs_test/ctrlword_qmode`
exp_resp='{"ctrlword_qmode":{"value":2}}'
check_resp "send value 2 true" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_bitset_2":{ "value":false}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs_test/ctrlword_qmode`
exp_resp='{"ctrlword_qmode":{"value":0}}'
check_resp "send value 2 false" $resp $exp_resp


echo "$0 tests passed" >> tests.out


