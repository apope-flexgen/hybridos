#!/bin/sh
echo setup Link command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"Link":{"value":"test",
                  "help": "link two variables",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"MakeLink"}]}]}}}'

#wait_pause
echo setup run command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}}}'

#wait_pause

echo setup stop command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"StopSched"}]}]}}}'



echo create  variables that we want to probe [inspect]
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
                    { "state":{"value":"Ok"}}'

# and another
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/config/bms '
                    { "MaxChargeCurrent":{"value":290}}'

# and another
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
                    { "ChargeCurrent":{"value":0}}'

echo create a probe list to publish 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:state",
                      "linkName":"/status/bms:state","inValue":"test"}}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:MaxChargeCurrent",
                      "linkName":"/config/bms:MaxChargeCurrent","inValue":0}}'


/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
                    { "Link":{"value":"Ok", 
                      "varName":"/probe/bms:ChargeCurrent",
                      "linkName":"/status/bms:ChargeCurrent","inValue":0}}'

# set up the pub function for the probe
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "probeBms":{"value":1,"table":"/probe/bms","sendas":"/components/probe",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'

echo /usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
     {"value":22,"uri":"/control/pubs:probeBms","every":0.5,"offset":0,"debug":0}'

#wait_pause



