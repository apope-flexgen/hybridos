#!/bin/sh
# script to stop the noisy gpio pubs

/usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/sched/gpio '{"gpioPub":{"value":"GpioPub","endTime":0}}'

