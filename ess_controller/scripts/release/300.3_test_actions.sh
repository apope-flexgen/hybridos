#!/bin/sh
# basic script to test the following actions.
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

# first test "enum"
#     "inValue", &inValue);
#            "uri", &uri);
#            "var", &var);
#            "outValue", &cjov);
# 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/ess '
{
    "test_enum_1":{ 
        "value":0,
        "debug":1,
        "actions":{
            "onSet":[{
                "enum":
                    [
                        {"shift": 0,"mask": 1, "inValue": 0,"uri": "/controls/enum_test:test_value","outValue": "VALUE_1_0"},
                        {"shift": 0,"mask": 1, "inValue": 1,"uri": "/controls/enum_test:test_value","outValue": "VALUE_1_1"},
                        {"shift": 0,"mask": 1, "inValue": 2,"uri": "/controls/enum_test:test_value","outValue": "VALUE_1_2"}
                    ]
            }]
        }
    },
    "test_enum_2":{
        "value":-1,
        "debug":true,
        "enabled":true,
        "defVal":"DefaultVal",
        "defUri":"/status/test:EnumTestDef",
        "actions":{
            "onSet": [{ 
                "enum":[
                        {"shift": 0, "mask": 3, "inValue": 0, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_0"},
                        {"shift": 0, "mask": 3, "inValue": 1, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_1"},
                        {"shift": 0, "mask": 3, "inValue": 2, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_2"}
                ]
            }]
        }
    }
}'
#response 
#exp_resp='{"test_enum_1":{"value":0,"debug":1,"actions":{"onSet":[{"enum":[{"shift":0,"mask":1,"inValue":0,"uri":"/controls/enum_test","outValue":"VALUE_1_0"},{"shift":0,"mask":1,"inValue":1,"uri":"/controls/enum_test","outValue":"VALUE_1_1"},{"shift":0,"mask":1,"inValue":2,"uri":"/controls/enum_test","outValue":"VALUE_1_2"}]}]}}}'
#exp_resp='{"test_enum_1":{"value":0,"debug":1,"actions":{"onSet":[{"enum":[{"shift":0,"mask":1,"inValue":0,"uri":"/controls/enum_test:test_value","outValue":"VALUE_1_0"},{"shift":0,"mask":1,"inValue":1,"uri":"/controls/enum_test:test_value","outValue":"VALUE_1_1"},{"shift":0,"mask":1,"inValue":2,"uri":"/controls/enum_test:test_value","outValue":"VALUE_1_2"}]}]}}}'
exp_resp='{"test_enum_1":{"value":0,"debug":1,"actions":{"onSet":[{"enum":[{"inValue":0,"mask":1,"outValue":"VALUE_1_0","shift":0,"uri":"/controls/enum_test:test_value"},{"inValue":1,"mask":1,"outValue":"VALUE_1_1","shift":0,"uri":"/controls/enum_test:test_value"},{"inValue":2,"mask":1,"outValue":"VALUE_1_2","shift":0,"uri":"/controls/enum_test:test_value"}]}]}}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_enum_1`
check_resp "setup action 1" $resp $exp_resp

exp_resp='"test_enum_2":{"value":-1,"debug":true,"enabled":true,"defVal":"DefaultVal","defUri":"/status/test:EnumTestDef","actions":{"onSet":[{"enum":[{"shift":0,"mask":3,"inValue":0,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_0"},{"shift":0,"mask":3,"inValue":1,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_1"},{"shift":0,"mask":3,"inValue":2,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_2"}]}]}}'
#actual_response="test_enum_2":{"value":-1,"debug":true,"enabled":true,"defVal":"DefaultVal","defUri":"/status/test:EnumTestDef","actions":{"onSet":[{"enum":[{"shift":0,"mask":3,"inValue":0,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_0"},{"shift":0,"mask":3,"inValue":1,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_1"},{"shift":0,"mask":3,"inValue":2,"uri":"/status/test:EnumTest1","outValue":"VALUE_1_2"}]}]}}]
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/ess/test_enum_2`
check_resp "setup action 2" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_enum_1":{ "value":1}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/enum_test/test_value`
exp_resp='{"test_value":{"value":"VALUE_1_1"}}'
check_resp "send 1 value 1" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_enum_1":{ "value":2}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/enum_test/test_value`
exp_resp='{"test_value":{"value":"VALUE_1_2"}}'
check_resp "send 1 value 2" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_enum_1":{ "value":0}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/enum_test/test_value`
exp_resp='{"test_value":{"value":"VALUE_1_0"}}'
check_resp "send 1 value 0" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '{ "test_enum_2":{ "value":4}}'
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/status/test/EnumTest1`
exp_resp='{"EnumTest1":{"value":"VALUE_1_2"}}'
check_resp "send 2 value 2 " $resp $exp_resp


echo "$0 tests passed" >> tests.out


