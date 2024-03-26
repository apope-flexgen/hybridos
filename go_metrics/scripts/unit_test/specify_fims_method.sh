#!/bin/bash

testname=specify_fims_method


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
gitdirs=$(cd `dirname $0` && cd ../.. && pwd)
echo "git1 " $gitdir
source $gitdir/scripts/test_funcs.sh unit_test
timeout=15
gitdir=$gitdirs
echo "git2 " $gitdir

test()
{

  ### TEST 1 ###
  usetime 2

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    ### TEST 2 ###

    fims_send -m set -u /some/input/a 5
    usetime 2

    expect='{"a":5,"b":0,"c":5,"d":5,"e":15}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get


    ### TEST 3 ###

    fims_send -m pub -u /some/input/a 3
    usetime 2

    expect='{"a":5,"b":3,"c":3,"d":3,"e":14}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    usetime $timeleft

    echo "test complete"

}

menu $@