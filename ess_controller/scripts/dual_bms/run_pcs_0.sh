#!/bin/sh
# p wilshire 03-13-2022

fims_send -m set -r /$$ -u /ess/config/pcs/enable true
fims_send -m set -r /$$ -u /ess/full/control/pubs/enable_pcs true
fims_send -m set -r /$$ -u /ess/control/pubs/runPubs 22
fims_send -m set -r /$$ -u /ess/control/sched/runPcs 22
