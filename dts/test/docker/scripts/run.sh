#!/bin/bash

/usr/local/bin/fims_server &> /dev/null &
sleep 1
/usr/local/bin/ftd --c=/home/config/ftd.json &

/home/fims_simulator &

/usr/local/bin/dts --c=/home/config/dts.json --logCfg=/home/config/dts_verbose.json