#!/bin/bash

logname=internal_outputs


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
source $gitdir/scripts/test_funcs.sh internal_outputs
timeout=15



test()
{

    ### TEST 1 ###
    usetime 2
    echo "Test fims get"
    get="fims_send -m get -r /$$ -u /test/output"
    res=`$get`
    echo $res

    ### TEST 2 ###

    fims_send -m pub -u /some/uri/external_input 5


    usetime 2
    echo "Test fims_send -m pub -u /some/uri/external_input 5"
    get="fims_send -m get -r /$$ -u /test/output"
    res=`$get`
    echo $res

    expect='{"first_external_output":5,"second_external_output":5}'
    get="fims_send -m get -r /$$ -u /test/output"
    runget $expect $get


    ### TEST 3 ###

    fims_send -m pub -u /some/uri '{"external_input":23}'
    usetime 2
    echo "Test fims_send -m pub -u /some/uri/external_input 23"


    expect='{"first_external_output":23,"second_external_output":23}'
    get="fims_send -m get -r /$$ -u /test/output"
    runget $expect $get

    
    ### DONE ###
    usetime $timeleft


    echo "test complete"
}

menu $@