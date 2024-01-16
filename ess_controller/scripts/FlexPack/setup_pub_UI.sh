#!/bin/sh
# setup_pubUI.sh

#wait_pause
echo set up run command
/usr/local/bin/fims_send -m set -r /$$ -u /ess/system/commands '
         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunSched"}]}]}}}'

#wait_pause

echo setup stop command
/usr/local/bin/fims_send -m set -r /$$ -u /ess/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"StopSched"}]}]}}}'

# set up the pub functions for the assets scheduler 
/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/control/pubs '
                    { "pubUI":{"value":1,"table":"/assets",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess", "func":"RunPub"}]}]}}}'
/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/control/pubs '
                    { "pubUIBms":{"value":1,"table":"/assets/bms",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"amap":"ess","func":"RunPub"}]}]}}}'
#wait_pause
