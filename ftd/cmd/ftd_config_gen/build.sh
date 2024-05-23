#!/bin/bash

# set working directory to the directory of this file
cd "${0%/*}"

# build executable in location specified
echo "##### building ftd config gen"
go build -o $1 .
echo "##### SUCCESS: ftd config gen build complete"
echo "vetting $name..."
go vet .
echo