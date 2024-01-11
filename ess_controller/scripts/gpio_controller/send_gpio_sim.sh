#!/bin/sh
#  turns on (1) of off (0) gpio real sim 

 /usr/local/bin/fims/fims_send -m set -r /$$ -u /gpio/full/config/gpio/GPIOsim $1

