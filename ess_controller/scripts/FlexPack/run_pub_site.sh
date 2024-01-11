#wait_pause
echo run Publish operations 
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubSiteHs","every":0.2,"offset":0,"debug":0}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/system/commands/run '
                    {"value":22,"uri":"/control/pubs:pubSiteLs","every":2.0, "offset":0.1, "debug":1}'


