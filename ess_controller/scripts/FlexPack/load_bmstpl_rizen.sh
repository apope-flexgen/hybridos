/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands '
         {"loadTpl":{"value":"test",
                "help": "load a AssetManager template ",
                "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"RunTpl"}]}]}}}'

echo set up a site interface
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/loadTpl '
             {"value":"someval","aname":"bms", "pname":"flex", "fname":"bms_rizen_manager.json"}'
            
