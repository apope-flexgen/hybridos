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

testname=uri_stuff_little_bit_of_everything


gitdir=$(cd `dirname $0` && cd ../.. && pwd)
gitdirs=$(cd `dirname $0` && cd ../.. && pwd)
echo "git1 " $gitdir
source $gitdir/scripts/test_funcs.sh unit_test
timeout=30
gitdir=$gitdirs
echo "git2 " $gitdir

## NOTE THAT WE EXPECT 2 TESTS TO FAIL FOR THIS

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

    expect='{"a":0,"b":0,"c":"","d":0,"e":"","f":"","g":0,"h":0,"i":"","j":0,"k":0,"naked_bobcat":0,"naked_cheetah":0}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    expect='{"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":0},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":0}}'
    get="fims_send -m get -r /$$ -u /some/two"
    runget $expect $get

    ### TEST 2 ###
    usetime 2
    fims_send -m pub -u /some '"6"'
    fims_send -m pub -u /some/1 1
    fims_send -m pub -u /some/2 2
    fims_send -m pub -u /some/input/uri '{"accontactor":51,"p":47,"plim":93,"pramprise":61,"value":"some_string"}'
    fims_send -m pub -u /another/input/uri '{"accontactor":30,"p":52,"plim":81,"pramprise":31,"sub_uri":{"accontactor":53,"p":31,"plim":69,"pramprise":-70}}'
    fims_send -m pub -u /new/some/input/uri '{"accontactor":18,"p":-82,"plim":-34,"pramprise":-17}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":-41,"p":0,"plim":-81,"pramprise":-29}'
    fims_send -m pub -u /some/output/uri '{"a":51,"value":"another_string"}'
    



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

    expect='{"a":51,"b":47,"c":"some_string","d":51,"e":"another_string","f":"6","g":1,"h":2,"i":"another_string","j":51,"k":51,"naked_bobcat":2,"naked_cheetah":2}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    expect='{"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":2},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":2}}'
    get="fims_send -m get -r /$$ -u /some/two"
    runget $expect $get


    ### TEST 3 ###

    fims_send -m pub -u /some '{
                                  "1":111,
                                  "2":222,
                                  "input": {
                                              "uri":{"accontactor":59,"p":16,"plim":4,"pramprise":98,"value":"aaa"}
                                  },
                                  "value":"12345",
                                  "enabled":false,
                                  "scale":1

                                }'
    fims_send -m pub -u /another/input/uri '{"accontactor":31,"p":98,"plim":85,"pramprise":74,"sub_uri":{"accontactor":47,"p":17,"plim":77,"pramprise":55}}'

    fims_send -m pub -u /new/some/input/uri '{"accontactor":{"value":28},"p":{"value":33},"plim":{"value":3},"pramprise":{"value":25}}'
    fims_send -m pub -u /new/another/input/uri '{"accontactor":{"value":64},"p":{"value":95},"plim":{"value":84},"pramprise":{"value":7}}'
    fims_send -m pub -u /some/output/uri '{"a":59,"enabled":false,"scale":1000,"value":"blah"}'
    

    usetime 2    

    expect='{"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri"
    runget $expect $get

    expect='{"a":59,"b":16,"c":4,"d":98,"e":16,"f":31,"g":98,"h":85,"i":74,"j":31,"k":47,"l":17,"m":77,"n":55,"o":47,"p":0,"q":0,"r":0,"s":0,"t":0,"u":0}'
    get="fims_send -m get -r /$$ -u /another/output/uri"
    runget $expect $get

    expect='{"a":28,"b":33,"c":3,"d":25,"e":33,"f":64,"g":95,"h":84,"i":7,"j":64,"k":0,"l":0,"m":0,"n":0,"o":0,"p":0}'
    get="fims_send -m get -r /$$ -u /some/output/uri/sub"
    runget $expect $get

    expect='{"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"another_string","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    expect='{"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}'
    get="fims_send -m get -r /$$ -u /some/two"
    runget $expect $get

    ### TEST 4 ###
    fims_send -m pub -u /some/output/uri '{"enabled":true,"value":"blah"}'
    fims_send -m pub -u /some/output/uri '{"enabled":true,"value":"blah"}'
    usetime 2
    expect='{"a":59,"b":16,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":59,"k":59,"naked_bobcat":222,"naked_cheetah":222}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    ### TEST 5 ###
    ### TEST DON'T RESPOND TO GETS ON INPUTS
    ### NOTE: WE EXPECT THIS TEST TO FAIL
    usetime 2
    expect="Receive Timeout."
    get="fims_send -m get -r /$$ -u /some/input/uri"
    runget $get

    ### TEST 6 ###
    ### TEST DON'T RESPOND TO GETS ON INPUTS
    ### NOTE: WE EXPECT THIS TEST TO FAIL
    usetime 2
    expect="Receive Timeout."
    get="fims_send -m get -r /$$ -u /some/input/uri/accontactor"
    runget $get

    ### TEST 7 ###


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

    expect='{"a":51,"b":85,"c":"aaa","d":59,"e":"blah","f":"12345","g":111,"h":222,"i":"blah","j":51,"k":59,"naked_bobcat":222,"naked_cheetah":222}'
    get="fims_send -m get -r /$$ -u /some/one"
    runget $expect $get

    expect='{"clothed_bobcat":{"scale":1000,"ui_type":"none","units":"degC","value":222},"clothed_cheetah":{"scale":1000,"ui_type":"none","units":"degC","value":222}}'
    get="fims_send -m get -r /$$ -u /some/two"
    runget $expect $get

    

    usetime $timeleft

    echo "test complete"

}

menu $@