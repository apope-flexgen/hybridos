#!/bin/bash

fims_send="/usr/local/bin/fims_send"

if [ "$#" -ne 1 ]; then
    echo -e "[config_load.sh] you have died of dysentery, try again."
    exit 1
fi

# remove the version information
path=$1
folders=($(ls $path))

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
    elif [ "$i" == "site_controller" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            dir=$(dirname ${f})
            echo "Loading $fName from $dir to database..."
            echo "/dbi/$i/${fName%.*}"
            $fims_send -m "set" -u /dbi/$i/${fName%.*} -f $f
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
    elif [ "$i" == "scheduler" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            if [ "$fName" == "scheduler.json" ]; then
                fName="configuration.json"
            fi
            dir=$(dirname ${f})
            echo "Loading $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/scheduler/${fName%.*} -f $f
        done
    elif [ "$i" == "cops" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            if [ "$fName" == "cops.json" ]; then
                fName="configuration.json"
            fi
            dir=$(dirname ${f})
            echo "Loading $fName from $dir to database..."
            $fims_send -m "set" -u /dbi/cops/${fName%.*} -f $f
        done
    elif [ "$i" == "ui_config" ]; then
        FILES=$(find $path/$i -type f -name '*.json' ! -name '*_file.json')
        for f in $FILES
        do
            fName=$(basename ${f})
            if [ "$fName" == "CA_configs.json" ]; then
                fName="assets.json"
            elif [ "$fName" == "layout_configs.json" ]; then
                fName="layout.json"
            fi
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