#!/bin/bash

set -e

fims_send="/usr/local/bin/fims_send"

function error_trap() # error message passed as $1
{
    echo -e "\n${ERROR}##### ERROR: $1${NONE}\n"
    exit 1 # hard stop exceution
}

# check the path is valid, and also remove any trailing slashes
if [ -d "$1" ]; then
    path="${1%/}"
    path=$(readlink -f $path) # expand symlinks to original path
    echo -e "config: $path"
else
    error_trap "you have died of dysentery, try again (also, the path specified doesn't exist)"
fi

# sanitize all configs (with a .json extension only)
echo -e "sanitizing configs with dos2unix..."
find "$path" -type f -name "*.json" -print0 | xargs -0 dos2unix --

# generate folder array, use find here because 'ls' might error out
folders=($(find "$path" -maxdepth 1 -mindepth 1 -type d -printf "%f\n"))

for i in "${folders[@]}"; do
    base=$(basename $i) # get the basename of the folder, in case symlink expansion failed
    if [ "$base" == "ess_controller" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/$base/configs_${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    elif [ "$base" == "scheduler" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            if [ "$fName" == "scheduler.json" ]; then
                fName="configuration.json"
            fi
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/scheduler/${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    elif [ "$base" == "cops" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            if [ "$fName" == "cops.json" ]; then
                fName="configuration.json"
            fi
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/cops/${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    elif [ "$base" == "web_ui" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/ui_config/${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    elif [ "$base" == "ui_config" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            if [ "$fName" == "CA_configs.json" ]; then
                fName="assets.json"
            elif [ "$fName" == "layout_configs.json" ]; then
                fName="layout.json"
            fi
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/ui_config/${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    elif [ -d "$path/$base" ]; then
        FILES=$(find $path/$base -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES; do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo -e "loading: $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/$base/${fName%.*} -f $f || error_trap "could not send $fName to dbi"
        done
    fi
done