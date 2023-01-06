#!/bin/sh
# todo add the rest of them
URIS='
/components
/components_bms_info
/components_bms_info_bms_volts
/components_bms_info_bms_power
/components_bms_info_bms_volts_dc
/components_bms_info_bms_volts_ac
/components_bms_info_bms_volts_dc@max_limit
/components_bms_info_bms_volts:ac
/components_bms_info_bms_volts:dc
/components_bms_info_bms_volts:dc@max_limit
'
# todo add the rest of them
OPTS='
/full
/naked
/ui
/hmm
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

RESP1=`echo -e '{"/components/bms_info/bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}},"/components/bms_info/bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}}'`
RESP2=`echo -e '{"/components/bms_info/bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234}},"/components/bms_info/bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'`
RESP3=`echo -e '{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234}}'`
RESP4=`echo -e '{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'`
RESP5=`echo -e '{"value":\t1234,"max_limit":\t3.5}'`
RESP6=`echo -e '{"value":\t34.5}'`
RESP7=`echo -e '{"value":\t3.5}'`

RESP8=`echo -e '"/components/bms_info/bms_volts":\t{"ac":\t34.5,"dc":\t1234},"/components/bms_info/bms_power":\t"active_power":\t300500,"reactive_power":\t3450}'`
RESP9=`echo -e '"ac":\t34.5,"dc":\t1234'`
RESP10=`echo -e '"active_power":\t300500,"reactive_power":\t3450'`
RESP11=`echo -e '1234'`
RESP12=`echo -e '34.5'`
RESP13=`echo -e '3.5'`

RESP14=`echo -e '"bms_info":\t{"bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}},"bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}}'`
RESP15=`echo -e '"bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}},"bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'`
RESP16=`echo -e '"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}'`
RESP17=`echo -e '"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}'`
RESP18=`echo -e '"value":\t1234,"max_limit":\t3.5'`
RESP19=`echo -e '"value":\t34.5'`


RESP20=`echo -e '"/bms_info/bms_volts":\t{"bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}}},"/bms_info/bms_power":\t{"bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}}'`
RESP21=`echo -e '"/bms_volts":\t{"bms_volts":\t{"ac":\t{"value":\t34.5},"dc":\t{"value":\t1234,"max_limit":\t3.5}}},"/bms_power":\t{"bms_power":\t{"active_power":\t{"value":\t300500},"reactive_power":\t{"value":\t3450}}}'`
RESP22=`echo -e 'hmm components/bms_info/bms_volts'`
RESP23=`echo -e 'hmm components/bms_info/bms_power'`
RESP24=`echo -e 'hmm components/bms_info/bms_volts/dc'`
RESP25=`echo -e 'hmm components/bms_info/bms_volts/ac'`
RESP26=`echo -e 'hmm components/bms_info/bms_volts/dc@max_limit'`

RESP27=`echo -e '"/components/bms_info/bms_volts":\t{"ac":\t34.5,"dc":\t1234},"/components/bms_info/bms_power":\t{"active_power":\t300500,"reactive_power":\t3450}'`


hput () {
    xx=$1_$2_resp
    echo "xx = $xx"
    xxx=`echo $xx | tr '/' '_' | tr ':' '_' | tr '@' '_'`
    echo "xxx = $xxx"    
    eval hash"$xxx"='$3'
}

hget () {
    xx=$1_$2_resp
    xxx=`echo $xx | tr '/' '_' | tr ':' '_' | tr '@' '_'`
    eval echo '${hash'"$xxx"'#hash}'
}

#full
hput /components /full "$RESP1"
hput /components/bms_info /full "$RESP2"
hput /components/bms_info/bms_volts /full "$RESP3"
hput /components/bms_info/bms_power /full "$RESP4"
hput /components/bms_info/bms_volts/dc /full "$RESP5"
hput /components/bms_info/bms_volts/ac /full "$RESP6"
hput /components/bms_info/bms_volts/dc@max_limit /full "$RESP7"
hput /components/bms_info/bms_volts:dc /full "$RESP5"
hput /components/bms_info/bms_volts:ac /full "$RESP6"
hput /components/bms_info/bms_volts:dc@max_limit /full "$RESP7"

