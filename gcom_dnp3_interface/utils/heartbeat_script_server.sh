#!/bin/bash

i=1

while true; do
    read -rsn1 -t 1 $REPLY
    if [ "$REPLY" = "" ]; then
        echo "read heartbeat:  $i"
        fims_send -m pub -u /components/test '{"analog_in_4":'$i'}'
        sleep 1
        echo -n "                     write heartbeat: "; fims_send -m get -u /local_server/components/test/analog_out_4 -r /$$
        ((i++))
    else
        echo "Paused. Press Enter to resume..."
        read -rsn1
    fi
done
