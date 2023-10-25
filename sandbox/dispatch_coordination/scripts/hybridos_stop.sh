#!/bin/sh
memcheck_log=~/git/hybridos/site_controller/valgrind.log
cfg_dir=/usr/local/etc/config
RED='\033[0;31m'
NC='\033[0m'

sudo echo "##### HYBRIDOS STOP #####"

echo "Stopping Docker containers"
# Remove containers with keyword "site"
site_containers=$(docker ps -a | grep site | awk '{print $1}')
if [ ! -z "$site_containers" ]; then
    docker container rm -f $site_containers > /dev/null
fi
# Remove containers with keyword "twins"
twins_containers=$(docker ps -a | grep twins | awk '{print $1}')
if [ ! -z "$twins_containers" ]; then
    docker container rm -f $twins_containers > /dev/null
fi
# Remove all networks with keyword "network"
networks=$(docker network ls | grep network | awk '{print $1}')
if [ ! -z "$networks" ]; then
    docker network rm $networks > /dev/null
fi
echo "Stopped all Docker containers"

sudo pkill MCP

sudo pkill cops

sudo pkill fleet_manager
sudo pkill dnp3
sleep 1 # no clue why, but this `sleep 1` makes the `sudo pkill dnp3` succeed
sudo pkill washer
sudo pkill dts
sudo pkill ftd
sudo pkill cloud_sync

pkill modbus_client
pkill site_controller

pkill metrics
pkill events
pkill dbi
pkill scheduler

sudo pkill influxd
sudo pkill mongod

sudo pkill modbus_server
sudo pkill web_server

pkill fims

# If memory leaks were captured report them
sleep 1s
if [ -f $memcheck_log ]; then
    # If file was modified in last 6 seconds
    if [ "$(find $memcheck_log -cmin -0.1)" ]; then
        if grep -q "definitely lost" "$memcheck_log"; then
            printf "\n${RED}### Memory Leak detected ###${NC}\n"
        fi
    fi
fi
