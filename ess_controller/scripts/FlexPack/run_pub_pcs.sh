#wait_pause
echo run Pcs Publish operations 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubPcsHs","every":0.5,"offset":0,"debug":0}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":23,"uri":"/control/pubs:pubPcsLs","every":2.0, "offset":0.1, "debug":1}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run 0


