#!/bin/sh

# todo add the rest of them
URIS='
/components
/components/bms_info
/components/bms_info/bms_volts
/components/bms_info/bms_power
/components/bms_info/bms_volts/dc
/components/bms_info/bms_volts/ac
'

# todo add the rest of them
OPTS='
/end
/full
/naked
/hmm
/ui
/full/naked
/full/hmm
/full/ui
'

function send_uris()
{
    echo
    echo "testing opt $1"
    for uri in $URIS ; do
        echo build/release/test_geturi $uri $1
        # todo uncomment this
        #build/release/test_geturi $uri  $1
    done
}
function send_opt()
{
    echo 
    for opt in $OPTS ; do
        echo $opt
        send_uris $opt
    done  
}


send_opt

