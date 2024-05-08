#!/bin/bash

/usr/local/bin/fims_server &> /dev/null &
sleep 1
/usr/local/bin/ftd --c=/home/config/ftd.json &

/home/fims_simulator &

# Enable the following pub to get a type conflict write error when writing to influx
# watch -n 1 "fims_send -m pub -u /test_string '{\"value\":\"test string\"}'" > /dev/null &

/usr/local/bin/dts --c=/home/config/dts.json --logCfg=/home/config/dts_verbose.json