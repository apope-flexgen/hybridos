#!/bin/sh
# todo add the rest of them
URIS='
/components
/components/bms_info
/components/bms_info/bms_volts
/components/bms_info/bms_power
/components/bms_info/bms_volts/dc
/components/bms_info/bms_volts/ac
/components/bms_info/bms_volts/dc@max_limit
/components/bms_info/bms_volts:dc
/components/bms_info/bms_volts:ac
/components/bms_info/bms_volts:dc@max_limit
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
/naked/hmm
/naked/ui
/hmm/ui
/full/hmm/naked
/full/naked/ui
/full/hmm/ui
/naked/hmm/ui
/full/naked/hmm/ui
'
function send_uris()
{
    echo
    echo "testing opt $1"
    for uri in $URIS ; do
        echo "##########################";echo ;echo "build/release/test_geturi $uri $1"
        # todo uncomment this
        build/release/test_geturi $uri  $1
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
