#!/bin/bash

set -xe

if [ "$#" -ne 1 ]
then
    echo "Please enter two arguments:"
    echo "1) number of seconds of grace period for disconnects"
fi

acceptable_diff=$(( $1 * 100 ))

lag_heartbeat=$(fims_send -m get -u /metrics/timestamp/FM_timestamp -r /$$)
local_heartbeat=$(fims_send -m get -u /features/site_operation/heartbeat_counter -r /$$ | jq .value)

diff=$((local_heartbeat-lag_heartbeat))
abs_diff=${diff#-}
echo "diff $abs_diff" >> /var/log/flexgen/dnp3_bounce.log

if [ $abs_diff -gt $acceptable_diff ]
then
    echo "restarting all dnp3 servers to refresh connection."
    echo "$(date +"%Y/%m/%d %T"): Restarting all dnp3 servers to refresh connection." >> /var/log/flexgen/dnp3_bounce.log
    echo "$(date +"%Y/%m/%d %T"): acceptable difference in heartbeat was $acceptable_diff , and actual difference in heartbeat was $abs_diff" >> /var/log/flexgen/dnp3_bounce.log
    systemctl restart dnp3_server@*
    # curl -d "text=<@U03DDELQ4RZ> <@U02N1L5ECM6> <@U01CEMN66CB> - $(hostname)'s DNP3 comms might be down with a $abs_diff" -d "channel=C04C1KDPKA7" -d "token=xoxb-34455185459-4725418458785-zhtRao8vSuI0piOIrZHdO3ZT" -X POST https://slack.com/api/chat.postMessage

fi
