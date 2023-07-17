#!/bin/bash

# setup client for connecting to server
cp /run/secrets/client_id_rsa /root/.ssh/id_rsa && cp /run/secrets/client_id_rsa.pub /root/.ssh/id_rsa.pub
# ask server for public key and add to known hosts
server_pub_key=""
while [ "$server_pub_key" = "" ]
do
    server_pub_key=$(ssh-keyscan -H 1.1.1.2)
    sleep 1
done
echo "$server_pub_key" > /root/.ssh/known_hosts
ls -la /root/.ssh
echo

# start our applications
/usr/local/bin/mock_archive_producer --out=/home/archives --period=5s --num=20 &
/usr/local/bin/cloud_sync --c=/home/config/cloud_sync.json --logCfg=/home/config/cloud_sync_verbose.json &
/usr/local/bin/mock_archive_consumer --in=/home/local_consumer --period=1s &

# keep the container running (even if one of our applications fails)
bash