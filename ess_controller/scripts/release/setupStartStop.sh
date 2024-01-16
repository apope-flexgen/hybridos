function startupEss()
{

    /usr/local/bin/ess_controller -f ess_1_file         > /tmp/ess1_out 2>&1 &
    /usr/local/bin/ess_controller -f ess_2_file         > /tmp/ess2_out 2>&1 &
    echo "sleeping while the controllers start"
    sleep 5

}

function stopEss()
{

    fims_send -m set -u /ess_1/control/ess/stopTime 1
    fims_send -m set -u /ess_2/control/ess/stopTime 1
    
}
