#!/bin/bash
while [ true ]
do
fims_send -m pub -u /some '"6"'
fims_send -m pub -u /some/1 1
fims_send -m pub -u /some/2 2
fims_send -m pub -u /some/input/uri '{"accontactor":51,"p":47,"plim":93,"pramprise":61,"value":"some_string"}'
fims_send -m pub -u /another/input/uri '{"accontactor":30,"p":52,"plim":81,"pramprise":31,"sub_uri":{"accontactor":53,"p":31,"plim":69,"pramprise":-70}}'
fims_send -m pub -u /new/some/input/uri '{"accontactor":18,"p":-82,"plim":-34,"pramprise":-17}'
fims_send -m pub -u /new/another/input/uri '{"accontactor":-41,"p":0,"plim":-81,"pramprise":-29}'
fims_send -m pub -u /some/output/uri '{"a":51,"value":"another_string"}'
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
fims_send -m pub -u /some/output/uri '{"enabled":true,"value":"blah"}'
fims_send -m pub -u /some/output/uri '{"enabled":true,"value":"blah"}'
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
done