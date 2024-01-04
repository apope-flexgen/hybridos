#!/bin/bash
#"name":"test_pow.sh",
#"author":"Phil Wilshire",
#"desc":"test the Pct function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  resd the log file

logname=bitfield


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh bitfield
timeout=15



test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /metrics"
    res=`$get`
    echo $res

    fims_send -m pub -u /assets/ess '{"fault_list":[{"value":0, "string":"some_string"}], "fault_list2":[{"value":0, "string":"string1"}]}'
    
    #echo "========> running ps"
    #ps ax | grep go_metrics

    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /metrics"
    res=`$get`
    echo $res

    expect='{"another_string":false,"bitfield_length":1,"combined_bitfield":[{"value":0,"string":"some_string"},{"value":4,"string":"string1"}],"etc.":false,"some_string":true,"value_in_bitfield":false,"yet_another_string":false}'
    get="fims_send -m get -r /$$ -u /metrics"
    runget $expect $get


    #echo "=========> running ps"
    #ps ax | grep go_metrics


    fims_send -m pub -u /assets/ess '{"fault_list":[{"value":0, "string":"some_string"},{"value":1, "string":"another_string"},{"value":2, "string":"yet_another_string"},{"value":3, "string":"etc."}], "fault_list2":[{"value":0, "string":"string1"},{"value":1, "string":"string2"},{"value":2, "string":"string3"},{"value":3, "string":"string4"}]}'
    usetime 2
    #echo "Test fims get"
    #get="fims_send -m get -r /$$ -u /some/output"
    #res=`$get`
    #echo $res

    expect='{"another_string":true,"bitfield_length":4,"combined_bitfield":[{"value":0,"string":"some_string"},{"value":1,"string":"another_string"},{"value":2,"string":"yet_another_string"},{"value":3,"string":"etc."},{"value":4,"string":"string1"},{"value":5,"string":"string2"},{"value":6,"string":"string3"},{"value":7,"string":"string4"}],"etc.":true,"some_string":true,"value_in_bitfield":true,"yet_another_string":true}'
    get="fims_send -m get -r /$$ -u /metrics"
    runget $expect $get

    fims_send -m pub -u /assets/ess '{"fault_list":[], "fault_list2":[]}'
    usetime 2
    #echo "Test fims get"
    #get="fims_send -m get -r /$$ -u /some/output"
    #res=`$get`
    #echo $res

    expect='{"another_string":false,"bitfield_length":0,"combined_bitfield":[],"etc.":false,"some_string":false,"value_in_bitfield":false,"yet_another_string":false}'
    get="fims_send -m get -r /$$ -u /metrics"
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@