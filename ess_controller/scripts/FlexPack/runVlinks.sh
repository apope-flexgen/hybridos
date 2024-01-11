#!/usr/bin/sh

echo setup VLink command
/usr/local/bin/fims/fims_send -m set -r /me -u /flex/system/commands '
         {"vlink":{"value":"test",
                    "help": "link two var values",
                    "ifChanged":false, "enabled":true,
                    "actions":{"onSet":[{"func":[{"func":"RunVLinks"}]}]}}}'

/usr/local/bin/fims/fims_send -m set -r /me -u /flex/system/commands/vlink '
      {"value":"test","to":true, "from":true}'
