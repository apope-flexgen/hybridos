#!/bin/bash

# set working directory to the directory of this file
cd "${0%/*}"

./exec_on_client_and_router.sh "tc qdisc add dev eth0 root netem delay $1"