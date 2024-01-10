#!/bin/bash

docker exec network-cloud_sync_test_network_router-1 tc qdisc change dev eth0 root netem delay "$1"