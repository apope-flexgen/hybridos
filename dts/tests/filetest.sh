#!/bin/bash

rm -rf /home/vagrant/test/*

for i in {0..1000..1}
do
    touch /home/vagrant/test/test$i.txt

    d=$(($i%100))
    if [ "$d" -eq 0 ]; then
        mkdir /home/vagrant/test/dir$i
    fi

    l=$(($i%200))
    if [ "$l" -eq 0 ]; then
        touch /home/vagrant/test/test$i.log
        sleep 3s
    fi
done

sleep 10s
mkdir /home/vagrant/test/end/
touch /home/vagrant/test/end/test.txt