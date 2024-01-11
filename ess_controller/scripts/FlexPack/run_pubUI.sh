#!/bin/sh 
#run_pubUI.sh

echo setup run Publish operations 
/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubUI","every":2.0,"offset":0,"debug":1}'

/usr/local/bin/fims_send -m set -r /$$ -u /ess/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubUIBms","every":2.0, "offset":0.1, "debug":1}'


