#!/bin/sh
# p wilshire 3/23/2022

# script to test  the  checkvalue changed system


echo set up initial data set

fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"value1":1,
"value2":2,
"value3":3,
"value4":4,
"value5":5,
"value6":6,
"value7":7
}'

echo inspect the junk

fims_send -m get -r/$$ -u /ess/full/components/test/pcs_1_general_info | jq

echo set up the function

fims_send -m set -r/$$ -u /ess/components/test/pcs_1_general_info '{
"testChanged":{
    "debug":true,
    "noreset":false,
    "value":false,
    "actions":{"onSet":[{"func":[{"func":"CheckValueChanged"}]}]}
 }
}'

echo see the result
fims_send -m get -r/$$ -u /ess/full/components/test/pcs_1_general_info | jq


echo see the test 1
fims_send -m get -r /$$ -u /ess/naked/components/test/pcs_1_general_info/testChanged 


echo change a value
fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"value1":2
}'

echo trigger the change test
fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"testChanged":false
}'

echo see the result 2 should be true

fims_send -m get -r /$$ -u /ess/naked/components/test/pcs_1_general_info/testChanged

echo trigger another  change test
fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"testChanged":false
}'

echo see the result 3 should be false
fims_send -m get -r /$$ -u /ess/naked/components/test/pcs_1_general_info/testChanged


echo set up an unchanged pub

fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"value1":2,
"value2":2,
"value3":3,
"value4":4,
"value5":5,
"value6":6,
"value7":7
}'

echo trigger yet another  change test
fims_send -m pub -u /ess/components/test/pcs_1_general_info '{
"testChanged":false
}'

echo see the result 3 should be false
fims_send -m get -r /$$ -u /ess/naked/components/test/pcs_1_general_info/testChanged









