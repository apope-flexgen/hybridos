#!/bin/bash

# Lists the open file descriptors currently held by the named process

pid=$(pgrep $1)
echo The process pid is $pid
let count=$(lsof -p $pid -n | wc -l)-1 # Use -n option because resolving host names can be very slow
echo The number of open files is $count
lsof -p $pid -n 