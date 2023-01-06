#wait_pause
echo run Publish operations 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/stop '
                    {"value":1,"uri":"/control/pubs:pubBmsHs","in":5.0,"offset":0,"debug":0}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/stop '
                    {"value":1,"uri":"/control/pubs:pubBmsLs","in":5.0, "offset":0.1, "debug":1}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/stop '
                    {"value":1,"uri":"/control/pubs:pubPcsHs","in":5.0,"offset":0,"debug":0}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/stop '
                    {"value":1,"uri":"/control/pubs:pubPcsHs","in":5.0,"offset":0,"debug":0}'

