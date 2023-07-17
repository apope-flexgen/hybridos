#!/bin/bash

empty_db_config="echo > /home/.cloud_sync/db/main/config.json"

# execute the given command on both nodes
docker exec network-cloud_sync_test_network_client-1 bash -c "$empty_db_config"
docker exec network-cloud_sync_test_network_router-1 bash -c "$empty_db_config"
docker exec network-cloud_sync_test_network_server-1 bash -c "$empty_db_config"