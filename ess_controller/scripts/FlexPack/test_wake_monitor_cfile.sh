
#!/bin/sh
#script to test setting up wake monitor throiugh the cfg loader system
fims_send -m set -f scripts/FlexPack/setup_wake_monitor.json -u /ess/cfg/cfile/ess/setup_wake_monitor
fims_send -m get -r /$$ -u /ess/full/config/cfile | jq
fims_send -m set -f scripts/FlexPack/setup_wake_monitor_x.json -u /ess/cfg/cfile/ess/setup_wake_monitor_x
# wake monitor should be running but we may be missong something ...
sleep 1
fims_send -m set -f scripts/FlexPack/setup_wake_monitor_xx.json -u /ess/cfg/cfile/ess/setup_wake_monitor_xx
sleep 1
fims_send -m set -f scripts/FlexPack/setup_wake_monitor_xxx.json -u /ess/cfg/cfile/ess/setup_wake_monitor_xxx

fims_send -m get -r /$$ -u /ess/full/config/cfile
fims_send -m get -r /$$ -u /ess/full/components/bms_info | jq
