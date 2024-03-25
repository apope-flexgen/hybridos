#!/bin/bash

/usr/local/bin/fims_server &
sleep 1
/usr/local/bin/ftd --c=/home/config/ftd.json &

watch -n 1 "fims_send -m post -u /events '{\"source\":\"test\",\"message\":\"test event\",\"severity\":3}'" &
watch -n 1 "fims_send -m pub -u /test '{\"value\":1}'" &

# Enable the following pub to get a type conflict write error when writing to influx
# watch -n 1 "fims_send -m pub -u /test_string '{\"value\":\"test string\"}'" &

/usr/local/bin/dts --c=/home/config/dts.json --logCfg=/home/config/dts_verbose.json