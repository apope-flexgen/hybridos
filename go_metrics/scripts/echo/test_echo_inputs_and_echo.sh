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

testname=echo_inputs_and_echo


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

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some/input/uri '{"accontactor":-49,"p":23,"plim":85,"pramprise":-63}'
    fims_send -m pub -u /another/input/uri '{"accontactor":-48,"p":12,"plim":-93,"pramprise":2}'



    usetime 2    

    expect='{"a":-49,"b":23,"c":85,"d":-63,"e":23,"f":-48,"g":12,"h":-93,"i":2,"j":-48,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


    ### TEST 3 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":{"value":-31},"p":{"value":-1},"plim":{"value":78},"pramprise":{"value":48}}'
    fims_send -m pub -u /another/input/uri '{"accontactor":{"value":21},"p":{"value":10},"plim":{"value":-74},"pramprise":{"value":18}}'
    usetime 2


    expect='{"a":-31,"b":-1,"c":78,"d":48,"e":-1,"f":21,"g":10,"h":-74,"i":18,"j":21,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 4 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":{"value":84},"p":{"value":-35},"plim":{"value":58},"pramprise":{"value":-12}}'
  fims_send -m pub -u /another/input/uri '{"accontactor":{"value":35},"p":{"value":-7},"plim":{"value":78},"pramprise":{"value":-26}}'
    usetime 2


    expect='{"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 5 ###


    fims_send -m set -u /some/output/uri '{"a":1,"b":1,"c":1,"d":1,"e":1,"k":{"value":84},"l":{"value":-35},"m":{"value":58},"n":{"value":-12},"o":{"value":-12},"p":{"value":-12}}'
    usetime 2


    expect='{"a":84,"b":-35,"c":58,"d":-12,"e":-35,"f":35,"g":-7,"h":78,"i":-26,"j":35,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@