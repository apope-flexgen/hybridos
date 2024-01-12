#!/bin/bash
#"name":"test_echo_1.sh",
#"author":"Phil Wilshire",
#"desc":"test the echo function",
#"date":"08_10_2023"

# this script has these commands
# "start " a fims _listen log for seconds and send the fims stuff
# "expect" show us what is expected
# "read log"  read the log file
#running /home/docker/git/go_metrics/logs/fims/echo
#timeout 20 go_metrics /home/docker/git/go_metrics/examples/basic/echo_1.json log  /home/docker/git/go_metrics/logs/run/echo_1

testname=echo_echo

gitdir=$(cd `dirname $0` && cd ../.. && pwd)
gitdirs=$(cd `dirname $0` && cd ../.. && pwd)
echo "git1 " $gitdir
source $gitdir/scripts/test_funcs.sh echo
timeout=20
gitdir=$gitdirs
echo "git2 " $gitdir



test()
{

  echo "Test basic echo registers"

  ### TEST 1: Basic Get ###
  usetime 2

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 2: Set forwarding, clothed message ###
    usetime 2
    fims_send -m set -u /some/output/uri '{
  "a": 3,
  "b": 5,
  "c": 7,
  "d": 12,
  "e": 5,
  "f":22
}'

    usetime 2
    

    expect='{"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


    ### TEST 3: Set forwarding doesn't happen for pubs ###


    fims_send -m pub -u /some/output/uri '{
  "a": 1,
  "b": 2,
  "c": 3,
  "d": 4,
  "e": 5,
  "f":6
}'
    usetime 2


    expect='{"a":3,"b":5,"c":7,"d":12,"e":5,"f":22}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


  ### TEST 4: Set forwarding, clothed message, take 2 ###


    fims_send -m set -u /some/output/uri '{
  "a": 1,
  "b": 2,
  "c": 3,
  "d": 4,
  "e": 5,
  "f":6
}'
    usetime 2


    expect='{"a":1,"b":2,"c":3,"d":4,"e":5,"f":6}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

  ### TEST 5 ###


    fims_send -m set -u /some/output/uri '{
  "a": {"value":9},
  "b": {"value":9},
  "c": {"value":9},
  "d": {"value":9},
  "e": {"value":9},
  "f": {"value":9}
}'
    usetime 2


    expect='{"a":9,"b":9,"c":9,"d":9,"e":9,"f":9}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 6 ###


    fims_send -m set -u /some/output/uri/a '{"value":123}'
    usetime 2


    expect='{"a":123,"b":9,"c":9,"d":9,"e":9,"f":9}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


    usetime $timeleft

    echo "test complete"
}

menu $@