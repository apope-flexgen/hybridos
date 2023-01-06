#!/bin/sh
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

# create a variable that we want to probe
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms '
                    { "state":{"value":"Ok"}}'

# set up the pub function for the probe
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "probeBmsState":{"value":1,"table":"/status/bms:state","sendas":"/bms/probe",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:probeBmsState","every":0.5,"offset":0,"debug":0}'

#wait_pause



