#!/bin/bash

# set working directory to the directory of this file
cd "${0%/*}"

# ensure we are connected at the beginning
./reconnect_client.sh &> /dev/null

# periodically disconnect and reconnect
while true
do
    ./disconnect_client.sh
    sleep $1s
    ./reconnect_client.sh
    sleep $2s
done