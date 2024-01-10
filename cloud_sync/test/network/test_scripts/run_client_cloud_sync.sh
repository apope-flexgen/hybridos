#!/bin/bash

run_cmd="/usr/local/bin/cloud_sync --c=/home/config/cloud_sync.json --logCfg=/home/config/cloud_sync_verbose.json"

docker exec -d network-cloud_sync_test_network_client-1 bash -c "$run_cmd"