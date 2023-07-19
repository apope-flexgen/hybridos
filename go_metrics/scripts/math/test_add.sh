#!/bin/bash
#"name":"test_add.sh",
#"author":"Phil Wilshire",
#"desc":"test the Add function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=add


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
    expect='{"float_output1":1303.6799999999998,"float_output2":1303.6799999999998,"string_output1":"10042","string_output2":"10042"}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get


    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :-345,
         "float3" :34,
         "string1":"200",
         "string2":"23"

         }'
    usetime 2
    expect='{"float_output1":-311,"float_output2":-311,"string_output1":"20023","string_output2":"20023"}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@

