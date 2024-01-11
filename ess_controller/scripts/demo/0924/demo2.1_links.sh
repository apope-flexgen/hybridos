#!/bin/s
# demo the links command
pause=1
wait_pause()
{
   if [ "$pause" == "1" ] ; then
      echo -n " press enter to continue " && read in
   fi
}
echo inspect bms links
wait_pause

fims_send -m get -r /$$ -u /flex/full/links/bms | jq

echo setup Link command
wait_pause
fims_send -m set -r /$$ -u /flex/system/commands '
        {"link":{"value":"test",
                   "help": "give a single assetvar multiple names",
                   "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunLinks"}]}]}}}' | jq

echo inspect commands
wait_pause
fims_send -m get -r /$$ -u /flex/full/system/commands | jq


echo run links for ess
wait_pause
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
                {"value":"ess", "pname":"flex", "aname":"ess"}' | jq

echo run links for pcs
wait_pause
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
                {"value":"pcs", "pname":"ess", "aname":"pcs"}' | jq

echo run links for bms
wait_pause
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
                {"value":"bms", "pname":"ess", "aname":"bms"}' | jq


echo run links for site
wait_pause
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/link '
                {"value":"site", "pname":"ess", "aname":"site"}' | jq

echo inspect amap/ess
wait_pause
fims_send -m get -r /$$ -u /flex/full/amap/ess | jq

echo inspect amap/pcs
wait_pause
fims_send -m get -r /$$ -u /flex/full/amap/pcs | jq

echo inspect amap/bms
wait_pause
fims_send -m get -r /$$ -u /flex/full/amap/bms | jq

exit

echo set component and check status 

/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/components/bms_info/bms_soc 1234.5
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/full/status/bms/bms_soc 

echo set status and check component
/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/status/bms/bms_soc  5432.1
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/components/bms_info/bms_soc 

echo check amap
/usr/local/bin/fims/fims_send  -m get -r /$$ -u /flex/amap/bms   | jq
 


