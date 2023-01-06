
echo setup commands

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

#wait_pause
#echo setup Link command
#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
#         {"link":{"value":"test",
#                     "help": "link two vars",
#                      "ifChanged":false, "enabled":true, "actions":{"onSet":[{"func":[{"func":"LinkVar"}]}]}}}'

#wait_pause
echo setup VLink command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"vlink":{"value":"test",
                    "help": "link two var values",
                    "ifChanged":false, "enabled":true, 
                    "actions":{"onSet":[{"func":[{"func":"RunVLinks"}]}]}}}'

#echo setup Link command
#/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
#         {"link":{"value":"test",
#                    "help": "give a single assetvar multiple names",
#                    "enabled":true, "actions":{"onSet":[{"func":[{"func":"LinkVar"}]}]}}}'

#wait_pause
echo setup LoadConfig command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadCfg":{"value":"test",
                    "help": "load a config file",
                    "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadConfig"}]}]}}}'

#wait_pause
echo setup DumpConfig command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"dumpCfg":{"value":"test",
                     "help": "dump a config file",
                      "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"DumpConfig"}]}]}}}'

#wait_pause
echo setup LoadServer command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadServer":{"value":"test",
                "help": "load a Modbus Server interface",
                "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadServer"}]}]}}}'

echo setup LoadClient command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadClient":{"value":"test",
                "help": "load a Modbus Client interface",
                "enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadClient"}]}]}}}'


echo setup LoadTpl command
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadTpl":{"value":"test",
                "help": "load a AssetManager template ",
                "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"RunTpl"}]}]}}}'

