#!/bin/sh
# hput and hget

URIS='
/components/bms_info/bms_volts
/components/bms_info/bms_power
'
OPTS='
/full
/full/naked
'

RESP1='{"/components/bms_info/bms_volts":\t{ "ac":\t{ "value":\t34.5 }, "dc":\t{"value":\t1234 }}, "/components/bms_info/bms_power":\t{"active_power":/t{"value":\t300500},"reactive_power":\t{"value":\t3450}}'



# hput /components/bms_info/bms_volts /full/naked $RESP1
hput () {
    xx=$1_$2_resp
    echo "hput xx = $xx"
    xxx=`echo $xx | tr '/' '_'|tr ':' '_'`
    echo "hput xxx = $xxx"    
    eval hash"$xxx"='$3'
}


hget () {
    xx=$1_$2_resp
    #echo "hget $xx"
    xxx=`echo $xx | tr '/' '_'| tr ':' '_'`
    eval echo '${hash'"$xxx"'#hash}'
}


echo "RESP1 = $RESP1"
yyy=`echo $RESP1  `
echo "yyy = $yyy"

hput "/components/bms_info/bms_volts:dc" "/full/naked" "$RESP1"

#hput compfoo "response foo"
#hput compfoo2 "response foo2"
#hput compfoo3 "response foo3"
echo "return from hget"
foo2=`hget "/components/bms_info/bms_volts:dc" "/full/naked"`

#yyy=`echo $RESP1 | hexdump`
if [ "$foo2" = "$yyy" ] ; then
   echo "it worked"
   echo "foo2 = $foo2"
else
   echo "it failed"
   echo "foo2 = $foo2"
   echo "yyy = $yyy"

fi



