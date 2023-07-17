#!/usr/bin/python3
import csv
import time
import os
# with open('esp.csv', newline='') as csvfile:
#     spamreader = csv.reader(csvfile, delimiter=',', quotechar='|')
#     for row in spamreader:
#         #print(', '.join(row))
#         print (row[1])
#         cmd="/usr/local/bin/fims/fims_send -m set -u /components/catl_sbmu_9 " + row[1]
#         print (cmd)
#         os.system(cmd)
#         time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9 '{\"sbmu_raw_current\":{\"value\":0,\"amap\":\"ess\",\"depth\":16,\"ifChanged\": false,\"vecAv\":\"/status/test/sbmu_9:raw_current_vec\",\"outFilt\":\"/status/test/sbmu_9:sbmu_filt_current\",\"filtFac\":0.75,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 2000"
os.system(cmd)
time.sleep(2)

##does not work when inital value is 0
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/test/sbmu_9/sbmu_raw_current 3000"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/status/test/sbmu_9/sbmu_filt_current "
os.system(cmd)
time.sleep(2)

# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '
# {
#  "test_movavg":{"value":0, 
#      "amap":"ess",
#      "depth":16,
#      "ifChanged": false,
#      "vecAv":"/test/sbmu_1:current_testVec",
#      "outAv":"/test/sbmu_1:avgCurrent",
#      "outSum":"/test/sbmu_1:sumCurrent",
#      "outFilt":"/test/sbmu_1:filtCurrent",
#      "filtFac":0.25,
#      "actions":{"onSet":[{                     
#          "func":[{"func":"MathMovAvg","amap":"ess"}]
#             }]}}}
# '

## 1 depth 1
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":1,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

## 2 depth 1
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":1,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)


cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

## 1 depth 2
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":2,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 5.15")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

## 2 depth 2  not correct
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":2,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 10.3")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 20.6")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

##4 depth 5
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":0,\"amap\":\"ess\",\"depth\":5,\"ifChanged\": false,\"vecAv\":\"/test/sbmu_1:current_testVec\",\"outAv\":\"/test/sbmu_1:avgCurrent\",\"outSum\":\"/test/sbmu_1:sumCurrent\",\"outFilt\":\"/test/sbmu_1:filtCurrent\",\"filtFac\":0.25,\"actions\":{\"onSet\":[{\"func\":[{\"func\":\"MathMovAvg\",\"amap\":\"ess\"}]}]}}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":14.3}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":12.3}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":10.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 9.44")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 47.2")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

## 5 depth 5 not correct
cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/test/sbmu_1 '{\"test_movavg\":{\"value\":8.3}}'"
os.system(cmd)
time.sleep(2)

print("should be 11.1")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/avgCurrent | jq"
os.system(cmd)
time.sleep(2)

print("should be 55.5")
cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/sumCurrent | jq"
os.system(cmd)
time.sleep(2)

cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/sbmu_1/filtCurrent | jq"
os.system(cmd)
time.sleep(2)

# # # test for multiple actions
# cmd="/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/test/pcs '{\"start_grad_p\":{\"value\": 11.0,\"actions\":{\"onSet\":[{\"limits\": [ {\"low\": 0.1,\"high\": 3010.0}]},{\"remap\":[{\"bit\": 0,\"uri\": \"/components/pcsm_control\",\"var\": \"start_grad_p\"}]}, {\"limits\":[{\"low\":0.2,\"high\":2400}]}]}}}'"
# os.system(cmd)
# time.sleep(2)

# # #response   {"start_grad_p":{"value":11,"actions":{"onSet":[{"limits":[{"low":0.1,"high":3010}]},{"remap":[{"bit":0,"uri":"/components/pcsm_control","var":"start_grad_p"}]},{"limits":[{"low":0.2,"high":2400}]}]}}}
# cmd="/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/test/pcs/start_grad_p | jq"
# os.system(cmd)
# time.sleep(2)