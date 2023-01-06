#!/bin/sh
# script to activate the scheduler operations.
#                          Id              Control Var                      function             # run datadata 
# "addSchedItem":{"value":"Every100mSP1",
#    "uri":"/sched/ess:every100mSP1", 
#   "func":"Every100mSP1",
#   "refTime":0.210,"runTime":0.210,"repTime":0.100},

echo " current state "
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/every100mSP1 | jq

echo " force shutdown, should just need endTime "
/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/sched/ess/every100mSP1 '
    {"value":	2.34, 
     "repTime":0.00,
     "endTime":1.00
	}'

echo " after forced shutdown"
sleep 0.1
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/every100mSP1 | jq

sleep 0.1
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/every100mSP1 | jq

#sleep 0.5
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/every1000mS | jq

#sleep 0.5
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/sched/ess/every100mSP3 | jq

#sleep 0.5
#/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/full/schedule/ess/addSchedItem | jq




# /usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/schedule/ess '{
# "addSchedItem":{"value":"None","actions":{"onSet":	[{ "func":	[{"func":	"HandleSchedItem","amap":	"ess"}]}]}},
# "addSchedItem":{"value":"EssSystemInit","uri":"/sched/ess:essSystemInit","fcn":"EssSystemInit","refTime":0.200,"runTime":0.200,"repTime":0.000},
# "addSchedItem":{"value":"Every1000mS",  "uri":"/sched/ess:every1000mS",  "fcn":"Every1000mS","refTime":0.210,"runTime":0.210,"repTime":1.000},
# "addSchedItem":{"value":"Every100mSP1", "uri":"/sched/ess:every100mSP1", "fcn":"Every100mSP1","refTime":0.220,"runTime":0.220,"repTime":0.100},
# "addSchedItem":{"value":"Every100mSP2", "uri":"/sched/ess:every100mSP2", "fcn":"Every100mSP2","refTime":0.230,"runTime":0.230,"repTime":0.100},
# "addSchedItem":{"value":"Every100mSP3", "uri":"/sched/ess:every100mSP3", "fcn":"Every100mSP3","refTime":0.240,"runTime":0.240,"repTime":0.100},
# "addSchedItem":{"value":"FastPub",      "uri":"/sched/ess:fastPub",      "fcn":"FastPub","refTime":0.250,"runTime":0.250,"repTime":0.050},
# "addSchedItem":{"value":"SlowPub",      "uri":"/sched/ess:slowPub",      "fcn":"SlowPub","refTime":0.260,"runTime":0.260,"repTime":2.000}
# }'

# modSchedItem
# delSchedItem

