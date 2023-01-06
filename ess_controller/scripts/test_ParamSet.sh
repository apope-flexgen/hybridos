/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/status/bms \
'{"Heartbeat":{
  "value": 1234,
  "initCnt": -1,
  "lastHeartbeat": -1,
  "rdAlarm": 0.5,
  "rdFault": 5,
  "rdLast": 3,
  "rdReset": 2.5,
  "seenAlarm": false,
  "seenFault": false,
  "seenInit": true,
  "seenOk": false,
  "totalHbAlarms": 0,
  "totalHbFaults": 0,
  "newval": 1234,
  "newstr":"new string"
}}'

