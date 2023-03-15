#!/bin/sh
# p wilshire 02-17-2022
# send /components/sel_735/active_power every second


while [ 1 ] ; do
   #fims_send -m pub -u /components/sel_735/active_power  123
   #sleep 0.1
   fims_send -m set -r /$$ -u /site/operation '{"test_int16":  123}'
   fims_send -m set -r /$$ -u /site/operation '{"test_signed_int16":  -123}'
   fims_send -m set -r /$$ -u /site/operation '{"test_int32":  123000}'
   fims_send -m set -r /$$ -u /site/operation '{"test_Float32":  12.3000}'
   fims_send -m set -r /$$ -u /site/operation '{"remote_enable_flag":  true}'
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  true}'
   sleep 0.5
   fims_send -m set -r /$$ -u /site/operation '{"remote_enable_flag":  false}'
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  false}'
   fims_send -m set -r /$$ -u /site/operation '{"test_int16":  124}'
   fims_send -m set -r /$$ -u /site/operation '{"test_signed_int16":  124}'
   fims_send -m set -r /$$ -u /site/operation '{"test_int32":  321000}'
   fims_send -m set -r /$$ -u /site/operation '{"test_Float32":  32.1000}'
   sleep 0.2
   fims_send -m set -r /$$ -u /site/operation '{"remote_enable_flag":  "LATCH_ON"}'
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  "LATCH_ON"}'
   sleep 0.1
   fims_send -m set -r /$$ -u /site/operation '{"remote_enable_flag":  "LATCH_OFF"}'
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  "LATCH_OFF"}'
   sleep 0.1
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  3}'
   sleep 0.1
   fims_send -m set -r /$$ -u /site/operation '{"remote_disable_flag":  4}'
   sleep 0.1
   fims_send -m set -r /$$ -u /site/operation '{"test_int16":  125}'
   sleep 0.5
   fims_send -m set -r /$$ -u /site/operation '{"test_int16":  126}'
   sleep 0.5
done

