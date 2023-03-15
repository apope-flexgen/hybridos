#!/bin/sh
n=1
while [ $n -le 100 ]
do
  sh  scripts/test_multi_os2.sh  $n
  sleep 1
  sh  scripts/test_multi_os1.sh  $n
  sleep 1
  n=$(( n+1))
done

