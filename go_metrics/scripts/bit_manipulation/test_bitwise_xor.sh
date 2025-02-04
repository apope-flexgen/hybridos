#!/bin/bash
#"name":"test_bitwise_and.sh",
#"author":"Phil Wilshire",
#"desc":"test the bitwise_and function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=bitwise_xor


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh bit_manipulation
timeout=10



test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/output"
    res=`$get`
    echo $res

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
    
    #echo "========> running ps"
    #ps ax | grep go_metrics

    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/output"
    res=`$get`
    echo $res

    expect='{"bool_output1":false,"bool_output2":false,"int_output1":20,"int_output2":20}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get


    #echo "=========> running ps"
    #ps ax | grep go_metrics


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
    #echo "Test fims get"
    #get="fims_send -m get -r /$$ -u /some/output"
    #res=`$get`
    #echo $res

    expect='{"bool_output1":false,"bool_output2":false,"int_output1":20,"int_output2":20}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@