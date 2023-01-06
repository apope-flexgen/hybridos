#!/bin/sh
# set up the pub functions for the scheduler
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubBmsHs":{"value":1,"table":"/site/bms_hs",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'
#wait_pause

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubBmsLs":{"value":0, "table":"/site/bms_ls",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubPcsHs":{"value":1,"table":"/site/pcs_hs",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'
#wait_pause

/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubPcsLs":{"value":0, "table":"/site/pcs_ls",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'


