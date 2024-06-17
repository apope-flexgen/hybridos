#!/bin/bash

# Run with "watch -n 0.1 /path/to/watch.sh"

echo
echo "~~~~~~~~~~~~~ Battery Inputs ~~~~~~~~~~~~~"
echo " var name:    [incoming uri] -> [datamap amap] "

echo -n "StartCmd:          ["
echo -n $(fims_send -m get -r /$$ -u /ess/controls/ess/bms_1/BatteryBalancingStartCmd)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/StartCmd)
echo -n "]"
echo

echo -n "StopCmd:           ["
echo -n $(fims_send -m get -r /$$ -u /ess/controls/ess/bms_1/BatteryBalancingStopCmd)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/StopCmd)
echo -n "]"
echo

echo -n "FineBalanceEnabled:["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1/FineBalanceEnabled)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/FineBalanceEnabled)
echo -n "]"
echo

echo -n "MaxBalancePower:   ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/pcs/MaxBalancePower)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/MaxBalancePower)
echo -n "]"
echo

echo -n "PActl:             ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/pcs/PActl)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/PActl)
echo -n "]"
echo 
echo

echo "~~~~~~~~~~~~~ Battery Outputs ~~~~~~~~~~~~~"
echo " var name:    [datamap amap] -> [outgoing uri] "

echo -n "PCmd:              ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/PCmd)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/controls/pcs/ActivePowerCmd)
echo -n "]"
echo 

echo -n "StateVariable:     ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/BatteryBalancing/StateVariable)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/ess/bms_1/StateVariable)
echo -n "]"
echo 
echo
echo

echo "~~~~~~~~~~~~~ Rack 1 Inputs ~~~~~~~~~~~~~"
echo " var name:    [incoming uri] -> [datamap amap] "

echo -n "voltage:           ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_1/voltage)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/voltage)
echo -n "]"
echo 

echo -n "soc:               ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_1/soc)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/soc)
echo -n "]"
echo 

echo -n "contactorStatus:   ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_1/contactorStatus)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/contactorStatus)
echo -n "]"
echo 

echo -n "ignoreExternal:    ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_1/ignoreExternal)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/ignoreExternal)
echo -n "]"
echo 

echo -n "enableFeedback:    ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_1/enableFeedback)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/enableFeedback)
echo -n "]"
echo 
echo


echo "~~~~~~~~~~~~~ Rack 1 Outputs ~~~~~~~~~~~~~"
echo " var name:    [datamap amap] -> [outgoing uri] "

echo -n "enableCmd:         ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_1/enableCmd)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/controls/bms_1_rack_1/enableCmd)
echo -n "]"
echo 
echo
echo


echo "~~~~~~~~~~~~~ Rack 2 Inputs ~~~~~~~~~~~~~"
echo " var name:    [incoming uri] -> [datamap amap] "

echo -n "voltage:           ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_2/voltage)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/voltage)
echo -n "]"
echo 

echo -n "soc:               ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_2/soc)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/soc)
echo -n "]"
echo 

echo -n "contactorStatus:   ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_2/contactorStatus)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/contactorStatus)
echo -n "]"
echo 

echo -n "ignoreExternal:    ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_2/ignoreExternal)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/ignoreExternal)
echo -n "]"
echo 

echo -n "enableFeedback:    ["
echo -n $(fims_send -m get -r /$$ -u /ess/status/bms_1_rack_2/enableFeedback)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/enableFeedback)
echo -n "]"
echo 
echo


echo "~~~~~~~~~~~~~ Rack 1 Outputs ~~~~~~~~~~~~~"
echo " var name:    [datamap amap] -> [outgoing uri] "

echo -n "enableCmd:         ["
echo -n $(fims_send -m get -r /$$ -u /ess/control/new_config_brb_bms_1/rack_2/enableCmd)
echo -n "] -> ["
echo -n $(fims_send -m get -r /$$ -u /ess/controls/bms_1_rack_2/enableCmd)
echo -n "]"
echo 
echo

