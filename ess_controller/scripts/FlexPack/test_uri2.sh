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
        echo build/release/test_geturi $uri $1
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
#send_opt

#/components /debug/vmdebug/end
#/components /full
RESP1='{"/components/bms_info/bms_volts":\t{ "ac":\t{ "value":\t34.5 }, "dc":\t{"value":\t1234 }}, "/components/bms_info/bms_power":\t{"active_power":/t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'
yyy=`echo $RESP1 | hexdump`
components_full_resp=`echo $yyy`
zzz=$yyy
aaaa=`echo "/components_/full_resp: | tr "/" "#"`
echo $aaaa


echo "/components_/full_resp=${zzz}"


echo "########################"
decare -A map
map["/components_/full_resp"]=${yyy}

echo $map["/components_/full_resp"]

exit

#/components/bms_info /debug/vmdebug/end
#/components/bms_info /full
RESP2='{"/components/bms_info/bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234}}, "/components/bms_info/bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'
/components_full_resp=$RESP2

#/components/bms_info/bms_volts /debug/vmdebug/end
#/components/bms_info/bms_volts /full             
RESP3='{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234}}'
/components_full_resp=$RESP3

#/components/bms_info/bms_power /debug/vmdebug/end
#/components/bms_info/bms_power /full
RESP4='{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'
/components_full_resp=$RESP4

#/components/bms_info/bms_volts/dc /debug/vmdebug/end
#/components/bms_info/bms_volts:dc /debug/vmdebug/end
RESP5='{"value":\t1234}'
/components_full_resp=$RESP5

#/components/bms_info/bms_volts/ac /debug/vmdebug/end
#/components/bms_info/bms_volts:ac /debug/vmdbug/end
#/components/bms_info/bms_volts/ac /full
#/components/bms_info/bms_volts:ac /full
RESP6='{"value":\t34.5}'
/components_full_resp=$RESP6

#/components/bms_info/bms_volts/dc@max_limit /debug/vmdebug/end
#/components/bms_info/bms_volts:dc@max_limit /debug/vmdebug/end
#/components/bms_info/bms_volts/dc@max_limit /full
#/components/bms_info/bms_volts:dc@max_limit /full
RESP7='{"value":\t3.5}'
/components_full_resp=$RESP7

#/components/bms_info/bms_volts/dc /full
#/components/bms_info/bms_volts:dc /full
RESP8='{"value":\t1234,"max_limit":\t3.5}'
/components_full_resp=$RESP8

