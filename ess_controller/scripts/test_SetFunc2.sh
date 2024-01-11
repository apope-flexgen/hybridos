#!/bin/sh

# send changes to the slow var BmsStatusString2
/usr/local/bin/fims/fims_send -m set -r /$$ -u /status/bms '{"BmsStatusString2":"valueone","BmsStatusString2":"valuetwo","BmsStatusString2":"valuethree"}'

#send Changes to the fast Var
/usr/local/bin/fims/fims_send -m set -r /$$ -u /status/bms '{"BmsStatusString":"valueone","BmsStatusString":"valuetwo","BmsStatusString":"valuethree"}'


