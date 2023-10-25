#!/bin/bash

config_dir=/usr/local/etc/config

function help()
{
    symlink=$(readlink --canonicalize $config_dir) # update symlink for reporting

    echo -e ""
    echo -e "config:\t/usr/local/etc/config -> $symlink"
    echo -e "usage:\t./hybridos_config.sh </path/to/config>"
}

if [ $# = 0 ]; then # no arguments provided, print link at end end
    echo -e "no arguments provided"
    
    help
    exit 0
else
    config_path=$(readlink --canonicalize "$1")
    echo -e "$config_path"

    if [ ! -d "$config_path" ]; then echo "##### ERROR: specified directory does not exist"; help; exit 1; fi

    sudo rm -rf $config_dir
    sudo ln -sf "$config_path" $config_dir

    help
    exit 0
fi
