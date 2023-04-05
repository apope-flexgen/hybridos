#!/bin/bash

bin_dir=/usr/local/bin
cfg_dir=/home/config
cfg_override=/usr/local/etc/config
scripts_dir=/home/scripts
bin_override=/home/bin
sock=/tmp/FlexGen_FIMS_Server.socket

ln -s $cfg_dir $config_override

ls /tmp

# move bin_override executables to bin_dir if applicable
if [ -d $bin_override ]
then
    cp $bin_override/* $bin_dir
fi

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
function launch_multiple()
{
    if [ -d "$cfg_dir/$1" ]
    then
    files=($(ls "$cfg_dir/$1/"))
        for file in "${files[@]}"
        do
            echo -e "launching $1: $file"
            $bin_dir/$1 "$cfg_dir/$1/$file" &
        done
    else
        echo -e "skipping $1, no configs exist."
    fi
}

# Start daemon loops using the provided exe path for all files in the specified dir.
# $1 exe path
# $2 directory path
function run_multiple_forever()
{
    if [ -d "$2" ]
    then
    files=($(ls "$2"))
        for file in "${files[@]}"
        do
            echo -e "Starting daemon using $1 with input $file"
            run_forever $1 "$2/$file" &
        done
    else
        echo -e "skipping $2, no configs exist."
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

# No scheduler in BRP sites
# configure time zone of container
# echo "Setting time zone of site container"
# TIME_ZONE_FILE=$cfg_dir/scheduler/timezone.txt
# if test -f "$TIME_ZONE_FILE"; then
#     time_zone=`cat $TIME_ZONE_FILE`
# else
#     time_zone='America/New_York'
# fi
# rm -rf /etc/localtime
# ln -s /usr/share/zoneinfo/$time_zone /etc/localtime
# echo "Set time zone to $time_zone"

# # TODO: remove temporary check for permissions.json
# if [ ! -f "$cfg_dir/web_server/permissions.json" ]
# then
#     echo -e "you forgot to include $cfg_dir/web_server/permissions.json, aborting."
#     exit 1
# fi

# TODO: separate databases into their own container
# TODO: launch influxdb when storage is rewritten in golang
#echo "launching influxdb..."
#sudo influxd 2/dev/null &
#sleep 1s

echo "launching mongodb..."
mongod --config /etc/mongod.conf &
sleep 1s

# Load a permanent, default user to the database
mongo hybridos_authentication scripts/mongo_users.js

# check if fims socket is mounted
if [ -S "$sock" ]
then
    echo -e "fims socket mounted, will not launch fims_server..."
# if no socket is mounted, and configs exist, start fims and configs
else
    echo -e "fims socket not mounted, will launch fims_server..."
    $bin_dir/fims_server &
fi
sleep 1s

# always launch dbi with configuration override
$bin_dir/dbi $cfg_dir/dbi/config.json &
sleep 2s
# check if config folder is mounted
if [ -d "$cfg_dir" ]
then
    # if mounted, then we're going to load configs from the filesystem
    echo -e "config folder mounted, launching from filesystem..."
    /home/scripts/dbi.sh "$cfg_dir"
else
    # if not mounted, then we're going to remote load configs from an external mongodb container
    echo -e "config folder not mounted, launching from external mongodb container..."
fi
sleep 1s

# launch modbus|dnp3 clients if configs exist
run_multiple_forever $bin_dir/modbus_client $cfg_dir/modbus_client
launch_multiple dnp3_client
sleep 1s

# launch events
launch events
sleep 1s

# launch metrics
launch metrics
sleep 1s

# No scheduler in BRP sites
# launch scheduler, no configs
# $bin_dir/scheduler &
# sleep 1s

# launch cops
launch cops
sleep 1s

# launch echo processes if configs exist
if [ -d "$cfg_dir/echo" ]
then
    files=($(ls "$cfg_dir/echo/"))
    for file in "${files[@]}"
    do
        echo -e "launching echo: $file"
        $bin_dir/echo "$cfg_dir/echo/$file" &
    done
fi
sleep 1s

# launch modbus|dnp3 servers if configs exist
run_multiple_forever $bin_dir/modbus_server $cfg_dir/modbus_server
run_multiple_forever $bin_dir/dnp3_server $cfg_dir/dnp3_server
sleep 1s

# launch web server
if [ -d "$cfg_dir/web_server" ] && [ -d "$cfg_dir/web_ui" ]
then
    echo "launching web_server..."
    $bin_dir/web_server "$bin_dir/web_ui" "$cfg_dir/web_ui" "$cfg_dir/web_server" &

    # Load a permanent, default user to the database
    mongo hybridos_authentication /home/scripts/utility/mongo_users.js

    # Rename copy of permissions file to permissions.json
    sleep 3s; cp $cfg_dir/web_server/permissions_copy.json $cfg_dir/web_server/permissions.json
fi
sleep 1s

# launch site controller
# always launch with 'dbi' flag, if configs are mounted then dbi_load.sh will load into mongodb
echo -e "launching site_controller..."
$bin_dir/site_controller "dbi" &

# Close BMS DC contactors and start the site
sleep 3s
sh $scripts_dir/utility/close_dc_contactors.sh &
sleep 10s
$bin_dir/fims_send -m set -u /site/input_sources/ui '{"value":true}'
sleep 1s
$bin_dir/fims_send -m set -u /site/operation/enable_flag_ui '{"value":true}'
sleep 1s
$bin_dir/fims_send -m set -u /site/input_sources/ui '{"value":false}'

sleep 15s

sh $script_dir/utility/close_dc_contactors.sh &

bash