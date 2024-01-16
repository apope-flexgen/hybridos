#!/bin/sh
# basic script to test checkMonitorVar function.
# add  different 400.xx version to show and fix problems.

echo "I am $0 and I am testing things"
echo "$0 tests running" >> tests.out
function send_val()
{
    /usr/local/bin/fims/fims_send -m set  -u $1 $2
    val=`/usr/local/bin/fims/fims_send -m get  -r /$$ -u $1`
    #v2=`echo $val | jq`
    #echo $v2
    sleep $3
    #return $val
}

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

# first test "bitfield"
#     "inValue", &inValue);
#            "uri", &uri);
#            "var", &var);
#            "outValue", &cjov);
# 
/usr/local/bin/fims/fims_send -m set -u /ess/test/ess '
{
    "test_func_1":{ 
        "value": 0,
        "EnableFaultCheck": true,
        "EnableMinValCheck": true,
        "MinAlarmThreshold": 10,
        "MinFaultThreshold": 5,
        "MinResetValue": 10.5,
        "MinAlarmTimeout": 2.5,
        "MinFaultTimeout": 1.5,
        "MinRecoverTimeout": 2.4,
        "ifChanged":false,
        "debug":20,
        "actions": {
            "onSet": [{"func": [{"func": "CheckMonitorVar","amap": "bms", "debug":1}]
                }]
        }
    }
}'
#response 
# question How do we discount tLast
greps1="grep -v tLast"
greps2="grep -v AlarmTime"
exp_resp='{"test_func_1":{"value":0,"EnableAlert":true,"EnableFaultCheck":true,"EnableMaxValCheck":false,"EnableMinValCheck":true,"FaultShutdownReset":false,"MaxAlarmThreshold":0,"MaxFaultThreshold":0,"MaxFaultTime":0,"MaxFaultTimeout":0,"MaxRecoverTime":0,"MaxRecoverTimeout":0,"MaxResetValue":0,"MinAlarmThreshold":10,"MinFaultThreshold":5,"MinFaultTime":0,"MinFaultTimeout":1.5,"MinRecoverTime":2.4,"MinRecoverTimeout":2.4,"MinResetValue":10.5,"debug":19,"ifChanged":false,"seenMaxAlarm":false,"seenMaxFault":false,"seenMaxReset":false,"seenMinAlarm":true,"seenMinFault":true,"seenMinReset":false,"actions":{"onSet":[{"func":[{"amap":"bms","debug":1,"func":"CheckMonitorVar"}]}]}}}'

foo='{"test_func_1":{"value":0,"EnableFaultCheck":true,"EnableMinValCheck":true,"MinAlarmThreshold":10,"MinFaultThreshold":5,"MinFaultTimeout":1.5,"MinRecoverTimeout":2.4,"MinResetValue":10.5,"debug":20,"ifChanged":false,"actions":{"onSet":[{"func":[{"amap":"bms","debug":1,"func":"CheckMonitorVar"}]}]}}}'
var="/ess/full/test/ess/test_func_1"
cmd="/usr/local/bin/fims/fims_send -m get"
resp=`$cmd -r /$$ -u $var | jq | $greps1 | $greps2  | tr -d "\n\t "`
#resp=`$run_resp`
check_resp "setup action 1" $resp $exp_resp

#exit
# now set a value  we have to keep hitting the value to trigger the timings
echo " waiting for reset"
send_val $var 12 1
send_val $var 12 1
send_val $var 12 1
send_val $var 12 1
resp=`$cmd  -r /$$ -u $var | jq | grep seenMinReset`
exp_resp='    "seenMinReset": true,'
check_resp "wait for reset" "$resp" "$exp_resp"

echo " trigger alarm"
send_val $var 9 1
send_val $var 9 1
send_val $var 9 1
send_val $var 9 1
send_val $var 9 1
resp=`$cmd  -r /$$ -u $var | jq | grep seenMinAlarm`
exp_resp='    "seenMinAlarm": true,'
check_resp "trigger alarm" "$resp" "$exp_resp"
#exit
echo " trigger fault"
send_val $var 4 1
send_val $var 4 1
send_val $var 4 1
send_val $var 4 1
send_val $var 4 1
resp=`$cmd  -r /$$ -u $var | jq | grep seenMinFault`
exp_resp='    "seenMinFault": true,'
check_resp "trigger fault" "$resp" "$exp_resp"

echo " restore reset"
send_val $var 12 1
send_val $var 12 1
send_val $var 12 1
send_val $var 12 1
send_val $var 12 1

resp=`$cmd  -r /$$ -u $var | jq | grep seenMinReset`
exp_resp='    "seenMinReset": true,'
check_resp "wait for reset" "$resp" "$exp_resp"

echo "$0 tests passed" >> tests.out


