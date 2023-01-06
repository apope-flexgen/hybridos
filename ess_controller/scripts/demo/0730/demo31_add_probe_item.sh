#!/bin/sh
# sample use 
#sh scripts/demo/0730/demo31_add_probe_item.sh /probe/bms /status/bms CellVoltge 3.45
probeLink=$1
probeComp=$2
probeVar=$3
probeVal=$4


# probeLink=/probe/bms
# probeComp=/status/bms
# probeVar=state
# probeVal=\"Ok\"


echo create dummy variables that we want to probe [inspect]
echo '/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/status/bms' 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/${probeLink} "
                    { \"${probeVar}\":{\"value\":${probeVal}}}"

echo
echo create a probe list to publish using links 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/system/commands "
                    { \"Link\":{\"value\":\"Ok\", 
                      \"varName\":\"${probeLink}:${probeVar}\",
                      \"linkName\":\"${probeComp}:${probeVar}\",\"inValue\":${probeVal}}}" | jq