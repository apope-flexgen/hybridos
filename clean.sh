#!/bin/bash

# source build functions (from repository path)
source build_utils.sh || error_trap "failed to import $cwd/build_utils.sh."

if [ ! -n "$components" ]; then
    error_trap "build_utils.sh does not specify 'components' field."
fi

sudo git clean -xdf

for i in "${components[@]}"; do
    if cd "$i" ; then
        sudo git clean -xdf
        cd ../
    fi
done