#naked
hput /components /naked "$RESP8"
hput /components/bms_info /naked "$RESP8"
hput /components/bms_info/bms_volts /naked "$RESP9"
hput /components/bms_info/bms_power /naked "$RESP10"
hput /components/bms_info/bms_volts/dc /naked "$RESP11"
hput /components/bms_info/bms_volts/ac /naked "$RESP12"
hput /components/bms_info/bms_volts/dc@max_limit /naked "$RESP13"
hput /components/bms_info/bms_volts:dc /naked "$RESP11"
hput /components/bms_info/bms_volts:ac /naked "$RESP12"
hput /components/bms_info/bms_volts:dc@max_limit /naked "$RESP13"

#ui
hput /components /ui "$RESP14"
hput /components/bms_info /ui "$RESP15"
hput /components/bms_info/bms_volts /ui "$RESP16"
hput /components/bms_info/bms_power /ui "$RESP17"
hput /components/bms_info/bms_volts/dc /ui "$RESP18"
hput /components/bms_info/bms_volts/ac /ui "$RESP19"
hput /components/bms_info/bms_volts/dc@max_limit /ui "$RESP13"
hput /components/bms_info/bms_volts:dc /ui "$RESP18"
hput /components/bms_info/bms_volts:ac /ui "$RESP19"
hput /components/bms_info/bms_volts:dc@max_limit /ui "$RESP13"

#hmm
hput /components /hmm "$RESP20"
hput /components/bms_info /hmm "$RESP21"
hput /components/bms_info/bms_volts /hmm "$RESP22"
hput /components/bms_info/bms_power /hmm "$RESP23"
hput /components/bms_info/bms_volts/dc /hmm "$RESP24"
hput /components/bms_info/bms_volts/ac /hmm "$RESP25"
hput /components/bms_info/bms_volts/dc@max_limit /hmm "$RESP26"
hput /components/bms_info/bms_volts:dc /hmm "$RESP24"
hput /components/bms_info/bms_volts:ac /hmm "$RESP25"
hput /components/bms_info/bms_volts:dc@max_limit /hmm "$RESP26"

#full/naked
hput /components /full/naked "$RESP27"
hput /components/bms_info /full/naked "$RESP27"
hput /components/bms_info/bms_volts /full/naked "$RESP9"
hput /components/bms_info/bms_power /full/naked "$RESP10"
hput /components/bms_info/bms_volts/dc /full/naked "$RESP11"
hput /components/bms_info/bms_volts/ac /full/naked "$RESP12"
hput /components/bms_info/bms_volts/dc@max_limit /full/naked "$RESP13"
hput /components/bms_info/bms_volts:dc /full/naked "$RESP11"
hput /components/bms_info/bms_volts:ac /full/naked "$RESP12"
hput /components/bms_info/bms_volts:dc@max_limit /full/naked "$RESP13"

num_failures=0
num_passes=0

function send_uris()
{
    echo
    echo "testing opt $1"
    for uri in $URIS ; do
        echo "##########################";echo ;echo "build/release/test_geturi $uri $1"
        foo2=`hget "$uri" "$1" `
        yyy=`hget "$uri" $1 $2`

        if [ "$foo2" = "$yyy" ] ; then
            echo "foo2 = $foo2"
            echo "yyy = $yyy"
            echo "it worked"
            num_passes=$(($num_passes + 1))
        else
            echo "foo2 = $foo2"
            echo "yyy = $yyy"
            echo "it failed"
            num_failures=$(($num_failures + 1))

        fi
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
echo "num failures ==> $num_failures"
echo "num passes ==> $num_passes"
