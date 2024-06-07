#!/bin/bash

# set working directory to the script's location
cd "${0%/*}"

sudo mkdir -p /var/log/flexgen/ftd
sudo chown -R $(whoami):$(whoami) /var/log/flexgen/ftd/

sudo /usr/local/bin/ftd -c=./test_config.json