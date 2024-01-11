#!/bin/sh
# test the limit operation 

pause=$#
resList=""

#echo "pause = $pause"
wait_pause()
{
   if [ "$pause" == "1" ] ; then 
      echo -n " press enter to continue " && read in 
   fi 
}
#send_expect 2 '{"EnumTest1":{"value":"VALUE_1_2"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

send_expect()
{
# $2{"EnumTest1":{"value":"VALUE_1_2"}}
# $3 /ess/full/components/ess/testEnumVec
# $1 2
# $4 /ess/full/status/test/EnumTest1
echo 
echo -n "  >>>>send value $2  "
echo -n "   expecting   $3 got >>> "
/usr/local/bin/fims/fims_send  -m set -u $4 $2
res=`/usr/local/bin/fims/fims_send  -m get -r /$$ -u $5`
echo -n "    >>> "
echo -n $res
echo -n "    >>> "

if [ $res == $3 ]; then 
  pf="[pass]"
else
  pf="[fail]"
fi
echo $pf

resList="$resList
 [$1] ==> $pf :: $2 ==> $res"

}

check_resp()
{
# $2{"EnumTest1":{"value":"VALUE_1_2"}}
# $3 /ess/full/components/ess/testEnumVec
# $1 2
# $4 /ess/full/status/test/EnumTest1
echo 
res=`/usr/local/bin/fims/fims_send  -m get -r /$$ -u $2`
echo -n "    >>> "
echo -n $res
echo -n "    >>> "

if [ $res == $1 ]; then 
  pf="[pass]"
else
  pf="[fail]"
fi
echo $pf

}

# limits have fixed upper and lower values 
#   or then can use vars for upper and lower values (post MVP)
#   they can be enbled , have enable and target uris
#

# bool uri   = abf->gotFeat("uri");
# bool gotmask  = abf->gotFeat("mask");
# bool gotsval  = abf->gotFeat("svalue");
# bool gotnval  = abf->gotFeat("nvalue");
# bool gotshift = abf->gotFeat("shift");
# bool gotbit   = abf->gotFeat("bit");
# if the av->valuestring == svalue set bit in uri
# if the av->valuestring == nvalue clear bit in uri

# basic --> given a bit number it either sets or clears a bit in the uri

echo -n '  >>>> setting up config limits action, simple limits '

#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/esstest" '{"testLimitVec": 
   { "value":0,"stuff":1234,"debug":false, "enabled":true,
   "actions": { "onSet": [{ "limits":     [
                                {"low":  -25, "high": 25, "uri":  "/system/limit:test"}
                                ]
                         }]
               }
      }}' | jq

#sleep 1

/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/system/limit" '{"test": 
   { "value":0}}' | jq

wait_pause



send_expect 'Limit 1 Set ' 27 '{"testLimitVec":{"value":25}}' /ess/full/components/esstest/testLimitVec /ess/components/esstest
send_expect 'Limit 2 Set ' '{"testLimitVec":{"value":-27}}' '{"testLimitVec":{"value":-25}}' /ess/full/components/esstest /ess/components/esstest
send_expect 'Limit 3 Set ' 10 '{"testLimitVec":{"value":10}}' /ess/full/components/esstest/testLimitVec /ess/components/esstest

wait_pause
echo -n '  >>>> setting up config limits action, av limits '

#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/config/esstest" '{"low":-22, "high":22}' 

echo " setting limits "
foo=`/usr/local/bin/fims/fims_send  -m get -r /$$ -u /ess/config/esstest`
echo $foo | jq
echo " limits set"



/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/esstest" '{"testLimitVec": 
   { "value":0,"stuff":1234,"debug":false, "enabled":true,
   "actions": { "onSet": [{ "limits":     [
                                {"lowuri": "/config/esstest:low", "highuri": "/config/esstest:high", "uri":  "/system/limit:test"}
                                ]
                         }]
               }
      }}' | jq

send_expect 'Limit 1 Set ' 27 '{"testLimitVec":{"value":22}}' /ess/full/components/esstest/testLimitVec /ess/components/esstest
send_expect 'Limit 2 Set ' '{"testLimitVec":{"value":-27}}' '{"testLimitVec":{"value":-22}}' /ess/full/components/esstest /ess/components/esstest
send_expect 'Limit 3 Set ' 10 '{"testLimitVec":{"value":10}}' /ess/full/components/esstest/testLimitVec /ess/components/esstest

foo=`/usr/local/bin/fims/fims_send  -m get -r /$$ -u /ess/full/components/esstest`

echo $foo | jq

echo " test completed"

echo "$resList"
