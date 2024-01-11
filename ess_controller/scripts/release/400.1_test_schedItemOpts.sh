#!/bin/sh
#
#   dbi_test
#
# this will update dbi every time the value is written with a change
# dbiStatus == "init" to also force a dbiWrite
# once that has been processes that value will change to "OK"
# EnableDbiUpdate must also be set to true
# if dbiStatus == "once"
#   EnableDbiUpdate will be set to false after a single update.
if [ -f /home/config/sim/setup.txt ] ; then
    . /home/config/sim/setup.txt
fi

if [ -f  ~/config_ess/install/configs/sim/setup.txt ] ; then 
    . ~/config_ess/install/configs/sim/setup.txt
    echo "using ~/config_ess/install/configs/sim/setup.txt"
fi


# load in the test setup defs...

#mkdir -p /home/config/tests/output

num_passed=0
num_failed=0
fails=()

tfile=./scripts/release/test_definitions.sh
. $tfile

rtest="400.1_test_schedItemOpts"

# load in the test setup defs...
deffile=./scripts/release/${rtest}.txt
. $deffile

mkdir -p /var/log/tests/output
outfile=/var/log/tests/output/${rtest}_results.txt
logfile=/var/log/tests/output/${rtest}_log.txt

rm -f $outfile
test_header $outfile

rm -f $logfile
test_header $logfile

fidx=1

run_tests $fidx $idx

ix=$(($idx-1))
test_tail $outfile

echo
echo

cat $outfile

exit





echo " setup DemoChargeCurrent"

act_resp=$(eval $test1_act)    

echo -n " >>>>>>>>>" > $outfile
check_resp "test1 setup DemoChargeCurrent" "$act_resp" "$test1_resp" >> $outfile

#
# set up response var
echo  "set up response var" 
#echo  "#set up response var" >> $outfile

act_resp=$(eval $test2_act)    

echo -n " >>>>>>>>>" >> $outfile
check_resp "test2 setup DemoChargeCurrent resp" "$act_resp" "$test2_resp" >> $outfile

## set value to 1234 
echo " set value to 1234.0 "
#echo "#set value to 1234.0 " >> $outfile
#echo 'test3_resp=$(cat<<EOF' >> $outfile
# act_resp=$(/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/controls/bms ' {
#         "DemoChargeCurrent": {
#             "value" :1234.0
#         }
#     }' | jq
#     )
act_resp=$(eval $test3_act)    
#echo $act_resp     >> $outfile
#echo 'EOF'         >> $outfile
#echo ')'           >> $outfile
#echo 'echo "test3_resp =[ $test3_resp ]"'>> $outfile

echo -n " >>>>>>>>>" >> $outfile
check_resp "test3 set DemoChargeCurrent value 1234" "$act_resp" "$test3_resp" >> $outfile

## check dbi  value like this
echo "check dbi  value"
#echo "#check dbi  value" >> $outfile

