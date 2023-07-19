#!/bin/bash
#"name":"test_float.sh",
#"author":"Phil Wilshire",
#"desc":"test the Float function",
#"date":"04_21_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=float


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
    expect='{"bool_to_float":1,"float_to_float":34.56,"int_to_float":5,"string_to_float":5,"uint_to_float":6}'
    runget $expect $get


    fims_send -m pub -u /components '{
         "float1" :0,
         "float2" :0,
         "float3" :34,
         "string1" :"false",
         "string2" :false
         }'

    usetime 2
    expect='{"bool_to_float":1,"float_to_float":0,"int_to_float":5,"string_to_float":5,"uint_to_float":6}'
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
    expect='{"bool_to_float":1,"float_to_float":0,"int_to_float":5,"string_to_float":5,"uint_to_float":6}'
    get="fims_send -m get -r /$$ -u /some/output"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@
