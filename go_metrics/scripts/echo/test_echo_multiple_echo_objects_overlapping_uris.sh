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

testname=echo_multiple_echo_objects_overlapping_uris


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

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":0,"b":0,"c":0,"d":0,"e":0,"f":0,"g":0,"h":0,"i":0,"j":0,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some/input/uri '{"accontactor":51,"p":47,"plim":93,"pramprise":61}'
    fims_send -m pub -u /another/input/uri '{"accontactor":30,"p":52,"plim":81,"pramprise":31,"sub_uri":{"accontactor":53,"p":31,"plim":69,"pramprise":-70}}'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":18,"p":-82,"plim":-34,"pramprise":-17}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":-41,"p":0,"plim":-81,"pramprise":-29}'
    



    usetime 2    

    expect='{"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":51,"b":47,"c":93,"d":61,"e":47,"f":30,"g":52,"h":81,"i":31,"j":30,"k":53,"l":31,"m":69,"n":-70,"o":53,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":18,"b":-82,"c":-34,"d":-17,"e":-82,"f":-41,"g":0,"h":-81,"i":-29,"j":-41,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get


    ### TEST 3 ###


    fims_send -m pub -u /some/input/uri '{"accontactor":-21,"p":4,"plim":-67,"pramprise":-20}'
    fims_send -m pub -u /another/input/uri '{"accontactor":-71,"p":-13,"plim":31,"pramprise":-30,
                                              "sub_uri":{"accontactor":-36,"p":-64,"plim":-34,"pramprise":70}
                                            }'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":{"value":19},"p":{"value":-4},"plim":{"value":-78},"pramprise":{"value":-63}}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":{"value":-100},"p":{"value":21},"plim":{"value":-62},"pramprise":{"value":24}}'
    
    usetime 2


    expect='{"a":-21,"b":4,"c":-67,"d":-20,"e":4,"f":-71,"g":-13,"h":31,"i":-30,"j":-71,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":-21,"b":4,"c":-67,"d":-20,"e":4,"f":-71,"g":-13,"h":31,"i":-30,"j":-71,"k":-36,"l":-64,"m":-34,"n":70,"o":-36,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":19,"b":-4,"c":-78,"d":-63,"e":-4,"f":-100,"g":21,"h":-62,"i":24,"j":-100,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get

    # ### TEST 4 ###

    fims_send -m pub -u /some/input/uri/accontactor 63
    fims_send -m pub -u /some/input/uri/p 35
    fims_send -m pub -u /some/input/uri/plim 29
    fims_send -m pub -u /some/input/uri/pramprise 5

    fims_send -m pub -u /another/input/uri/accontactor 87
    fims_send -m pub -u /another/input/uri/p 71
    fims_send -m pub -u /another/input/uri/plim 73
    fims_send -m pub -u /another/input/uri/pramprise 97

    fims_send -m pub -u /another/input/uri/sub_uri/accontactor 5
    fims_send -m pub -u /another/input/uri/sub_uri/p 43
    fims_send -m pub -u /another/input/uri/sub_uri/plim 69
    fims_send -m pub -u /another/input/uri/sub_uri/pramprise 53

    fims_send -m pub -u /new/some/input/uri/accontactor 54
    fims_send -m pub -u /new/some/input/uri/p 52
    fims_send -m pub -u /new/some/input/uri/plim 18
    fims_send -m pub -u /new/some/input/uri/pramprise 10

    fims_send -m pub -u /new/another/input/uri/accontactor 40
    fims_send -m pub -u /new/another/input/uri/p 3
    fims_send -m pub -u /new/another/input/uri/plim 87
    fims_send -m pub -u /new/another/input/uri/pramprise 11

    fims_send -m set -u /some/output/uri/k 1
    fims_send -m set -u /some/output/uri/l 1
    fims_send -m set -u /some/output/uri/m 1
    fims_send -m set -u /some/output/uri/n 1
    fims_send -m set -u /some/output/uri/o 1
    fims_send -m set -u /some/output/uri/p 1

    fims_send -m set -u /another/output/uri/p 1
    fims_send -m set -u /another/output/uri/q 1
    fims_send -m set -u /another/output/uri/r 1
    fims_send -m set -u /another/output/uri/s 1
    fims_send -m set -u /another/output/uri/t 1
    fims_send -m set -u /another/output/uri/u 1

    fims_send -m set -u /some/output/uri/sub/k 1
    fims_send -m set -u /some/output/uri/sub/l 1
    fims_send -m set -u /some/output/uri/sub/m 1
    fims_send -m set -u /some/output/uri/sub/n 1
    fims_send -m set -u /some/output/uri/sub/o 1
    fims_send -m set -u /some/output/uri/sub/p 1
    
    usetime 2


    expect='{"a":63,"b":35,"c":29,"d":5,"e":35,"f":87,"g":71,"h":73,"i":97,"j":87,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":63,"b":35,"c":29,"d":5,"e":35,"f":87,"g":71,"h":73,"i":97,"j":87,"k":5,"l":43,"m":69,"n":53,"o":5,"p":1,"q":1,"r":1,"s":1,"t":1,"u":1}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":54,"b":52,"c":18,"d":10,"e":52,"f":40,"g":3,"h":87,"i":11,"j":40,"k":1,"l":1,"m":1,"n":1,"o":1,"p":1}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get

    ### TEST 5 ###


    fims_send -m pub -u /some/input/uri/accontactor '{"value":51}'
    fims_send -m pub -u /some/input/uri/p '{"value":85}'
    fims_send -m pub -u /some/input/uri/plim '{"value":49}'
    fims_send -m pub -u /some/input/uri/pramprise '{"value":34}'

    fims_send -m pub -u /another/input/uri/accontactor '{"value":36}'
    fims_send -m pub -u /another/input/uri/p '{"value":98}'
    fims_send -m pub -u /another/input/uri/plim '{"value":83}'
    fims_send -m pub -u /another/input/uri/pramprise '{"value":28}'

    fims_send -m pub -u /another/input/uri/sub_uri/accontactor '{"value":56}'
    fims_send -m pub -u /another/input/uri/sub_uri/p '{"value":40}'
    fims_send -m pub -u /another/input/uri/sub_uri/plim '{"value":34}'
    fims_send -m pub -u /another/input/uri/sub_uri/pramprise '{"value":88}'

    fims_send -m pub -u /new/some/input/uri/accontactor '{"value":52}'
    fims_send -m pub -u /new/some/input/uri/p '{"value":15}'
    fims_send -m pub -u /new/some/input/uri/plim '{"value":45}'
    fims_send -m pub -u /new/some/input/uri/pramprise '{"value":12}'

    fims_send -m pub -u /new/another/input/uri/accontactor '{"value":81}'
    fims_send -m pub -u /new/another/input/uri/p '{"value":64}'
    fims_send -m pub -u /new/another/input/uri/plim '{"value":12}'
    fims_send -m pub -u /new/another/input/uri/pramprise '{"value":63}'

    fims_send -m set -u /some/output/uri/k '{"value":5}'
    fims_send -m set -u /some/output/uri/l '{"value":5}'
    fims_send -m set -u /some/output/uri/m '{"value":5}'
    fims_send -m set -u /some/output/uri/n '{"value":5}'
    fims_send -m set -u /some/output/uri/o '{"value":5}'
    fims_send -m set -u /some/output/uri/p '{"value":5}'

    fims_send -m set -u /another/output/uri/p '{"value":5}'
    fims_send -m set -u /another/output/uri/q '{"value":5}'
    fims_send -m set -u /another/output/uri/r '{"value":5}'
    fims_send -m set -u /another/output/uri/s '{"value":5}'
    fims_send -m set -u /another/output/uri/t '{"value":5}'
    fims_send -m set -u /another/output/uri/u '{"value":5}'

    fims_send -m set -u /some/output/uri/sub/k '{"value":5}'
    fims_send -m set -u /some/output/uri/sub/l '{"value":5}'
    fims_send -m set -u /some/output/uri/sub/m '{"value":5}'
    fims_send -m set -u /some/output/uri/sub/n '{"value":5}'
    fims_send -m set -u /some/output/uri/sub/o '{"value":5}'
    fims_send -m set -u /some/output/uri/sub/p '{"value":5}'


    usetime 2


    expect='{"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":51,"b":85,"c":49,"d":34,"e":85,"f":36,"g":98,"h":83,"i":28,"j":36,"k":56,"l":40,"m":34,"n":88,"o":56,"p":5,"q":5,"r":5,"s":5,"t":5,"u":5}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":52,"b":15,"c":45,"d":12,"e":15,"f":81,"g":64,"h":12,"i":63,"j":81,"k":5,"l":5,"m":5,"n":5,"o":5,"p":5}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get

    usetime $timeleft

    echo "test complete"
    #read_fims_log

}

menu $@