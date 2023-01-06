#!/bin/sh
# p wilshire 03-07-2022
# script to run the dual_bms system


 
fims_send -m set -f configs/dbi/ess_controller/ess_final.json        -u /ess/cfg/cfile/ess/ess_final
#fims_send -m set -f configs/dbi/ess_controller/ess_controller.json  -u /ess/cfg/cfile/ess/ess_controller
#fims_send -m set -f configs/dbi/ess_controller/ess_manager.json     -u /ess/cfg/cfile/ess/ess_manager
#                { "file":"bms_manager",      "aname":"bms",   "pname":"ess"  },
#                { "file":"bms_1_manager",    "aname":"bms_1", "pname":"bms"  },
#                { "file":"bms_2_manager",    "aname":"bms_2", "pname":"bms"  },
#                { "file":"pcs_manager",      "aname":"pcs",   "pname":"ess"  },
#                { "file":"site_manager",     "aname":"site",  "pname":"ess"  }

# test to load up something to do when /scheduler/startup:bms runs
#fims_send -m set -r /$$ -u /ess/full/scheduler/startup/bms '{"value":true,"options":[{"uri":"/system/bms/some:value","func":"SomeFunc"}]}'| jq
