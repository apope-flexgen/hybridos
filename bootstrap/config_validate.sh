#!/bin/bash

path=$(readlink -f "$1")
echo -e "$path"
files=($(ls "$path"))

if [ "$#" -ne 1 ]; then
    echo -e "[config_validate.sh] you have died of dysentery, try again."
    exit 1
fi


#Decides which struct to use for checking
for file in "${files[@]}"
do
    if [ "$file" == "ess_controller" ] && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "site_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "fleet_manager" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "twins";
    then
        struct=ess_controller
        break
    elif [ "$file" == "fleet_manager" ] && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "site_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "ess_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "twins";
    then
        struct=fleet_manager
        break
    elif [ "$file" == "site_controller" ] && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "fleet_manager" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "ess_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "twins";
    then
        struct=site_controller
        break
    elif [ "$file" == "twins" ] && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "site_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "ess_controller" && ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "fleet_manager";
    then
        struct=twins
        break
    fi
done
if [ -z $struct ]; then
    echo "You have an extra product in this folder that needs to be removed"
    exit 0
fi
#Checks to see if the struct is the correct one and has the correct folders
if [ $struct == "fleet_manager" ]; then
    fleetno=( "storage" "site_controller" "ess_controller" "twins" "modbus_server" "modbus_client" "dts" "gpio_controller" "mcp" )
    for folder in "${files[@]}"
    do
        if echo ${fleetno[@]} | tr " " '\n' | grep -F -q -x "$folder"; then
            echo "This is not valid $struct config folder because $folder shouldnt be in it"
            break
        fi
    done
fi 

if [ $struct == "site_controller" ]; then
    siteno=( "fleet_manager" "ess_controller" "twins" "echo" "dnp3_client" "gpio_controller" "modbus_server" )
    for folder in "${files[@]}"
    do
        if echo ${siteno[@]} | tr " " '\n' | grep -F -q -x "$folder"; then
            echo "This is not valid because $folder shouldnt be in $struct config folder"
            break
        fi
    done
fi 

if [ $struct == "ess_controller" ]; then
    essno=( "scheduler" "site_controller" "fleet_manager" "twins" "cops" "dbi" "go_logging" "ftd" "cloud_sync" "echo" "dnp3_client" "dnp3_server" "dts" "mcp" )
    for folder in "${files[@]}"
    do
        if echo ${essno[@]} | tr " " '\n' | grep -F -q -x "$folder"; then
            echo "This is not valid because $folder shouldnt be in $struct config folder"
            break
        fi
    done
fi 

if [ $struct == "twins" ]; then
    twinsno=( "twins" "echo" "modbus_server" )
    for folder in "${twinno[@]}"
    do
        if ! echo ${files[@]} | tr " " '\n' | grep -F -q -x "$folder"; then
            echo "This is not valid because $folder shouldnt be in $struct config folder"
            break
        fi
    done
fi 

#Checks to see if JSON is valid and if folders have the correct file
for folder in "${files[@]}"
do
    infolder=($(ls "$path/$folder"))
    if [ $folder == "twins" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "twins.json" ; then echo "no twins.json"; fi
    if [ $folder == "site_controller" ];
    then 
        sitelist=("assets.json" "sequences.json" "variables.json")
        for file in "${sitelist[@]}"
        do
           if ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "$file" ; then echo "no $file"; fi
        done 
    fi
    if [ $folder == "cloud_sync" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "cloud_sync.json" ; then echo "no cloud_sync.json"; fi
    if [ $folder == "cops" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "cops.json" ; then echo "no cops.json"; fi
    if [ $folder == "cloud_sync" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "cloud_sync.json" ; then echo "no cloud_sync.json"; fi
    if [ $folder == "dbi" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "dbi.json" ; then echo "no dbi.json"; fi
    if [ $folder == "dts" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "dts.json" ; then echo "no dts.json"; fi
    if [ $folder == "events" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "events.json" ; then echo "no events.json"; fi
    if [ $folder == "ftd" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "ftd.json" ; then echo "no ftd.json"; fi
    if [ $folder == "scheduler" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "scheduler.json" ; then echo "no schedular.json"; fi
    if [ $folder == "storage" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "storage.json" ; then echo "no storage.json"; fi
    if [ $folder == "web_server" ];
    then 
        weblist=("permissions.json" "permissions_copy.json" "web_server.json")
        for file in "${weblist[@]}"
        do
           if ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "$file" ; then echo "no $file"; fi
        done 
    fi
    if [ $folder == "web_ui" ] && ! echo ${infolder[@]} | tr " " '\n' | grep -F -q -x "web_ui.json" ; then echo "no web_ui.json"; fi

    for file in "${infolder[@]}"
    do
        if ! echo python -mjson.tool $file > /dev/null ; then
            echo "File: $file, is not valid json please redo it"
        fi
    done
done

