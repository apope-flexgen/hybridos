#!/bin/bash
#"name":"test_and.sh",
#"author":"Phil Wilshire",
#"desc":"test the and function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

testname=example_1


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh basic
timeout=20



test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    # echo "Test fims get"
    # get="fims_send -m get -r /$$ -u /some/output"
    # res=`$get`
    # echo $res

    fims_send -m pub -u /components/int1 16
    fims_send -m pub -u /components/int2 3
    fims_send -m pub -u /components/int3 7
    fims_send -m pub -u /components/float2 4.56
    fims_send -m pub -u /components/bool1 true
    fims_send -m pub -u /components/bool2 true
    fims_send -m pub -u /components/feeder_52m1/v1 100
    fims_send -m pub -u /components/feeder_52m1/v2 200
    fims_send -m pub -u /components/feeder_52m1/v3 300
    fims_send -m pub -u /components/feeder_52m1/v4 400
    fims_send -m pub -u /components/feeder_52m1/v5 500
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
    get="fims_send -m get -r /$$ -u /some/value/output"
    res=`$get`
    echo $res

    expect='{"v1_times_5":500,"v2_times_5":1000,"v3_times_5":1500,"v4_times_5":2000}'
    get="fims_send -m get -r /$$ -u /some/value/output"
    runget $expect $get


    #echo "=========> running ps"
    #ps ax | grep go_metrics


    fims_send -m pub -u /components '{
         "int1" :345,
         "int2" :23,
         "int3" :5,
         "bool1" :true,
         "bool2" :flase,
         "bool3" :flase,
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

    expect='{"v1_times_5":500,"v2_times_5":1000,"v3_times_5":1500,"v4_times_5":2000}'
    get="fims_send -m get -r /$$ -u /some/value/output"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@