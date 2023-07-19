# !/bin/bash

echo Setting value to 1
fims_send -m set -u /var_5 1
sleep 5
echo Setting value to 2
fims_send -m set -u /var_5 2
sleep 5
echo Setting value to 4
fims_send -m set -u /var_5 4
sleep 5
echo Setting value to 5
fims_send -m set -u /var_5 5
sleep 5
echo Setting value to 8
fims_send -m set -u /var_5 8
sleep 5
echo Setting value to 15
fims_send -m set -u /var_5 15
echo done