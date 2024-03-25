#!/bin/bash

# set working directory to the repo's base
cd "${0%/*}"
cd ../../

# build dependencies
../package_utility/build.sh

# set working directory to the directory of this file
cd ./test/docker

# build docker images
docker compose build