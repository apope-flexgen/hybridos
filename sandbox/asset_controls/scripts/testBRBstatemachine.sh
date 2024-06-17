#!/bin/bash

# ess_controller -f ess_file

#configurations
# make sure this numRacks is the same numRacks as in aV.json
numRacks=15



fimsSend="/usr/local/bin/fims_send -m set -u"
fimsGet="/usr/local/bin/fims_send -m get -r /$$ -u"


#Set up test environment
echo "Begin testing battery balancing"
echo "Turning on debug messages"
$fimsSend /ess/control/new_config_brb_bms_1/BatteryBalancing/debug true


# set initial values for BRB_bms_1
echo "Setting start command to false (will restart state machine if previously running)"
$fimsSend /ess/control/new_config_brb_bms_1/BatteryBalancing/StartCmd false


# echo "Battery Balancing input RackCloseDeltaVoltage is set to 50"
# $fimsSend /ess/control/new_config_brb_bms_1/BatteryBalancing/RackCloseDeltaVoltage 50


echo "set up racks"

for (( i=1; i<=$numRacks; i++ ))
    do
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/voltage 1200
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/soc 0.5
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/contactorStatus true
    done

echo "Setting battery status to closed"
$fimsSend /ess/status/bms/DCClosed true

#Test racks 0 and 1
$fimsSend /ess/control/new_config_brb_bms_1/Rack_1/voltage 1300
$fimsSend /ess/control/new_config_brb_bms_1/Rack_1/soc 0.9
$fimsSend /ess/control/new_config_brb_bms_1/Rack_1/contactorStatus false

$fimsSend /ess/control/new_config_brb_bms_1/Rack_2/voltage 1150
$fimsSend /ess/control/new_config_brb_bms_1/Rack_2/soc 0.2
$fimsSend /ess/control/new_config_brb_bms_1/Rack_2/contactorStatus false
# echo -n "Current state is now " && $fimsGet /ess/control/new_config_brb_bms_1/BatteryBalancing/StateVariable

echo
echo "Rack 1 and Rack 2 are unbalanced in this Battery. Watch how this Battery Balancing Algorithm can fix these two racks and bring all rack voltages to about the same level"

echo "  -> Press any key to start <-"
read -n 1 -s

echo "Sending start command"

$fimsSend /ess/control/new_config_brb_bms_1/BatteryBalancing/StartCmd true
sleep 1
# echo -n "Current state is now " && $fimsGet /ess/control/new_config_brb_bms_1/BatteryBalancing/StateVariable

# read -n 1 -s

# needs to be 1150
read -p "  -> Enter closed rack voltage (1150): " volts

for ((i=3; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/voltage $volts
    done
echo

# send rack enableFeedback
echo "Upon entering the Contactor Control State, the system needs to wait for the battery to relax before sending any commands"
echo "Once the battery has relaxed for a sufficient amount of time, the system enables all racks"
echo "  -> During Battery Relaxation, press any key to simulate modbus enabling all racks <- "
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/enableFeedback true
    done
echo

# open racks
echo "After Battery relaxation, the system sends an Open Contactor Request for all enabled racks"
echo "  -> While waiting in Open Contactor state, press any key to simulate battery opening all rack contactors <- "
echo "  (this will set all rack's contactorStatus to false and DCClosed to false)"
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/contactorStatus false
    done
$fimsSend /ess/status/bms/DCClosed false
echo 

# get rack enable feedback
echo "After a successful Open Contactor Request, the system disables all racks not in our target racks before sending a close contactors request"
echo "  -> Press any key to simulate rack feedback from modbus after disabling all racks that are not in our targetRacks <-"
echo "  (To continue past Rack Feedback state, each rack's enableFeedback needs to be the same as their enableCmd)"
read -n 1 -s
$fimsSend /ess/control/new_config_brb_bms_1/Rack_1/enableFeedback false
echo

# close racks
echo "Once only our target racks are enabled, the system sends a Close Contactor Request"
echo "  -> While waiting in Close Contactor state, press any key simulate the battery closing all of our enabled racks <- "
echo "  (this will close all racks except rack 1 and will set DCClose to true)"
read -n 1 -s
for ((i=2; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/contactorStatus true
    done
$fimsSend /ess/status/bms/DCClosed true
echo

echo
echo "Rack 2 has been successfully balanced!"
echo


echo "Begin balancing Rack 1"

# needs to be 1300
read -p "  -> Enter new closed rack voltage (1300): " volts

for ((i=2; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/voltage $volts
    done
echo

# send rack enableFeedback
echo "Upon entering the Contactor Control State, the system needs to wait for the battery to relax before sending any commands"
echo "Once the battery has relaxed for a sufficient amount of time, the system enables all racks"
echo "  -> During Battery Relaxation, press any key to simulate modbus enabling all racks <- "
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/enableFeedback true
    done
echo

# open racks
echo "After Battery relaxation, the system sends an Open Contactor Request for all enabled racks"
echo "  -> While waiting in Open Contactor state, press any key to simulate battery opening all rack contactors <- "
echo "  (this will set all rack's contactorStatus to false and DCClosed to false)"
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/contactorStatus false
    done
$fimsSend /ess/status/bms/DCClosed false
echo 

# get rack enable feedback (all racks already enabled)
echo "After a successful Open Contactor Request, the system disables all racks not in our target racks before sending a close contactors request"
echo "All racks are in our targetRacks and enableFeedback is true for all racks so no check or waiting is needed"
echo

# close racks
echo "Once our target racks are enabled, the system sends a Close Contactor Request"
echo "  -> While waiting in Close Contactor state, press any key simulate the battery closing all of our enabled racks <- "
echo "  (this will close all racks and set DCClose to true)"
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/new_config_brb_bms_1/Rack_$i/contactorStatus true
    done
$fimsSend /ess/status/bms/DCClosed true
echo

echo " * All Racks are Balanced! *"
echo
