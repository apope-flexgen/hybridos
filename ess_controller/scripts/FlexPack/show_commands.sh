echo setup commands
echo setup run command
/usr/local/bin/fims/fims_send -m get -r /$$ -u /flex/full/system/commands  | jq

exit 0


         {"run":{"value":"test",
                  "help": "run a schedule var",
                   "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunSched"}]}]}}}'

#wait_pause

echo setup stop command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"stop":{"value":"test",
                   "help": "stop a schedule var",
                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"StopSched"}]}]}}}'

#wait_pause
echo setup Link command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"link":{"value":"test",
                     "help": "link two vars",
                      "enabled":true, "actions":{"onSet":[{"func":[{"func":"LinkVar"}]}]}}}'

#wait_pause
echo setup VLink command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"vlink":{"value":"test",
                    "help": "link two var values",
                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"VLinkVar"}]}]}}}'

echo setup Link command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"link":{"value":"test",
                    "help": "give a single assetvar multiple names",
                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"LinkVar"}]}]}}}'

#wait_pause
echo setup LoadConfig command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadCfg":{"value":"test",
                    "help": "load a config file",
                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadConfig"}]}]}}}'

#wait_pause
echo setup DumpConfig command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"dumpCfg":{"value":"test",
                     "help": "dump a config file",
                       "enabled":true, "actions":{"onSet":[{"func":[{"func":"DumpConfig"}]}]}}}'

#wait_pause
echo setup LoadServer command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadServer":{"value":"test",
                "help": "load a Modbus Server interface",
                "enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadServer"}]}]}}}'


