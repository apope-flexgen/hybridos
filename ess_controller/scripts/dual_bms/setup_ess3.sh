#!/bin/sh 
# p. wilshire 03-09-2022

#setup for a specific test video 
# use $1 to pick 

cd configs/dbi
 
ln -sf ess_controller$1 ess_controller
