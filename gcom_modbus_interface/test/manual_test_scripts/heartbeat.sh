#!/bin/bash

for ((i=1; ; i++)); do
    fims_send -m pub -u /components/sel_2440 '{"input_2_1":'$i'}'
    sleep 0.1
done