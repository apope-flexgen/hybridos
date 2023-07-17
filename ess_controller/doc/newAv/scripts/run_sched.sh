#!/bin/sh 
# basic sched test.

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/components/ess/SystemFault ' {
                        "refTime":      0.25,
                        "repTime":      0.5,
                        "aname":        "ess",
                        "angle":        34.56,
                        "id":   7,
                        "fcn":  "TestFunc"}'




/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/ess

/usr/local/bin/fims/fims_send -m sched -r /$$ -u /ess/components/ess/SystemFault
sleep 2
/usr/local/bin/fims/fims_send -m get -r /$$ -u /ess/components/ess/SystemFault
