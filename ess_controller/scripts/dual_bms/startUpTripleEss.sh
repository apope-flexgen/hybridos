#!/bin/sh
# p wilshire  03072022
# this starts up the now triple ess_controller
#

function startupEss()
{

    /usr/local/bin/ess_controller -f ess_file           > /tmp/ess_out 2>&1 &
    /usr/local/bin/ess_controller -f bms_1_file         > /tmp/bms1_out 2>&1 &
    /usr/local/bin/ess_controller -f bms_2_file         > /tmp/bms2_out 2>&1 &
    echo "sleeping while the controllers start"
    sleep 5

}

startupEss


