#!/bin/bash

# setup server to accept client connection
/usr/sbin/sshd # start ssh server daemon
cat /run/secrets/client_id_rsa.pub > /root/.ssh/authorized_keys # add client public key to authorized keys
ls -la /root/.ssh
echo

# start our applications
/usr/local/bin/cloud_sync --c=/home/config/cloud_sync.json --logCfg=/home/config/cloud_sync_verbose.json &
/usr/local/bin/mock_archive_consumer --in=/home/local_consumer --period=1s &

# keep the container running (even if one of our applications fails)
bash