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

testname=echo_multiple_echo_objects


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

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /yet_another/output/uri"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some/input/uri '{"accontactor":-21,"p":-15,"plim":55,"pramprise":-57}'
    fims_send -m pub -u /another/input/uri '{"accontactor":-87,"p":9,"plim":16,"pramprise":-70}'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":53,"p":31,"plim":-83,"pramprise":-22}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":-32,"p":-24,"plim":-61,"pramprise":-84}'
    



    usetime 2    

    expect='{"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":-21,"b":-15,"c":55,"d":-57,"e":-15,"f":-87,"g":9,"h":16,"i":-70,"j":-87,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":53,"b":31,"c":-83,"d":-22,"e":31,"f":-32,"g":-24,"h":-61,"i":-84,"j":-32,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /yet_another/output/uri"
    runget $expect $get


    ### TEST 3 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":-45,"p":44,"plim":63,"pramprise":-35}'
    fims_send -m pub -u /another/input/uri '{"accontactor":20,"p":-51,"plim":-63,"pramprise":-83}'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":17,"p":-21,"plim":-99,"pramprise":90}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":40,"p":-16,"plim":84,"pramprise":-23}'
    usetime 2


    expect='{"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":-45,"b":44,"c":63,"d":-35,"e":44,"f":20,"g":-51,"h":-63,"i":-83,"j":20,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":17,"b":-21,"c":-99,"d":90,"e":-21,"f":40,"g":-16,"h":84,"i":-23,"j":40,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /yet_another/output/uri"
    runget $expect $get

    ### TEST 4 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":{"value":15},"p":{"value":-92},"plim":{"value":-93},"pramprise":{"value":-71}}'
    fims_send -m pub -u /another/input/uri '{"accontactor":{"value":-73},"p":{"value":69},"plim":{"value":50},"pramprise":{"value":-59}}'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":{"value":11},"p":{"value":-8},"plim":{"value":87},"pramprise":{"value":-45}}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":{"value":-34},"p":{"value":-95},"plim":{"value":56},"pramprise":{"value":-37}}'
    usetime 2


    expect='{"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /yet_another/output/uri"
    runget $expect $get

    ### TEST 5 ###


    fims_send -m set -u /some/output/uri '{"a":1,"b":1,"c":1,"d":1,"e":1,"k":{"value":84},"l":{"value":-35},"m":{"value":58},"n":{"value":-12},"o":{"value":-12},"p":{"value":-12}}'
    fims_send -m set -u /another/output/uri '{"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}'
    fims_send -m set -u /yet_another/output/uri/k 22
    fims_send -m set -u /yet_another/output/uri/l '{"value":11}'
    usetime 2


    expect='{"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":84,"l":-35,"m":58,"n":-12,"o":-12,"p":-12}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":15,"b":-92,"c":-93,"d":-71,"e":-92,"f":-73,"g":69,"h":50,"i":-59,"j":-73,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":11,"b":-8,"c":87,"d":-45,"e":-8,"f":-34,"g":-95,"h":56,"i":-37,"j":-34,"k":22,"l":11,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /yet_another/output/uri"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@