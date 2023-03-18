#!/bin/bash

# execute the given command on both nodes
docker exec network-cloud_sync_test_network_router-1 $1
docker exec network-cloud_sync_test_network_server-1 $1