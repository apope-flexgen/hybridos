#!/bin/bash
#"name":"test_ceil.sh",
#"author":"Phil Wilshire",
#"desc":"test the Ceil function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=ceil


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
    fims_send -m pub -u /components/string1 '{"value":"100"}'
    fims_send -m pub -u /components/string2 '{"value":"42"}'
    

    usetime 2
    expect='{"float_output1":35,"float_output2":35}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get


    fims_send -m pub -u /components '{
         "float1" :-22.1,
         "float2" :-345,
         "float3" :34,
         "string1":"200",
         "string2":"23"

         }'
    usetime 2
    expect='{"float_output1":-22,"float_output2":-22}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@
