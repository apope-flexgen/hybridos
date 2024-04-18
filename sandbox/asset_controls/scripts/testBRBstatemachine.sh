#!/bin/bash

# ess_controller -f ess_file

#configurations

numRacks=15



fimsSend="/usr/local/bin/fims_send -m set -u"
fimsGet="/usr/local/bin/fims_send -m get -r /$$ -u"


#Set up test environment
echo "Begin testing battery balancing"
echo "Turning on debug messages"
$fimsSend /ess/control/BatteryBalancing_1/debug true

echo "Setting start command to false (will restart state machine if previously running)"
$fimsSend /ess/control/BatteryBalancing_1/StartCmd false


echo "set up racks"

for (( i=1; i<=$numRacks; i++ ))
    do
    $fimsSend /ess/control/Rack_$i/voltage 1200
    $fimsSend /ess/control/Rack_$i/soc 0.5
    $fimsSend /ess/control/Rack_$i/contactorStatus true
    done

#Test racks 0 and 1
$fimsSend /ess/control/Rack_1/voltage 1300
$fimsSend /ess/control/Rack_1/soc 0.9
$fimsSend /ess/control/Rack_1/contactorStatus false

$fimsSend /ess/control/Rack_2/voltage 1150
$fimsSend /ess/control/Rack_2/soc 0.2
$fimsSend /ess/control/Rack_2/contactorStatus false
# echo -n "Current state is now " && $fimsGet /ess/control/BatteryBalancing_1/StateVariable

echo -n "Press any key to start"
read -n 1 -s

echo "Sending start command"

$fimsSend /ess/control/BatteryBalancing_1/StartCmd true
sleep 1
# echo -n "Current state is now " && $fimsGet /ess/control/BatteryBalancing_1/StateVariable

# read -n 1 -s

read -p "Enter closed rack voltage " volts

for ((i=3; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/Rack_$i/voltage $volts
    done

echo -n "Press any key to close rack 2 contactor"
read -n 1 -s

echo "Closing rack 2 contactor"

$fimsSend /ess/control/Rack_2/contactorStatus true

read -p "Enter closed rack voltage " volts

for ((i=2; i<=$numRacks; i++))
    do 
    $fimsSend /ess/control/Rack_$i/voltage $volts
    done

echo -n "Press any key to close rack 1 contactor"
read -n 1 -s

echo "Closing rack 1 contactor"

$fimsSend /ess/control/Rack_1/contactorStatus true
