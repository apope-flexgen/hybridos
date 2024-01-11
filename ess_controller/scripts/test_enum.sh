#!/bin/sh
# test the ebum action 

ResList=""

Col='\033[0m'    # Text Reset

# Regular           Bold                Underline           High Intensity      BoldHigh Intens     Background          High Intensity Backgrounds
Bla='\033[0;30m';     BBla='\033[1;30m';    UBla='\033[4;30m';    IBla='\033[0;90m';    BIBla='\033[1;90m';   On_Bla='\033[40m';    On_IBla='\033[0;100m';
Red='\033[33;31m';     BRed='\033[1;31m';    URed='\033[4;31m';    IRed='\033[0;91m';    BIRed='\033[1;91m';   On_Red='\033[41m';    On_IRed='\033[0;101m';
Gre='\033[33;32m';     BGre='\033[1;32m';    UGre='\033[4;32m';    IGre='\033[0;92m';    BIGre='\033[1;92m';   On_Gre='\033[42m';    On_IGre='\033[0;102m';
Yel='\033[0;33m';     BYel='\033[1;33m';    UYel='\033[4;33m';    IYel='\033[0;93m';    BIYel='\033[1;93m';   On_Yel='\033[43m';    On_IYel='\033[0;103m';
Blu='\033[0;34m';     BBlu='\033[1;34m';    UBlu='\033[4;34m';    IBlu='\033[0;94m';    BIBlu='\033[1;94m';   On_Blu='\033[44m';    On_IBlu='\033[0;104m';
Pur='\033[0;35m';     BPur='\033[1;35m';    UPur='\033[4;35m';    IPur='\033[0;95m';    BIPur='\033[1;95m';   On_Pur='\033[45m';    On_IPur='\033[0;105m';
Cya='\033[0;36m';     BCya='\033[1;36m';    UCya='\033[4;36m';    ICya='\033[0;96m';    BICya='\033[1;96m';   On_Cya='\033[46m';    On_ICya='\033[0;106m';
Whi='\033[0;37m';     BWhi='\033[1;37m';    UWhi='\033[4;37m';    IWhi='\033[0;97m';    BIWhi='\033[1;97m';   On_Whi='\033[47m';    On_IWhi='\033[0;107m';
#echo -e "${Blu}blue ${Red}red ${RCol}etc...."

pause=$#
#echo "pause = $pause"

#if [ "$pause" == "1" ] ; then 
#   echo " pause was 1" 
#fi

#if [ "$pause" == "1" ] ; then 
#   echo -n " press enter to continue " && read in 
#fi
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
   pf"[fail]"
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
  echo "[pass]"
else
  echo "[fail]"
fi

}

