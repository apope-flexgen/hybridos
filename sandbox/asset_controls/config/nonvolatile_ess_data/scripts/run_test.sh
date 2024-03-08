#!/bin/bash

# Make sure you've run start_ess.sh first
# Then run this script in a separate terminal

## Look at what is currently in /dbi/ess_controller/saved_registers (should be nothing if this is the first run)
echo 'fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$'
fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$
sleep 2.5
echo

## Look at what is currently in /ess/controls/pcs/active_power_gradient (should be 10 if nothing is currently in dbi)
echo 'fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$'
fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$
sleep 2
echo

## Send a set to /controls/pcs/active_power_gradient
echo 'fims_send -m set -u /controls/pcs/active_power_gradient 25'
fims_send -m set -u /ess/controls/pcs/active_power_gradient 25
sleep 2
echo

## Look at what is currently in /dbi/ess_controller/saved_registers (should be 25)
echo 'fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$'
echo 'Expect: {"active_power_gradient":{"value":25}}.'
echo -n 'Got   : '
fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$
sleep 2
echo

## Look at what is currently in /ess/controls/pcs/active_power_gradient (should be 25)
echo 'fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$'
echo 'Expect: 25'
echo -n 'Got   : '
fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$
sleep 2
echo

echo 'Restarting ess_controller'
pkill ess_controller
sleep 2
ess_controller -f ess_file > /dev/null 2>&1 &
sleep 2
echo 'Ess_controller has restarted'
echo

## Look at what is currently in /dbi/ess_controller/saved_registers (should be 25)
echo 'fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$'
echo 'Expect: {"active_power_gradient":{"value":25}}.'
echo -n 'Got   : '
fims_send -m get -u /dbi/ess_controller/saved_registers -r /$$
sleep 2
echo

## Look at what is currently in /ess/controls/pcs/active_power_gradient (should be 25)
echo 'fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$'
echo 'Expect: 25'
echo -n 'Got   : '
fims_send -m get -u /ess/controls/pcs/active_power_gradient -r /$$
sleep 2