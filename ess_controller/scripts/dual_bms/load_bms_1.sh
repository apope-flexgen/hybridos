#!/bin/sh
# p wilshire 03-13-2022
# script to send start up files to the bms component of the triple ess_controllers.
ess_dir=/ess_controller5

 
fims_send -m set -f configs/dbi/${ess_dir}/ess_init1.json                      -u /ess/cfg/cfile/ess/ess_init1
fims_send -m set -f configs/dbi/${ess_dir}/ess_controller_bms_1.json           -u /ess/cfg/cfile/ess/ess_controller
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_1_init.json               -u /ess/cfg/cfile/ess/risen_bms_1_init
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_1_modbus_init.json        -u /ess/cfg/cfile/ess/risen_bms_1_modbus_init

fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_manager.json              -u /ess/cfg/ctmpl/ess/risen_bms_manager
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_manager_modbus.json       -u /ess/cfg/ctmpl/ess/risen_bms_manager_modbus
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_rack_modbus.json          -u /ess/cfg/ctmpl/ess/risen_bms_rack_modbus

fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_manager_modbus_env.json   -u /ess/cfg/ctmpl/ess/risen_bms_manager_modbus_env


fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_1_modbus_env_init.json    -u /ess/cfg/cfile/ess/risen_bms_1_modbus_env_init

fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_1_controls_init.json      -u /ess/cfg/cfile/ess/risen_bms_1_controls_init

fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_manager_controls.json     -u /ess/cfg/ctmpl/ess/risen_bms_manager_controls
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_rack_controls.json        -u /ess/cfg/ctmpl/ess/risen_bms_rack_controls


fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_1_faults_init.json        -u /ess/cfg/cfile/ess/risen_bms_1_faults_init
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_manager_faults.json       -u /ess/cfg/ctmpl/ess/risen_bms_manager_faults
fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_rack_faults.json          -u /ess/cfg/ctmpl/ess/risen_bms_rack_faults


fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_1_init.json             -u /ess/cfg/cfile/ess/sungrow_pcs_1_init
fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_manager.json            -u /ess/cfg/ctmpl/ess/sungrow_pcs_manager

fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_1_modbus_init.json      -u /ess/cfg/cfile/ess/sungrow_pcs_1_modbus_init
fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_manager_modbus.json     -u /ess/cfg/ctmpl/ess/sungrow_pcs_manager_modbus

fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_1_controls_init.json    -u /ess/cfg/cfile/ess/sungrow_pcs_1_controls_init
fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_manager_controls.json   -u /ess/cfg/ctmpl/ess/sungrow_pcs_manager_controls


fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_1_faults_init.json      -u /ess/cfg/cfile/ess/sungrow_pcs_1_faults_init
fims_send -m set -f configs/dbi/${ess_dir}/sungrow_pcs_manager_faults.json     -u /ess/cfg/ctmpl/ess/sungrow_pcs_manager_faults

fims_send -m set -f configs/dbi/${ess_dir}/flexgen_ess_server.json             -u /ess/cfg/ctmpl/ess/flexgen_ess_server

fims_send -m set -f configs/dbi/${ess_dir}/site_manager_bms1.json              -u /ess/cfg/cfile/ess/site_manager

fims_send -m set -f configs/dbi/${ess_dir}/risen_bms_rack.json                -u /ess/cfg/ctmpl/ess/risen_bms_rack

fims_send -m set -f configs/dbi/${ess_dir}/ess_final_bms1.json                -u /ess/cfg/cfile/ess/ess_final



#fims_send -m set -f configs/dbi/ess_controller/ess_manager.json     -u /ess/cfg/cfile/ess/ess_manager

#fims_send -m set -f configs/dbi/ess_controller5/pcs_0_sc2750_manager.json    -u /ess/cfg/cfile/ess/pcs_manager
#                { "file":"bms_manager",      "aname":"bms",   "pname":"ess"  },
#                { "file":"bms_1_manager",    "aname":"bms_1", "pname":"bms"  },
#                { "file":"bms_2_manager",    "aname":"bms_2", "pname":"bms"  },
#                { "file":"pcs_manager",      "aname":"pcs",   "pname":"ess"  },
#                { "file":"site_manager",     "aname":"site",  "pname":"ess"  }

# test to load up something to do when /scheduler/startup:bms runs
#fims_send -m set -r /$$ -u /ess/full/scheduler/startup/bms '{"value":true,"options":[{"uri":"/system/bms/some:value","func":"SomeFunc"}]}'| jq
