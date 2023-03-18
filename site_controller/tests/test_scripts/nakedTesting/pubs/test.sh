#!/bin/bash

for status in "start_stop" "negative"
do
    echo $status
    
    filter="has(\"$status\") fakedComponents/components_ess_twins.json"
    inHighSpeed=$( jq $filter) 
    echo $inHighSpeed
    if [[ inHighSpeed == "true" ]]
    then
        echo "it's there"
    else
        echo "it's not there"
    fi
done