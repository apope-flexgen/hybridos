#!/bin/sh

clear

echo setup Link command to allow us to create links to varaibles 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"Link":{"value":"test",
                  "help": "link two variables",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"MakeLink"}]}]}}}' | jq

echo -n " press enter to continue " && read in
echo

#wait_pause
echo setup run command to allow us to periodically trigger an assetVar
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}}}' | jq

#wait_pause
echo -n " press enter to continue " && read in
echo
echo setup stop command to allow us to stop the run action
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"StopSched"}]}]}}}' | jq


echo -n " press enter to continue " && read in
clear
echo
echo create  dome dummy variables that we want to probe [inspect]
echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms' 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
                    { "state":{"value":"Ok"}}'

echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/config/bms '
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/config/bms '
                    { "MaxChargeCurrent":{"value":290}}'

echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
                    { "ChargeCurrent":{"value":0}}'

echo
echo

echo -n " press enter to continue " && read in

clear
echo
echo create a probe list to publish using links 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:state",
                      "linkName":"/status/bms:state","inValue":"test"}}' | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:MaxChargeCurrent",
                      "linkName":"/config/bms:MaxChargeCurrent","inValue":0}}' | jq


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:ChargeCurrent",
                      "linkName":"/status/bms:ChargeCurrent","inValue":0}}' | jq


echo -n " press enter to continue " && read in

clear
echo
echo set up the pub function for the probe
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "probeBms":{"value":1,"table":"/probe/bms","sendas":"/components/probe",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}' | jq

echo -n " press enter to continue " && read in

clear
echo
echo  tell the scheduler to trigger probeBms every 0.5 seconds
echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
     {"value":22,"uri":"/control/pubs:probeBms","every":0.5,"offset":0,"debug":0}' | jq

clear
echo
echo check the probe pubs
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control/pubs | jq

echo -n " press enter to continue " && read in


echo
echo check the probe pubs
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/control/pubs | jq
echo '/usr/local/bin/fims/fims_listen'
echo -n " press enter to continue " && read in

#wait_pause



