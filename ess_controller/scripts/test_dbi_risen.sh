#!/bin/sh

# this tests the new config loader for the risen bms system.. Still need to check the ui component.
# start the new config system off

 fims_send -m set -f ~/config_ess/install/dbicfg/ess_config_risen_sungrow.json  -u /ess/cfg/cfile/ess/ess_config_risen_sungrow
# snd the risen_bms_manager config
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_manager.json  -u /ess/cfg/cfile/ess/risen_bms_manager
# send the component configs
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_status.json  -u /ess/cfg/cfile/ms/risen_bms_status
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_modbus.json  -u /ess/cfg/cfile/ms/risen_bms_modbus
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_alarms.json  -u /ess/cfg/cfile/ms/risen_bms_alarms
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_controls.json  -u /ess/cfg/cfile/ms/risen_bms_controls
#send the template.
# the system should expand the template now
 fims_send -m set -f ~/config_ess/install/dbicfg/risen_bms_template.json  -u /ess/cfg/ctmpl/bms/risen_bms_template
 sleep 10 
 # look for racks in the amap...
 fims_send  -m get -r /$$ -u /ess/full/amap | jq
 