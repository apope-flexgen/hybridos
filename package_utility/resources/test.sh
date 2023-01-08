#!/bin/bash

# clear sourced variables
name=
type=
image=
artifacts=()
submodules=()
tests=()

# capture script and build relative paths
substr=""
IFS_bak=$IFS  # backup the existing IFS
IFS='/'
dir=$(dirname "$0")
read -ra substr <<< "$dir"
IFS=$IFS_bak # reset IFS

script_path="" # relative path to package_utility directory
for i in "${substr[@]}"; do
    if [ index = ${#substr[@]} ]; then break; fi
    script_path+=$i
    script_path+="/"  # optionally add trailing slash
done
# echo "$script_path"

build_path=$(readlink -f ${substr[0]}) # resolve relative path to build directory
build_output="$build_path/build/$mode_arg"
# echo "$build_path"
# echo "$build_output"

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
