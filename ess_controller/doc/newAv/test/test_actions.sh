#!/bin/sh 
# load up and test the actions concept.

sp='{"foo":{"fvalue":21,"myval":100,"actions":{"onSet":[{"enum":[{"eone":1,"two_1":2,"three_1":"sthree"},{"etwo":1,"two_2":2,"three_2":"sthree"}]}]},"value":234}}'

echo $sp | jq

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/stuff/test $sp



