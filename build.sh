#!/bin/bash

# capture script and build relative paths
cwd="$(pwd)"
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
#echo "script_path: $script_path"

# source build functions (from repository path)
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

# complete checks before proceeding...

if [ ! -n "$components" ]; then
    error_trap "build_utils.sh does not specify 'components' field."
fi

# conditionally create build_output directory
build_output="${cwd}/build/release"
rm -rf "build_output" || error_trap "failed to remove "$build_output" directory."
mkdir -p "$build_output" || error_trap "failed to create "$build_output" directory."

# iterate through modules
for i in "${components[@]}"; do
    echo -e "\n##### component: $i..."
    if cd "$i" ; then
        # building fims as a submodule for meta-RPMs, we need to install by default
        if [ ! -d "./package_utility" ]; then
            # we don't have a package utility submodule, use the monorepo package utility
            cp -R ../package_utility . 
        fi
        if [ "$i" == "fims" ]; then
            ./package_utility/build.sh "-bi" || warning_trap "failed to build submodule $i."
        else
            ./package_utility/build.sh "$@" || warning_trap "failed to build submodule $i."
        fi
        cd ../
    fi
done
