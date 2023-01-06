#!/bin/sh
# sets up basic ess_controller to simulate the sungrow pcs

ess_controller -n pcs -x > /tmp/pcs_sim.out 2>&1 &

fims_send -m set -r /$$ -u /pcs/system/commands '
         {"loadServer":{"value":"test",
                "help": "load a Modbus Server interface",
                "ifChanged":false,"enabled":true, "actions":{"onSet":[{"func":[{"func":"LoadServer"}]}]}}}'

fims_send -m set -r /$$ -u /pcs/full/system/commands/loadServer '
             {"value":"test","server":"pcs_2750_server.json"}'

