#!/bin/bash
#"name":"test_and.sh",
#"author":"Phil Wilshire",
#"desc":"test the and function",
#"date":"04_19_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file

logname=selectn


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
    addcmd "fims_send -m pub -u /components/feeder_52m1/v1 {\"value\":42.1}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v2 {\"value\":21}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v3 {\"value\":31}"
    addcmd "fims_send -m pub -u /components/feeder_52m1/v4 {\"value\":41}"
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
    expect='{"selection":0}'
    runget $expect $get


    #echo "=========> running ps"
    #ps ax | grep go_metrics

    #fims_send -m set  -u /run/test/341

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 1"
    runcmds
    #fims_send -m set  -u /run/test/342


    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":42.1}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 2"
    runcmds

    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":21}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 3"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":31}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m set -u /components/feeder_52m1/selection_index {\"value\":4}"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":41}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index -- -1"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":41}'
    runget $expect $get


    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 21"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":41}'
    runget $expect $get

    cmds=()
    addcmd "fims_send -m pub -u /components/feeder_52m1/selection_index 0"
    runcmds
    usetime 2

    get="fims_send -m get -r /$$ -u /some/selected/output"
    expect='{"selection":41}'
    runget $expect $get

    usetime $timeleft


    echo "test complete"
    #read_fims_log

}

menu $@