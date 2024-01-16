#!/usr/bin/sh

echo "/ess/status/bms"
/usr/local/bin/fims_send -m get -r /me -u /ess/status/bms | jq
echo "/ess/config/locks"
/usr/local/bin/fims_send -m get -r /me -u /ess/config/locks | jq
echo "/ess/system/commands/locks"
/usr/local/bin/fims_send -m get -r /me -u /ess/full/system/commands/locks | jq