#!/bin/bash

pgrep -x fims_server > /dev/null
if [ $? -ne 0 ]; then
    /usr/local/bin/fims_server &
fi
pgrep -x mongod > /dev/null
if [ $? -ne 0 ]; then
    mongod --config /etc/mongod.conf
fi
pgrep -x dbi > /dev/null
if [ $? -ne 0 ]; then
    /usr/local/bin/dbi &
fi