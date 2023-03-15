#!/bin/sh 
# run a test but you need $1 to be a name
# you may even need $2
#LD_LIBRARY_PATH=/usr/local/lib64:build/release_obj  valgrind -v --leak-check=full build/release_test/$1 $2
LD_LIBRARY_PATH=/usr/local/lib64:build/release_obj  valgrind -v --leak-check=full build/release_test/$1 $2
