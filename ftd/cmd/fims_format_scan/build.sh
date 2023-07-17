#!/bin/bash

# set working directory to the directory of this file
cd "${0%/*}"

# build executable in location specified
echo "##### building fims format scan"
go build -o $1 .
echo "##### SUCCESS: fims format scan build complete"
echo "vetting $name..."
go vet .
echo