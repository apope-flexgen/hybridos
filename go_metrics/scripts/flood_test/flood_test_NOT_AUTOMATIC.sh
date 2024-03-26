#!/bin/bash
go_metrics /home/docker/hybridos/go_metrics/examples/unit_test/uri_stuff_little_bit_of_everything.json &
for i in {1..100}
do
sh /home/docker/hybridos/go_metrics/scripts/unit_test/some_traffic_NOT_A_TEST.sh &
done