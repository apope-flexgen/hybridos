#!/bin/bash

docker network connect network_cloud_sync_test_network network-cloud_sync_test_network_client-1
docker exec network-cloud_sync_test_network_client-1 tc qdisc add dev eth0 root netem delay 200ms