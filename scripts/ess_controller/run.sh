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

# Start modbus_client daemons
run_multiple_forever $bin_dir/modbus_client $cfg_dir/modbus_client

# Start modbus_server daemons
run_multiple_forever $bin_dir/modbus_server $cfg_dir/modbus_server

# Start dnp3_client daemons
run_multiple_forever $bin_dir/dnp3_client $cfg_dir/dnp3_client

# Start dnp3_server daemons
run_multiple_forever $bin_dir/dnp3_server $cfg_dir/dnp3_server

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

# Load a permanent, default user to the database
mongo hybridos_authentication scripts/mongo_users.js


# always launch dbi with configuration override
$bin_dir/dbi $cfg_dir/dbi/config.json &
# check if config folder is mounted
if [ -d "$cfg_dir" ]
then
    # if mounted, then we're going to load configs from the filesystem
    echo -e "config folder mounted, launching from filesystem..."
    /home/scripts/dbi.sh "$cfg_dir" > /tmp/dbi.out 2>&1
else
    # if not mounted, then we're going to remote load configs from an external mongodb container
    echo -e "config folder not mounted, launching from external mongodb container..."
fi
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

# launch echo processes if configs exist
if [ -d "$cfg_dir/echo" ]
then
    files=($(ls "$cfg_dir/$1/"))
    for file in "${files[@]}"
    do
        echo -e "launching echo: $file"
        $bin_dir/echo -run -c "$file" &
    done
fi
sleep 1s

# launch web server
if [ -d "$cfg_dir/web_server" ] && [ -d "$cfg_dir/web_ui" ]
then
    # Create a copy of permissions.json
    cp $cfg_dir/web_server/permissions.json $cfg_dir/web_server/permissions_copy.json

    echo "launching web_server..."
    $bin_dir/web_server "$bin_dir/web_ui" "$cfg_dir/web_ui" "$cfg_dir/web_server" &

    # Rename copy of permissions file to permissions.json
    sleep 5s; cp $cfg_dir/web_server/permissions_copy.json $cfg_dir/web_server/permissions.json
fi
sleep 1s

# launch ess controller
if [ ! -d $cfg_dir/ess_controller ]
then
    echo -e "launching ess_controller without configs..."
    $bin_dir/ess_controller -x
else
    local_cfg_path="/usr/local/etc/config"
    ln -s $cfg_dir $local_cfg_path
    file_names=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | sed -e 's/\.json$//'`
    num_files=`ls -1 "$cfg_dir/ess_controller" | grep _file.json | wc -l`
    count=0
    for f in $file_names
    do
        ((count+=1))
        if (( $count==$num_files ))
        then
            echo -e "launching last ess $f with configs..."
            $bin_dir/ess_controller -f $f
        else
            echo -e "launching $f with configs..."
            $bin_dir/ess_controller -f $f &
        fi
    done
fi
