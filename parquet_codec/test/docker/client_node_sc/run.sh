#!/bin/bash

# setup client for connecting to server
cp /run/secrets/client_id_rsa /root/.ssh/id_rsa && cp /run/secrets/client_id_rsa.pub /root/.ssh/id_rsa.pub
# ask server for public key and add to known hosts
server_pub_key=""
while [ "$server_pub_key" = "" ]
do
    server_pub_key=$(ssh-keyscan -H powercloud_server)
    sleep 1
done
echo "$server_pub_key" > /root/.ssh/known_hosts
ls -la /root/.ssh
echo

# add some network latency to better simulate a real connection
tc qdisc add dev eth0 root netem delay 10ms

# start our applications
/usr/local/bin/cloud_sync --c=/home/config/cloud_sync.json &
/usr/local/bin/fims_server &> /dev/null &
sleep 1 # wait a little for fims server to start up
/usr/local/bin/ftd --c=/home/config/ftd.json &
/home/fims_simulator /home/config/fims_simulator.json &

# keep the container running (even if one of our applications fails)
bash