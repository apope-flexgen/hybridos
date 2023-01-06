#!/bin/sh
# script to set up a specific config directory
# $1 is required the version of the configs to use


echo "running $0 with  $1 args $# "

if [ $# -lt 1 ]; then
  echo "#############################"
  echo "please supply  the config  id"
  echo " no config change made       "
  echo "#############################"
else
  echo "setting config dir to $1 args $# "
  sudo rm -d /usr/local/etc/config
  sudo ln -sf /home/pi/install/configs/$1 /usr/local/etc/config
  touch /usr/local/etc/config/$1
fi

ls -ld /usr/local/etc/config
tree /usr/local/etc/config

