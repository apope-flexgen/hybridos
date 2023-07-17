#!/bin/bash

crash_cmd="pkill cloud_sync --signal SIGSEGV"

# execute the given command on both nodes
docker exec network-cloud_sync_test_network_client-1 bash -c "$crash_cmd"
docker exec network-cloud_sync_test_network_router-1 bash -c "$crash_cmd"
docker exec network-cloud_sync_test_network_server-1 bash -c "$crash_cmd"