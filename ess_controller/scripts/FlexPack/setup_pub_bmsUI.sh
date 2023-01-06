#!/bin/sh
# set up the pub functions for the scheduler
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubBmsUIHs":{"value":1,"table":"/assets/bms/summary",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'
/usr/local/bin/fims/fims_send -m set -r /$$ -u /flex/full/control/pubs '
                    { "pubBmsUILs":{"value":1,"table":"/assets/bms/hvac_1",
                                  "enabled":true, "actions":{"onSet":[{"func":[{"func":"RunPub"}]}]}}}'
#wait_pause



