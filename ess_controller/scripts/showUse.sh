#!/bin/sh 
# script to find where things are used 
# showUse SomeVar
 echo 
 echo
 echo "search configs"
 echo
 grep -B 5 -A 10 $1 configs/*.json
 echo 
 echo
 echo "search cfuncs"
 echo
 grep -B 5 -A 10 $1 funcs/*.cpp
 echo

 