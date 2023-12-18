#!/bin/bash

# List of commands to run
commands=(
"fims_send -m get -u /components/dnp3_client/_points -r /$$ | jq"
"fims_send -m get -u /components/dnp3_client/_points/analog_inputs -r /$$ | jq"
"fims_send -m get -u /components/dnp3_client/_stats -r /$$ | jq"
"fims_send -m get -u /components/dnp3_client/_config -r /$$ | jq"
"fims_send -m get -u /components/dnp3_client/_timings -r /$$ | jq"
"fims_send -m set -u /components/dnp3_client/_reset_timings"
"fims_send -m get -u /components/dnp3_client/_timings -r /$$ | jq"
"fims_send -m get -u /components/dnp3_client/_errors -r /$$ | jq"
"fims_send -m set -u /components/dnp3_client/_reset_errors"
"fims_send -m get -u /components/dnp3_client/_errors -r /$$ | jq"
"fims_send -m set -u /components/dnp3_client/_debug"
"fims_send -m set -u /components/dnp3_client/_debug"
)




for cmd in "${commands[@]}"; do
    echo "$cmd"  # Echo the command
    eval "$cmd"          # Run the command using 'eval'
    sleep 2             # Sleep for 2 seconds
done