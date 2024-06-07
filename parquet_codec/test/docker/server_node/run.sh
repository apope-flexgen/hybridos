#!/bin/bash

# setup server to accept client connection
/usr/sbin/sshd # start ssh server daemon
cat /run/secrets/client_id_rsa.pub > /root/.ssh/authorized_keys # add client public key to authorized keys
ls -la /root/.ssh
echo

# add some network latency to better simulate a real connection
tc qdisc add dev eth0 root netem delay 10ms

# start our applications
/usr/local/bin/dts --c=/home/config/dts.json &
/usr/local/bin/cloud_sync --c=/home/config/cloud_sync.json &

# keep the container running (even if one of our applications fails)
bash