echo -n '  >>>> setting up config action for enum '
# {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs:PCSStatusResp", "outValue": "PUP"}
#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/ess" '{"testEnumVec": 
   { "value":-1,"debug":true, "enabled":true,
   "ifChanged":false,
   "defVal":"DefaultVal","defUri":"/status/test:EnumTestDef",
   "actions": { "onSet": [{ "enum":     [
            {"shift": 0, "mask": 3, "inValue": 0, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_0"},
            {"shift": 0, "mask": 3, "inValue": 1, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_1"},
            {"shift": 0, "mask": 3, "inValue": 2, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_2"},
            {"shift": 2, "mask": 3, "inValue": 1,   "uri": "/status/test:EnumTest2", "outValue": "VALUE_2_1"},
            {"shift": 2, "mask": 3, "inValue": 2,   "uri": "/status/test:EnumTest2", "outValue": "VALUE_2_2"},
            {"shift": 2, "mask": 3, "inValue": 3,   "uri": "/status/test:EnumTest2", "outValue": "VALUE_2_3"}
                                ]
                         }]
               }
      }}' | jq

#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/status/test" '{"EnumTest1":   { "value":"Unset"}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/status/test" '{"EnumTest2":   { "value":"Unset"}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/status/test" '{"EnumTestDef": { "value":"Unset"}}'

/usr/local/bin/fims/fims_send  -m get -r /$$ -u /ess/full/status/test           

wait_pause

send_expect 'Basic Enum' 2 '{"EnumTest1":{"value":"VALUE_1_2"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1
send_expect 'Basic Enum' 0 '{"EnumTest1":{"value":"VALUE_1_0"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

send_expect 'Basic Enum' 4 '{"EnumTest2":{"value":"VALUE_2_1"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest2
send_expect 'Basic Enum' 8 '{"EnumTest2":{"value":"VALUE_2_2"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest2
send_expect 'Basic Enum' 12 '{"EnumTest2":{"value":"VALUE_2_3"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest2


wait_pause

send_expect 'Basic Enum' 2 '{"EnumTest1":{"value":"VALUE_1_2"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1
wait_pause


send_expect 'Multiple Enum' 9 '{"EnumTest1":{"value":"VALUE_1_1"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1
check_resp '{"EnumTest2":{"value":"VALUE_2_2"}}' /ess/full/status/test/EnumTest2


#/usr/local/bin/fims/fims_send  -m get -r /$$ -u /ess/full/status/test/EnumTest2
           
wait_pause

echo -n '  >>>> setting up revised config action for enum '
# {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs:PCSStatusResp", "outValue": "PUP"}
#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/ess" '{"testEnumVec": 
   { "value":-1,"debug":true, "enabled":true,
   "defVal":"DefaultVal","defUri":"/status/test:EnumTestDef",
   "actions": { "onSet": [{ "enum":     [
            {"shift": 0, "mask": 3, "inValue": 0, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_0"},
            {"shift": 0, "mask": 3, "inValue": 1, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_1"},
            {"shift": 0, "mask": 3, "inValue": 2, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_2"}
                                ]
                         }]
               }
      }}' | jq

wait_pause

echo 
/usr/local/bin/fims/fims_send  -m set -u /ess/full/status/test '{"EnumTestDef":{"value":"SomeVal"}}'

echo " look at current values"
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /ess/full/status/test   

send_expect 'Enum Default' 3 '{"EnumTestDef":{"value":"DefaultVal"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTestDef

wait_pause

echo -n '  >>>> setting up config action for enum wth ifChanged and resetChange false'
# {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs:PCSStatusResp", "outValue": "PUP"}
#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/ess" '{"testEnumVec": 
   { "value":-1,"debug":true, "enabled":true,
   "ifChanged":true,
   "resetChange":false,
   "defVal":"DefaultVal","defUri":"/status/test:EnumTestDef",
   "actions": { "onSet": [{ "enum":     [
            {"shift": 0, "mask": 3, "inValue": 0, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_0"},
            {"shift": 0, "mask": 3, "inValue": 1, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_1"},
            {"shift": 0, "mask": 3, "inValue": 2, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_2"}
                                ]
                         }]
               }
      }}' | jq

wait_pause

send_expect 'ifChanged no reset' 0 '{"EnumTest1":{"value":"VALUE_1_0"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

send_expect 'ifChanged no reset' '{"EnumTest1":{"value":"OtherValue"}}' '{"EnumTest1":{"value":"OtherValue"}}' /ess/full/status/test /ess/full/status/test/EnumTest1

wait_pause

echo -n '  >>>>send value  0 should  cause change '

send_expect 'ifChanged no reset' 0 '{"EnumTest1":{"value":"VALUE_1_0"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

send_expect 'ifChanged no reset' 1 '{"EnumTest1":{"value":"VALUE_1_1"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

echo -n '  >>>> setting up config action for enum wth ifChanged and resetChange true (default)'
# {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs:PCSStatusResp", "outValue": "PUP"}
#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/ess" '{"testEnumVec": 
   { "value":-1,"debug":true, "enabled":true,
   "ifChanged":true,
   "resetChange":true,
   "defVal":"DefaultVal","defUri":"/status/test:EnumTestDef",
   "actions": { "onSet": [{ "enum":     [
            {"shift": 0, "mask": 3, "inValue": 0, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_0"},
            {"shift": 0, "mask": 3, "inValue": 1, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_1"},
            {"shift": 0, "mask": 3, "inValue": 2, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_2"}
                                ]
                         }]
               }
      }}' | jq

wait_pause

echo -n '  >>>>send value  0 should  cause change '

send_expect 'ifChanged with reset' 0 '{"EnumTest1":{"value":"VALUE_1_0"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1


wait_pause

send_expect 'ifChanged no reset' '{"EnumTest1":{"value":"OtherValue"}}' '{"EnumTest1":{"value":"OtherValue"}}' /ess/full/status/test /ess/full/status/test/EnumTest1

wait_pause
 
echo -n '  >>>>send value  0 should  Not cause change '

send_expect 'ifChanged no reset' 0 '{"EnumTest1":{"value":"OtherValue"}}' /ess/full/status/test /ess/full/status/test/EnumTest1

wait_pause

send_expect 'ifChanged no reset' 1 '{"EnumTest1":{"value":"VALUE_1_1"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause
 
echo -n '  >>>> setting up config action for enum wth inRange'
# {"shift": 0, "mask": 255, "inValue": 0, "uri": "/status/pcs:PCSStatusResp", "outValue": "PUP"}
#sleep 1
/usr/local/bin/fims/fims_send -m set -r /$$ -u "/ess/components/ess" '{"testEnumVec": 
   { "value":-1,"debug":true, "enabled":true,
   "ifChanged":false,
   "defVal":"DefaultVal","defUri":"/status/test:EnumTestDef",
   "actions": { "onSet": [{ "enum":     [
            {"shift": 0, "mask": 255, "inValue": 10, "inValue-": 1,"inValue+": 2,"useRange":true, "uri": "/status/test:EnumTest1",
                   "outValue": "VALUE_9_to_12"},
            {"shift": 0, "mask": 255, "inValue": 20, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_20"},
            {"shift": 0, "mask": 255, "inValue": 30, "uri": "/status/test:EnumTest1", "outValue": "VALUE_1_30"}
                                ]
                         }]
               }
      }}' | jq

wait_pause


/usr/local/bin/fims/fims_send  -m set -u /ess/full/status/test '{"EnumTest1": "NotInRange"}' 

send_expect 'inRange out of range' 8 '{"EnumTest1":{"value":"NotInRange"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1


wait_pause
send_expect 'inRange within range' 9.1 '{"EnumTest1":{"value":"VALUE_9_to_12"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

send_expect 'inRange within range' 20 '{"EnumTest1":{"value":"VALUE_1_20"}}' /ess/full/components/ess/testEnumVec /ess/full/status/test/EnumTest1

wait_pause

echo " test completed"
echo "$resList"


