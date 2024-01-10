#!/bin/bash

# set working directory to the repo's base
cd "${0%/*}"
cd ../../

# build dependencies
../package_utility/build.sh
./test/mock_archive/build.sh

# set working directory to the directory of this file
cd ./test/network

# build docker images
docker compose build

# create an ssh key pair for use by the client node 
# (created here so that we can also pass the key to the server node, so it can authorize the client)
mkdir -p ./build/client_ssh/
# echo "yes" will automatically answer the prompt asking the user if they want to overwrite a key from a previous build
ssh-keygen -t rsa -N "" -f ./build/client_ssh/id_rsa < <(echo "yes")