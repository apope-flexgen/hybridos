#!/bin/bash

# set working directory to the repo's base
cd "${0%/*}"
cd ../../

# build dts
../package_utility/build.sh

# set working directory to the directory of this file
cd ./test/docker

# build fims_simulator
go build -o ./build/fims_simulator ./fims_simulator 

# build docker images
docker compose build