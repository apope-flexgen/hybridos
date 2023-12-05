#!/bin/bash

docker exec network-cloud_sync_test_network_client-1 tc qdisc change dev eth0 root netem delay "$1"