#!/bin/bash
#"name":"test_bool.sh",
#"author":"Phil Wilshire",
#"desc":"test the Bool function",
#"date":"04_21_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=bool


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh type_conversion
timeout=10



test()
{
    usetime 2
    fims_send -m pub -u /test/step '{"test_7.00":"type bool "}'
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    fims_send -m pub -u /components/float1 34.56
    fims_send -m pub -u /components/float2 -- -1
    fims_send -m pub -u /components/float3 1
    fims_send -m pub -u /components/string1 '"string1"'
    fims_send -m pub -u /components/string2 true

    usetime 2
    get="fims_send -m get -r /$$ -u /some/output"
    expect='{"bool_to_bool":true,"float_to_bool":true,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":true}'
    runget $expect $get


    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :0,
         "float3" :34,
         "string1" :"false",
         "string2" :false
         }'

    usetime 2
    expect='{"bool_to_bool":true,"float_to_bool":false,"int_to_bool":true,"string_to_bool":false,"uint_to_bool":true}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :34,
         "float3" :34,
         "string1" :"true",
         "string2" :false
         }'

    usetime 2
    expect='{"bool_to_bool":true,"float_to_bool":false,"int_to_bool":true,"string_to_bool":true,"uint_to_bool":true}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@
