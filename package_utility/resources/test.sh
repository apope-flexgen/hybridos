#!/bin/bash

# clear sourced variables
name=
type=
image=
artifacts=()
submodules=()
tests=()

# capture script and build relative paths
# $0 is the actual running script, ./package_utility/build.sh
script_path=$(readlink -f "$(dirname "$0")")/ # this is absolute path to the script, ./package_utility/
echo -e "script_path: $script_path"

# resolve relative path to build directory
build_output="$(pwd)/build/$mode_arg/"
echo -e "build_output: $build_output"

# source helper functions (from relative path)
functions_path="${script_path}functions.sh"
source $functions_path
CATCH=$?
if [ $CATCH -eq 1 ]; then
    echo -e "failed to import functions.sh."
    exit 1
fi

# source version from version.sh
build_script_path="${script_path}build.sh"
source $build_script_path -bm test

if [ ${#tests[@]} -eq 0 ]; then
    echo "no tests specified, terminating."
else
    for i in "${tests[@]}"; do
        echo -e "##### running test $i..."
        test "$i" # TODO: expand this as needed
    done
fi
