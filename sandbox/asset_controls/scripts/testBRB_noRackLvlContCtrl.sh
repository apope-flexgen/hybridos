#!/bin/bash

# ess_controller -f ess_file

#configurations
# make sure this numRacks is the same numRacks as in aV.json
numRacks=15

# NOTE: RackLevelContactorControl must be set to false on the aV to run this script


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
    $fimsSend /ess/status/bms_1_rack_$i/voltage 1200
    $fimsSend /ess/status/bms_1_rack_$i/soc 0.5
    $fimsSend /ess/status/bms_1_rack_$i/contactorStatus true
    done

echo "Setting battery status to closed"
$fimsSend /ess/status/bms/DCClosed true

#Test racks 0 and 1
$fimsSend /ess/status/bms_1_rack_1/voltage 1300
$fimsSend /ess/status/bms_1_rack_1/soc 0.9
$fimsSend /ess/status/bms_1_rack_1/contactorStatus false

$fimsSend /ess/status/bms_1_rack_2/voltage 1150
$fimsSend /ess/status/bms_1_rack_2/soc 0.2
$fimsSend /ess/status/bms_1_rack_2/contactorStatus false
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
    $fimsSend /ess/status/bms_1_rack_$i/voltage $volts
    done
echo

# send rack enableFeedback
echo "Upon entering the Contactor Control State, the system needs to wait for the battery to relax before sending any commands"
echo "Once the battery has relaxed for a sufficient amount of time, the system enables all racks"
echo "  -> During Battery Relaxation, press any key to simulate modbus enabling all racks <- "
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/status/bms_1_rack_$i/enableFeedback true
    done
echo

# close racks
echo "After Battery relaxation, the system sends a Close Contactor Request"
echo "  -> While waiting in Close Contactor state, press any key simulate the battery closing all of our enabled racks <- "
echo "  (this will close all racks except rack 1 and will set DCClose to true)"
read -n 1 -s
for ((i=2; i<=$numRacks; i++))
    do 
    $fimsSend /ess/status/bms_1_rack_$i/contactorStatus true
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
    $fimsSend /ess/status/bms_1_rack_$i/voltage $volts
    done
echo

# send rack enableFeedback
echo "Upon entering the Contactor Control State, the system needs to wait for the battery to relax before sending any commands"
echo "Once the battery has relaxed for a sufficient amount of time, the system enables all racks"
echo "  -> During Battery Relaxation, press any key to simulate modbus enabling all racks <- "
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/status/bms_1_rack_$i/enableFeedback true
    done
echo

# close racks
echo "After Battery relaxation, the system sends a Close Contactor Request"
echo "  -> While waiting in Close Contactor state, press any key simulate the battery closing all of our enabled racks <- "
echo "  (this will close all racks and set DCClose to true)"
read -n 1 -s
for ((i=1; i<=$numRacks; i++))
    do 
    $fimsSend /ess/status/bms_1_rack_$i/contactorStatus true
    done
$fimsSend /ess/status/bms/DCClosed true
echo

echo " * All Racks are Balanced! *"
echo