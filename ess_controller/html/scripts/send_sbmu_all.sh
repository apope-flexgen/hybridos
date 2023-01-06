#!/bin/sh
# send to all the sbmu
for i in {1..9}
do
    echo "output: $i"
    sh scripts/send_sbmu_1.sh $i
done
