#!/bin/sh
# simple script to stop the gpio_controller (new sched) after 21 seconds)

/usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/control/gpio/stopTime 21

