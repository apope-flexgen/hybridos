fSend=/usr/local/bin/fims/fims_send

# Helper function for repeatedly sending out heartbeats to a specific uri
#
# $1 = uri and variable name (ex.: /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = max number of iterations
simulateHeartbeat()
{ 
    currCnt=0
    maxCnt=$2
    while [ $currCnt -lt $maxCnt ]
    do
        fimsSet $1 $currCnt
        sleep 0.1
        ((currCnt++))
    done
}

# Helper function for repeatedly sending out timestamps to a specific uri
#
# $1 = uri and variable name (ex.: /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = max number of iterations
simulateTimestamp()
{ 
    currCnt=0
    maxCnt=$2
    while [ $currCnt -lt $maxCnt ]
    do
        timestamp="Timestamp ${currCnt}"
        echo $timestamp
        $fSend -m set -u $1 "\"$timestamp\"" -r /me | jq
        sleep 0.1
        ((currCnt++))
    done
}

# Helper function for doing a fims set
#
# $1 = uri and/or variable name (ex.: /components/catl_mbmu_control_r or /components/catl_mbmu_control_r/mbmu_max_cell_voltage)
# $2 = value to set
fimsSet()
{
    $fSend -m set -u $1 $2 -r /me | jq
}

# simulate heartbeat
simulateHeartbeat "/ess/components/catl_mbmu_stat_r/bms_heartbeat" 120