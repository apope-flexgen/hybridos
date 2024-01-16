#!/bin/sh
# this turns off the ess controller 
#just turn off all tasks in ess_sched
# we can then load simulator tass as needed.

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/sched/ess '
{ 
    "every1000mS": {
    "value": "Every1000mS",
    "enabled": false,
    "endTime": 1
  },
  "every100mSP1": {
    "value": "Every100mSP1",
    "enabled": false,
    "endTime": 1
  },
  "fastPub": {
    "value": "FastPub",
    "enabled": false,
    "endTime": 1
  },
  "slowPub": {
    "value": "SlowPub",
    "enabled": false,
    "endTime": 1
    }
}
'
exit
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess | jq


