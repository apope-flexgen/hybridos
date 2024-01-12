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

testname=echo_inputs


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
gitdirs=$(cd `dirname $0` && cd ../.. && pwd)
echo "git1 " $gitdir
source $gitdir/scripts/test_funcs.sh echo
timeout=20
gitdir=$gitdirs
echo "git2 " $gitdir



test()
{

  ### TEST 1 ###
  usetime 2

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some/input/uri '{"accontactor":-16,"p":-73,"plim":-13,"pramprise":-21}'
fims_send -m pub -u /another/input/uri '{"accontactor":-45,"p":-97,"plim":-57,"pramprise":94}'



    usetime 2
    echo "Test echo"
    

    expect='{"a":-16,"b":-73,"c":-13,"d":-21,"e":-73,"f":-45,"g":-97,"h":-57,"i":94,"j":-45}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


    ### TEST 3 ###


    fims_send -m pub -u /some/input/uri '{
  "accontactor": 123,
  "p": 456,
  "plim": 789,
  "pramprise": 121
}'
fims_send -m pub -u /another/input/uri '{
  "accontactor": 111,
  "p": 222,
  "plim": 333,
  "pramprise": 444
}'
    usetime 2


    expect='{"a":123,"b":456,"c":789,"d":121,"e":456,"f":111,"g":222,"h":333,"i":444,"j":111}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 4 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":{"value":84},"p":{"value":-35},"plim":{"value":58},"pramprise":{"value":-12}}'
fims_send -m pub -u /another/input/uri '{"accontactor":{"value":35},"p":{"value":-7},"plim":{"value":78},"pramprise":{"value":-26}}'
    usetime 2


    expect='{"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@