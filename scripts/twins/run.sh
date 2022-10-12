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

# check for twins.json, otherwise this container is pointless
if [ ! -f "$cfg_dir/twins/twins.json" ]
then
    echo -e "you forgot to include $cfg_dir/twins/twins.json, aborting."
    exit 1
fi

# generate modbus_server and echo files
/home/scripts/echo.sh "$cfg_dir"

# TODO: separate databases into their own container
# TODO: launch influxdb when storage is rewritten in golang
#echo "launching influxdb..."
#sudo influxd 2/dev/null &
#sleep 1s

echo -e "launching fims_server..."
$bin_dir/fims_server &
#chmod 777 $sock
sleep 1

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
launch_multiple modbus_server
launch_multiple dnp3_server
sleep 1s

# launch twins (as the last process to keep the container running)
echo -e "launching twins..."
$bin_dir/twins "$cfg_dir/twins"
rm -f $sock # remove flexgen.sock on host machine after twins terminates