#!/bin/bash
#"name":"test_and_nested.sh",
#"author":"Phil Wilshire",
#"desc":"test the and function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

testname=and_nested


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
gitdirs=$(cd `dirname $0` && cd ../.. && pwd)
echo "git1 " $gitdir
source $gitdir/scripts/test_funcs.sh logic
timeout=20
gitdir=$gitdirs
echo "git2 " $gitdir




test()
{

     ### TEST 1 ###
    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/output"
    res=`$get`
    echo $res

    expect='{"bool_output1":false,"bool_output2":false,"bool_output3":false}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

     ### TEST 2 ###

    fims_send -m pub -u /components/int1 16
    fims_send -m pub -u /components/int2 3
    fims_send -m pub -u /components/int3 7
    fims_send -m pub -u /components/float2 4.56
    fims_send -m pub -u /components/bool1 true
    fims_send -m pub -u /components/bool2 true
    fims_send -m pub -u /components/bool3 false
    fims_send -m pub -u /components/float1 125.0
    fims_send -m pub -u /components/float2 4.56
    fims_send -m pub -u /components/float3 1134.56
    fims_send -m pub -u /components/string1 '{"value":"100"}'
    fims_send -m pub -u /components/string2 '{"value":"42"}'
    

    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/output"
    res=`$get`
    echo $res

    expect='{"bool_output1":true,"bool_output2":true,"bool_output3":false}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get


    ### TEST 3 ###


    fims_send -m pub -u /components '{
         "int1" :345,
         "int2" :23,
         "int3" :5,
         "bool1" :true,
         "bool2" :false,
         "bool3" :false,
         "float1" :-22.1,
         "float2" :-3,
         "float3" :34,
         "string1":"200",
         "string2":"23"

         }'
    usetime 2

    expect='{"bool_output1":false,"bool_output2":false,"bool_output3":false}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    ### TEST 4 ###

    fims_send -m pub -u /components '{
         "int1" :0,
         "int2" :0,
         "int3" :5,
         "bool1" :true,
         "bool2" :false,
         "bool3" :false,
         "float1" :-22.1,
         "float2" :-3,
         "float3" :34,
         "string1":"200",
         "string2":"23"
         }'

    fims_send -m pub -u /components/int1 '{
         "sub1" :345
         }'
    usetime 2
    echo "Test fims int1/sub1"

    expect='{"bool_sub_output3":true}'
    get="fims_send -m get -r /$$ -u /some/output/sub"
    runget $expect $get

    ### TEST 5 ###

    fims_send -m pub -u /components/int1 '{
         "sub1" :0
         }'
    usetime 2
    expect='{"bool_sub_output3":false}'
    get="fims_send -m get -r /$$ -u /some/output/sub"
    runget $expect $get

    usetime $timeleft


    echo "test complete"

}

menu $@