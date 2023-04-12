#!/bin/bash

sys_cfg_path=/usr/local/etc/config
tmp_cfg_dir=/usr/local/config
bin_dir=/usr/local/bin

log_dir=/var/log

staging_dir=/home/staging
cfg_dir=/home/config

# helper function to conditionally launch processes if configs exist
# '$1' is both the binary's config folder and the binary itself
function launch()
{
    if [ -d "$cfg_dir/$1" ]
    then
        echo -e "launching $1..."
        $bin_dir/$1 "$cfg_dir/$1" &
    else
        echo -e "skipping $1, no configs exist."
    fi
}

# Run a blocking program in an infinite loop forever.
# $1: exe path
# $2: cmdarg 1
function run_forever()
{
    while true; do
        sleep 3s; $1 "$2"
    done 
}

# Start daemon loops using the provided exe path for all files in the specified dir.
# $1 exe path
# $2 directory path
function run_multiple_forever()
{
    if [ -d "$2" ]
    then
        files=($(find "$2" -maxdepth 1 -type f -name '*.json' -printf '%f\n'))
        for file in "${files[@]}"
        do
            echo -e "Starting daemon using $1 with input $file"
            run_forever $1 "$2/$file" &
        done
    else
        echo -e "skipping $2, no configs exist."
    fi
}

# generate modbus_server and/or dnp3_server and echo files
# from their respective client configs
function generate_configs_from_clients()
{
    name=$(echo $1 | sed 's/_client//')
    if [ -d "$cfg_dir/$1" ]
    then
        files=($(find "$cfg_dir/$1" -maxdepth 1 -type f -name '*.json' -printf '%f\n'))
        for file in "${files[@]}"; do
            echo -e "generating echo and server files for $1/$file"
            $bin_dir/echo -config="$cfg_dir/$1/$file" -mode=$name
        done
        # move new configs to appropriate locations
        mkdir -p $cfg_dir/$name"_server"
        mkdir -p $cfg_dir/echo
        mv $cfg_dir/$1/*_server.json $cfg_dir/$name"_server"/
        mv $cfg_dir/$1/*_echo.json $cfg_dir/echo/
    else
        echo -e "skipping $1, no configs exist."
    fi
}

# sanitize all scripts (with a .sh extension only)
echo -e "sanitizing scripts with dos2unix..."
find /home/scripts -type f -name "*.sh" -print0 | xargs -0 dos2unix --
find /home/utility -type f -name "*.sh" -exec dos2unix {} \; -exec chmod +x {} \;

# make config directory and symlink if it doesn't exist
mkdir -p $cfg_dir
ln -s $cfg_dir $sys_cfg_path

# move configs to correct location and modify their ip addresses to work in virtual site
if [ -d $staging_dir ]
then
    cp -r $staging_dir/* $cfg_dir
    python /home/scripts/ip_change.py
fi

# move any template configs to correct location
if [ -d $tmp_cfg_dir ]
then
    cp -r $tmp_cfg_dir/* $cfg_dir/*
fi

# set the timezone for scheduler
TIME_ZONE_FILE=$cfg_dir/scheduler/timezone.txt
if test -f "$TIME_ZONE_FILE"; then
    time_zone=`cat $TIME_ZONE_FILE`
else
    time_zone='America/New_York'
fi
rm -rf /etc/localtime
ln -s /usr/share/zoneinfo/$time_zone /etc/localtime
echo -e "set container timezone to $time_zone..."

echo -e "launching fims"
$bin_dir/fims_server &
sleep 1s

echo -e "launching mongodb..."
mongod --config /etc/mongod.conf &
sleep 1s

echo -e "load default flexgen user into hybridos_authentication..."
mongo hybridos_authentication /home/scripts/web/mongo_users.js
sleep 1s

# TODO: in the future, handle the case where configs are located
# in an external mongodb container
if [ -d "$cfg_dir" ]
then
    echo -e "launching dbi..."
    $bin_dir/dbi $cfg_dir/dbi/dbi.json &
    sleep 3s

    echo -e "loading configs from file system to dbi"
    /home/scripts/dbi.sh "$cfg_dir"
fi
sleep 1s

# if the container has a twins instance, need to generate echo and server files from clients before launching them
if [ -d $cfg_dir/twins ]
then
    echo -e "generating servers and echo files from modbus clients..."
    generate_configs_from_clients "modbus_client"
    echo -e "generating servers and echo files from dnp3 clients..."
    generate_configs_from_clients "dnp3_client"
fi

# Start echo daemons
echo -e "running all echo processes..."
run_multiple_forever $bin_dir/echo $cfg_dir/echo

# Start modbus_client daemons
echo -e "running all modbus client processes..."
run_multiple_forever $bin_dir/modbus_client $cfg_dir/modbus_client

# Start modbus_server daemons
echo -e "running all modbus server processes..."
run_multiple_forever $bin_dir/modbus_server $cfg_dir/modbus_server

# Start dnp3_client daemons
echo -e "running all dnp3 client processes..."
run_multiple_forever $bin_dir/dnp3_client $cfg_dir/dnp3_client

# Start dnp3_server daemons
echo -e "running all dnp3 server processes..."
run_multiple_forever $bin_dir/dnp3_server $cfg_dir/dnp3_server

# launch events
echo -e "launching events..."
launch events
sleep 1s

# launch metrics
echo -e "running all metrics processes..."
run_multiple_forever $bin_dir/metrics $cfg_dir/metrics

# launch scheduler
echo -e "launching scheduler..."
launch scheduler
sleep 1s

# launch cops
echo -e "launching cops..."
launch cops
sleep 1s

# if the container has a web_ui, ensure there is a permissions.json to launch
if [ -d $cfg_dir/web_server ] && [ -d $cfg_dir/web_ui ]
then
    if [ -f "$cfg_dir/web_server/permissions.json" ]
    then
        cp "$cfg_dir/web_server/permissions.json" "$cfg_dir/web_server/permissions_copy.json" # take a backup
    elif [ -f "$cfg_dir/web_server/permissions_copy.json" ]
    then
        cp "$cfg_dir/web_server/permissions_copy.json" "$cfg_dir/web_server/permissions.json"
    else
        echo -e "no permissions.json or permissions_copy.json found, aborting"
        exit 1
    fi

    echo -e "launching web_server..."
    $bin_dir/web_server "$bin_dir/web_ui" "$cfg_dir/web_ui" "$cfg_dir/web_server" &
    sleep 5s

    # restore permissions.json from copy
    echo -e "restore permissions.json from copy"
    cp $cfg_dir/web_server/permissions_copy.json $cfg_dir/web_server/permissions.json
fi

# "main" processes in the containers
if [ -d $cfg_dir/twins ]
then
    echo -e "launching twins..."
    $bin_dir/twins "$cfg_dir/twins" &
fi

if [ -d $cfg_dir/ess_controller ]
then
    file_names=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | sed -e 's/\.json$//'`
    for f in $file_names
    do
        echo -e "launching $f with configs..."
        $bin_dir/ess_controller -f $f &
    done
fi

if [ -d $cfg_dir/site_controller ]
then
    echo -e "launching site_controller..."
    $bin_dir/site_controller "dbi" &
fi

if [ -d $cfg_dir/fleet_manager ]
then
    echo -e "launching fleet_manager..."
    $bin_dir/fleet_manager "dbi" &
fi

# find all executable scripts in the /home/utility directory
find /home/utility -type f -executable -name "*.sh" | while read file; do
    # execute each script found
    bash "$file"
done

# keep container alive forever
while :
do
    sleep 10000
done