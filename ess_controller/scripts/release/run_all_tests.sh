#!/bin/sh
# p. wilshire 
# 3-18-2022
# master test script to run all release tests
sh scripts/release/100.1_test_monitor_var.sh
sh scripts/release/300.1_test_actions_2.sh
sh scripts/release/300.1_test_bitfield_2.sh


sh scripts/release/300.1_test_bitmap_2.sh
sh scripts/release/300.1_test_bitset_2.sh
sh scripts/release/300.1_test_enum_2.sh
sh scripts/release/300.1_test_func_2.sh

sh scripts/release/300.1_test_limits_2.sh
sh scripts/release/300.1_test_newremap_2.sh
sh scripts/release/300.2_test_noval_2.sh
sh scripts/release/300.3_test_blocked_2.sh



