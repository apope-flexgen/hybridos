#wait_pause
echo run Bms Publish operations 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubBmsUIHs","every":1.0,"offset":0,"debug":0}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubBmsUILs","every":5.0, "offset":0.1, "debug":1}'


