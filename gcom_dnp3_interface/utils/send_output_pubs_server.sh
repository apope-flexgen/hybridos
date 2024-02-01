#!/bin/bash

# List of commands to run
commands=(
"fims_send -m pub -u /components/test/analog_out_7 50"
"fims_send -m pub -u /components/test/analog_out_7 50"
"fims_send -m pub -u /components/test/analog_out_7 50"
"fims_send -m pub -u /components/test/analog_out_7 50"
"fims_send -m pub -u /components/test/analog_out_7 23.4"
"fims_send -m pub -u /components/test/analog_out_7 50"
"fims_send -m pub -u /components/test/binary_out_4 false"
"fims_send -m pub -u /components/test/binary_out_4 false"
"fims_send -m pub -u /components/test/binary_out_4 false"
"fims_send -m pub -u /components/test/binary_out_4 false"
"fims_send -m pub -u /components/test/binary_out_4 true"
"fims_send -m pub -u /components/test/binary_out_4 false"
)




for cmd in "${commands[@]}"; do
    echo "Press enter to run the following command:"
    echo "$cmd"  # Echo the command
    read
    eval "$cmd"          # Run the command using 'eval'
done
