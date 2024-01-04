#!/bin/bash

kill_cmd="pkill cloud_sync"

docker exec network-cloud_sync_test_network_client-1 bash -c "$kill_cmd"