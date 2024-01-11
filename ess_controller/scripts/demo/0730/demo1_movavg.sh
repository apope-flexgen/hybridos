#!/bin/sh
# simple training / demo 
SYS=/ess
# echo remap incoming sbmu_current to sbmu_raw_current
# /usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9 '
# {
#      "sbmu_current":{
#           "value":0,
#           "ifChanged":false,
#           "actions":{
#                "onSet":
#                [
#                     { 
#                     "remap":
#                          [
#                               {"uri":"/status/sbmu_9:sbmu_raw_current","amap":"ess"}
#                          ]
#                     }
#                ]
#           }
#      }
# }'

echo  set up filter function to work on rack 9 sbmu_current to produce sbmu_filt_current

/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9 '
{
     "sbmu_current":{
          "value":2000,
          "amap":"ess",
          "depth":16,
          "ifChanged": false,
          "vecAv":"/status/sbmu_9:raw_current_vec",
          "outMax":"/status/sbmu_9:sbmu_max_current",
          "outMin":"/status/sbmu_9:sbmu_min_current",
          "outSum":"/status/sbmu_9:sbmu_sum_current",
          "outAvg":"/status/sbmu_9:sbmu_avg_current",
          "outFilt":"/status/sbmu_9:sbmu_filt_current",
          "filtFac":0.75,
          "actions":{
               "onSet":
               [{ "func":[
                              {"func":"MathMovAvg","amap":"ess"}
                         ]
               }]}}
}' | jq

echo -n " press enter to continue " && read in

echo now send some values to sbmu_current and check the output values.
echo preload value  to about 2000
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2000
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2100
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 1900
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 1800
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2000
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2100
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2200
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9/sbmu_current 2100

echo -n "check raw response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/status/sbmu_9 | jq

echo -n " press enter to continue " && read in

echo
echo -n "check filter values :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/components/catl_sbmu_9/sbmu_current | jq
echo -n " press enter to continue " && read in

echo
echo set value to 3000
/usr/local/bin/fims/fims_send -m set  -u $SYS/components/catl_sbmu_9/sbmu_current 3000

echo -n "check filt response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/status/sbmu_9/sbmu_filt_current 

echo 
echo set value to 3000
/usr/local/bin/fims/fims_send -m set  -u $SYS/components/catl_sbmu_9/sbmu_current 3000

echo -n "check filt response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/status/sbmu_9/sbmu_filt_current 

echo
echo set value to 3000
/usr/local/bin/fims/fims_send -m set  -u $SYS/components/catl_sbmu_9/sbmu_current 3000

echo -n "check  response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/status/sbmu_9  | jq 

echo
echo -n "check filter values :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/components/catl_sbmu_9/sbmu_current | jq

echo -n " press enter to continue " && read in

#echo -n " press enter to continue " && read in

echo set value to 1000 and reset 
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/components/catl_sbmu_9 '{"sbmu_current":{"value":1000,"reset":true}}' | jq


echo -n "check  response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/status/sbmu_9  |jq

echo
echo -n "check filter values :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/components/catl_sbmu_9/sbmu_current | jq


echo -n " press enter to continue " && read in

echo
echo set value to 1000
/usr/local/bin/fims/fims_send -m set  -u $SYS/components/catl_sbmu_9/sbmu_current 1000

echo
echo -n "check filt response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/status/sbmu_9/sbmu_filt_current 

echo

echo set value to 1000
/usr/local/bin/fims/fims_send -m set  -u $SYS/components/catl_sbmu_9/sbmu_current 1000

echo
echo  "check filt response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/status/sbmu_9/sbmu_filt_current 

echo  "check  response :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/status/sbmu_9   | jq

echo
echo  "check filter values :"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/components/catl_sbmu_9/sbmu_current | jq

echo -n " press enter to continue " && read in


exit


/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
{
 "test_movavg":{"value":0, 
     "amap":"ess",
     "depth":16,
     "ifChanged": false,
     "vecAv":"/test/sbmu_1:current_testVec",
     "outAv":"/test/sbmu_1:avgCurrent",
     "outSum":"/test/sbmu_1:sumCurrent",
     "outFilt":"/test/sbmu_1:filtCurrent",
     "filtFac":0.25,
     "actions":{"onSet":[{                     
         "func":[{"func":"MathMovAvg","amap":"ess"}]
            }]}}}
'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":14.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":12.3}}'
sleep 2
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
sleep 2
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
sleep 2
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":10.3}}'
echo "should be 11.29"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/sbmu_1/avgCurrent | jq
echo "should be 67.8"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/sbmu_1/sumCurrent | jq
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/test/sbmu_1 '
     {"test_movavg":{"value":8.3}}'
echo "should be 9.198951721191406"
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/sbmu_1/filtCurrent | jq


# test for multiple actions
/usr/local/bin/fims/fims_send -m set -r /$$ -u $SYS/full/test/pcs '
{"start_grad_p":{
     "value": 11.0,
          "actions": {
               "onSet": [
                    {"limits": [ {"low": 0.1,"high": 3010.0}]},
                    {"remap": [{"bit": 0,"uri": "/components/pcsm_control","var": "start_grad_p"}]}, {"limits":[{"low":0.2,"high":2400}]}
               ]
}}}'
sleep 2
#response   {"start_grad_p":{"value":11,"actions":{"onSet":[{"limits":[{"low":0.1,"high":3010}]},{"remap":[{"bit":0,"uri":"/components/pcsm_control","var":"start_grad_p"}]},{"limits":[{"low":0.2,"high":2400}]}]}}}
/usr/local/bin/fims/fims_send -m get -r /$$ -u $SYS/full/test/pcs/start_grad_p | jq
