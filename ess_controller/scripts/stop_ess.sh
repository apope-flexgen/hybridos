#!/bin/sh
# simple script to stop the ess_controller (new sched) after 21 seconds)

/usr/local/bin/fims/fims_send -m set -r /$$ -u /ess/full/control/ess/stopTime 21

