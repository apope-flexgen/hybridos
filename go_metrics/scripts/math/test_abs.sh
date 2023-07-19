#!/bin/bash
#"name":"test_abs.sh",
#"author":"Phil Wilshire",
#"desc":"test the Abs function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=abs

gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh


test()
{
    usetime 2
    fims_send -m pub -u /test/step '{"test_7.00":"type bool "}'
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    fims_send -m pub -u /components/float1 34.56
    fims_send -m pub -u /components/float2 134.56
    fims_send -m pub -u /components/float3 1134.56

    usetime 2
    expect='{"float_output1":34.56,"float_output2":1303.6799999999998}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get


    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :-345,
         "float3" :34
         }'
    usetime 2
    expect='{"float_output1":0,"float_output2":379}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}


menu $@

