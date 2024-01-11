#!/bin/sh
# simple test for the remap modifications

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
{
 "test_remap":{"value":0, 
     "amap":"ess",
     "ifChanged": false,
     "debug":1,
     "actions":{"onSet":[{"remap":[
          {"inValue":23, "uri" :"/test/remap:test23","amap":"ess"},
          {"inValue":24, "uri":"/test/remap:test24_1","amap":"ess"},
          {"inVar":"/test/remap:inVar21", "useAv":true, 
                    "inValue":204, "uri":"/test/remap:testin26useAv","amap":"ess"},
          {"inVar":"/test/remap:inVar21", "useAv":false, 
                    "inValue":204, "uri":"/test/remap:testin26useinvarnotuseAv","amap":"ess"},
          {"inVar":"/test/remap:inVar21", "useAv":true, 
                    "inValue":204, "uri":"/test/remap:testin26useinvarwithuseav","amap":"ess"},
          {"inVar":"/test/remap:inVar21", "useAv":false, "outValue":456, 
                    "inValue":204,  "uri":"/test/remap:testin26outval456","amap":"ess"}
          ]
            }]}}
}'

echo send value 0 , set inVar21 to 222
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":0}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/remap '{"inVar21":222}'

echo  we should just get inVar 222
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/remap | jq


#/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":0}}'
echo send value 23 to test_remap
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":23}}'

echo after 23  expect test23 = 23 , invar 222
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/remap | jq
# exit

echo send value 23 to test_remap
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":24}}'

echo after 24  expect test24 = 24 test23 = 23 , invar 222

/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/remap | jq


#exit
echo send value 204 to test_remap

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":204}}'

echo after 204  expect testin204=204  test24 = 24 test23 = 23 , invar 222

/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/remap | jq

echo send value 204 to  inVar21 this should allow any write to test_remap to pass through to 
echo testin26useinvar

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/remap '{"inVar21":204}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{"test_remap":{"value":1204}}'



/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/naked/test/remap | jq

exit


/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":12.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
echo "should be 11.29"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq
echo "should be 67.8"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
     {"test_movavg":{"value":8.3}}'
echo "should be 9.198951721191406"
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/filtCurrent | jq


# test for multiple actions
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/test/pcs '
{"start_grad_p":{
    "value": 11.0,
            "actions": {
                "onSet": [
                    {"limits": [ {"low": 0.1,"high": 3010.0}]},
                    {"remap": [{"bit": 0,"uri": "/components/pcsm_control","var": "start_grad_p"}]}, {"limits":[{"low":0.2,"high":2400}]}
                ]
            }}}'
#response   {"start_grad_p":{"value":11,"actions":{"onSet":[{"limits":[{"low":0.1,"high":3010}]},{"remap":[{"bit":0,"uri":"/components/pcsm_control","var":"start_grad_p"}]},{"limits":[{"low":0.2,"high":2400}]}]}}}
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/pcs/start_grad_p | jq
