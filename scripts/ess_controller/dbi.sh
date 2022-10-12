#!/bin/bash

mapfile -t rpms < rpm.txt
path=$(readlink -f "$1")
echo -e "$path"

fims_send="/usr/local/bin/fims_send"

if [ "$#" -ne 1 ]; then
    echo -e "[dbi_load.sh] you have died of dysentery, try again."
    exit 1
fi

# remove the version information
folders=("${rpms[@]%-*}")
folders+=("ui_config") # TODO: remove temporary fix inserting ui_config

for i in "${folders[@]}"; do
    if [ "$i" == "ess_controller" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
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