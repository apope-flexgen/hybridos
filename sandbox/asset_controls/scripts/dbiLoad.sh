#!/bin/bash

fims_send="/usr/local/bin/fims_send"
# args=("$@")

if [ "$#" -lt 1 ]; then
    echo -e "[dbi_load.sh] you have died of dysentery, try again."
    exit 1
fi
path=$1

folders=()
# j=1
# for arg in "$@"; do
#     if [ "$j" -gt 1 ]; then
#         folders+=("$arg")
#     fi
#     ((j++))
# done

# remove the version information
folders=("ess_controller")

for i in "${folders[@]}"; do
    echo $i
    if [ "$i" == "ess_controller" ]; then
        FILES=$(find $path/asset_controlls/config/ess_controller -type f -name '*.json') # ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo "Loading $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/$i/configs_${fName%.*} -f $f
        done
    elif [ "$i" == "web_ui" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo "Loading $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/ui_config/${fName%.*} -f $f
        done
    elif [ -d "$path/$i" ]; then
        index=0
        names=($(ls -1 "$path/$i" | grep .json | sed -e 's/\.json$//')) # names only, no extension, for the uri
        files=($(ls -1 "$path/$i" | grep .json)) # names with the extension, for the "-f" to send
        for j in "${names[@]}"; do
            echo -e "loading $path/$i/${files[index]} to dbi..."
            $fims_send -m "set" -u "/dbi/$i/$j" -f "$path/$i/${files[index]}"
            index=$index+1
        done
    fi
done