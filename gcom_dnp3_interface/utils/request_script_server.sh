#!/bin/bash

# List of commands to run
commands=(
"fims_send -m get -u /local_server/_points -r /$$ | jq"
"fims_send -m get -u /local_server/_points/analog_inputs -r /$$ | jq"
"fims_send -m get -u /local_server/_stats -r /$$ | jq"
"fims_send -m get -u /local_server/_config -r /$$ | jq"
"fims_send -m get -u /local_server/_timings -r /$$ | jq"
"fims_send -m set -u /local_server/_reset_timings"
"fims_send -m get -u /local_server/_timings -r /$$ | jq"
"fims_send -m get -u /local_server/_errors -r /$$ | jq"
"fims_send -m set -u /local_server/_reset_errors"
"fims_send -m get -u /local_server/_errors -r /$$ | jq"
"fims_send -m set -u /local_server/_debug"
"fims_send -m set -u /local_server/_debug"
)




for cmd in "${commands[@]}"; do
    echo "$cmd"  # Echo the command
    eval "$cmd"          # Run the command using 'eval'
    sleep 2             # Sleep for 2 seconds
done