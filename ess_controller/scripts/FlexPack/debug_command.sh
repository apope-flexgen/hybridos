/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
       {"debug":{"value": 0,
            "help": " turn on or off debug",
            "enabled":false, "actions":{"onSet":[{"func":[{"func":"SetDebug"}]}]}}}'

