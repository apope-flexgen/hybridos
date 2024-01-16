#!/bin/s
# load the bms_sim managers and template
echo setup Link command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
        {"vlink":{"value":"test",
                   "help": "give multiple assetvars the same value",
                   "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunVLinks"}]}]}}}'



/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/vlink '
                {"value":"ess", "pname":"flex", "aname":"ess"}'

/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/vlink '
                {"value":"bms", "pname":"ess", "aname":"bms"}'


/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/vlink '
                {"value":"pcs", "pname":"ess", "aname":"pcs"}'


/usr/local/bin/fims/fims_send  -m set -r /$$ -u /flex/full/system/commands/vlink '
                {"value":"site", "pname":"ess", "aname":"site"}'