act_resp=$(eval $test4_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test4 set check dbi response" "$act_resp" "$test4_resp" >> $outfile


# expect 1234

## set value to 4567 
echo " set value to 4567.0 "
act_resp=$(eval $test5_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test5 set check dbi response" "$act_resp" "$test5_resp" >> $outfile


## check dbi  value like this
echo " check dbi  value"
echo "# check dbi  value" >> $outfile

act_resp=$(eval $test6_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test6 set check dbi value" "$act_resp" "$test6_resp" >> $outfile

# expect 4567

# this is a simple check for dbiStatus 
echo " get the dbiStatus"
echo "# get the dbiStatus" >> $outfile
#echo 'test7_resp=$(cat<<EOF' >> $outfile

# act_resp=$(\
#     /usr/local/bin/fims/fims_send -m get -r /$$ \
#     -u /ess/full/controls/bms/DemoChargeCurrent | jq | grep dbiStatus)

#echo $act_resp  >> $outfile
#echo 'EOF' >> $outfile
#echo ')' >> $outfile
#echo 'echo "test7_resp =[ $test7_resp ]"'>> $outfile
act_resp=$(eval $test7_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test7 get dbi status" "$act_resp" "$test7_resp" >> $outfile

echo " get the response variable"
echo "#get the response variable" >> $outfile
# this is a check for the response variable
#echo 'test8_resp=$(cat<<EOF' >> $outfile
# act_resp=$(/usr/local/bin/fims/fims_send -m get -r /$$ \
#     -u /ess/full/dbi/controls/bms/DemoChargeCurrent | jq \
#     | grep -v dbiAct | grep -v dbiCnt | grep -v  dbiSkip | grep -v tLast | grep -v yactions
#     )
#echo $act_resp  >> $outfile

#echo 'EOF' >> $outfile
#echo ')' >> $outfile
#echo 'echo "test8_resp =[ $test8_resp ]"'>> $outfile
act_resp=$(eval $test8_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test8 get dbi resp var" "$act_resp" "$test8_resp" >> $outfile


# now reset the variable and disable the actions 
# dbi should no longer be updated
echo " disable actions on controls var"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/controls/bms ' {
        "DemoChargeCurrent": {
            "value" :0.0,
            "dbiStatus":"init",
            "EnableDbiUpdate": true,
            "UpdateTimeCfg": 30,
            "debug":1,
            "actions":{
                    "onSet": [{ "func": [
                        {"enabled":false,"ifChanged":true,"func": "CheckDbiVar", "amap": "bms"}
              ]}]
          }
        }
    }'
# same for the response var
echo " disable actions on response var"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/dbi/controls/bms '
{
    "DemoChargeCurrent": {
        "value" :0.0,
        "debug":1,
        "ifChanged":false,
        "actions":{
            "onSet": [{ "func": [
                      {"enabled":false, "ifChanged":false,"func": "CheckDbiResp", "amap": "bms"}
              ]}]
        }
    }
}'


## set value to 4567 
echo " set value to 7777.0 "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/controls/bms ' {
        "DemoChargeCurrent": {
            "value" :7777.0
        }
    }'


## check dbi  value like this
echo " check dbi value , should be unchanged well not 7777 anyway "
echo "# check dbi value , should be unchanged well not 7777 anyway " >> $outfile
#echo 'test9_resp=$(cat<<EOF' >> $outfile
# act_resp=$(\
#      /usr/local/bin/fims/fims_send -m get -r /$$ \
#      -u /dbi/ess_controller/_controls_bms/DemoChargeCurrent/value | jq
#      )
#echo $act_resp >> $outfile

#echo 'EOF' >> $outfile
#echo ')' >> $outfile
#echo 'echo "test9_resp =[ $test9_resp ]"'>> $outfile
act_resp=$(eval $test9_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test9 get dbi value" "$act_resp" "$test9_resp" >> $outfile

## now set up the wake_monitor value.
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/wake_monitor/bms '{
    "/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiVar"}
}'
sleep 0.2
## check dbi 
echo " check dbi value , should now be  7777  "
echo "# check dbi value , should now be  7777  " >> $outfile
#echo 'test10_resp=$(cat<<EOF' >> $outfile
# act_resp=$(
#    /usr/local/bin/fims/fims_send -m get -r /$$ \
#    -u /dbi/ess_controller/_controls_bms/DemoChargeCurrent/value | jq
#    )
#echo $act_resp >> $outfile
#echo 'EOF' >> $outfile
#echo ')' >> $outfile
#echo 'echo "test10_resp =[ $test10_resp ]"'>> $outfile
act_resp=$(eval $test10_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test10 get dbi value" "$act_resp" "$test10_resp" >> $outfile


echo " set up check for dbi resp"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/wake_monitor/bms '{
    "/dbi/controls/bms:DemoChargeCurrent": { "enable": true, "rate":0.1, "amap": "bms", "func":"CheckDbiResp"}
}'


echo " change the dbi value" 
 /usr/local/bin/fims/fims_send -m set -r /$$ -u /dbi/ess_controller/_controls_bms '
{"DemoChargeCurrent":
    {"value":328,
      "EnableDbiUpdate":true,
      "UpdateTimeCfg":5,"UpdateTimeRemain":0
    }
}'

echo " trigger a refetch"
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/controls/bms ' {
        "DemoChargeCurrent": {
            "value" :0.0,
            "dbiStatus":"init",
            "EnableDbiUpdate": true,
            "UpdateTimeCfg": 5
        }
    }'

echo " wait for refetch"
sleep 1
echo " check we got the value updated "
echo "# check we got the value updated " >> $outfile

act_resp=$(eval $test11_act)    
echo -n " >>>>>>>>>" >> $outfile
check_resp "test11 get dbi value" "$act_resp" "$test11_resp" >> $outfile

echo "# test $0 completed" >> $outfile

