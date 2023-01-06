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
URIS='
/components
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
#run a reduced set for now 
OPTS='
/debug/vmdebug/end
/full
'
RESP1=`echo -e '{ "/components/bms_info/bms_volts":\t{ "ac":\t{ "value":\t34.5 }, "dc":\t{ "value":\t1234, "max_limit":\t3.5 } }, "/components/bms_info/bms_power":\t{ "active_power":\t{ "value":\t300500 }, "reactive_power":\t{ "value":\t3450 } } }'`

/components_/full_resp=$RESP1

function send_uris()
{
    echo
    echo "testing opt $1"
    for uri in $URIS ; do
        echo "##########################";echo ;echo "build/release/test_geturi $uri $1"
        # todo uncomment this
        foo=`build/release/test_geturi $uri  $1`
        echo "test ... ${uri}_$1"
        echo $foo
        # you need to look up how to do string compares in sh scripting 
        if [ "$foo" = "$RESP1" ] ; then
            echo "${uri}_$1  >>> PASS"
        else
            echo "foo  =  $foo"
            xxx=`echo $foo | hexdump`
            echo "RESP1= $RESP1"
            yyy=`echo $RESP1 | hexdump`
            rr=${uri}_$1_resp
            echo $rr
            echo "${uri}_$1  >>> FAIL"
            if [ "$xxx" = "$yyy" ] ; then
                echo "OK it worked"
            else
                echo " still failed"
            fi
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
