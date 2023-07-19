#!/bin/bash
#"name":"test_selectorn.sh",
#"author":"Phil Wilshire",
#"desc":"test the selectorn function",
#"date":"04_20_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

logname=selectorn


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh logic
timeout=20


test()
{
    usetime 2
    #fims_send -m pub -u /test/expect '{"test_7.00":"/some/selected/output pulse_on:true"}'
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /some/selected/output"
    res=`$get`
    echo $res


    cmds=()

    addcmd "fims_send -m pub -u /components/int1 16"
    addcmd "fims_send -m pub -u /components/int2 3"
    #addcmd "fims_send -m get -r /$$ -u /some/selected/output"
    addcmd "fims_send -m pub -u /components/int3 7"
    addcmd "fims_send -m pub -u /components/float2 4.56"
    addcmd "fims_send -m pub -u /components/bool1 true"
    addcmd "fims_send -m pub -u /components/bool2 true"
    addcmd "fims_send -m pub -u /components/bool3 false"
    addcmd "fims_send -m pub -u /components/float1 125.0"
    addcmd "fims_send -m pub -u /components/float2 4.56"
    addcmd "fims_send -m pub -u /components/float3 1134.56"
    addcmd "fims_send -m pub -u /components/string1 {\"value\":\"100\"}"
    addcmd "fims_send -m pub -u /components/string2 {\"value\":\"42\"}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v1 {\"value\":false}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v2 {\"value\":false}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v3 {\"value\":false}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v4 {\"value\":false}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v5 {\"value\":false}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 0"
    #showcmds
    runcmds

    #echo "========> running ps"
    #ps ax | grep go_metrics

    usetime 2
    #echo "Test fims get"
    #get="fims_send -m get -r /$$ -u /some/selected/output"
    #res=`$get`
    #echo $res

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":-1}'
    runget $expect $get


    #echo "=========> running ps"
    #ps ax | grep go_metrics

    #fims_send -m set  -u /run/test/341

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v1 true"
    runcmds
    #fims_send -m set  -u /run/test/342


    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":1}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v1 false"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v2 true"
    runcmds

    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":2}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v2 false"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v3 true"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":3}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v3 false"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v4 true"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":4}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v4 false"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v5 true"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":5}'
    runget $expect $get


    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v5 false"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":-1}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/v4 true"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":4}'
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@