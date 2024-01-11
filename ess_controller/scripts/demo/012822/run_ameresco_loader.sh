#!/bin/sh
# script to load a whole sysem 

export cfg_dir=/home/vagrant/docker_testing/ess/configs_split/ess_controller
export Sys=ess

fims_send -f $cfg_dir/ess_config_lg.json            -m set  -u /$Sys/cfg/cfile/$Sys/ess_config_lg
fims_send -f $cfg_dir/ess_controller.json           -m set  -u /$Sys/cfg/cfile/$Sys/ess_controller              

fims_send -f $cfg_dir/lg_bms_manager.json           -m set  -u /$Sys/cfg/cfile/$Sys/lg_bms_manager
sleep 0.5

fims_send -f $cfg_dir/lg_bms_tmpl.json              -m set  -u /$Sys/cfg/ctmpl/$Sys/lg_bms_tmpl             
fims_send -f $cfg_dir/lg_bms_manager_modbus.json    -m set  -u /$Sys/cfg/cfile/$Sys/lg_bms_manager_modbus
sleep 0.5

fims_send -f $cfg_dir/lg_bms_modbus_data_tmpl.json  -m set  -u /$Sys/cfg/ctmpl/$Sys/lg_bms_modbus_data_tmpl
fims_send -f $cfg_dir/lg_bms_controls.json          -m set  -u /$Sys/cfg/cfile/$Sys/lg_bms_controls     
fims_send -f $cfg_dir/lg_pcs_manager.json           -m set  -u /$Sys/cfg/cfile/$Sys/lg_pcs_manager
fims_send -f $cfg_dir/lg_pcs_modbus_data.json       -m set  -u /$Sys/cfg/cfile/$Sys/lg_pcs_modbus_data
fims_send -f $cfg_dir/lg_pcs_controls.json          -m set  -u /$Sys/cfg/cfile/$Sys/lg_pcs_controls
sleep 1.0
fims_send -f $cfg_dir/lg_ess_server.json            -m set  -u /$Sys/cfg/ctmpl/$Sys/lg_ess_server
sleep 1.0
fims_send -f $cfg_dir/ess_final.json                -m set  -u /$Sys/cfg/cfile/$Sys/ess_final     
