#!/bin/sh

echo "I am $0 and I am testing things"
echo "$0 tests passed" >> tests.out

function check_resp()
{
    if [ "$2" = "$3" ] ; then
        echo "test $1 >>  it worked"
        echo "test $1 >>  worked" >> tests.out

    else
        echo "test $1 >> failed"
        echo "resp [$2]"
        echo "test $1 >>  failed" >> tests.out
    fi 
}

./fims_send -m get -r /$$ -u /ess/full/status/bms/BMSAvgCellTemp | jq  
#"BMSAvgCellTemp": {
#    "value": 0,
#    "actions": {
#      "onSet": [
#        {
#         "func": [
#            {
#              "amap": "bms",
#              "func": "CheckMonitorVar"
#            }
#          ]
#        }
#      ]
#    }
#  }
#}

#"BMSAvgCellTemp": {
#    "value": -50,
#    "EnableAlert": true,
#    "EnableFaultCheck": false,
#    "EnableMaxValCheck": false,
#    "EnableMinValCheck": false,
#    "FaultShutdownReset": true,
#    "MaxAlarmThreshold": 0,
#    "MaxAlarmTime": 0,
#    "MaxAlarmTimeout": 0,
#    "MaxFaultThreshold": 0,
#   "MaxFaultTime": 0,
#    "MaxFaultTimeout": 0,
#    "MaxRecoverTime": 0,
#    "MaxRecoverTimeout": 0,
#    "MaxResetValue": 0,
#    "MinAlarmThreshold": 0,
#    "MinAlarmTime": 0,
#    "MinAlarmTimeout": 0,
#    "MinFaultThreshold": 0,
#    "MinFaultTime": 0,
#    "MinFaultTimeout": 0,
#    "MinRecoverTime": 0,
#    "MinRecoverTimeout": 0,
#    "MinResetValue": 0,
#    "seenMaxAlarm": false,
#    "seenMaxFault": false,
#    "seenMaxReset": false,
#    "seenMinAlarm": false,
#    "seenMinFault": false,
#    "seenMinReset": false,
#    "tLast": 5881.006157000011,
#    "actions": {
#      "onSet": [
#        {
#          "func": [
#            {
#              "amap": "bms",
#              "func": "CheckMonitorVar"
#            }
#          ]
#        }
#      ]
#    }
#  }
#}


#./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSAvgCellTemp":{"value":80}}'
./fims_send -m set -r /$$ -u /ess/full/status/bms '{"BMSAvgCellTemp":{"value":60}}'

exp_resp='"BMSAvgCellTemp":{"value":80,"EnableAlert":true,"EnableFaultCheck":false,"EnableMaxValCheck":false,"EnableMinValCheck":false,"FaultShutdownReset":true,"MaxAlarmThreshold":0,"MaxAlarmTime":0,"MaxAlarmTimeout":0,"MaxFaultThreshold":0,"MaxFaultTime":0,"MaxFaultTimeout":0,"MaxRecoverTime":0,"MaxRecoverTimeout":0,"MaxResetValue":0,"MinAlarmThreshold":0,"MinAlarmTime":0,"MinAlarmTimeout":0,"MinFaultThreshold":0,"MinFaultTime":0,"MinFaultTimeout":0,"MinRecoverTime":0,"MinRecoverTimeout":0,"MinResetValue":0,"seenMaxAlarm":false,"seenMaxFault":false,"seenMaxReset":false,"seenMinAlarm":false,"seenMinFault":false,"seenMinReset":false,"tLast":6926.983320000014,"actions":{"onSet":[{"func":[{"amap": "bms","func":"CheckMonitorVar"}]}]}}'


check_resp "setup action" $resp $exp_resp


/usr/local/bin/fims/fims_send -m set  -u /ess/test/ess '
{ "test_bitfield":{ "value":0,"debug":1}}'

/usr/local/bin/fims/fims_send -m set  -u /ess/full/test/ess/test_bitfield 1
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs/oncmd_test`

exp_resp='{"oncmd_test":{"value":true}}'

check_resp "send value 1" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/full/test/ess/test_bitfield 4
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs/oncmd_test`
exp_resp='{"oncmd_test":{"value":false}}'

check_resp "send value 4" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/full/test/ess/test_bitfield 32
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/system/controls/kacopencmd_test`
exp_resp='{"kacopencmd_test":{"value":true}}'

check_resp "send value 32" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/full/test/ess/test_bitfield 64
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/system/controls/kacopencmd_test`
exp_resp='{"kacopencmd_test":{"value":false}}'

check_resp "send value 64" $resp $exp_resp

/usr/local/bin/fims/fims_send -m set  -u /ess/full/test/ess/test_bitfield 33
resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/system/controls/kacopencmd_test`
exp_resp='{"kacopencmd_test":{"value":true}}'
check_resp "send value 33.1" $resp $exp_resp

resp=`/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/controls/pcs/oncmd_test`
exp_resp='{"oncmd_test":{"value":true}}'
check_resp "send value 33.2" $resp $exp_resp

echo "$0 tests passed" >> tests.out