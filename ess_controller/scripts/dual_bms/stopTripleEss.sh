#!/bin/sh
# p wilshire  03072022
# this shuts down up the new triple ess_controller
#

function shutdownEss()
{

    /usr/local/bin/fims_send -m "set" -u /ess/control/ess/stopTime 1
    /usr/local/bin/ess_controller -f bms_1_file         > /tmp/bms1_out 2>&1 &
    /usr/local/bin/ess_controller -f bms_2_file         > /tmp/bms2_out 2>&1 &
    echo "sleeping while the controllers stop"
    sleep 1
}

shutdownEss
