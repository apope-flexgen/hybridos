#!/bin/sh
# p wilshire  03072022
# this shuts down up the new triple ess_controller
#

function shutdownEss()
{

    /usr/local/bin/fims_send -m set -u /ess/control/ess/stopTime 1
    /usr/local/bin/fims_send -m set -u /bms_1/control/ess/stopTime 1
    /usr/local/bin/fims_send -m set -u /bms_2/control/ess/stopTime 1
    echo "sleeping while the controllers stop"
    sleep 1

}

shutdownEss
