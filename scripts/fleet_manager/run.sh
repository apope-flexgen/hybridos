#!/bin/bash

bin_dir=/usr/local/bin
cfg_dir=/home/config
bin_override=/home/bin
sock=/tmp/FlexGen_FIMS_Server.socket

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

# TODO: remove temporary check for permissions.json
if [ ! -f "$cfg_dir/web_server/permissions.json" ]
then
    echo -e "you forgot to include $cfg_dir/web_server/permissions.json, aborting."
    exit 1
fi

# TODO: separate databases into their own container
# TODO: launch influxdb when storage is rewritten in golang
#echo "launching influxdb..."
#sudo influxd 2/dev/null &
#sleep 1s

echo "launching mongodb..."
mongod --config /etc/mongod.conf &
sleep 1s

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
launch_multiple modbus_client
launch_multiple dnp3_client
sleep 1s

# TODO: launch when storage is rewritten in golang
# launch storage
#if [ -d "$cfg_dir/storage" ]
#then
#    echo "launching storage..."
#    $bin_dir/storage "$cfg_dir/storage" &
#fi
#sleep 1s

# launch events
launch events
sleep 1s

# launch metrics
launch metrics
sleep 1s

# launch scheduler, no configs
$bin_dir/scheduler &
sleep 1s

# launch cops
launch cops
sleep 1s

# launch echo processes if configs exist
if [ -d "$cfg_dir/echo" ]
then
    files=($(ls "$cfg_dir/$1/"))
    for file in "${files[@]}"
    do
        echo -e "launching echo: $file"
        $bin_dir/echo -run -config="$file" &
    done
fi
sleep 1s

# launch modbus|dnp3 servers if configs exist
launch_multiple modbus_server
launch_multiple dnp3_server
sleep 1s

# launch web server
if [ -d "$cfg_dir/web_server" ] && [ -d "$cfg_dir/web_ui" ]
then
    echo "launching web_server..."
    $bin_dir/web_server "$bin_dir/web_ui" "$cfg_dir/web_ui" "$cfg_dir/web_server" &
fi
sleep 1s

# launch fleet manager
# always launch with 'dbi' flag, if configs are mounted then dbi_load.sh will load into mongodb
echo -e "launching fleet_manager..."
$bin_dir/fleet_manager "dbi"
