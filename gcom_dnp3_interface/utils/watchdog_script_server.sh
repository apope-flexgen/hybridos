#!/bin/bash

i=1

while true; do
    read -rsn1 -t 1 $REPLY
    if [ "$REPLY" = "" ]; then
        echo "analog_in_4 value:  $i"
        fims_send -m pub -u /components/test '{"analog_in_4":'$i'}'
        ((i++))
    else
        echo "Paused. Press Enter to resume..."
        read -rsn1
    fi
done
