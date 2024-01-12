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

testname=echo_inputs_and_nested_inputs_and_echo


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

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some/input/uri '{"accontactor":-27,"p":89,"plim":53,"pramprise":56}'
    fims_send -m pub -u /another/input/uri '{"accontactor":71,"p":-61,"plim":-82,"pramprise":-17,"sub_uri":{"value":5,"alpha":-55,"beta":-56,"gamma":85,"delta":-68,"sub_sub":{"alpha":-69,"beta":53,"gamma":30,"delta":-25}}}'



    usetime 2    

    expect='{"a":-27,"b":89,"c":53,"d":56,"e":89,"f":71,"g":-61,"h":-82,"i":-17,"j":71,"k":-55,"l":-56,"m":85,"n":-68,"o":-56,"p":-69,"q":53,"r":30,"s":-25,"t":53,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":5}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get


    ### TEST 3 ###

  fims_send -m pub -u /some/input/uri '{"accontactor":{"value":28},"p":{"value":-36},"plim":{"value":80},"pramprise":{"value":-63}}'
  fims_send -m pub -u /another/input/uri '{"accontactor":{"value":68},"p":{"value":85},"plim":{"value":-24},"pramprise":{"value":38},"sub_uri":{"value":3,"alpha":{"value":-15},"beta":{"value":-47},"gamma":{"value":-37},"delta":{"value":47},"sub_sub":{"alpha":{"value":34},"beta":{"value":-30},"gamma":{"value":32},"delta":{"value":-66}}}}'
    
    usetime 2


    expect='{"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":0,"v":0,"w":0,"x":0,"y":0,"z":0,"zz":3}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

  ### TEST 4 ###

  fims_send -m set -u /some/output/uri '{"a":1,"b":1,"c":1,"d":1,"e":1,"u":5,"v":5,"w":5,"x":5,"y":5,"z":5}'
    
    usetime 2


    expect='{"a":28,"b":-36,"c":80,"d":-63,"e":-36,"f":68,"g":85,"h":-24,"i":38,"j":68,"k":-15,"l":-47,"m":-37,"n":47,"o":-47,"p":34,"q":-30,"r":32,"s":-66,"t":-30,"u":5,"v":5,"w":5,"x":5,"y":5,"z":5,"zz":3}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@