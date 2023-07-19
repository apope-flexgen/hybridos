#!/bin/bash
#"name":"test_current_time_milliseconds.sh",
#"author":"Phil Wilshire",
#"desc":"test the current time  function",
#"date":"04_20_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

logname=milliseconds_since


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh time_based
timeout=10



test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/selected/output"
    res=`$get`
    echo $res

    cmds=()

    addcmd "fims_send -m pub -u /components/int1 1682017107014"
    # addcmd "fims_send -m pub -u /components/int2 3"
    # addcmd "fims_send -m pub -u /components/int3 7"
    # addcmd "fims_send -m pub -u /components/float2 4.56"
    # addcmd "fims_send -m pub -u /components/bool1 true"
    # addcmd "fims_send -m pub -u /components/bool2 true"
    # addcmd "fims_send -m pub -u /components/bool3 false"
    # addcmd "fims_send -m pub -u /components/float1 125.0"
    # addcmd "fims_send -m pub -u /components/float2 4.56"
    # addcmd "fims_send -m pub -u /components/float3 1134.56"
    # addcmd "fims_send -m pub -u /components/string1 {\"value\":\"100\"}"
    # addcmd "fims_send -m pub -u /components/string2 {\"value\":\"42\"}"
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v1 false"
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v2 false"
    
    runcmds
 
    #echo "========> running ps"
    #ps ax | grep go_metrics

    usetime 2
    echo "Test fims get #1"
    get="fims_send -m get -r /$$ -u /some/selected/output"
    res=`$get`
    echo $res

    usetime 2
    echo "Test fims get #2"
    get="fims_send -m get -r /$$ -u /some/selected/output"
    res=`$get`
    echo $res
    # expect='{"selection":false}'
    # get="fims_send -m get -r /$$ -u /some/selected/output"
    # runget $expect $get


    # #echo "=========> running ps"
    # #ps ax | grep go_metrics


    # cmds=()

    # addcmd "fims_send -m pub -u /components/feeder_52m1/v1 true"
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v2 false"
    
    # runcmds

    # usetime 2
    # #echo "Test fims get"
    # #get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # #res=`$get`
    # #echo $res

    # expect='{"selection":true}'
    # get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # runget $expect $get


    # cmds=()
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v1 false"
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v2 false"
    
    # runcmds

    # usetime 2
    # #echo "Test fims get"
    # #get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # #res=`$get`
    # #echo $res

    # expect='{"selection":true}'
    # get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # runget $expect $get

    # cmds=()
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v1 false"
    # addcmd "fims_send -m pub -u /components/feeder_52m1/v2 true"
    
    # runcmds

    # usetime 2
    # #echo "Test fims get"
    # #get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # #res=`$get`
    # #echo $res

    # expect='{"selection":false}'
    # get="fims_send -m get -r /$$ -u /some/selected/output/selection"
    # runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@