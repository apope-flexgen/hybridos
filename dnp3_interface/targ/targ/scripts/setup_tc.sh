#!/bin/sh 
# script to set up the bad network 
#setup_tc eno1 server
#setup_tc eno2 client

sudo tc qdisc change dev $1 root netem duplicate 20% delay 200ms 5ms loss 10% corrupt 15%
