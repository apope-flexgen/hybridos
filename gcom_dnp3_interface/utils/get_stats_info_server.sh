#!/bin/bash

# List of commands to run
commands=(
"fims_send -m get -u /components/dnp3_server/_stats -r /$$ | jq"
"fims_send -m get -u /components/dnp3_server/_stats/open_channels -r /$$ | jq"
"fims_send -m get -u /components/dnp3_server/_stats/total_errors -r /$$ | jq"
)




for cmd in "${commands[@]}"; do
    echo "Press enter to run the following command:"
    echo "$cmd"  # Echo the command
    read
    eval "$cmd"          # Run the command using 'eval'
done