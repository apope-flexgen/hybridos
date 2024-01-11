#!/bin/sh
# script to start the dual risen load
# p wilshire 3/3/2022

# starts the load process
  fims_send -m set -f configs/ess_controller_split/ess_1_dual_init.json   -u /ess_1/cfg/cfile/ess/ess_1_dual_init
  fims_send -m set -f configs/ess_controller_split/ess_1_controller.json  -u /ess_1/cfg/cfile/ess/ess_1_controller
  fims_send -m set -f configs/ess_controller_split/ess_2_dual_init.json   -u /ess_2/cfg/cfile/ess/ess_2_dual_init
  fims_send -m set -f configs/ess_controller_split/ess_2_controller.json  -u /ess_2/cfg/cfile/ess/ess_2_controller
  fims_send -m get -r /$$ -u /ess_1/config/cfile | jq
  fims_send -m get -r /$$ -u /ess_1/amap | jq
  fims_send -m get -r /$$ -u /ess_2/config/cfile | jq
  fims_send -m get -r /$$ -u /ess_2/amap | jq
  