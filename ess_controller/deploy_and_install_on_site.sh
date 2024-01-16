#!/bin/bash

display_usage() { 
	echo "This script must be run as follows, requires the install directory where you run it:" 
	echo -e "\nUsage: $0 target_username target_ip \n" 
}

trap display_usage ERR # "catches" all errors with display_usage

# must have at least 2 arguments
if [  $# -le 1 ] 
then 
    display_usage
    exit 1
fi

USER=$1 # first arg is user name of target
IP=$2 # second arg is ip address of target
INSTALLDIR=install # uses the current install dir.

scp -r $INSTALLDIR $USER@$IP:~/ # put INSTALLDIR into home folder of target user on target system

# runs the rest of the install scripts inside the install folder
ssh -t $USER@$IP "
    sh $INSTALLDIR/scripts/install_third_party.sh
    sh $INSTALLDIR/scripts/install_release.sh
    sh $INSTALLDIR/scripts/install_configs.sh
"

exit 1