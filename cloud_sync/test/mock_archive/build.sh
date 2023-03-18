#!/bin/bash

# set working directory to the directory of this file
cd "${0%/*}"

# build mock archive executables
echo "##### building cloud_sync test mock archive programs"
go build -o ./build/producer ./cmd/producer
go build -o ./build/consumer ./cmd/consumer
echo "##### SUCCESS: cloud_sync test mock archive programs build complete"
echo